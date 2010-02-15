#! /bin/sh
#
# doxygen.sh Copyright (C) 2005 by Adriaan de Groot
#            Based on some code from Doxyfile.am, among other things.
# License:   GPL version 2.
#            See file COPYING in kdelibs for details.

echo "*** doxygen.sh"

# Recurse handling is a little complicated, since normally
# subdir (given on the command-line) processing doesn't recurse
# but you can force it to do so.
recurse=1
recurse_given=NO
use_modulename=1
cleanup=YES
preprocess=0
manpages=0
qhppages=0
searchengine=0

while test -n "$1" ; do
case $1 in
--no-cleanup)
	cleanup=NO
	;;
--no-recurse)
	recurse=0
	recurse_given=YES
	;;
--recurse)
	recurse=1
	recurse_given=YES
	;;
--no-modulename)
	use_modulename=0
	;;
--modulename)
	use_modulename=1
	;;
--preprocess)
	preprocess=1
	;;
--manpages)
        manpages=1
        ;;
--qhppages)
        qhppages=1
        ;;
--searchengine)
        searchengine=1
        ;;
--help)
	echo "doxygen.sh usage:"
	echo "doxygen.sh [--options] <srcdir> [<subdir>]"
	echo "  --no-cleanup     Do not remove Doxyfile and other cruft from build"
	echo "  --no-recurse     Build only the given top-level directory"
	echo "  --no-modulename  Build in apidocs/, not apidocs-module/"
	echo "  --doxdatadir=dir Use dir as the source of global Doxygen files"
	echo "  --installdir=dir Use dir as target to install to"
	echo "  --preprocess     Generate source code (KConfigXT, uic, etc.)"
        echo "  --manpages       Generate man pages in addition to html"
        echo "  --qhppages       Generate pages for Qt Assistant"
        echo "  --searchengine   Enable search engine"
	exit 2
	;;
--doxdatadir=*)
	DOXDATA=`echo $1 | sed -e 's+--doxdatadir=++'`
	;;
--installdir=*)
	PREFIX=`echo $1 | sed -e 's+--installdir=++'`
	;;
--*)
	echo "Unknown option: $1"
	exit 1
	;;
*)
	top_srcdir="$1"
	break
	;;
esac
shift
done


### Sanity check the mandatory "top srcdir" argument.
if test -z "$top_srcdir" ; then
	echo "Usage: doxygen.sh <top_srcdir>"
	exit 1
fi
if test ! -d "$top_srcdir" ; then
	echo "top_srcdir ($top_srcdir) is not a directory."
	exit 1
fi

### Normalize top_srcdir so it is an absolute path.
if expr "x$top_srcdir" : "x/" > /dev/null ; then
	# top_srcdir is absolute already
	:
else
	top_srcdir=`cd "$top_srcdir" 2> /dev/null && pwd`
	if test ! -d "$top_srcdir" ; then
		echo "top_srcdir ($top_srcdir) is not a directory."
		exit 1
	fi
fi

### Normalize DOXDATA so it is an absolute path.
if test -n "$DOXDATA"; then
    if expr "x$DOXDATA" : "x/" > /dev/null ; then
	# DOXDATA is absolute already
	:
    else
	DOXDATA=`cd "$DOXDATA" 2> /dev/null && pwd`
	if test ! -d "$DOXDATA" ; then
		echo "DOXDATA ($DOXDATA) is not a directory."
		exit 1
	fi
    fi
fi


### Sanity check and guess QTDOCDIR.
if test -z "$QTDOCDIR" ; then
	if test -z "$QTDIR" ; then
		for i in /usr/X11R6/share/doc/qt/html
		do
			QTDOCDIR="$i"
			test -d "$QTDOCDIR" && break
		done
	else
		for i in share/doc/qt/html doc/html
		do
			QTDOCDIR="$QTDIR/$i"
			test -d "$QTDOCDIR" && break
		done
	fi
fi
if test -z "$QTDOCDIR"  || test ! -d "$QTDOCDIR" ; then
	if test -z "$QTDOCDIR" ; then
		echo "* QTDOCDIR could not be guessed."
	else
		echo "* QTDOCDIR does not name a directory."
	fi
	if test -z "$QTDOCTAG" ; then
		echo "* QTDOCDIR set to \"\""
		QTDOCDIR=""
	else
		echo "* But I'll use $QTDOCDIR anyway because of QTDOCTAG."
	fi
fi

### Get the "top srcdir", also its name, and handle the case that subdir "."
### is given (which would be top_srcdir then, so it's equal to none-given
### but no recursion either).
###
# top_srcdir="$1" # Already set by options processing
module_name=`basename "$top_srcdir"`
subdir="$2"
if test "x." = "x$subdir" ; then
	subdir=""
	if test "x$recurse_given" = "xNO" ; then
		recurse=0
	fi
