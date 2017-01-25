ARDUINO_LIBS = SoftwareSerial Wire makeblock
BOARD_TAG = uno

-include $(HOME)/.config/arduino/config.mk
-include config.mk
CPPFLAGS += -DMONITOR_BAUDRATE=$(MONITOR_BAUDRATE)
CPPFLAGS += -DDISPLAY_REVISION=$(shell date +%H%M | sed s/^0//)
include $(ARDUINO_MK)
