# luci-app-cloud-control

## 项目简介

cloud_control 是基于 OpenWrt 路由器的云端远程主机电源管理守护进程。它通过配置文件管理云端通信参数，实现远程开机（Wake-on-LAN）、关机（SSH/云端API通知）等功能，适用于需要远程控制家庭、办公等场景下主机电源的环境。

## 功能特性

- 通过巴法云（ [bemfa.com](https://www.bemfa.com/)）发布/接收电源管理指令
- 支持局域网内主机远程开机（Wake-on-LAN）
- 支持通过 SSH 远程关机
- 支持云端 API 状态上报
- 配置热加载，支持运行中动态刷新参数
- 兼容 OpenWrt Luci 服务管理，支持前端一键启停

## 安装与使用

### 1. 编译

请确保你的 OpenWrt 路由器已安装好编译环境及依赖（如 libuci、libcurl 等），然后在源码根目录下执行：

```sh
gcc -o /usr/bin/cloud_control cloud_control.c -luci
```

### 2. 配置

编辑 `/etc/config/cloud_control`，示例内容如下：

```conf
config main
    option enabled '1'
    option client_id '你的云控UID'(注册个巴法云账户，创建一个主题，默认PC001，将上面的密钥复制下来)
    option ip '192.168.1.50'
    option user '主机用户名'
    option password '主机密码'
    option nic 'br-lan'
    option mac 'XX:XX:XX:XX:XX:XX'
    option topic '你的主题'
```

- `enabled`：是否启用服务（1为启用，0为关闭）
- 其余字段请根据实际环境填写

### 3. 启动与管理

通过 OpenWrt 的 `/etc/init.d/cloud_control` 管理服务：

```sh
/etc/init.d/cloud_control start    # 启动服务
/etc/init.d/cloud_control stop     # 停止服务
/etc/init.d/cloud_control restart  # 重启服务
```

也可通过 Luci 前端启用/禁用服务。

###测试成功后就可以在米家绑定巴法云，然后让小爱同学控制了
### 4. 日志查看

程序日志默认写入 `/var/log/cloud_control.log`。可通过以下命令查看运行状态：

```sh
tail -f /var/log/cloud_control.log
```

### 5. 常见问题

- **服务无法常驻/一闪而过？**
  - 请确认编译版本已包含守护进程化（daemonize）逻辑，且配置文件参数填写正确。
- **Luci 启用服务无效？**
  - 请确保 `/etc/init.d/cloud_control` 可执行，且 cloud_control 可被 root 用户启动。

## 贡献与支持

欢迎提交 issue 或 pull request 共同完善本项目。
---

**作者：** sudo-bai  
**许可证：** MIT