fi
if test "x" != "x$subdir" ; then
	# If no recurse option given explicitly, default to
	# no recurse when processing subdirs given on the command-line.
	if test "x$recurse_given" = "xNO" ; then
		recurse=0
	fi
fi

if test -z "$DOXDATA" || test ! -d "$DOXDATA" ; then
	if test -n "$DOXDATA" ; then
		echo "* \$DOXDATA is '$DOXDATA' which does not name a directory"
	fi
	DOXDATA="$top_srcdir/doc/common"
fi

if test ! -d "$DOXDATA" ; then
	echo "* \$DOXDATA does not name a directory ( or is unset ), tried \"$DOXDATA\""
	exit 1
fi

if test -n "$PREFIX" && test ! -d "$PREFIX" ; then
	echo "* \$PREFIX does not name a directory, tried \"$PREFIX\""
	echo "* \$PREFIX is disabled."
	PREFIX=""
fi

TOPNAME=`grep '^/.*DOXYGEN_NAME' "$top_srcdir/Mainpage.dox" | sed -e 's+.*=++' | sed s+\"++g`
if test -z "$TOPNAME" ; then
    TOPNAME="API Reference"
fi

COPYRIGHT=`grep '^/.*DOXYGEN_COPYRIGHT' "$top_srcdir/Mainpage.dox" | sed -e 's+[^=]*=++' | sed s+\"++g`
if test -z "$COPYRIGHT" ; then
    COPYRIGHT="1996-`date +%Y` The KDE developers"
fi

#
# Preprocess source dir and generate source files (KConfigXT, uic, etc.)
# @param $1: create or cleanup (parameter for doxygen-preprocess-foo.sh)
#
preprocess_sources()
{
	for dir in `find $top_srcdir/$subdir -type d | grep -v ".svn"`; do
		# execute global preprocessing modules
		local preproc_base="`dirname $0`"
		for proc in `find "$preproc_base" -name "doxygen-preprocess-*.sh"`; do
			(cd $dir && $proc $1)
		done
		# execute local preprocessing modules
		for proc in `find $dir -maxdepth 1 -name "doxygen-preprocess-*.sh"`; do
			echo "* running $proc"
			(cd $dir && $proc $1)
		done
	done
}

if test "$preprocess" = "1"; then
	echo "* Generating source code in $top_srcdir/$subdir"
	preprocess_sources create
	echo "* Generating source code completed"
fi

### We need some values from top-level files, which
### are not preserved between invocations of this
### script, so factor it out for easy use.
create_doxyfile_in() 
{
	VERSION=`grep '^/.*DOXYGEN_VERSION' "$top_srcdir/Mainpage.dox" | sed -e 's+.*=++'`
	NAME=`grep '^/.*DOXYGEN_NAME' "$top_srcdir/Mainpage.dox" | sed -e 's+.*=++'`
	echo "PROJECT_NUMBER = $VERSION" > Doxyfile.in
	echo "PROJECT_NAME = $NAME" >> Doxyfile.in
}

apidoxdir="$module_name"-apidocs
test "x$use_modulename" = "x0" && apidoxdir="apidocs"

### If we're making the top subdir, create the structure
### for the apidox and initialize it. Otherwise, just use the
### structure assumed to be there.
if test -z "$subdir" ; then
	if test ! -d "$apidoxdir" ; then
		mkdir "$apidoxdir" > /dev/null 2>&1
	fi
	cd "$apidoxdir" > /dev/null 2>&1 || { 
		echo "Cannot create and cd into $apidoxdir"
		exit 1
	}

	test -f "Doxyfile.in" || create_doxyfile_in

	# Copy in logos and the like
	for i in "favicon.ico" "block_title_bottom.png" "block_title_mid.png" "block_title_top.png" "top-kde.jpg" "top-left.jpg" "top-right.jpg" "top.jpg" "kde.css" "flat.css" "print.css" "tabs.css"
	do
		cp "$DOXDATA/$i" . > /dev/null 2> /dev/null
	done
	for i in "$top_srcdir/doc/api/Dox-"*.png
	do
		T=`basename "$i" | sed -e 's+Dox-++'`
		test -f "$i" && cp "$i" "./$T" > /dev/null 2> /dev/null
	done

	top_builddir="."
	srcdir="$1"
	subdir="."
