################################################################################
#
# mymodule
#
################################################################################

MYMODULE_SITE = $(TOPDIR)/package/mymodule
MYMODULE_SITE_METHOD = local

$(eval $(kernel-module))
$(eval $(generic-package))
