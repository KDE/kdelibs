#!/bin/sh

DO_SH=0
DO_CSH=0
DO_KEYVAL=0

for arg in "$@"; do
	case "$arg" in
		--sh-syntax) DO_SH=1 ;;
		--csh-syntax) DO_CSH=1 ;;
		--kde-mac) DO_KEYVAL=1 ;;
	esac
done

# find common dbus locations
PATH="/opt/kde4-deps/bin:/sw/bin:/usr/local/bin:$PATH"
DBUS_LAUNCH=`which dbus-launch`
if [ -z "$DBUS_LAUNCH" ]; then
	echo "could not find dbus-launch!"
	exit 1
fi

ID=`id -u`
if [ -f "/tmp/kde-mac-$ID" ]; then
	. "/tmp/kde-mac-$ID"
fi

if [ -z "$DBUS_SESSION_BUS_PID" ] || [ `/bin/ps -p "${DBUS_SESSION_BUS_PID}" | /usr/bin/grep -c -E "^ *${DBUS_SESSION_BUS_PID} "` -eq 0 ]; then
	eval `$DBUS_LAUNCH --sh-syntax`
	cat <<END >"/tmp/kde-mac-$ID"
DBUS_SESSION_BUS_PID=$DBUS_SESSION_BUS_PID
DBUS_SESSION_BUS_ADDRESS=$DBUS_SESSION_BUS_ADDRESS
END
fi

if [ "$DO_SH" -eq 1 ]; then
	echo "DBUS_SESSION_BUS_ADDRESS='${DBUS_SESSION_BUS_ADDRESS}';"
	echo "export DBUS_SESSION_BUS_ADDRESS;"
	echo "DBUS_SESSION_BUS_PID=${DBUS_SESSION_BUS_PID};"
fi
if [ "$DO_CSH" -eq 1 ]; then
	echo "setenv DBUS_SESSION_BUS_ADDRESS '${DBUS_SESSION_BUS_ADDRESS}';"
	echo "set DBUS_SESSION_BUS_PID=${DBUS_SESSION_BUS_PID};"
fi
if [ "$DO_KEYVAL" -eq 1 ]; then
	cat "/tmp/kde-mac-$ID"
fi

exit 0