else
	cd "$apidoxdir" > /dev/null 2>&1 || {
		echo "Cannot cd into $apidoxdir -- maybe you need to"
		echo "build the top-level dox first."
		exit 1
	}

	if test "x1" = "x$recurse" ; then
		# OK, so --recurse was requested
		if test ! -f "subdirs.top" ; then
			echo "* No subdirs.top available in the $apidoxdir."
			echo "* The --recurse option will be ignored."
			recurse=0
		fi
	fi
fi

### Read a single line (TODO: support \ continuations) from the Mainpage.dox.
### Used to extract variable assignments from it.
extract_line()
{
	file="$2" ; test -z "$file" && file="$srcdir/Mainpage.dox"
	test -f "$file" || return
	pattern=`echo "$1" | tr + .`
	grep "^//[[:space:]]*$1" "$file" | \
		sed -e "s+//[[:space:]]*$pattern.*=[[:space:]]*++"
}

### Try to order the top-level subdirectories so that we get the smallest
### amount of re-processing due to missing tag files.
create_subdirs()
{
echo "* Sorting top-level subdirs"

( cd "$top_srcdir" && 
for dir in *
do
	# For each Mainpage.dox, get the list (possibly empty) of
	# referenced tag files.
	list=""
	if test -d "$dir" && test -f "$dir/Mainpage.dox" ; then
		# Need this dir at the very least, even if it doesn't reference
		echo "$dir" "$dir"
		list=`grep DOXYGEN_REFERENCES $dir/Mainpage.dox | sed 's/.*=//'`
	fi

	# Print the partial order pairs that we now know. This
	# is *expected* to produce loops in the partial order;
	# these cannot be avoided.
	for req in $list ; do
		echo $req $dir
	done

done ) | grep -v / | tsort 2> /dev/null > subdirs.top
}


apidox_manpages()
{
        mp=`extract_line DOXYGEN_GENERATE_MAN`
        if test -z "$mp" ; then
               mp="YES"
        fi
	echo "GENERATE_MAN           = $mp" >> "$subdir/Doxyfile"
        echo "MAN_OUTPUT             = man" >> "$subdir/Doxyfile"
        echo "MAN_EXTENSION          = .3"  >> "$subdir/Doxyfile"
        echo "MAN_LINKS              = YES" >> "$subdir/Doxyfile"
}

apidox_qhppages()
{
        qp=`extract_line DOXYGEN_GENERATE_QHP`
        if test -z "$qp" ; then
               qp="YES"
        fi
	echo "GENERATE_QHP           = $qp" >> "$subdir/Doxyfile"
        echo "QHP_NAMESPACE          = org.kde.www" >> "$subdir/Doxyfile"
        if test -z "$PROJECT_NAME" ; then
               PROJECT_NAME="KDE"
        fi
        if test -z "$PROJECT_VERSION" ; then
               PROJECT_VERSION="4.5"
        fi
        vf="$PROJECT_NAME"-"$PROJECT_VERSION"
        echo "QHP_VIRTUAL_FOLDER     = $vf" >> "$subdir/Doxyfile"
        echo "QHG_LOCATION           = qhelpgenerator" >> "$subdir/Doxyfile"
}

apidox_searchengine()
{
        se=`extract_line DOXYGEN_SEARCHENGINE`
        if test -z "$se" ; then
               se="YES"
        fi
	echo "SEARCHENGINE           = $se" >> "$subdir/Doxyfile"
}

### Add HTML header, footer, CSS tags to Doxyfile.
### Assumes $subdir is set. Argument is a string
### to stick in front of the file if needed.
apidox_htmlfiles()
{
	dox_header="$top_srcdir/doc/api/$1header.html"
	dox_footer="$top_srcdir/doc/api/$1footer.html"
	dox_css="$top_srcdir/doc/api/doxygen.css"
	test -f "$dox_header" || dox_header="$DOXDATA/$1header.html"
	test -f "$dox_footer" || dox_footer="$DOXDATA/$1footer.html"
	test -f "$dox_css" || dox_css="$DOXDATA/doxygen.css"

	echo "HTML_HEADER            = $dox_header" >> "$subdir/Doxyfile" ; \
	echo "HTML_FOOTER            = $dox_footer" >> "$subdir/Doxyfile" ; \
	echo "HTML_STYLESHEET        = $dox_css" >> "$subdir/Doxyfile"
}

apidox_specials()
{
	line=`extract_line DOXYGEN_PROJECTNAME "$1"`
	test -n "$line" && echo "PROJECT_NAME = \"$line\"" >> "$2"
	line=`extract_line DOXYGEN_PROJECTVERSION "$1"`
	test -n "$line" && echo "PROJECT_NUMBER = \"$line\"" >> "$2"
}

