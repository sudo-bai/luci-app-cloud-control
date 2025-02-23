#include <libubox/uloop.h>
#include <uci.h>

struct Config {
    char client_id[64];
    char ip[32];
    char password[64];
    char user[32];
    char nic[32];
    char mac[32];
    char topic[32];
};

static void load_config(struct Config *cfg) {
    struct uci_context *ctx = uci_alloc_context();
    struct uci_package *pkg = NULL;

    if (uci_load(ctx, "cloud_control", &pkg)) {
        fprintf(stderr, "Failed to load config\n");
        return;
    }

    struct uci_section *s = uci_lookup_section(ctx, pkg, "main");
    if (s) {
        uci_lookup_option_string(ctx, s, "client_id", cfg->client_id, sizeof(cfg->client_id));
        uci_lookup_option_string(ctx, s, "ip", cfg->ip, sizeof(cfg->ip));
        uci_lookup_option_string(ctx, s, "password", cfg->password, sizeof(cfg->password));
        uci_lookup_option_string(ctx, s, "user",cfg->user,sizeof(cfg->user));
        uci_lookup_option_string(ctx, s, "nic",cfg->nic,sizeof(cfg->nic));
        uci_lookup_option_string(ctx, s, "mac",cfg->mac,sizeof(cfg->mac));
        uci_lookup_option_string(ctx, s, "topic",cfg->topic,sizeof(cfg->topic));
    }

    uci_free_context(ctx);
}

// 在main函数中初始化配置
int main() {
    struct Config config;
    memset(&config, 0, sizeof(config));
    load_config(&config);
    // 后续代码使用config结构体中的值...
}