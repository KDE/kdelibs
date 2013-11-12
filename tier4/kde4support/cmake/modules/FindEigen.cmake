# Note that Eigen1 is deprecated in favor of Eigen2. So this file is deprecated
# in favor of FindEigen2.cmake. It is kept only for compatibility.
#
# - Try to find Eigen1 library
# Once done this will define
#
#  EIGEN_FOUND - system has eigen lib
#  EIGEN_INCLUDE_DIR - the eigen include directory

# Copyright (c) 2006, 2007 Montel Laurent, <montel@kde.org>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_path(EIGEN_INCLUDE_DIR NAMES eigen/matrix.h
     PATHS
     ${INCLUDE_INSTALL_DIR}
   )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Eigen DEFAULT_MSG EIGEN_INCLUDE_DIR )

mark_as_advanced(EIGEN_INCLUDE_DIR)