apidox_local()
{
	for i in "$top_srcdir/doc/api/Doxyfile.local"
	do
		if test -f "$i" ; then
			cat "$i" >> "$subdir/Doxyfile"
			break
		fi
	done
}

### Post-process HTML files by substituting in the menu files
#
# In non-top directories, both <!-- menu --> and <!-- gmenu -->
# are calculated and replaced. Top directories get an empty <!-- menu -->
# if any.
doxyndex()
{
	# Special case top-level to have an empty MENU.
	if test "x$subdir" = "x." ; then
		MENU=""
		htmldir="."
		htmltop="$top_builddir" # Just ., presumably
		echo "* Post-processing top-level files"
	else
		MENU="<ul>"
		htmldir="$subdir/html"
		htmltop="$top_builddir.." # top_builddir ends with /
		echo "* Post-processing files in $htmldir"

		# Build a little PHP file that maps class names to file
		# names, for the quick-class-picker functionality.
		# (The quick-class-picker is disabled due to styling
		# problems in IE & FF).
		(
		echo "<?php \$map = array(";  \
		for htmlfile in `find $htmldir/ -type f -name "class[A-Z]*.html" | grep -v "\-members.html$"`; do
			classname=`echo $htmlfile | sed -e "s,.*/class\\(.*\\).html,\1," -e "s,_1_1,::,g" -e "s,_01, ,g" -e "s,_4,>,g" -e "s+_00+,+g" -e "s+_3+<+g" | tr "[A-Z]" "[a-z]"`
			echo "  \"$classname\" => \"$htmlfile\","
		done | sort ; \
		echo ") ?>"
		) > "$subdir/classmap.inc"

		# This is a list of pairs, with / separators so we can use
		# basename and dirname (a crude shell hack) to split them
		# into parts. For each, if the file part exists (as a html
		# file) tack it onto the MENU variable as a <li> with link.
		for i in "Main Page/index" \
			"Modules/modules" \
			"Namespace List/namespaces" \
			"Class Hierarchy/hierarchy" \
			"Alphabetical List/classes" \
			"Class List/annotated" \
			"File List/files" \
			"Directories/dirs" \
			"Namespace Members/namespacemembers" \
			"Class Members/functions" \
			"Related Pages/pages"
		do
			NAME=`dirname "$i"`
			FILE=`basename "$i"`
			test -f "$htmldir/$FILE.html" && MENU="$MENU<li><a href=\"$FILE.html\">$NAME</a></li>"
		done

		MENU="$MENU</ul>"
	fi


	# Get the list of global Menu entries.
	GMENU=`cat subdirs | tr -d '\n'`

	PMENU=`grep '<!-- pmenu' "$htmldir/index.html" | sed -e 's+.*pmenu *++' -e 's+ *-->++' | awk '{ c=split($0,a,"/"); for (j=1; j<=c; j++) { printf " / <a href=\""; if (j==c) { printf("./index.html"); } for (k=j; k<c; k++) { printf "../index.html"; } if (j<c) { printf("../html/index.html"); } printf "\">%s</a>\n" , a[j]; } }' | tr -d '\n'`

	# Map the PHP file into HTML options so that
	# it can be substituted in for the quick-class-picker.
	CMENU=""
	# For now, leave the CMENU disabled
	CMENUBEGIN="<!--"
	CMENUEND="-->"

	if test "x$subdir" = "x." ; then
		# Disable CMENU on toplevel anyway
		CMENUBEGIN="<!--"
		CMENUEND="-->"
	else
		test -f "$subdir/classmap.inc" && \
		CMENU=`grep '=>' "$subdir/classmap.inc" | sed -e 's+"\([^"]*\)" => "'"$subdir/html/"'\([^"]*\)"+<option value="\2">\1<\/option>+' | tr -d '\n'`

		if test -f "$subdir/classmap.inc" && grep "=>" "$subdir/classmap.inc" > /dev/null 2>&1 ; then
			# Keep the menu, it's useful
			:
		else
			CMENUBEGIN="<!--"
			CMENUEND="-->"
		fi
	fi

	# Now substitute in the MENU in every file. This depends
	# on HTML_HEADER (ie. header.html) containing the
	# <!-- menu --> comment.
	for i in "$htmldir"/*.html
	do
		if test -f "$i" ; then
			sed -e "s+<!-- menu -->+$MENU+" \
				-e "s+<!-- gmenu -->+$GMENU+" \
				-e "s+<!-- pmenu.*-->+$PMENU+" \
				-e "s+<!-- cmenu.begin -->+$CMENUBEGIN+" \
				-e "s+<!-- cmenu.end -->+$CMENUEND+" \
				< "$i"  | sed -e "s+@topdir@+$htmltop+g" | sed -e "s+@topname@+$TOPNAME+g" | sed -e "s+@copyright@+$COPYRIGHT+g" > "$i.new" && mv "$i.new" "$i"
			sed -e "s+<!-- cmenu -->+$CMENU+" < "$i" > "$i.new"
			test -s "$i.new" && mv "$i.new" "$i"
			echo "* Added menus to $i"
		fi
	done

## Allen: Comment this out for now.
##        Basically, I think it's annoying. We need something less obtrusive.
## 	# For each class, post-process it to check for bc-ness
## 	for i in "$htmldir"/class*.html
## 	do
## 		test "$htmldir/classes.html" = "$i" && continue
## 		if test -f "$i" ; then
## 			grep -l 'bc.html#_bc' "$i" > /dev/null || \
## 			{ sed -e 's+<!-- BC -->+<div class="bic">This class is not guaranteed to be binary compatible across releases.</div>+' "$i" > "$i.new" ; test -s "$i.new" && mv "$i.new" "$i" ; }
## 			echo "* Added BC warning to $i"
## 		fi
## 	done
}






### Handle the Doxygen processing of a toplevel directory.
apidox_toplevel()
{
	echo ""
	echo "*** Creating API documentation main page for $module_name"
	echo "*"
	#rm -f "Doxyfile"
	for i in "$top_srcdir/doc/api/Doxyfile.global" \
		"$DOXDATA/Doxyfile.global"
	do
		if test -f "$i" ; then
			cp "$i" Doxyfile
			break
		fi
	done

	if test ! -f "Doxyfile" ; then
		echo "* Cannot create Doxyfile."
		exit 1
	fi

	cat "$top_builddir/Doxyfile.in" >> Doxyfile


	{
		echo "INPUT                  = $top_srcdir"
		echo "DOTFILE_DIRS           = $top_srcdir"
		echo "OUTPUT_DIRECTORY       = $top_builddir"
		echo "RECURSIVE              = NO"
		echo "ALPHABETICAL_INDEX     = NO"
		echo "HTML_OUTPUT            = ." 
	} >> Doxyfile
	apidox_htmlfiles "main"
        if test "$manpages" = "1"; then
                apidox_manpages
        fi
        if test "$qhppages" = "1"; then
                apidox_qhppages
        fi
        if test "$searchengine" = "1"; then
                apidox_searchengine
        fi

	# KDevelop has a top-level Makefile.am with settings.
	for i in "$top_srcdir/Mainpage.dox"
	do
		if test -f "$i" ; then
			grep '^//[[:space:]]*DOXYGEN_SET_' "$i" | \
				sed -e 's+//[[:space:]]*DOXYGEN_SET_++' -e "s+@topdir@+$top_srcdir+g" >> Doxyfile
			apidox_specials "$srcdir/Mainpage.dox" "$subdir/Doxyfile"

			break
		fi
	done

	apidox_local

	doxygen Doxyfile

	( cd "$top_srcdir" && find . -name Mainpage.dox ) | sed -e 's+/Mainpage.dox$++' -e 's+^\./++' | sort > subdirs.in
	for i in `cat subdirs.in`
	do
		test "x." = "x$i" && continue;

		dir=`dirname "$i"`
		file=`basename "$i"`
		if test "x." = "x$dir" ; then
			dir=""
		else
			dir="$dir/"
		fi
		indent=`echo "$dir" | sed -e 's+[^/]*/+\&nbsp;\&nbsp;+g' | sed -e 's+&+\\\&+g'`
		entryname=`extract_line DOXYGEN_SET_PROJECT_NAME "$top_srcdir/$dir/$file/Mainpage.dox"`
		test -z "$entryname" && entryname="$file"

		if grep DOXYGEN_EMPTY "$top_srcdir/$dir/$file/Mainpage.dox" > /dev/null 2>&1 ; then
			echo "<li>$indent$file</li>"
		else
			echo "<li>$indent<a href=\"@topdir@/$dir$file/html/index.html\">$entryname</a></li>"
		fi
	done > subdirs

	doxyndex
}

