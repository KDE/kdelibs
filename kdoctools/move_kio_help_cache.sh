#!/bin/sh

bas=`kde4-config --localprefix`share/apps/kio_help
mv "$bas/cache" "`kde4-config --path cache`kio_help"
rmdir "$bas"
exit 0
