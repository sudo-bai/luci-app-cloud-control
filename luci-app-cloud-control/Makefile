include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-cloud-control
PKG_VERSION:=1.0
PKG_RELEASE:=1
PKG_MAINTAINER:=skadi <love@skadi.ltd>

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-cloud-control
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  TITLE:=LUCI support for cloud-control
  DEPENDS:=+cloud-control
  PKGARCH:=all
endef

define Package/luci-app-cloud-control/description
  LuCI interface for cloud-control
endef

define Build/Compile
	# No build step needed for LuCI app
endef

define Package/luci-app-cloud-control/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) ./files/usr/lib/lua/luci/controller/*.lua $(1)/usr/lib/lua/luci/controller/
	$(CP) ./files/usr/lib/lua/luci/model/cbi/*.lua $(1)/usr/lib/lua/luci/model/cbi/
	$(CP) ./files/etc/config/cloud_control $(1)/etc/config/
	$(INSTALL_BIN) ./files/etc/init.d/cloud_control $(1)/etc/init.d/
endef

$(eval $(call BuildPackage,luci-app-cloud-control))
