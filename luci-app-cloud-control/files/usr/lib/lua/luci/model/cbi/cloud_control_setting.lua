local sys = require "luci.sys"

local function get_status()
    local running = sys.call("pgrep -f /usr/bin/cloud_control >/dev/null") == 0
    if running then
        return "<b><span style='color:green;'>正在运行</span></b>"
    else
        return "<b><span style='color:red;'>已停止</span></b>"
    end
end

m = Map("cloud_control", "Cloud PC Control Settings")

s_status = m:section(TypedSection, "_dummy", "状态")
s_status.anonymous = true
s_status.addremove = false
status = s_status:option(DummyValue, "_status", "服务状态")
status.rawhtml = true
status.default = get_status()

s = m:section(TypedSection, "main", "Main Settings")
s.addremove = false
s.anonymous = true

s:option(Value, "client_id", "Client ID").default = " "
s:option(Value, "ip", "Host IP").default = "192.168.1.50"
s:option(Value, "password", "Password").password = true
s:option(Value, "user", "Username").default = ""
s:option(Value, "nic", "Network Interface").default = "br-lan"
s:option(Value, "mac", "MAC Address").default = "00:00:00:00:00:00"
s:option(Value, "topic", "Topic").default = "PC001"
s:option(Flag, "enabled", "Enable Cloud Control").default = "0"
return m
