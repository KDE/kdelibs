# - Try to find the UDev library
# Once done this will define
#
#  UDEV_FOUND - system has UDev
#  UDEV_INCLUDE_DIR - the libudev include directory
#  UDEV_LIBS - The libudev libraries
#  UDEV_HAVE_GET_SYSATTR_LIST_ENTRY - TRUE if the udev library has the function
#              udev_device_get_sysattr_list_entry(), added in version 167 of libudev

# Copyright (c) 2010, Rafael Fernández López, <ereslibre@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_path(UDEV_INCLUDE_DIR libudev.h)
find_library(UDEV_LIBS udev)

if(UDEV_INCLUDE_DIR AND UDEV_LIBS)
   include(CheckFunctionExists)
   include(CMakePushCheckState)
   cmake_push_check_state()
   set(CMAKE_REQUIRED_LIBRARIES ${UDEV_LIBS} )
   check_function_exists(udev_device_get_sysattr_list_entry  UDEV_HAVE_GET_SYSATTR_LIST_ENTRY )

   cmake_pop_check_state()

endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(UDev DEFAULT_MSG UDEV_INCLUDE_DIR UDEV_LIBS)

mark_as_advanced(UDEV_INCLUDE_DIR UDEV_LIBS)