### Handle the Doxygen processing of a non-toplevel directory.
#
# $1 is empty to generate tag files only.
# $1 is non-empty to generate HTML.
#
# Additionally, the environment variable $NO_APPEND_TAG may be set 
# to a non-empty value to suppress adding the tag for this subdir
# to the list of tag files (used when re-generating a single subdir).
#
apidox_subdir()
{
	echo ""
	if test -z "$1" ; then
		echo "*** Creating tag file in $subdir"
	else
		echo "*** Creating apidox in $subdir"
	fi
	echo "*"
	#rm -f "$subdir/Doxyfile"
	if test ! -d "$top_srcdir/$subdir" ; then
		echo "* No source (sub)directory $subdir"
		return
	fi
	for i in "$top_srcdir/doc/api/Doxyfile.global" \
		"$DOXDATA/Doxyfile.global"
	do
		if test -f "$i" ; then
			cp "$i" "$subdir/Doxyfile"
			break
		fi
	done


	test -f "Doxyfile.in" || create_doxyfile_in
	cat "Doxyfile.in" >> "$subdir/Doxyfile"

	{
	echo "PROJECT_NAME           = \"$subdir\""
	if test -d "$top_srcdir/doc/common" ; then
		echo "INPUT                  = $top_srcdir/doc/common $srcdir"
	else
		echo "INPUT                  = $srcdir"
	fi
	echo "DOTFILE_DIRS           = $top_srcdir $srcdir"
	echo "OUTPUT_DIRECTORY       = ."
	if grep -l "$subdir/" subdirs.in > /dev/null 2>&1 ; then
		echo "RECURSIVE              = NO"
	fi
	echo "HTML_OUTPUT            = $subdir/html"
	if test -d "$top_srcdir/doc/api"; then
		echo "IMAGE_PATH             = $top_srcdir/doc/api $srcdir $srcdir/doc/pics"
                echo "EXAMPLE_PATH           = $top_srcdir/doc/api/examples $srcdir/examples $srcdir/doc/examples"
	else
		echo "IMAGE_PATH	     = $srcdir $srcdir/doc/pics"
		echo "EXAMPLE_PATH	     = $srcdir/examples $srcdir/doc/examples"
	fi
	} >> "$subdir/Doxyfile"

	apidox_htmlfiles ""
        if test "$manpages" = "1"; then
                apidox_manpages
        fi
        if test "$qhppages" = "1"; then
                apidox_qhppages
        fi
        if test "$searchengine" = "1"; then
                apidox_searchengine
        fi

	# Mainpage.doxs may contain overrides to our settings,
	# so copy them in.
	grep '^//[[:space:]]*DOXYGEN_SET_' "$srcdir/Mainpage.dox" | \
		sed -e 's+//[[:space:]]*DOXYGEN_SET_++' -e "s+@topdir@+$top_srcdir+g" >> "$subdir/Doxyfile"
	apidox_specials "$srcdir/Mainpage.dox" "$subdir/Doxyfile"

	excludes=`extract_line DOXYGEN_EXCLUDE`
	if test -n "$excludes"; then
		patterns=""
		dirs=""
		for item in `echo "$excludes"`; do
			if test -d "$top_srcdir/$subdir/$item"; then
				dirs="$dirs $top_srcdir/$subdir/$item/"
			else
				patterns="$patterns $item"
			fi
		done
		echo "EXCLUDE_PATTERNS      += $patterns" >> "$subdir/Doxyfile"
		echo "EXCLUDE               += $dirs" >> "$subdir/Doxyfile"
	fi

	echo "TAGFILES = \\" >> "$subdir/Doxyfile"
	## For now, don't support \ continued references lines
	tags=`extract_line DOXYGEN_REFERENCES`
	for i in $tags qt ; do
		tagsubdir=`dirname $i` ; tag=`basename $i`
		tagpath=""
		not_found=""

		if test "x$tagsubdir" = "x." ; then
			tagsubdir=""
		else
			tagsubdir="$tagsubdir/"
		fi

		# Find location of tag file
		if test -f "$tagsubdir$tag/$tag.tag" ; then
			file="$tagsubdir$tag/$tag.tag"
			loc="$tagsubdir$tag/html"
		else
			# This checks for dox built with_out_ --no-modulename
			# in the same build dir as this dox run was started in.
			file=`ls -1 ../*-apidocs/"$tagsubdir$tag/$tag.tag" 2> /dev/null`

			if test -n "$file" ; then
				loc=`echo "$file" | sed -e "s/$tag.tag\$/html/"`
			else
				# If the tag file doesn't exist yet, but should
				# because we have the right dirs here, queue
				# this directory for re-processing later.
				if test -d "$top_srcdir/$tagsubdir$tag" ; then
					echo "* Need to re-process $subdir for tag $i"
					echo "$subdir" >> "subdirs.later"
				else
					# Re-check in $PREFIX if needed.
					test -n "$PREFIX" && \
					file=`cd "$PREFIX" && \
					ls -1 *-apidocs/"$tagsubdir$tag/$tag.tag" 2> /dev/null`

					# If something is found, patch it up. The location must be
					# relative to the installed location of the dox and the
					# file must be absolute.
					if test -n "$file" ; then
						loc=`echo "../$file" | sed -e "s/$tag.tag\$/html/"`
						file="$PREFIX/$file"
						echo "* Tags for $tagsubdir$tag will only work when installed."
						not_found="YES"
					fi
				fi
			fi
		fi
		if test "$tag" = "qt" ; then
			if test -z "$QTDOCDIR" ; then
				echo "  $file" >> "$subdir/Doxyfile"
			else
				if test -z "$file" ; then
					# Really no Qt tags
					echo "\\" >> "$subdir/Doxyfile"
				else
					echo "  $file=$QTDOCDIR \\" >> "$subdir/Doxyfile"
				fi
			fi
		else
			if test -n "$file"  ; then
				test -z "$not_found" && echo "* Found tag $file"
				echo "  $file=../$top_builddir$loc \\" >> "$subdir/Doxyfile"
			fi
		fi
	done

	if test -z "$1" ; then
		echo "" >> "$subdir/Doxyfile"
		echo "GENERATE_HTML=NO" >> "$subdir/Doxyfile"
		echo "GENERATE_TAGFILE=$subdir/$subdirname.tag" >> "$subdir/Doxyfile"
		test -z "$NO_APPEND_TAG" && echo "	$subdir/$subdirname.tag \\" >> subdirs.tag
	else
		#test -s subdirs.tag && grep -v "$subdir/$subdirname.tag" subdirs.tag >> "$subdir/Doxyfile"
		echo "" >> "$subdir/Doxyfile"
		echo "GENERATE_HTML=YES" >> "$subdir/Doxyfile"
	fi

	apidox_local

	if grep '^DOXYGEN_EMPTY' "$srcdir/Mainpage.dox" > /dev/null 2>&1 ; then
		# This directory is empty, so don't process it, but
		# *do* handle subdirs that might have dox.
		:
	else
		# Regular processing
		doxygen "$subdir/Doxyfile"
		if test -n "$1" ; then
			doxyndex
		fi
	fi
}

