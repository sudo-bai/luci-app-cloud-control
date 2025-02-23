module("luci.controller.cloud_control", package.seeall)

function index()
    entry({"admin", "services", "cloud_control"}, cbi("cloud_control_setting"), _("Cloud Control"), 60)
end