dnl aclocal.m4 generated automatically by aclocal 1.1n


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

AC_DEFUN(AC_PATH_QT_DIRECT,
[if test "$ac_qt_includes" = NO; then
AC_TRY_CPP([#include <qtstream.h>],
[
ac_qt_includes=
],[
])
fi
])

AC_DEFUN(AC_PATH_QT_MOC,
[
AC_PATH_PROG(MOC, moc, /usr/bin/moc,
 $PATH:/usr/bin:/usr/X11R6/bin:$QTDIR/bin:/usr/lib/qt/bin:/usr/local/qt/bin)
])

AC_DEFUN(K_PATH_X,
[
AC_MSG_CHECKING(for X)
AC_CACHE_VAL(ac_cv_have_x,
[# One or both of the vars are not set, and there is no cached value.
ac_x_includes=NO ac_x_libraries=NO
AC_PATH_X_DIRECT
AC_PATH_X_XMKMF
if test "$ac_x_includes" = NO || test "$ac_x_libraries" = NO; then
  AC_MSG_ERROR([Can't find X. Please add the correct paths. View configure --help for usage!])
else
  # Record where we found X for the cache.
  ac_cv_have_x="have_x=yes \
                ac_x_includes=$ac_x_includes ac_x_libraries=$ac_x_libraries"
fi])dnl
eval "$ac_cv_have_x"
 
if test "$have_x" != yes; then
  AC_MSG_RESULT($have_x)
  no_x=yes
else
  # If each of the values was on the command line, it overrides each guess.
  test "x$x_includes" = xNONE && x_includes=$ac_x_includes
  test "x$x_libraries" = xNONE && x_libraries=$ac_x_libraries
  # Update the cache value to reflect the command line values.
  ac_cv_have_x="have_x=yes \
                ac_x_includes=$x_includes ac_x_libraries=$x_libraries"
  AC_MSG_RESULT([libraries $x_libraries, headers $x_includes])
fi

if test -z "$x_includes" || test "x$x_includes" = xNONE; then
  X_INCLUDES=""
  x_includes="."; dnl better than nothing :-
 else
  X_INCLUDES="-I$x_includes"
fi

if test -z "$x_libraries" || test "$x_libraries" = xNONE; then
  X_LDFLAGS=""
  x_libraries="/usr/lib"; dnl better than nothing :-
  all_libraries=""
 else
  X_LDFLAGS="-L$x_libraries"
  all_libraries=$X_LDFLAGS
fi

AC_SUBST(X_INCLUDES)
AC_SUBST(X_LDFLAGS)
all_includes=$X_INCLUDES
])
AC_DEFUN(AC_PATH_QT,
[
AC_REQUIRE([K_PATH_X])

AC_MSG_CHECKING([for QT])
ac_qt_includes=NO ac_qt_libraries=NO
qt_libraries=""
qt_includes=""
AC_CACHE_VAL(ac_cv_have_qt,
AC_PATH_QT_DIRECT
[#try to guess qt locations

qt_incdirs="/usr/lib/qt/include /usr/local/qt/include /usr/include/qt /usr/include /usr/X11R6/include/X11/qt $x_includes $QTINC"
test -n "$QTDIR" && qt_incdirs="$QTDIR/include $QTDIR $qt_incdirs"
AC_FIND_FILE(qslider.h, $qt_incdirs, qt_incdir)
ac_qt_includes=$qt_incdir

qt_libdirs="/usr/lib/qt/lib /usr/local/qt/lib /usr/lib/qt /usr/lib $x_libraries $QTLIB"
test -n "$QTDIR" && qt_libdirs="$QTDIR/lib $QTDIR $qt_libdirs"
AC_FIND_FILE(libqt.so libqt.a libqt.sl, $qt_libdirs, qt_libdir)
ac_qt_libraries=$qt_libdir

ac_cxxflags_safe=$CXXFLAGS
ac_ldflags_safe=$LDFLAGS
CXXFLAGS="$CXXFLAGS -I$qt_incdir"
LDFLAGS="-lqt -L$qt_libdir $X_LDFLAGS -lX11 -lXext"

AC_LANG_CPLUSPLUS
cat > conftest.$ac_ext <<EOF
#include "confdefs.h"
#include <qslider.h>
int main() {
  QSlider s;
  s.addStep();
  return 0;
};
EOF

if AC_TRY_EVAL(ac_link) && test -s conftest; then
  rm -f conftest*
else
  echo "configure: failed program was:" >&AC_FD_CC
  cat conftest.$ac_ext >&AC_FD_CC
  ac_qt_libraries="NO"
fi
rm -f conftest*
CXXFLAGS=$ac_cxxflags_safe
LDFLAGS=$ac_ldflags_safe
 
if test "$ac_qt_includes" = NO || test "$ac_qt_libraries" = NO; then
  ac_cv_have_qt="have_qt=no"
  ac_qt_notfound=""
  if test "$ac_qt_includes" = NO; then
    if test "$ac_qt_libraries" = NO; then
      ac_qt_notfound="(headers and libraries)";
    else
      ac_qt_notfound="(headers)";
    fi
  else
    ac_qt_notfound="(libraries)";
  fi

  AC_MSG_ERROR([QT1.2 $ac_qt_notfound not found. Please check your installation! ]);
else
  ac_cv_have_qt="have_qt=yes \
  ac_qt_includes=$ac_qt_includes ac_qt_libraries=$ac_qt_libraries"
fi])dnl

eval "$ac_cv_have_qt"

if test "$have_qt" != yes; then
  AC_MSG_RESULT([$have_qt]);
else
  ac_cv_have_qt="have_qt=yes \
    ac_qt_includes=$ac_qt_includes ac_qt_libraries=$ac_qt_libraries"
  AC_MSG_RESULT([libraries $ac_qt_libraries, headers $ac_qt_includes])
  
  qt_libraries=$ac_qt_libraries
  qt_includes=$ac_qt_includes
fi
AC_SUBST(qt_libraries)
AC_SUBST(qt_includes)

if test "$qt_includes" = "$x_includes"; then
 QT_INCLUDES="";
else
 QT_INCLUDES="-I$qt_includes"
 all_includes="$all_includes $QT_INCLUDES"
fi

if test "$qt_libraries" = "$x_libraries"; then
 QT_LDFLAGS=""
else
 QT_LDFLAGS="-L$qt_libraries"
 all_libraries="$all_libraries $QT_LDFLAGS"
fi

AC_SUBST(QT_INCLUDES)
AC_SUBST(QT_LDFLAGS)
AC_PATH_QT_MOC
])

AC_DEFUN(AC_PATH_KDE,
[
AC_REQUIRE([AC_PATH_QT])dnl
AC_MSG_CHECKING([for KDE])
if test "${prefix}" != NONE; then
  kde_libraries=${prefix}/lib
  kde_includes=${prefix}/include
  AC_MSG_RESULT(["will be installed in" $prefix])
else
ac_kde_includes=NO ac_kde_libraries=NO
kde_libraries=""
kde_includes=""
AC_CACHE_VAL(ac_cv_have_kde,
[#try to guess kde locations

kde_incdirs="/usr/lib/kde/include /usr/local/kde/include /usr/kde/include /usr/include/kde /usr/include $x_includes $qt_includes"

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

if test "$kde_includes" = "$x_includes" || test "$kde_includes" = "$qt_includes" ; then
 KDE_INCLUDES=""
else
 KDE_INCLUDES="-I$kde_includes"
 all_includes="$all_includes $KDE_INCLUDES"
fi

if test "$kde_libraries" = "$x_libraries" || test "$kde_libraries" = "$qt_libraries" ; then
 KDE_LDFLAGS=""
else
 KDE_LDFLAGS="-L$kde_libraries"
 all_libraries="$all_libraries $KDE_LDFLAGS"
fi

AC_SUBST(KDE_LDFLAGS)
AC_SUBST(KDE_INCLUDES)
AC_SUBST(all_includes)
AC_SUBST(all_libraries)

])

dnl slightly changed version of AC_CHECK_FUNC(setenv)
AC_DEFUN(AC_CHECK_SETENV,
[AC_MSG_CHECKING([for setenv])
AC_CACHE_VAL(ac_cv_func_setenv,
[AC_LANG_C
AC_TRY_LINK(
dnl Don't include <ctype.h> because on OSF/1 3.0 it includes <sys/types.h>
dnl which includes <sys/select.h> which contains a prototype for
dnl select.  Similarly for bzero.
[#include <assert.h>
]ifelse(AC_LANG, CPLUSPLUS, [#ifdef __cplusplus
extern "C"
#endif
])dnl
[/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
#include <stdlib.h>
], [
/* The GNU C library defines this for functions which it implements
    to always fail with ENOSYS.  Some functions are actually named
    something starting with __ and the normal name is an alias.  */
#if defined (__stub_$1) || defined (__stub___$1)
choke me
#else
setenv("TEST", "alle", 1);
#endif
], eval "ac_cv_func_setenv=yes", eval "ac_cv_func_setenv=no")])

if test "$ac_cv_func_setenv" = "yes"; then
  AC_MSG_RESULT(yes)
  AC_DEFINE_UNQUOTED(HAVE_FUNC_SETENV)
else
  AC_MSG_RESULT(no)
  MISCOBJS="$MISCOBJS setenv.lo"
fi
])

AC_DEFUN(AC_FIND_GIF,
   [AC_MSG_CHECKING(for giflib)
AC_CACHE_VAL(ac_cv_lib_gif,
[ac_save_LIBS="$LIBS"
LIBS="$all_libraries -lgif -lX11"
AC_TRY_LINK(dnl
ifelse([main], [main], , dnl Avoid conflicting decl of main.
[/* Override any gcc2 internal prototype to avoid an error.  */
]ifelse(AC_LANG, CPLUSPLUS, [#ifdef __cplusplus
extern "C"
#endif
])dnl
[/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
char main();
]),
            [main()],
            eval "ac_cv_lib_gif=yes",
            eval "ac_cv_lib_gif=no")
LIBS="$ac_save_LIBS"
])dnl
if eval "test \"`echo $ac_cv_lib_gif`\" = yes"; then
  AC_MSG_RESULT(yes)
  AC_DEFINE_UNQUOTED(HAVE_LIBGIF)
else
  AC_MSG_ERROR(You need giflib23. Please install the kdesupport package)
fi
])

AC_DEFUN(AC_FIND_JPEG,
   [AC_MSG_CHECKING(for jpeglib)
AC_CACHE_VAL(ac_cv_lib_jpeg,
[ac_save_LIBS="$LIBS"
LIBS="$all_libraries -ljpeg -lm"
AC_TRY_LINK(dnl
ifelse([main], [main], , dnl Avoid conflicting decl of main.
[/* Override any gcc2 internal prototype to avoid an error.  */
]ifelse(AC_LANG, CPLUSPLUS, [#ifdef __cplusplus
extern "C"
#endif
])dnl
[/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
char main();
]),
            [main()],
            eval "ac_cv_lib_jpeg=yes",
            eval "ac_cv_lib_jpeg=no")
LIBS="$ac_save_LIBS"
])dnl
if eval "test \"`echo $ac_cv_lib_jpeg`\" = yes"; then
  AC_MSG_RESULT(yes)
  AC_DEFINE_UNQUOTED(HAVE_LIBJPEG)
else
  AC_MSG_ERROR(You need jpeglib6a. Please install the kdesupport package)
fi
])

AC_DEFUN(AC_CHECK_BOOL,
        [AC_MSG_CHECKING(for bool);
        AC_LANG_CPLUSPLUS
        AC_TRY_COMPILE([],
                [bool aBool = true;],
                AC_MSG_RESULT(yes) ; AC_DEFINE(HAVE_BOOL),
                AC_MSG_RESULT(no))])

AC_DEFUN(AC_SET_DEBUG,
[
 test "$CFLAGS" = "" && CFLAGS="-g -Wall" 
 test "$CXXFLAGS" = "" && CXXFLAGS="-g -Wall"
 test "$LDFLAGS" = "" && LDFLAGS="" dnl looks stupid
])

AC_DEFUN(AC_SET_NODEBUG,
[
 test "$CFLAGS" = "" && CFLAGS="-O2 -Wall"
 test "$CXXFLAGS" = "" && CXXFLAGS="-O2 -Wall"
 test "$LDFLAGS" = "" && LDFLAGS="-s"
])

AC_DEFUN(AC_CHECK_DEBUG,
[AC_ARG_ENABLE(debug,[ --enable-debug 	creates debugging code [default=no]],
[ if test $enableval = "no"; then AC_SET_NODEBUG
else AC_SET_DEBUG 
fi
],
AC_SET_NODEBUG)
])

dnl just a test
AC_DEFUN(AC_CHECK_FLAGS, 
[
AC_REQUIRE([AC_CHECK_DEBUG])
AC_SUBST(CXXFLAGS)
AC_SUBST(CFLAGS)
AC_SUBST(LDFLAGS)
])

dnl Check for the type of the third argument of getsockname
AC_DEFUN(AC_CHECK_KSIZE_T,
[AC_MSG_CHECKING(for the third argument of getsockname)  
AC_CACHE_VAL(ac_ksize_t_int,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/socket.h>],[int a=0; getsockname(0,(struct sockaddr*)NULL, &a);],
eval "ac_ksize_t_int=yes",
eval "ac_ksize_t_int=no")])
if eval "test \"`echo `$ac_ksize_t_int\" = yes"; then
  AC_MSG_RESULT(int)
  AC_DEFINE(ksize_t, int)
else
  AC_MSG_RESULT(size_t)
  AC_DEFINE(ksize_t, size_t)
fi
])

dnl this should be included by aclocal, but it wont't
dnl Don't know, why
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

# Do all the work for Automake.  This macro actually does too much --
# some checks are only needed if your package does certain things.
# But this isn't really a big deal.

# serial 1

dnl Usage:
dnl AM_INIT_AUTOMAKE(package,version, [no-define])

AC_DEFUN(AM_INIT_AUTOMAKE,
[AC_REQUIRE([AM_PROG_INSTALL])
PACKAGE=[$1]
AC_SUBST(PACKAGE)
VERSION=[$2]
AC_SUBST(VERSION)
ifelse([$3],,
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE")
AC_DEFINE_UNQUOTED(VERSION, "$VERSION"))
AM_SANITY_CHECK
AC_ARG_PROGRAM
dnl FIXME This is truly gross.
missing_dir=`cd $ac_aux_dir && pwd`
AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOMAKE, automake, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)
AM_MISSING_PROG(MAKEINFO, makeinfo, $missing_dir)
AC_PROG_MAKE_SET])


# serial 1

AC_DEFUN(AM_PROG_INSTALL,
[AC_REQUIRE([AC_PROG_INSTALL])
test -z "$INSTALL_SCRIPT" && INSTALL_SCRIPT='${INSTALL_PROGRAM}'
AC_SUBST(INSTALL_SCRIPT)dnl
])

#
# Check to make sure that the build environment is sane.
#

AC_DEFUN(AM_SANITY_CHECK,
[AC_MSG_CHECKING([whether build environment is sane])
# Just in case
sleep 1
echo timestamp > conftestfile
# Do `set' in a subshell so we don't clobber the current shell's
# arguments.  Must try -L first in case configure is actually a
# symlink; some systems play weird games with the mod time of symlinks
# (eg FreeBSD returns the mod time of the symlink's containing
# directory).
if (
   set X `ls -Lt $srcdir/configure conftestfile 2> /dev/null`
   if test "$@" = "X"; then
      # -L didn't work.
      set X `ls -t $srcdir/configure conftestfile`
   fi
   test "[$]2" = conftestfile
   )
then
   # Ok.
   :
else
   AC_MSG_ERROR([newly created file is older than distributed files!
Check your system clock])
fi
rm -f conftest*
AC_MSG_RESULT(yes)])

dnl AM_MISSING_PROG(NAME, PROGRAM, DIRECTORY)
dnl The program must properly implement --version.
AC_DEFUN(AM_MISSING_PROG,
[AC_MSG_CHECKING(for working $2)
# Run test in a subshell; some versions of sh will print an error if
# an executable is not found, even if stderr is redirected.
if ($2 --version) > /dev/null 2>&1; then
   $1=$2
   AC_MSG_RESULT(found)
else
   $1="$3/missing $2"
   AC_MSG_RESULT(missing)
fi
AC_SUBST($1)])

# Like AC_CONFIG_HEADER, but automatically create stamp file.

AC_DEFUN(AM_CONFIG_HEADER,
[AC_PREREQ([2.12])
AC_CONFIG_HEADER([$1])
dnl When config.status generates a header, we must update the stamp-h file.
dnl This file resides in the same directory as the config header
dnl that is generated.  We must strip everything past the first ":",
dnl and everything past the last "/".
AC_OUTPUT_COMMANDS(changequote(<<,>>)dnl
test -z "<<$>>CONFIG_HEADERS" || echo timestamp > patsubst(<<$1>>, <<^\([^:]*/\)?.*>>, <<\1>>)stamp-h<<>>dnl
changequote([,]))])