### Run a given subdir by setting up global variables first.
#
# $1 is the subdir to process
# $2 is the flag for apidox_subdir
#
do_subdir()
{
	local subdir=`echo "$1" | sed -e 's+/$++'`
	srcdir="$top_srcdir/$subdir"
	subdirname=`basename "$subdir"`
	mkdir -p "$subdir" 2> /dev/null
	if test ! -d "$subdir" ; then
		echo "Can't create dox subdirectory $subdir"
		return
	fi
	top_builddir=`echo "/$subdir" | sed -e 's+/[^/]*+../+g'`
	apidox_subdir "$2"
}


### Create installdox-slow in the toplevel
create_installdox()
{
# Fix up the installdox script so it accepts empty args
#
# This code is copied from the installdox generated by Doxygen,
# copyright by Dimitri van Heesch and released under the GPL.
# This does a _slow_ update of the dox, because it loops
# over the given substitutions instead of assuming all the
# needed ones are given.
#
cat <<\EOF
#! /usr/bin/env perl

%subst = () ;
$quiet   = 0;

if (open(F,"search.cfg"))
{
  $_=<F> ; s/[ \t\n]*$//g ; $subst{"_doc"} = $_;
  $_=<F> ; s/[ \t\n]*$//g ; $subst{"_cgi"} = $_;
}

while ( @ARGV ) {
  $_ = shift @ARGV;
  if ( s/^-// ) {
    if ( /^l(.*)/ ) {
      $v = ($1 eq "") ? shift @ARGV : $1;
      ($v =~ /\/$/) || ($v .= "/");
      $_ = $v;
      if ( /(.+)\@(.+)/ ) {
          $subst{$1} = $2;
      } else {
        print STDERR "Argument $_ is invalid for option -l\n";
        &usage();
      }
    }
    elsif ( /^q/ ) {
      $quiet = 1;
    }
    elsif ( /^\?|^h/ ) {
      &usage();
    }
    else {
      print STDERR "Illegal option -$_\n";
      &usage();
    }
  }
  else {
    push (@files, $_ );
  }
}


if ( ! @files ) {
  if (opendir(D,".")) {
    foreach $file ( readdir(D) ) {
      $match = ".html";
      next if ( $file =~ /^\.\.?$/ );
      ($file =~ /$match/) && (push @files, $file);
      ($file =~ "tree.js") && (push @files, $file);
    }
    closedir(D);
  }
}

if ( ! @files ) {
  print STDERR "Warning: No input files given and none found!\n";
}

foreach $f (@files)
{
  if ( ! $quiet ) {
    print "Editing: $f...\n";
  }
  $oldf = $f;
  $f   .= ".bak";
  unless (rename $oldf,$f) {
    print STDERR "Error: cannot rename file $oldf\n";
    exit 1;
  }
  if (open(F,"<$f")) {
    unless (open(G,">$oldf")) {
      print STDERR "Error: opening file $oldf for writing\n";
      exit 1;
    }
    if ($oldf ne "tree.js") {
      while (<F>) {
	foreach $sub (keys %subst) {
          s/doxygen\=\"$sub\:([^ \"\t\>\<]*)\" (href|src)=\"\1/doxygen\=\"$sub:$subst{$sub}\" \2=\"$subst{$sub}/g;
          print G "$_";
	}
      }
    }
    else {
      while (<F>) {
	foreach $sub (keys %subst) {
          s/\"$sub\:([^ \"\t\>\<]*)\", \"\1/\"$sub:$subst{$sub}\" ,\"$subst{$sub}/g;
          print G "$_";
	}
      }
    }
  }
  else {
    print STDERR "Warning file $f does not exist\n";
  }
  unlink $f;
}

sub usage {
  print STDERR "Usage: installdox [options] [html-file [html-file ...]]\n";
  print STDERR "Options:\n";
  print STDERR "     -l tagfile\@linkName   tag file + URL or directory \n";
  print STDERR "     -q                    Quiet mode\n\n";
  exit 1;
}
EOF
}

