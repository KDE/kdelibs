#!/bin/sh
found_ids=""
getId()
{
	printf "%4s" `lshal -l -u "$1"|grep "$2"|cut -d"(" -f2|cut -d")" -f1|cut -dx -f2`|tr ' ' '0'
}

createAlsaEntryFromHal()
{
	vendor_id=`getId "$2" "\\.vendor_id"`
	product_id=`getId "$2" "\\.product_id"`
	subsystem=`lshal -l -u "$2"|grep info.subsystem|cut -d"'" -f2`
	test -z "$subsystem" && subsystem=`lshal -l -u "$2"|grep linux.subsystem|cut -d"'" -f2`
	pref=35
	if test "$subsystem" = "usb"; then
		id="usb:$vendor_id:$product_id"
		pref=$(($pref-10))
	elif test "$subsystem" = "pci"; then
		subsys_vendor_id=`getId "$2" "\\.subsys_vendor_id"`
		subsys_product_id=`getId "$2" "\\.subsys_product_id"`
		id="pci:$vendor_id:$product_id:$subsys_vendor_id:$subsys_product_id"
	fi
	alsa_device=`lshal -l -u "$1"|grep "alsa\\.device = "|cut -d"=" -f2|cut -d' ' -f2`
	id="$id:$3:alsa:$alsa_device"
	if echo "$found_ids"|grep -q -v "$id"; then
		found_ids="$found_ids $id"
		echo "[$id]"
		card_id=`lshal -l -u "$1"|grep "alsa\\.card_id = "|cut -d"'" -f2`
		device_id=`lshal -l -u "$1"|grep "alsa\\.device_id = "|cut -d"'" -f2`
		if echo "$card_id $device_id"|grep -q -E -i "(spdif|s/pdif|iec958)"; then
			pref=$(($pref-20))
		fi
		echo "# name=$card_id ($device_id)"
		echo "initialPreference=$pref"
		echo "# icon="
		echo ""
	fi
}

createOssEntryFromHal()
{
	vendor_id=`getId "$2" "\\.vendor_id"`
	product_id=`getId "$2" "\\.product_id"`
	subsystem=`lshal -l -u "$2"|grep info.subsystem|cut -d"'" -f2`
	test -z "$subsystem" && subsystem=`lshal -l -u "$2"|grep linux.subsystem|cut -d"'" -f2`
	pref=35
	if test "$subsystem" = "usb"; then
		id="usb:$vendor_id:$product_id"
		pref=$(($pref-10))
	elif test "$subsystem" = "pci"; then
		subsys_vendor_id=`getId "$2" "\\.subsys_vendor_id"`
		subsys_product_id=`getId "$2" "\\.subsys_product_id"`
		id="pci:$vendor_id:$product_id:$subsys_vendor_id:$subsys_product_id"
	fi
	id="$id:both:oss"
	if echo "$found_ids"|grep -q -v "$id"; then
		found_ids="$found_ids $id"
		echo "[$id]"
		oss_card_id=`lshal -l -u "$1"|grep "oss\\.card_id = "|cut -d"'" -f2`
		oss_device_id=`lshal -l -u "$1"|grep "oss\\.device_id = "|cut -d"'" -f2`
		if echo "$oss_card_id $oss_device_id"|grep -q -E -i "(spdif|s/pdif|iec958)"; then
			pref=$(($pref-20))
		fi
		echo "# name=$oss_card_id ($oss_device_id)"
		echo "initialPreference=$pref"
		echo "# icon="
		echo ""
	fi
}

echo "/proc/asound/cards:"
cat /proc/asound/cards
echo "\n/proc/asound/pcm:"
cat /proc/asound/pcm
for card in `cat /proc/asound/cards|cut -c1-2`; do
	num=0
	codec="/proc/asound/card$card/codec#$num"
	while test -e "$codec"; do
		echo "\n$codec:"
		cat "$codec"
		num=$(($num+1))
		codec="/proc/asound/card$card/codec#$num"
	done
	num=0
	codec="/proc/asound/card$card/codec97#$num"
	while test -d "$codec"; do
		echo -n "\n$codec/ac97#"?-?
		echo ":"
		cat "$codec/ac97#"?-?
		num=$(($num+1))
		codec="/proc/asound/card$card/codec97#$num"
	done
done
echo ""
alsaplayback=`lshal -s|grep _alsa_playback`
for playback_udi in $alsaplayback; do
	card_udi="${playback_udi%_alsa_playback_*}"
	createAlsaEntryFromHal "$playback_udi" "$card_udi" "playback"
done
alsacapture=`lshal -s|grep _alsa_capture`
for capture_udi in $alsacapture; do
	card_udi="${capture_udi%_alsa_capture_*}"
	createAlsaEntryFromHal "$capture_udi" "$card_udi" "capture"
done
oss=`lshal -s|grep _oss_pcm`
for udi in $oss; do
	card_udi="${udi%_oss_pcm*}"
	createOssEntryFromHal "$udi" "$card_udi"
done
