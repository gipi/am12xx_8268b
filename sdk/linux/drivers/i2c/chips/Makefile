#
# Makefile for miscellaneous I2C chip drivers.
#
# Think twice before you add a new driver to this directory.
# Device drivers are better grouped according to the functionality they
# implement rather than to the bus they are connected to. In particular:
# * Hardware monitoring chip drivers go to drivers/hwmon
# * RTC chip drivers go to drivers/rtc
# * I/O expander drivers go to drivers/gpio
#

obj-$(CONFIG_DS1682)		+= ds1682.o
obj-$(CONFIG_AXP173)	+= axp173.o
obj-$(CONFIG_AL1601)	+= al1601.o
obj-$(CONFIG_DPP2607)	+= dpp2607.o
obj-$(CONFIG_BTVD)	+= btvd_i2c.o
obj-$(CONFIG_EDID_HW)	+= edid_i2c_hw.o
obj-$(CONFIG_EDID_GPIO)	+= edid_i2c_gpio.o
obj-$(CONFIG_I2S_I2C)	+= i2s_i2c.o
obj-$(CONFIG_I2C_WM8988)+= i2c_wm8988.o
obj-$(CONFIG_HDCP)	+= hdcp_i2c.o
obj-$(CONFIG_AT24_I2C)	+= at24_i2c.o
obj-$(CONFIG_AT24)		+= at24.o
obj-$(CONFIG_SENSORS_EEPROM)	+= eeprom.o
obj-$(CONFIG_SENSORS_MAX6875)	+= max6875.o
obj-$(CONFIG_SENSORS_PCA9539)	+= pca9539.o
obj-$(CONFIG_SENSORS_PCF8574)	+= pcf8574.o
obj-$(CONFIG_PCF8575)		+= pcf8575.o
obj-$(CONFIG_SENSORS_PCF8591)	+= pcf8591.o
obj-$(CONFIG_ISP1301_OMAP)	+= isp1301_omap.o
obj-$(CONFIG_TPS65010)		+= tps65010.o
obj-$(CONFIG_MENELAUS)		+= menelaus.o
obj-$(CONFIG_SENSORS_TSL2550)	+= tsl2550.o

ifeq ($(CONFIG_I2C_DEBUG_CHIP),y)
EXTRA_CFLAGS += -DDEBUG
endif

