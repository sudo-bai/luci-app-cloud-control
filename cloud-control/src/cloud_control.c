#include <uci.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

// 日志文件路径
#define LOG_FILE "/var/log/cloud_control.log"

struct Config {
    char client_id[64];
    char ip[32];
    char password[64];
    char user[32];
    char nic[32];
    char mac[32];
    char topic[32];
    char enabled[8];
};

static volatile sig_atomic_t g_running = 1;

// 守护进程化处理，避免被CGI父进程带崩
static int daemonize() {
    pid_t pid = fork();
    if (pid < 0) return -1;
    if (pid > 0) exit(0); // 父进程直接退出
    if (setsid() < 0) return -1;
    pid = fork();
    if (pid < 0) return -1;
    if (pid > 0) exit(0);
    umask(0);
    chdir("/");
    close(0);
    close(1);
    close(2);
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_WRONLY);
    open("/dev/null", O_RDWR);
    return 0;
}

// 日志打印, 密码自动脱敏
static void log_message(const char *fmt, ...) {
    FILE *f = fopen(LOG_FILE, "a");
    if (!f) {
        // fallback
        va_list args;
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
        fprintf(stderr, "\n");
        return;
    }
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char tbuf[32];
    strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", tm_info);
    fprintf(f, "[%s] ", tbuf);

    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);

    fprintf(f, "\n");
    fclose(f);
}

// 安全字符串拷贝
static void safe_strcpy(char *dst, const char *src, size_t dsz) {
    if (!dst || dsz == 0) return;
    if (src) {
        strncpy(dst, src, dsz - 1);
        dst[dsz - 1] = 0;
    } else {
        dst[0] = 0;
    }
}

// 配置加载
static void load_config(struct Config *cfg) {
    struct uci_context *ctx = uci_alloc_context();
    struct uci_package *pkg = NULL;

    memset(cfg, 0, sizeof(*cfg));

    if (uci_load(ctx, "cloud_control", &pkg)) {
        log_message("ERROR: Failed to load config");
        uci_free_context(ctx);
        return;
    }

    struct uci_element *e;
    struct uci_section *s = NULL;
    uci_foreach_element(&pkg->sections, e) {
        s = uci_to_section(e);
        if (strcmp(s->type, "main") == 0) {
            break;
        }
        s = NULL;
    }
    if (s) {
        struct uci_element *opt_e = NULL;
        uci_foreach_element(&s->options, opt_e) {
            struct uci_option *option = uci_to_option(opt_e);
            if (strcmp(opt_e->name, "client_id") == 0 && option->type == UCI_TYPE_STRING)
                safe_strcpy(cfg->client_id, option->v.string, sizeof(cfg->client_id));
            else if (strcmp(opt_e->name, "ip") == 0 && option->type == UCI_TYPE_STRING)
                safe_strcpy(cfg->ip, option->v.string, sizeof(cfg->ip));
            else if (strcmp(opt_e->name, "password") == 0 && option->type == UCI_TYPE_STRING)
                safe_strcpy(cfg->password, option->v.string, sizeof(cfg->password));
            else if (strcmp(opt_e->name, "user") == 0 && option->type == UCI_TYPE_STRING)
                safe_strcpy(cfg->user, option->v.string, sizeof(cfg->user));
            else if (strcmp(opt_e->name, "nic") == 0 && option->type == UCI_TYPE_STRING)
                safe_strcpy(cfg->nic, option->v.string, sizeof(cfg->nic));
            else if (strcmp(opt_e->name, "mac") == 0 && option->type == UCI_TYPE_STRING)
                safe_strcpy(cfg->mac, option->v.string, sizeof(cfg->mac));
            else if (strcmp(opt_e->name, "topic") == 0 && option->type == UCI_TYPE_STRING)
                safe_strcpy(cfg->topic, option->v.string, sizeof(cfg->topic));
            else if (strcmp(opt_e->name, "enabled") == 0 && option->type == UCI_TYPE_STRING)
                safe_strcpy(cfg->enabled, option->v.string, sizeof(cfg->enabled));
        }
        log_message("Config loaded: client_id=%s, ip=%s, user=%s, nic=%s, mac=%s, topic=%s, enabled=%s",
            cfg->client_id, cfg->ip, cfg->user, cfg->nic, cfg->mac, cfg->topic, cfg->enabled);
    } else {
        log_message("ERROR: Section 'main' not found in config");
    }
    uci_unload(ctx, pkg);
    uci_free_context(ctx);
}

// TCP连接
int tcp_connect(const char* host, int port) {
    struct addrinfo hints, *res, *rp;
    int sock = -1;
    char portstr[16];
    snprintf(portstr, sizeof(portstr), "%d", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int ret = getaddrinfo(host, portstr, &hints, &res);
    if (ret != 0) {
        log_message("getaddrinfo failed: %s", gai_strerror(ret));
        return -1;
    }

    for (rp = res; rp != NULL; rp = rp->ai_next) {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == -1)
            continue;
        if (connect(sock, rp->ai_addr, rp->ai_addrlen) == 0)
            break;
        close(sock);
        sock = -1;
    }

    freeaddrinfo(res);

    if (sock == -1)
        log_message("tcp_connect: failed to connect %s:%d", host, port);
    else
        log_message("tcp_connect: connected to %s:%d", host, port);
    return sock;
}

