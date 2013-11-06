#
# Generates and cleans KConfigXT source code during a API dox build
#

kcfg_compiler="`kde4-config --prefix`/bin/kconfig_compiler"
if test -z "$kcfg_compiler"; then
	echo "kconfig_compiler not found!"
	exit 1;
fi

for i in `find . -maxdepth 1 -name "*.kcfgc"`; do
	kcfg_file=`grep "^File=" $i | sed -e "s/File=//"`
	echo "* preprocessing KConfigXT file $kcfg_file $i"
	if test "$1" = "create"; then 
		$kcfg_compiler $kcfg_file $i 
	elif test "$1" = "cleanup"; then
		rm -f `echo $i | sed -e "s/\.kcfgc$/.cpp/"`
		rm -f `echo $i | sed -e "s/\.kcfgc$/.h/"`
	fi
done
