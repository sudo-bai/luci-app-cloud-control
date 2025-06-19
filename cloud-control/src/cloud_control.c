#include <libubox/uloop.h>
#include <uci.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#define LOG_FILE "/var/log/cloud_control.log"

struct Config {
    char client_id[64];
    char ip[32];
    char password[64];
    char user[32];
    char nic[32];
    char mac[32];
    char topic[32];
};

// 日志函数，带时间戳
static void log_message(const char *fmt, ...) {
    FILE *f = fopen(LOG_FILE, "a");
    if (!f) return;
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

static void load_config(struct Config *cfg) {
    struct uci_context *ctx = uci_alloc_context();
    struct uci_package *pkg = NULL;

    if (uci_load(ctx, "cloud_control", &pkg)) {
        fprintf(stderr, "Failed to load config\n");
        log_message("ERROR: Failed to load config");
        uci_free_context(ctx);
        return;
    }

    struct uci_section *s = uci_lookup_section(ctx, pkg, "main");
    if (s) {
        struct uci_element *e = NULL;
        uci_foreach_element(&s->options, e) {
            struct uci_option *option = uci_to_option(e);
            if (strcmp(e->name, "client_id") == 0 && option->type == UCI_TYPE_STRING)
                strncpy(cfg->client_id, option->v.string, sizeof(cfg->client_id));
            else if (strcmp(e->name, "ip") == 0 && option->type == UCI_TYPE_STRING)
                strncpy(cfg->ip, option->v.string, sizeof(cfg->ip));
            else if (strcmp(e->name, "password") == 0 && option->type == UCI_TYPE_STRING)
                strncpy(cfg->password, option->v.string, sizeof(cfg->password));
            else if (strcmp(e->name, "user") == 0 && option->type == UCI_TYPE_STRING)
                strncpy(cfg->user, option->v.string, sizeof(cfg->user));
            else if (strcmp(e->name, "nic") == 0 && option->type == UCI_TYPE_STRING)
                strncpy(cfg->nic, option->v.string, sizeof(cfg->nic));
            else if (strcmp(e->name, "mac") == 0 && option->type == UCI_TYPE_STRING)
                strncpy(cfg->mac, option->v.string, sizeof(cfg->mac));
            else if (strcmp(e->name, "topic") == 0 && option->type == UCI_TYPE_STRING)
                strncpy(cfg->topic, option->v.string, sizeof(cfg->topic));
        }
        log_message("Config loaded: client_id=%s, ip=%s, user=%s, nic=%s, mac=%s, topic=%s",
            cfg->client_id, cfg->ip, cfg->user, cfg->nic, cfg->mac, cfg->topic);
    } else {
        log_message("ERROR: Section 'main' not found in config");
    }

    uci_unload(ctx, pkg);
    uci_free_context(ctx);
}

int main() {
    struct Config config;
    memset(&config, 0, sizeof(config));
    log_message("cloud_control starting...");
    load_config(&config);
    // 后续代码使用config结构体中的值...
    log_message("cloud_control stopped.");
    return 0;
}
