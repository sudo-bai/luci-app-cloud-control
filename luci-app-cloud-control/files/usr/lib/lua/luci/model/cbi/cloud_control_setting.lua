local sys = require "luci.sys"

local function get_status()
    local running = sys.call("pidof cloud_control >/dev/null") == 0
    if running then
        return "<b><span style='color:green;'>正在运行</span></b>"
    else
        return "<b><span style='color:red;'>已停止</span></b>"
    end
end
local function get_log()
    local log_path = "/var/log/cloud_control.log"
    local f = io.open(log_path, "r")
    if not f then
        return "<i>暂无日志</i>"
    end
    local lines = {}
    for line in f:lines() do
        lines[#lines + 1] = line
    end
    f:close()
    local total = #lines
    local show_from = math.max(1, total - 49)
    local out = table.concat({ unpack(lines, show_from, total) }, "<br>")
    return "<div style='font-family:monospace;font-size:13px;max-height:300px;overflow:auto;background:#fafbfc;border:1px solid #eee;padding:8px;'>" .. out .. "</div>"
end

m = Map("cloud_control", "Cloud PC Control Settings")

local s_status = m:section(SimpleSection)
s_status.title = "服务状态"
s_status.description = get_status()

local s_log = m:section(SimpleSection)
s_log.title = "运行日志"
s_log.description = get_log()

local s = m:section(TypedSection, "main", "Main Settings")
s.addremove = false
s.anonymous = true

s:option(Value, "client_id", "Client ID").default = ""
s:option(Value, "ip", "Host IP").default = "192.168.1.50"
s:option(Value, "password", "Password").password = true
s:option(Value, "user", "Username").default = ""
s:option(Value, "nic", "Network Interface").default = "br-lan"
s:option(Value, "mac", "MAC Address").default = "00:00:00:00:00:00"
s:option(Value, "topic", "Topic").default = "PC001"
local enabled = s:option(Flag, "enabled", "Enable Cloud Control")
enabled.default = "0"
enabled.rmempty = false

m.on_after_commit = function(self)
    luci.sys.call("/etc/init.d/cloud_control reload >/dev/null 2>&1 &")
end

return m
