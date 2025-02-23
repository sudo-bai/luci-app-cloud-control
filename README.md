# luci-app-cloud-control
这是一个通过巴法云让小爱远程控制电脑开关机的openwrt插件


#编译

make menuconfig  在Utilities中选择cloud-control，在LUCI中选中对应模块


make package/cloud-control/compile V=99


make package/luci-app-cloud-control/compile V=99
