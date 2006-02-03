#! /bin/sh

prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${CMAKE_INSTALL_PREFIX}/bin
exit `${CMAKE_INSTALL_PREFIX}/bin/meinproc --check --stdout $@ > /dev/null`

