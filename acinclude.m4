##
## Checks for the QT environment (library and header path)
## It's styled after the autoconf X11 check, first tries
## to compile a program without any flags in case everything
## is accessible that way.

## ------------------------------------------------------------------------
## Find a file (or one of more files in a list of dirs)
## ------------------------------------------------------------------------
##
AC_DEFUN(AC_FIND_FILE,
[
$3=NO
for i in $2;
do
  for j in $1;
  do
    if test -r "$i/$j"; then
      $3=$i
      break 2
    fi
  done
done
])

## ------------------------------------------------------------------------
## Internal macros to find if we don't need extra flags to find these
## This is not yet used
## ------------------------------------------------------------------------
##
AC_DEFUN(AC_PATH_QT_DIRECT,
[if test "$ac_qt_includes" = NO; then
AC_TRY_CPP([#include <qtstream.h>],
[
ac_qt_includes=
],[
])
fi
])

## ------------------------------------------------------------------------
## Find the meta object compiler in the PATH, in $QTDIR/bin, and some
## more usual places
## ------------------------------------------------------------------------
##
AC_DEFUN(AC_PATH_QT_MOC,
[
AC_PATH_PROG(MOC, moc, /usr/bin/moc,
 $PATH:/usr/bin:/usr/X11R6/bin:$QTDIR/bin:/usr/lib/qt/bin:/usr/local/qt/bin)
])

## ------------------------------------------------------------------------
## Try to find the QT headers and libraries.
## $(qt_libraries) will be -Lqtliblocation (if needed)
## and $(qt_includes) will be -Iqthdrlocation (if needed)
## ------------------------------------------------------------------------
##
AC_DEFUN(AC_PATH_QT,
[
AC_REQUIRE([AC_PATH_X])dnl
AC_MSG_CHECKING([for QT])
ac_qt_includes=NO ac_qt_libraries=NO
qt_libraries=""
qt_includes=""
AC_CACHE_VAL(ac_cv_have_qt,
[#try to guess qt locations

qt_incdirs="/usr/lib/qt/include /usr/local/qt/include /usr/include/qt /usr/include"
test -n "$QTDIR" && qt_incdirs="$QTDIR/include $QTDIR $qt_incdirs"
AC_FIND_FILE(qtstream.h, $qt_incdirs, qt_incdir)
ac_qt_includes=$qt_incdir

qt_libdirs="/usr/lib/qt/lib /usr/local/qt/lib /usr/lib/qt /usr/lib"
test -n "$QTDIR" && qt_libdirs="$QTDIR/lib $QTDIR $qt_libdirs"
AC_FIND_FILE(libqt.so libqt.a libqt.sl, $qt_libdirs, qt_libdir)
ac_qt_libraries=$qt_libdir

if test "$ac_qt_includes" = NO || test "$ac_qt_libraries" = NO; then
  ac_cv_have_qt="have_qt=no"
  AC_MSG_ERROR("QT not found. Please check your installation!")
else
  ac_cv_have_qt="have_qt=yes \
  ac_qt_includes=$ac_qt_includes ac_qt_libraries=$ac_qt_libraries"
fi])dnl

eval "$ac_cv_have_qt"

if test "$have_qt" != yes; then
  AC_MSG_RESULT([$have_qt])
else
  ac_cv_have_qt="have_qt=yes \
    ac_qt_includes=$ac_qt_includes ac_qt_libraries=$ac_qt_libraries"
  AC_MSG_RESULT([libraries $ac_qt_libraries, headers $ac_qt_includes])
  
  qt_libraries=$ac_qt_libraries
  qt_includes=$ac_qt_includes
fi
AC_SUBST(qt_libraries)
AC_SUBST(qt_includes)
AC_PATH_QT_MOC
])

## ------------------------------------------------------------------------
## Now, the same with KDE
## $(kde_libraries) will be the kdeliblocation (if needed)
## and $(kde_includes) will be the kdehdrlocation (if needed)
## ------------------------------------------------------------------------
##
AC_DEFUN(AC_PATH_KDE,
[
AC_REQUIRE([AC_PATH_X])dnl
AC_MSG_CHECKING([for KDE])
if test "${prefix}" != NONE; then
  kde_libraries=${prefix}/lib
  kde_includes=${prefix}/include
  AC_MSG_RESULT(["will be installed in" $prefix])
else
ac_kde_includes=NO ac_kde_libraries=NO
KDE_LDFLAGS=""
KDE_DEFS=""
AC_CACHE_VAL(ac_cv_have_kde,
[#try to guess kde locations

kde_incdirs="/usr/lib/kde/include /usr/local/kde/include /usr/kde/include /usr/include/kde /usr/include /usr/X11R6/include /usr/X11R6/kde/include"
test -n "$KDEDIR" && kde_incdirs="$KDEDIR/include $KDEDIR $kde_incdirs"
AC_FIND_FILE(ksock.h, $kde_incdirs, kde_incdir)
ac_kde_includes=$kde_incdir

kde_libdirs="/usr/lib/kde/lib /usr/local/kde/lib /usr/kde/lib /usr/lib/kde /usr/lib /usr/X11R6/lib /usr/X11R6/kde/lib"
test -n "$KDEDIR" && kde_libdirs="$KDEDIR/lib $KDEDIR $kde_libdirs"
AC_FIND_FILE(libkdecore.la, $kde_libdirs, kde_libdir)
ac_kde_libraries=$kde_libdir

if test "$ac_kde_includes" = NO || test "$ac_kde_libraries" = NO; then
  ac_cv_have_kde="have_kde=no"
else
  ac_cv_have_kde="have_kde=yes \
    ac_kde_includes=$ac_kde_includes ac_kde_libraries=$ac_kde_libraries"
fi])dnl

eval "$ac_cv_have_kde"

if test "$have_kde" != yes; then
 if test "${prefix}" = NONE; then
  ac_kde_prefix=$ac_default_prefix
 else
  ac_kde_prefix=$prefix
 fi
 AC_MSG_RESULT(["will be installed in" $ac_kde_prefix])

 kde_libraries=${ac_kde_prefix}/lib
 kde_includes=${ac_kde_prefix}/include

else
  ac_cv_have_kde="have_kde=yes \
    ac_kde_includes=$ac_kde_includes ac_kde_libraries=$ac_kde_libraries"
  AC_MSG_RESULT([libraries $ac_kde_libraries, headers $ac_kde_includes])
  
  kde_libraries=$ac_kde_libraries
  kde_includes=$ac_kde_includes
fi
fi
AC_SUBST(kde_libraries)
AC_SUBST(kde_includes)
])


AC_DEFUN(AC_CHECK_BOOL,
        [AC_MSG_CHECKING(for bool);
        AC_LANG_CPLUSPLUS
        AC_TRY_COMPILE([],
                [bool aBool = true;],
                AC_MSG_RESULT(yes) ; AC_DEFINE(HAVE_BOOL),
                AC_MSG_RESULT(no))])

# serial 1 AM_PROG_LIBTOOL
AC_DEFUN(AM_PROG_LIBTOOL,
[AC_REQUIRE([AC_CANONICAL_HOST])
AC_REQUIRE([AC_PROG_CC])
AC_REQUIRE([AC_PROG_RANLIB])
 
# Always use our own libtool.
LIBTOOL='$(top_builddir)/libtool'
AC_SUBST(LIBTOOL)
 
dnl Allow the --disable-shared flag to stop us from building shared libs.
AC_ARG_ENABLE(shared,
[  --enable-shared         build shared libraries [default=yes]],
test "$enableval" = no && libtool_shared=" --disable-shared",
libtool_shared=)
 
libtool_flags="$libtool_shared"
test "$silent" = yes && libtool_flags="$libtool_flags --silent"
test "$ac_cv_prog_gcc" = yes && libtool_flags="$libtool_flags --with-gcc"
 
# Actually configure libtool.  ac_aux_dir is where install-sh is found.
CC="$CC" CFLAGS="$CFLAGS" CPPFLAGS="$CPPFLAGS" LD="$LD" RANLIB="$RANLIB" \
$ac_aux_dir/ltconfig $libtool_flags --no-verify $ac_aux_dir/ltmain.sh $host \
|| AC_MSG_ERROR([libtool configure failed])
])
 

