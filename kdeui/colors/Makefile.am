colors:
	: > colors.cpp
	list=`grep '^[0-9]' /usr/X11R6/lib/X11/rgb.txt  | tr '	' ' ' | sed -e "s#^[0-9]* [0-9]* [0-9]* *##" | grep -v " " | grep -v gray | grep -v grey | sort -u` ;\
	for i in $$list; do \
		echo "i18n(\"color\", \"$$i\");" | sed -e "s#!# #g" >> colors.cpp ;\
	done

messages:
	$(XGETTEXT) colors.cpp -o $(podir)/kdelibs_colors.pot

.PHONY: colors