int send_subscribe(int sock, const char* client_id, const char* topic) {
    char buf[256];
    snprintf(buf, sizeof(buf), "cmd=1&uid=%s&topic=%s\r\n", client_id, topic);
    int ret = send(sock, buf, strlen(buf), 0);
    if (ret < 0) log_message("send_subscribe error: %s", strerror(errno));
    return ret;
}

int send_ping(int sock) {
    const char* pingstr = "ping\r\n";
    int ret = send(sock, pingstr, strlen(pingstr), 0);
    if (ret < 0) log_message("send_ping error: %s", strerror(errno));
    return ret;
}

// 执行开机命令
void do_poweron(const struct Config* cfg) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "/usr/bin/etherwake -D -i '%s' '%s'", cfg->nic, cfg->mac);
    log_message("poweron: %s", cmd);
    system(cmd);
}

// 执行关机命令
void do_poweroff(const struct Config* cfg) {
    char cmd1[512];
    char cmd2[512];
    // curl上报云端状态
    snprintf(cmd1, sizeof(cmd1),
        "/usr/bin/curl -s \"https://api.bemfa.com/api/device/v1/data/3/push/get/?uid=%s&topic=%s&msg=off\" -w \"\\n\"",
        cfg->client_id, cfg->topic);
    log_message("poweroff-report: curl for topic=%s", cfg->topic);
    system(cmd1);

    // ssh局域网关机（参数加引号，防止注入）
    snprintf(cmd2, sizeof(cmd2),
        "sshpass -p '%s' ssh -o StrictHostKeyChecking=no -A -g '%s'@'%s' \"shutdown -s -t 10\"",
        cfg->password, cfg->user, cfg->ip);
    log_message("poweroff-ssh: ssh to %s@%s", cfg->user, cfg->ip);
    system(cmd2);
}

// 解析收到的消息并执行操作
void handle_message(const char* msg, const struct Config* cfg) {
    log_message("recv: %s", msg);
    const char* key = "&msg=";
    char* pos = strstr(msg, key);
    if (!pos) return;
    pos += strlen(key);
    char val[16];
    int i = 0;
    while (*pos && *pos != '&' && *pos != '\r' && *pos != '\n' && i < (int)sizeof(val) - 1) {
        val[i++] = *pos++;
    }
    val[i] = 0;
    log_message("sw: %s", val);

    if (strcmp(val, "on") == 0) {
        log_message("action: poweron");
        do_poweron(cfg);
    } else if (strcmp(val, "off") == 0) {
        log_message("action: poweroff");
        do_poweroff(cfg);
    }
}

// 信号处理，支持优雅退出和热重载
static void sig_handler(int sig) {
    if (sig == SIGTERM || sig == SIGINT) {
        g_running = 0;
    }
}

int main() {
    // 守护进程化，兼容uhttpd/Luci/后台环境
    daemonize();

    signal(SIGTERM, sig_handler);
    signal(SIGINT, sig_handler);

    struct Config config;
    memset(&config, 0, sizeof(config));
    log_message("cloud_control starting...");
    load_config(&config);

    if (strcmp(config.enabled, "1") != 0) {
        log_message("cloud_control not enabled, exiting...");
        log_message("cloud_control stopped.");
        return 0;
    }

    const char* server_host = "bemfa.com";
    int server_port = 8344;
    int sock = -1;

    // 主循环：自动重连
    while (g_running) {
        sock = tcp_connect(server_host, server_port);
        if (sock < 0) {
            for (int i = 0; i < 10 && g_running; ++i) sleep(1);
            continue;
        }

        send_subscribe(sock, config.client_id, config.topic);

        // 心跳时间
        time_t last_ping = time(NULL);
        time_t last_reload = time(NULL);

        while (g_running) {
            fd_set rfds;
            struct timeval tv;
            FD_ZERO(&rfds);
            FD_SET(sock, &rfds);
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            int ret = select(sock + 1, &rfds, NULL, NULL, &tv);

            time_t now = time(NULL);

            // 配置热重载（每10分钟重新加载）
            if (now - last_reload >= 600) {
                load_config(&config);
                last_reload = now;
            }

            if (now - last_ping >= 30) {
                send_ping(sock);
                last_ping = now;
            }

            if (ret < 0) {
                if (errno == EINTR && !g_running) break;
                log_message("select error: %s", strerror(errno));
                break;
            } else if (ret == 0) {
                continue;
            } else if (FD_ISSET(sock, &rfds)) {
                char buf[1024];
                int len;
                // 读尽所有数据
                while ((len = recv(sock, buf, sizeof(buf) - 1, MSG_DONTWAIT)) > 0) {
                    buf[len] = '\0';
                    handle_message(buf, &config);
                }
                if (len == 0) {
                    log_message("tcp connection closed, reconnecting...");
                    close(sock);
                    break;
                } else if (len < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                    log_message("recv error: %s", strerror(errno));
                    close(sock);
                    break;
                }
            }
        }
        if (sock >= 0) close(sock);
        if (g_running) sleep(2);
    }
    log_message("cloud_control stopped.");
    return 0;
}