# Do only the subdirs that match the RE passed in as $1
do_subdirs_re()
{
	RE=`echo "$1" | sed -e 's+/$++'`

	# Here's a queue of dirs to re-process later when
	# all the rest have been done already.
	> subdirs.later

	# subdirs.top lists _all_ subdirs of top in the order they
	# should be handled; subdirs.in lists those dirs that contain
	# dox. So the intersection of the two is the ordered list
	# of top-level subdirs that contain dox.
	#
	# subdirs.top also doesn't contain ".", so that special
	# case can be ignored in the loop.


	(
	for i in `grep "^$RE" subdirs.top`
	do
		if test "x$i" = "x." ; then
			continue
		fi
		# Calculate intersection of this element and the
		# set of dox dirs.
		if grep "^$i\$" subdirs.in > /dev/null 2>&1 ; then
			echo "$i"
			mkdir -p "$i" 2> /dev/null

			# Handle the subdirs of this one
			for j in `grep "$i/" subdirs.in`
			do
				echo "$j"
				mkdir -p "$j" 2> /dev/null
			done
		fi
	done

	# Now we still need to handle whatever is left
	for i in `cat subdirs.in`
	do
		test -d "$i" || echo "$i"
		mkdir -p "$i" 2> /dev/null
	done
	) > subdirs.sort

	# Run once for the tags
	for i in `cat subdirs.sort`
	do
		do_subdir "$i"
	done

	# Run again for the HTML
	for i in `cat subdirs.sort`
	do
		do_subdir "$i" true
	done

}

