include $(TOPDIR)/rules.mk

PKG_NAME:=cloud-control
PKG_VERSION:=1.0
PKG_RELEASE:=1
include $(INCLUDE_DIR)/package.mk

define Package/cloud-control
SECTION:=utils
CATEGORY:=Utilities
TITLE:=Cloud PC Control
DEPENDS:=+libuci +libubox +libpthread +etherwake +sshpass +curl
endef

define Package/cloud-control/description
Cloud controlled PC power management utility
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) \
		-I$(STAGING_DIR)/usr/include \
		-o $(PKG_BUILD_DIR)/cloud_control $(PKG_BUILD_DIR)/cloud_control.c \
		$(TARGET_LDFLAGS) -L$(STAGING_DIR)/usr/lib -lpthread -lubox -luci
endef

define Package/cloud-control/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cloud_control $(1)/usr/bin/
endef

$(eval $(call BuildPackage,cloud-control))
