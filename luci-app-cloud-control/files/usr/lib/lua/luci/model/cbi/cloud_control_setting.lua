m = Map("cloud_control", "Cloud PC Control Settings")

s = m:section(TypedSection, "main", "Main Settings")
s.addremove = false
s.anonymous = true

s:option(Value, "client_id", "Client ID").default = " "
s:option(Value, "ip", "Host IP").default = "192.168.1.50"
s:option(Value, "password", "Password").password = true
s:option(Value, "user", "Username").default = ""
s:option(Value, "nic", "Network Interface").default = "br-lan"
s:option(Value, "mac", "MAC Address").default = "0****************F5"
s:option(Value, "topic", "Topic").default = "PC001"
s:option(Flag, "enabled", "Enable Cloud Control").default = "0"
return m