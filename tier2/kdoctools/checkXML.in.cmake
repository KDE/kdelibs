#! /bin/sh

prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${CMAKE_INSTALL_PREFIX}/bin
exit `${CMAKE_INSTALL_PREFIX}/bin/meinproc5 --check --stdout $@ > /dev/null`

