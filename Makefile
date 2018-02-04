#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := smarthome-clock-bledetect

# EXTRA_CXXFLAGS := -std=c++14

# EXTRA_CFLAGS := -DSNTP_DEBUG -DLWIP_DEBUG
# EXTRA_CFLAGS := -DSNTP_DEBUG
# EXTRA_CFLAGS := -include="$(IDF_PATH)/projects/smarthome-module/main/ntp_cb.h" -DSNTP_DEBUG -DSNTP_SET_SYSTEM_TIME_US="ntp_settime"

include $(IDF_PATH)/make/project.mk

