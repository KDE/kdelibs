#!/bin/sh

bas=`kde-config --localprefix`share/apps/kio_help
mv "$bas/cache" "`kde-config --path cache`kio_help"
rmdir "$bas"
exit 0