if test "x." = "x$top_builddir" ; then
	apidox_toplevel
	create_subdirs
	create_installdox > installdox-slow
	if test "x$recurse" = "x1" ; then
		if test "x$module_name" = "xkdelibs" ; then
			if test -z "$QTDOCTAG" && test -d "$QTDOCDIR" && \
				test ! -f "qt/qt.tag" ; then
				# Special case: create a qt tag file.
				echo "*** Creating a tag file for the Qt library:"
				mkdir qt
				doxytag -t qt/qt.tag "$QTDOCDIR" > /dev/null 2>&1
			fi
		fi
		if test -n "$QTDOCTAG" && test -r "$QTDOCTAG" ; then
			echo "*** Copying tag file for the Qt library:"
			mkdir qt
			cp "$QTDOCTAG" qt/qt.tag
		fi

		do_subdirs_re "."

	fi
else
	if test "x$recurse" = "x1" ; then
		do_subdirs_re "$subdir"
	else
		NO_APPEND_TAG=true do_subdir "$subdir"
		do_subdir "$subdir" true
	fi
fi


# At the end of a run, clean up stuff.
if test "YES" = "$cleanup" ; then
	rm -f subdirs.in  subdirs.later subdirs.sort subdirs.top Doxyfile.in
	#rm -f `find . -name Doxyfile`
#	rm -f qt/qt.tag
#	rmdir qt > /dev/null 2>&1
	if test "$preprocess" = "1"; then
		echo "* Cleaning up previously generated source code in $top_srcdir/$subdir"
		preprocess_sources cleanup
		echo "* Cleaning up previously generated source code done"
	fi
fi


exit 0

