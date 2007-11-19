#!/bin/sh
found_ids=""
all_entries=""
getId()
{
	printf "%4s" `lshal -l -u "$1"|grep "$2 ="|cut -d"(" -f2|cut -d")" -f1|cut -dx -f2`|tr ' ' '0'
}
getNumber()
{
	lshal -l -u "$1"|grep "$2 = "|cut -d"=" -f2|cut -d' ' -f2
}
getString()
{
	lshal -l -u "$1"|grep "$2 ="|cut -d"'" -f2
}

createAlsaEntryFromHal()
{
	subsystem=`getString "$2" "info\.subsystem"`
	test -z "$subsystem" && subsystem=`getString "$2" "linux\.subsystem"`
	vendor=`getString "$2" "$subsystem\\.vendor"`
	product=`getString "$2" "$subsystem\\.product"`
	vendor_id=`getId "$2" "$subsystem\\.vendor_id"`
	product_id=`getId "$2" "$subsystem\\.product_id"`
	pref=35
	comment="$vendor $product"
	if test "$subsystem" = "usb"; then
		id="usb:$vendor_id:$product_id"
		pref=$(($pref-10))
	elif test "$subsystem" = "pci"; then
		subsys_vendor=`getString "$2" "\\.subsys_vendor"`
		subsys_vendor_id=`getId "$2" "\\.subsys_vendor_id"`
		subsys_product_id=`getId "$2" "\\.subsys_product_id"`
		id="pci:$vendor_id:$product_id:$subsys_vendor_id:$subsys_product_id"
		comment="$comment $subsys_vendor"
	fi
	alsa_device=`getNumber "$1" "alsa\\.device"`
	id="$id:$3:alsa:$alsa_device"
	if echo -e "$found_ids"|grep -q -v "$id"; then
		found_ids="$found_ids $id"
		card_id=`getString "$1" "alsa\\.card_id"`
		device_id=`getString "$1" "alsa\\.device_id"`
		comment="$comment $card_id $device_id"
		if echo -e "$card_id $device_id"|grep -q -E -i "(spdif|s/pdif|iec958)"; then
			pref=$(($pref-20))
		fi
		all_entries="$all_entries\n# $comment\t[$id]\tname=$card_id ($device_id)\tinitialPreference=$pref\t"
		echo -n "."
	fi
}

createOssEntryFromHal()
{
	subsystem=`getString "$2" "info\.subsystem"`
	test -z "$subsystem" && subsystem=`getString "$2" "linux\.subsystem"`
	vendor=`getString "$2" "$subsystem\\.vendor"`
	product=`getString "$2" "$subsystem\\.product"`
	vendor_id=`getId "$2" "$subsystem\\.vendor_id"`
	product_id=`getId "$2" "$subsystem\\.product_id"`
	pref=20
	comment="$vendor $product"
	if test "$subsystem" = "usb"; then
		id="usb:$vendor_id:$product_id"
		pref=$(($pref-10))
	elif test "$subsystem" = "pci"; then
		subsys_vendor=`getString "$2" "\\.subsys_vendor"`
		subsys_vendor_id=`getId "$2" "\\.subsys_vendor_id"`
		subsys_product_id=`getId "$2" "\\.subsys_product_id"`
		id="pci:$vendor_id:$product_id:$subsys_vendor_id:$subsys_product_id"
		comment="$comment $subsys_vendor"
	fi
	id="$id:both:oss"
	if echo -e "$found_ids"|grep -q -v "$id"; then
		found_ids="$found_ids $id"
		card_id=`getString "$1" "oss\\.card_id"`
		device_id=`getString "$1" "oss\\.device_id"`
		comment="$comment $card_id $device_id"
		if echo -e "$card_id $device_id"|grep -q -E -i "(spdif|s/pdif|iec958)"; then
			pref=$(($pref-20))
		fi
		all_entries="$all_entries\n# $comment\t[$id]\tname=$card_id ($device_id)\tinitialPreference=$pref\t"
		echo -n "."
	fi
}

echo "/proc/asound/cards:"
cat /proc/asound/cards
echo -e "\n/proc/asound/pcm:"
cat /proc/asound/pcm
for card in `cat /proc/asound/cards|cut -c1-2`; do
	num=0
	codec="/proc/asound/card$card/codec#$num"
	while test -e "$codec"; do
		echo -e "\n$codec:"
		cat "$codec"
		num=$(($num+1))
		codec="/proc/asound/card$card/codec#$num"
	done
	num=0
	codec="/proc/asound/card$card/codec97#$num"
	while test -d "$codec"; do
		echo -e -n "\n$codec/ac97#"?-?
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
	card_udi="${card_udi%_sound_card_*}"
	createAlsaEntryFromHal "$playback_udi" "$card_udi" "playback"
done
alsacapture=`lshal -s|grep _alsa_capture`
for capture_udi in $alsacapture; do
	card_udi="${capture_udi%_alsa_capture_*}"
	card_udi="${card_udi%_sound_card_*}"
	createAlsaEntryFromHal "$capture_udi" "$card_udi" "capture"
done
oss=`lshal -s|grep _oss_pcm`
for udi in $oss; do
	card_udi="${udi%_oss_pcm*}"
	card_udi="${card_udi%_sound_card_*}"
	createOssEntryFromHal "$udi" "$card_udi"
done
echo ""
echo -e "$all_entries"|sort|sed "s,\t,\n,g"
