
                   TechInfo # 5563

Short Description:
Using ESQL/C with a C++ Compiler

Long Description:
Original-From: johnl@informix.com (Jonathan Leffler)
Original-Date: Tuesday 12th December 1995
Updated-From: jleffler@informix.com (Jonathan Leffler)
Updated-Date: Tuesday 16th March 1999
Subject: Using ESQL/C with C++

One of the more common requests these days is:

    How do I compile ESQL/C code for use with C++?

There have been a variety of answers, but not any one systematic answer.
Now there is what I believe is a workable answer -- it's all in the
attached shell archive.

What do you get?
*   Updated ESQL/C prototypes for 4.00, 4.1x, 5.0x and 7.2x
*   An esql++ compiler
*   A pair of patch files for modifying your standard ESQL/C compiler.
    Only use these if your ESQL/C is version 4.1 or 5.x; later versions
    already have the necessary change).
*   A minimal test application.
*   No documentation apart from this message!

What do you need to do?
*   Unpack the shell archive.  For this discussion, it is assumed that you
    do this in the directory /tmp/esql.  Ideally, you need to have enough
    permissions to copy files into $INFORMIXDIR, which means you should be
    either informix or root.  See below if this is not feasible.
*   Place the esql* headers in either $INFORMIXDIR/incl (version 4)
    or $INFORMIXDIR/incl/esql (version 5 or later)
*   Place the esql++ script in $INFORMIXDIR/bin
*   If you are using version 4.1x or 5.x ESQL/C, then do:
        cd $INFORMIXDIR/bin; patch < /tmp/esql/patch.vN.esql
    where you replace the N with either 4 or 5.  If you don't have patch,
    get it from the Internet.  It is widely available.  For example, it can
    be tracked down via http://www.yahoo.com, following the Software link
    (under Computers & Internet) and then GNU Software, or you can go
    direct to ftp://prep.ai.mit.edu/pub/gnu (a heavily loaded system, so
    use a mirror site if you can).
*   Decide which version of ESQL/C you are using and set the ESQLC_VERSION
    environment variable to a value such as:
        export ESQLC_VERSION=506    # 5.06.UC1
    If you do not do this, the esql++ script will run the esql script to
    determine the correct version number for you each time it is run, but
    it will be slower than setting the environment variable.  Note that the
    ClientSDK versions of ESQL/C have numbers like 916 (for ClientSDk
    2.10).  You should not try to use 730 as the version, even if you are
    using ClientSDK 2.10 with IDS 7.30.  It will probably work, but is
    certainly not guaranteed to work.  The automatic system uses the output
    of 'esql -V', and so should you.
*   Decide which C++ compiler you use -- if it isn't CC, either set:
        export INFORMIXCPP=g++
    or edit the esql++ script to make your C++ compiler into the default.
*   Try compiling the test application:
        make -f Makefile
*   If your C++ compiler is ISO 14882:1998 compliant (eg EGCS 1.1.1) to the
    extent of not recognizing the <iostream.h> header, compile the code
    with CPP_FLAGS=-DISO14882_HEADERS; this will use <iostream> and <cstdlib> in
    place of <iostream.h> and <stdlib.h>:
        make -f Makefile CPP_FLAGS=-DISO14882_HEADERS
*   Run it against your local stores database -- it should print a SELECT
    statement and say that the tabid for the syscolumns table is 2.

What if you can't (or do not wish to) modify your INFORMIXDIR?
*   If you need to patch the esql script (because you are using 4.1x or 5.x
    ESQL/C), copy $INFORMIXDIR/bin/esql to somewhere where you can modify
    it (eg /some/where/bin/esql).  Remember that the directory needs to be
    on your PATH and in front of the 'real' esql script which you cannot
    modify.  Then patch it as described above.
*   Place the headers in another directory (eg /some/where/include).
*   Modify esql++ so that:
    -- arglist starts of with -I/some/where/include as one component
    -- ESQL defaults to /some/where/bin/esql
*   Copy the modified esql++ script into /some/where/bin.
*   You may need to rehash your PATH, too, after doing these changes.

How should you use it after testing it?
*   Any ESQL/C source file which also contains C++ statements should be
    compiled with the esql++ compiler instead of the esql compiler.
*   Any program which contains any C++ code should be linked with the
    esql++ compiler instead of the esql compiler.
*   Any ESQL/C source file which contains only C statements should still be
    compiled with the esql compiler because the esql++ compiler always and
    unconditionally compiles with the C++ compiler.
*   Note that you can adjust the behaviour of the esql++ compiler with the
    following environment variables:
        ESQL
        ESQLC_CPLUSPLUS_EXTENSION
        ESQLC_RM_COMMAND
        ESQLC_VERSION
        INFORMIXC
        INFORMIXCPP
        TMPDIR

What about other versions of ESQL/C?

*   If you are still using a pre-4.00 version of ESQL/C, the chances are
    that the code which edits the 4.xx code will be useful.  However, the
    prototypes for the ESQL/C interface functions are not known and will
    have to be determined by inspection of the generated code.  Good luck!
    It will probably be easier to upgrade to a later version.

*   I haven't been able to test version 8.xx ESQL/C; it isn't available to
    me.  However, the indications are that it will need the same treatment
    as the other post-5.00 implementations of ESQL/C.

*   Testing with the version 9.10 ESQL/C compiler indicates that this
    script is still necessary.  See also the notes on version 7.24 below.

*   Ideally, you won't need this script at all for post-9.xx versions.  If
    you are using some later version, you will need to revise the script so
    that the case statement on $ESQLC_VERSION handles the new (presumably
    4-digit) version number.

*   Note that ESQL/C 7.24 has some support for C++ built in.  In
    particular, it preprocesses files with the .ecpp extension and then
    compiles them with the C++ compiler.  However, it is not very clever
    about what it does; for example, it still includes the following stanza
    in the generated output, which is an apalling idea for C++:

    #if !defined(__STDC__)
    #define const
    #endif

    On HP-UX 10.20, and probably on other machines too, the C++ compiler
    does not define __STDC__ (which is probably correct as that define is
    for ANSI C compilers, not C++ compilers).  The 7.24 esql script does
    not link the object code with the C++ compiler, which can also cause
    problems.  You can work around this by running the link command it with
    INFORMIXC="C++ compiler" in the environment.  Do not try to do this
    when compiling traditional ESQL/C files to object code; they will be
    compiled with the C++ compiler which will not, in general, work.

*   ClientSDK 2.01 (which supplies ESQL/C 9.14) also does not work very
    cleanly.  For example, the -D options on the command line are not
    relayed to the C++ compiler when the source is a .ecpp file, and the
    linking is (still) done by the C compiler, which does not work in
    general.  C++ programs must be linked by the C++ compiler -- anything
    else is to error-prone to be worth considering.  One issue for the
    design of the esql script is how to identify that the linker should be
    the C++ compiler rather than the C compiler.  When any C++ source is
    passed to the script, this should be deducible by the script.  Ideally,
    there should be an argument to the esql script to specify each of the C
    compiler, the C++ compiler, and the linker, and there should be an
    option to specify that linking should be done by the C++ compiler
    without specifying it as the linker.  Note that you can specify the two
    compilers only via the INFORMIXC and INFORMIXCPP environment variables;
    there is no linker environment variable.  Most of these observations
    also apply to ESQL/C 7.24, but there are also the additional problems
    shown in the previous bullet point.

*   Note that in a pure C++ and ESQL/C program, you can get ClientSDK 2.01
    to produce a program provided you set INFORMIXC and INFORMIXCPP to the
    C++ compiler.  However, if any of the code is pure C code (which cannot
    be compiled by the C++ compiler), then this strategy for building the
    program fails unless you compile the object files separately and then
    link them with the standard esql script.  For example, a prior version
    of the test code did not use testc.c and testmain.ec included
    <stdlib.h> and the main() function returned EXIT_SUCCESS directly.
    When this code was modified so that the .ec files were renamed to .ecpp
    files, the compilation succeeded when the command line was:
        INFORMIXC="g++ -DESQLC_VERSION=914" \
        INFORMIXCPP="g++ -DESQLC_VERSION=914" \
        esql -o testapp testmain.ecpp testsub.ecpp testcpp.cc
    However, this does not work in a single command with mixed C and C++
    code as in the current version of the test.  It will work if you
    compile testc.c to object code with the C compiler before running the
    following command:
        INFORMIXC="g++ -DESQLC_VERSION=914" \
        INFORMIXCPP="g++ -DESQLC_VERSION=914" \
        esql -o testapp testmain.ecpp testsub.ecpp testcpp.cc testc.o
    Obviously, you could compile the .ecpp and .cc files to object files
    before doing the linking and the link will work if you name the object
    files on the command line, but only if INFORMIXC points to the C++
    compiler at link time.  The compilation will not work if you do not
    rename the .ec files to the convention expected by the ClientSDK 2.01
    ESQL/C compiler.

*   Since the above comments were written, the ESQL/C group stated in email
    dated 1998-11-09 that the support was added but not tested, and will
    neither be fixed nor documented.  Obviously, the support should then be
    removed, but inertia and past experience indicates that this will not
    happen during the 20th Century (and might not happen during the 21st
    either).  Holding your breath waiting for this to be remedied is not
    recommended.

Which versions of ESQL/C are are known to work with this?
*   The code has been tested against Informix-ESQL/C versions 4.12.UC1,
    5.08.UD1, 6.00.UE1, 7.13.UC1, 7.24.UC1, 9.12.UC2 on a Sun Sparc 20
    running Solaris 2.5.1 with both SUNWspro C++ SPARCompiler SC4.0.1 and
    GNU G++ 2.7.2.2 (circa December 1997)
*   The code has also been tested against ESQL/C 7.24.UC1 and ClientSDK
    2.01.UC1 on a Sun Sparc 20 running Solaris 2.6 with GNU G++ 2.8.1
    (circa August 1998).
*   The code has also been tested against ClientSDK 2.10.UC1 (ESQL/C
    9.16.UC1), and with ClientSDK 2.20.UC1 (ESQL/C 9.20.UC1), on a Sun Sparc
    20 running Solaris 2.6 with EGCS 1.1.1 (circa March 1999).

Are there any known bugs?
*   Life will get very difficult if you pass arguments with quotes or
    spaces in them to your compiler:

        esql++ -DOPENQUOTE="'" -DSOMMAT="string with blanks" thingy.ec

    I do have a solution: it is another program called escape which
    deals with precisely that sort of problem.
        arglist="$arglist `escape $arg`"
    But that requires some more source and isn't generally necessary.
    Also, the underlying esql script does not work reliably with such
    arguments, so the changes needed are even more extensive than this
    comment suggests.

*   The esql7_20.h header is needed until bug B73951 is fixed.

*   With ESQL/C versions prior to ClientSDK 2.01 (2.00 untested), the esql
    script passed all the library options to the C and C++ compilers even
    when generating object code and not an executable (indicated by the -c
    option).  With ClientSDK 2.01, this no longer occurs.  This matters
    because the GNU C and C++ compilers warn when the options are not used,
    and it therefore makes for a very noisy compilation when the libraries
    are passed to the compiler when linking to object code.

*   Command line options such as -D, -ED, -I and -EI are not passed to the
    compilers correctly.

Any problems?  Better let me know.  Especially if you've got a fix.

---------------------------------------------------------------------------
NB: This material is supplied 'as is' with no warranty of any kind.  You
use it at your own risk.  It is not officially supported by Informix
Software Inc.
---------------------------------------------------------------------------

Jonathan Leffler (jleffler@informix.com)
Informix Software Inc

@(#)$Id$

:	"@(#): shar.sh,v 2.1 1998/06/02 17:13:43 jleffler Exp $"
#! /bin/sh
#
#	This is a shell archive.
#	Remove everything above this line and run sh on the resulting file.
#	If this archive is complete, you will see this message at the end:
#	"All files extracted"
#
#	Created on: Tue Mar 16 11:18:46 PST 1999
#	Created by: jleffler at Informix Software Inc.
#
#	Files archived in this archive:
#	esql4_00.h
#	esql4_10.h
#	esql5_00.h
#	esql7_20.h
#	esqlc.h
#	esqllib.h
#	esql++.sh
#	esql.v4.patch
#	esql.v5.patch
#	Makefile
#	testapp.h
#	testc.c
#	testcpp.cc
#	testmain.ec
#	testsub.ec
#
#--------------------
if [ -f esql4_00.h -a "$1" != "-c" ]
then echo shar: esql4_00.h already exists
else
echo 'x - esql4_00.h (1129 characters)'
sed -e 's/^X//' >esql4_00.h <<'SHAR-EOF'
X/*
X@(#)File:            $RCSfile$
X@(#)Version:         $Revision$
X@(#)Last changed:    $Date$
X@(#)Purpose:         Function prototypes for ESQL/C Version 4.00
X@(#)Author:          J Leffler
X@(#)Copyright:       (C) JLSS 1992-93,1995-97
X@(#)Product:         :PRODUCT:
X*/
X
X/*TABSTOP=4*/
X
X/*
X** Function prototypes for functions found in:
X** Informix ESQL/C application-engine interface library Version 4.00.
X** List of names derived from output of "strings $INFORMIXDIR/lib/esqlc"
X** iec_stop() is called by WHENEVER ERROR STOP.
X*/
X
X#ifndef ESQL4_00_H
X#define ESQL4_00_H
X
X#ifdef MAIN_PROGRAM
X#ifndef lint
Xstatic const char esql4_00_h[] = "@(#)$Id$";
X#endif	/*lint */
X#endif	/*MAIN_PROGRAM */
X
X#ifdef __cplusplus
Xextern "C" {
X#endif /* __cplusplus */
X
X#include "esql4_10.h"
X
Xextern int      sqgetdbs(int *ret_fcnt,
X                         char **fnames,
X                         int fnsize,
X                         char *farea,
X                         int fasize);
X
X#ifdef __cplusplus
X}
X#endif /* __cplusplus */
X
X#endif	/* ESQL4_00_H */
SHAR-EOF
chmod 444 esql4_00.h
if [ `wc -c <esql4_00.h` -ne 1129 ]
then echo shar: esql4_00.h unpacked with wrong size
fi
# end of overwriting check
fi
#--------------------
if [ -f esql4_10.h -a "$1" != "-c" ]
then echo shar: esql4_10.h already exists
else
echo 'x - esql4_10.h (5539 characters)'
sed -e 's/^X//' >esql4_10.h <<'SHAR-EOF'
X/*
X@(#)File:            $RCSfile$
X@(#)Version:         $Revision$
X@(#)Last changed:    $Date$
X@(#)Purpose:         Function prototypes for ESQL/C Version 4.10
X@(#)Author:          J Leffler
X@(#)Copyright:       (C) JLSS 1992-93,1995-97
X@(#)Product:         :PRODUCT:
X*/
X
X/*TABSTOP=4*/
X
X/*
X** Function prototypes for functions found in:
X** Informix ESQL/C application-engine interface library Version 4.10.
X** Same as for Version 4.00 except: sqgetdbs() does not link in 4.10!
X** List of names derived from output of "strings $INFORMIXDIR/lib/esqlc"
X** iec_stop() is called by WHENEVER ERROR STOP.
X*/
X
X#ifndef ESQL4_10_H
X#define ESQL4_10_H
X
X#ifdef MAIN_PROGRAM
X#ifndef lint
Xstatic const char esql4_10_h[] = "@(#)$Id$";
X#endif	/*lint */
X#endif	/*MAIN_PROGRAM */
X
X#ifdef __cplusplus
Xextern "C" {
X#endif /* __cplusplus */
X
X#include <sqlhdr.h>
X#include <sqlda.h>
X
X/* Pre-declare struct value to keep compilers quiet */
Xstruct value;
X
Xextern int      _iqbeginwork(void);
Xextern int      _iqclose(_SQCURSOR *cursor);
Xextern int      _iqcommit(void);
Xextern int      _iqcopen(_SQCURSOR *cursor,
X                         int icnt,
X                         struct sqlvar_struct *ibind,
X                         struct sqlda *idesc,
X                         struct value *ivalues,
X                         int useflag);
Xextern int      _iqdatabase(char *db_name,
X                            int exclusive,
X                            int icnt,
X                            struct sqlvar_struct *ibind);
Xextern int      _iqdbclose(void);
Xextern int      _iqdclcur(_SQCURSOR *cursor,
X                          char *curname,
X                          char **cmdtxt,
X                          int icnt,
X                          struct sqlvar_struct *ibind,
X                          int ocnt,
X                          struct sqlvar_struct *obind,
X                          int flags);
Xextern int      _iqddclcur(_SQCURSOR *cursor,
X                           char *curname,
X                           int flags);
Xextern int      _iqdscribe(_SQCURSOR *cursor, struct sqlda **descp);
Xextern int      _iqeximm(char *stmt);
Xextern int      _iqflush(_SQCURSOR *cursor);
Xextern int      _iqfree(_SQCURSOR *cursor);
Xextern int      _iqftch(_SQCURSOR *cursor,
X                        int ocnt,
X                        struct sqlvar_struct *obind,
X                        struct sqlda *odescriptor,
X                        int chkind);
Xextern int      _iqinsput(_SQCURSOR *cursor,
X                          int icnt,
X                          struct sqlvar_struct *ibind,
X                          struct sqlda *idesc,
X                          struct value *ivalues);
Xextern int      _iqnftch(_SQCURSOR *cursor,
X                         int ocnt,
X                         struct sqlvar_struct *obind,
X                         struct sqlda *odescriptor,
X                         int fetch_type,
X                         long val,
X                         int icnt,
X                         struct sqlvar_struct *ibind,
X                         struct sqlda *idescriptor,
X                         int chkind);
Xextern int      _iqpclose(_SQCURSOR *cursor);
Xextern int      _iqpdclcur(_SQCURSOR *cursor,
X                           char *cursor_name,
X                           int statement_type,
X                           char *table_name,
X                           char **select_list,
X                           char **orderby_list,
X                           char **where_text,
X                           int icnt,
X                           struct sqlvar_struct *ibind,
X                           int ocnt,
X                           struct sqlvar_struct *obind,
X                           int for_update);
Xextern int      _iqpdelete(_SQCURSOR *cursor);
Xextern int      _iqpopen(_SQCURSOR *cursor,
X                         int icnt,
X                         struct sqlvar_struct *ibind,
X                         struct sqlda *idesc,
X                         struct value *ivalues,
X                         int useflag);
Xextern int      _iqpput(_SQCURSOR *cursor,
X                        int icnt,
X                        struct sqlvar_struct *ibind,
X                        struct sqlda *idesc,
X                        struct value *ivalues);
Xextern int      _iqprepare(_SQCURSOR *cursor, char *stmt);
Xextern int      _iqpupdate(_SQCURSOR *cursor,
X                           char **ucolumn_list,
X                           int icnt,
X                           struct sqlvar_struct *ibind);
Xextern int      _iqrollback(void);
Xextern int      _iqslct(_SQCURSOR *cursor,
X                        char **cmdtxt,
X                        int icnt,
X                        struct sqlvar_struct *ibind,
X                        int ocnt,
X                        struct sqlvar_struct *obind,
X                        int chkind);
Xextern int      _iqstmnt(_SQSTMT *scb,
X                         char **cmdtxt,
X                         int icnt,
X                         struct sqlvar_struct *ibind,
X                         struct value *ivalues);
Xextern int      _iqxecute(_SQCURSOR *cursor,
X                          int icnt,
X                          struct sqlvar_struct *ibind,
X                          struct sqlda *idesc,
X                          struct value *ivalues);
Xextern int      iec_stop(void);
Xextern int      sqlbreak(void);
Xextern int      sqlexit(void);
Xextern int      sqlstart(void);
X
X#ifdef __cplusplus
X}
X#endif /* __cplusplus */
X
X#endif	/* ESQL4_10_H */
SHAR-EOF
chmod 444 esql4_10.h
if [ `wc -c <esql4_10.h` -ne 5539 ]
then echo shar: esql4_10.h unpacked with wrong size
fi
# end of overwriting check
fi
#--------------------
if [ -f esql5_00.h -a "$1" != "-c" ]
then echo shar: esql5_00.h already exists
else
echo 'x - esql5_00.h (6135 characters)'
sed -e 's/^X//' >esql5_00.h <<'SHAR-EOF'
X/*
X@(#)File:            $RCSfile$
X@(#)Version:         $Revision$
X@(#)Last changed:    $Date$
X@(#)Purpose:         Function prototypes for ESQL/C Versions 5.00..5.07
X@(#)Author:          J Leffler
X@(#)Copyright:       (C) JLSS 1992-93,1995-97
X@(#)Product:         :PRODUCT:
X*/
X
X/*
X**	@(#)Informix ESQL/C Version 5.0x ANSI C Function Prototypes
X*/
X
X/*
X**  Beware:
X**  ESQL/C version 5.00 has a 4-argument version of _iqlocate_cursor(), but
X**  ESQL/C versions 5.02 and upwards (to 5.07 at least) have a 3-argument
X**  version of _iqlocate_cursor().  Opinion is divided on whether version
X**  5.01 used 3 or 4 arguments.  On SunOS 4.1.3, 5.01.UC1 used 4 arguments.
X**  You must set ESQLC_VERSION accurately.
X*/
X
X#ifndef ESQL5_00_H
X#define ESQL5_00_H
X
X#ifdef __cplusplus
Xextern "C" {
X#endif /* __cplusplus */
X
X/* There is an unprototyped declaration of _iqlocate_cursor() in <sqlhdr.h> */
X#undef _iqlocate_cursor
X#define _iqlocate_cursor _iq_non_existent
X#include <sqlhdr.h>
X#undef _iqlocate_cursor
X
X#include <sqlda.h>
X#include <value.h>
X
X#ifdef MAIN_PROGRAM
X#ifndef lint
Xstatic const char esql5_00_h[] = "@(#)$Id$";
X#endif	/* lint */
X#endif	/* MAIN_PROGRAM */
X
X#ifdef __cplusplus
X#define ESQLC_STRICT_PROTOTYPES
X#endif /* __cplusplus */
X
X#ifdef ESQLC_STRICT_PROTOTYPES
X/* A non-prototyped declaration for _iqnprep() is emitted by the compiler */
Xextern _SQCURSOR *_iqnprep(const char *name, char *stmt, short cs_sens);
X#else
Xextern _SQCURSOR *_iqnprep();
X#endif /* ESQLC_STRICT_PROTOTYPES */
X
X#ifdef ESQLC_SLOPPY_CONST
Xtypedef char **CCPCCPC;
X#else
X/* The code generator emits a dubious declaration for the cmdtxt parameter. */
X/* The CCPCCPC (const char pointer to const char pointer to const) typedef */
X/* isn't entirely accurate (because the called code isn't prototyped, much */
X/* less declared with const attributes, but the code in the called routines */
X/* honours the restrictions (it doesn't modify anything), so it does OK. */
Xtypedef const char *const *const CCPCCPC;
X#endif /* ESQLC_SLOPPY_CONST */
X
X#if ESQLC_VERSION == 500 || ESQLC_VERSION == 501
Xextern _SQCURSOR *_iqlocate_cursor(const char *name, int type, int cs, int xx);
X#else
Xextern _SQCURSOR *_iqlocate_cursor(const char *name, int type, int cs);
X#endif /* ESQLC_VERSION in {500, 501} */
X
Xextern int      _iqalloc(char *descname, int occurrence);
Xextern int      _iqbeginwork(void);
Xextern int      _iqcdcl(_SQCURSOR *cursor,
X                        char *curname,
X                        CCPCCPC cmdtxt,
X                        struct sqlda *idesc,
X                        struct sqlda *odesc,
X                        int flags);
Xextern int      _iqcddcl(_SQCURSOR *cursor,
X                         const char *curname,
X                         _SQCURSOR *stmt,
X                         int flags);
Xextern int      _iqcftch(_SQCURSOR *cursor,
X                         struct sqlda *idesc,
X                         struct sqlda *odesc,
X                         char *odesc_name,
X                         _FetchSpec *fetchspec);
Xextern int      _iqclose(_SQCURSOR *cursor);
Xextern int      _iqcommit(void);
Xextern int      _iqcopen(_SQCURSOR *cursor,
X                         int icnt,
X                         struct sqlvar_struct *ibind,
X                         struct sqlda *idesc,
X                         struct value *ivalues,
X                         int useflag);
Xextern int      _iqcput(_SQCURSOR *cursor,
X                        struct sqlda *idesc,
X                        char *desc_name);
Xextern int      _iqcrproc(char *fname);
Xextern int      _iqdbase(char *db_name, int exclusive);
Xextern int      _iqdbclose(void);
Xextern int      _iqdcopen(_SQCURSOR *cursor,
X                          struct sqlda *idesc,
X                          char *desc_name,
X                          char *ivalues,
X                          int useflag);
Xextern int      _iqdealloc(char *desc_name);
Xextern int      _iqdescribe(_SQCURSOR *cursor,
X                            struct sqlda **descp,
X                            char *desc_name);
Xextern int      _iqexecute(_SQCURSOR *cursor,
X                           struct sqlda *idesc,
X                           char *desc_name,
X                           struct value *ivalues);
Xextern int      _iqeximm(char *stmt);
Xextern int      _iqexproc(_SQCURSOR *cursor,
X                          CCPCCPC cmdtxt,
X                          int icnt,
X                          struct sqlvar_struct *ibind,
X                          int ocnt,
X                          struct sqlvar_struct *obind,
X                          int chkind);
Xextern int      _iqflush(_SQCURSOR *cursor);
Xextern int      _iqfree(_SQCURSOR *cursor);
Xextern int      _iqgetdesc(char *desc_name,
X                           int sqlvar_num,
X                           struct hostvar_struct *hosttab,
X                           int xopen_flg);
Xextern int      _iqprepare(_SQCURSOR *cursor, char *stmt);
Xextern int      _iqrollback(void);
Xextern int      _iqsetdesc(char *desc_name,
X                           int sqlvar_num,
X                           struct hostvar_struct *hosttab,
X                           int xopen_flg);
Xextern int      _iqslct(_SQCURSOR *cursor,
X                        CCPCCPC cmdtxt,
X                        int icnt,
X                        struct sqlvar_struct *ibind,
X                        int ocnt,
X                        struct sqlvar_struct *obind,
X                        int chkind);
Xextern int      _iqstmnt(_SQSTMT *scb,
X                         CCPCCPC cmdtxt,
X                         int icnt,
X                         struct sqlvar_struct *ibind,
X                         struct value *ivalues);
Xextern void     iec_stop(void);
Xextern int      sqgetdbs(int *ret_fcnt,
X                         char **fnames,
X                         int fnsize,
X                         char *farea,
X                         int fasize);
Xextern int      sqlbreak(void);
Xextern void     sqldetach(void);
Xextern void     sqlexit(void);
Xextern int      sqlstart(void);
X
X#ifdef __cplusplus
X}
X#endif /* __cplusplus */
X
X#endif	/* ESQL5_00_H */
SHAR-EOF
chmod 444 esql5_00.h
if [ `wc -c <esql5_00.h` -ne 6135 ]
then echo shar: esql5_00.h unpacked with wrong size
fi
# end of overwriting check
fi
#--------------------
if [ -f esql7_20.h -a "$1" != "-c" ]
then echo shar: esql7_20.h already exists
else
echo 'x - esql7_20.h (1320 characters)'
sed -e 's/^X//' >esql7_20.h <<'SHAR-EOF'
X/*
X@(#)File:            $RCSfile$
X@(#)Version:         $Revision$
X@(#)Last changed:    $Date$
X@(#)Purpose:         Function prototypes for ESQL/C Versions 7.20..7.22
X@(#)Author:          J Leffler
X@(#)Copyright:       (C) JLSS 1997
X@(#)Product:         :PRODUCT:
X*/
X
X/*TABSTOP=4*/
X
X#ifndef ESQL7_20_H
X#define ESQL7_20_H
X
X#ifdef MAIN_PROGRAM
X#ifndef lint
Xstatic const char esql7_20_h[] = "@(#)$Id$";
X#endif	/* lint */
X#endif	/* MAIN_PROGRAM */
X
X/*
X** The 7.2x ESQL/C compiler can generate calls to the following functions
X** but sqlhdr.h does not define prototypes for these functions.  Although
X** byfill() is declared in esqllib.h, this is not normally needed by 7.x
X** ESQL/C compilations (though if byfill() is missing, there is room to
X** think that other functions may be missing too).
X*/
Xextern void  byfill(char *to, int len, char ch);
Xextern void  iec_dclcur(char *, char **, int, int, int);
Xextern void  iec_free(char *);
Xextern void  iec_hostbind(struct hostvar_struct *, int, int, int, int, char *);
Xextern void  iec_ibind(int, char *, int, int, char *, int);
Xextern void  iec_obind(int, char *, int, int, char *, int);
Xextern void *iec_alloc_isqlda(int);
Xextern void *iec_alloc_osqlda(int);
X
X#endif	/* ESQL7_20_H */
SHAR-EOF
chmod 444 esql7_20.h
if [ `wc -c <esql7_20.h` -ne 1320 ]
then echo shar: esql7_20.h unpacked with wrong size
fi
# end of overwriting check
fi
#--------------------
if [ -f esqlc.h -a "$1" != "-c" ]
then echo shar: esqlc.h already exists
else
echo 'x - esqlc.h (5413 characters)'
sed -e 's/^X//' >esqlc.h <<'SHAR-EOF'
X/*
X@(#)File:            $RCSfile$
X@(#)Version:         $Revision$
X@(#)Last changed:    $Date$
X@(#)Purpose:         Include all relevant ESQL/C type definitions
X@(#)Author:          J Leffler
X@(#)Copyright:       (C) JLSS 1992-93,1995-98
X@(#)Product:         :PRODUCT:
X*/
X
X#ifndef ESQLC_H
X#define ESQLC_H
X
X#ifdef MAIN_PROGRAM
X#ifndef lint
Xstatic const char esqlc_h[] = "@(#)$Id$";
X#endif	/* lint */
X#endif	/* MAIN_PROGRAM */
X
X/* If ESQLC_VERSION isn't defined, use version 0 */
X#ifndef ESQLC_VERSION
X#define ESQLC_VERSION 0
X#endif /* ESQLC_VERSION */
X
X/*
X** On DEC OSF/1 and 64-bits machines, __STDC__ is not necessarily defined,
X** but the use of prototypes is necessary under optimization to ensure that
X** pointers are treated correctly (sizeof(void *) != sizeof(int)).
X** The <sqlhdr.h> prototypes for version 6.00 and above are only active if
X** __STDC__ is defined (whether 1 or 0 or something else does not matter).
X** Ensure that the compilation options set __STDC__.
X*/
X
X#ifdef __cplusplus
Xextern "C" {
X#endif /* __cplusplus */
X
X/* -- Include Files	*/
X
X#include <datetime.h>
X#include <decimal.h>
X#include <locator.h>
X#include <sqlca.h>
X#include <sqlda.h>
X#include <sqlstype.h>
X#include <sqltypes.h>
X
X#if ESQLC_VERSION >= 400
X#include <varchar.h>
X#endif /* ESQLC_VERSION >= 400 */
X
X/* _WIN32 (Windows 95/NT code from Harald Ums <Harald.Ums@sevensys.de> */
X
X#if ESQLC_VERSION < 400
X/* No prototypes available -- for earlier versions, you are on your own! */
X#elif ESQLC_VERSION < 410
X#include "esql4_00.h"
X#include "esqllib.h"
X#elif ESQLC_VERSION < 500
X#include "esql4_10.h"
X#include "esqllib.h"
X#elif ESQLC_VERSION < 600
X#ifdef _WIN32
X#include <windows.h>
X#include <sqlhdr.h>
X#include <sqlproto.h>
X#else
X#include "esql5_00.h"
X#include "esqllib.h"
X#endif /* _WIN32 */
X#else
X/* For later versions, sqlhdr.h contains the requisite declarations. */
X/* However, these declarations are protected by __STDC__ so you need */
X/* to ensure that your compiler has it defined.  Note that compilers */
X/* on some machines do complain if you try to define __STDC__.       */
X#include <sqlhdr.h>
X
X/*
X** ClientSDK 2.01 or later needs sqliapi.h; it is unknown whether
X** ClientSDK 2.00 does too.  It is also unknown which version is
X** reported by the ESQL/C compiler in ClientSDK 2.00 (with 2.01,
X** the ESQL/C compiler reports 9.14).
X*/
X#if ESQLC_VERSION >= 730 && ESQLC_VERSION < 800
X/* Probably an inaccurate value for ESQLC_VERSION -- should be 914 or greater */
X#include <sqliapi.h>
X#elif ESQLC_VERSION >= 914
X#include <sqliapi.h>
X#endif
X
X#ifdef _WIN32
X#include <sqlproto.h>
X#else
X#if ESQLC_VERSION >= 720 && ESQLC_VERSION < 730
X#include "esql7_20.h"
X#endif /* ESQLC_VERSION is 7.2x */
X
Xextern int      sqgetdbs(int *ret_fcnt,
X                         char **fnames,
X                         int fnsize,
X                         char *farea,
X                         int fasize);
X#endif /* _WIN32 */
X
X#endif /* ESQLC_VERSION */
X
X#if (ESQLC_VERSION >= 500 && ESQLC_VERSION < 600) || \
X	(ESQLC_VERSION >= 710 && ESQLC_VERSION < 720)
X/*
X** Some code, notably esqlutil.h, relies on the typedef for value_t.
X** However, value.h is not included by sqlhdr.h on 7.1x platforms.  The
X** problem was found by David Edge <dedge@ak.blm.gov> in 7.10.UC1 on AIX
X** 4.2.1; it was subsequently also revealed on Solaris 2.6 with ESQL/C
X** 5.08.  The symbol MAXADDR is defined in value.h.  The 5.08 version of
X** value.h does not prevent multiple includes, leading to problems.  This
X** test is not perfect; if code after #include "esqlc.h" includes value.h
X** explicitly, it will not compile under 5.0x ESQL/C.
X*/
X#ifndef MAXADDR
X#include <value.h>
X#endif /* MAXADDR */
X#endif /* 5.xx or 7.1x */
X
X/*
X** Supply missing type information for IUS (IDS/UDO) data types.
X** Two edged sword; it means you have to test rather carefully in
X** your code whether to build with IUS data types or not.
X** Should be keyed off ESQLC_VERSION, rather than features...
X*/
X#if ESQLC_VERSION < 900
X#include "esql_ius.h"
X#endif
X
X/* -- Constant Definitions */
X
X/* A table name may be: database@server:"owner".table */
X/* This contains 5 punctuation characters and a null */
X/*
X** Note that from 9.2 up (and maybe 7.3 up and maybe from 8.3
X** up), identifier names can be much longer -- up to 128 bytes
X** each -- and user names can be up to 32 characters.
X** Prior versions only allowed 18 characters for table, column,
X** database and server names, and only 8 characters for user
X** identifiers.
X*/
X#if (ESQLC_VERSION >= 730 && ESQLC_VERSION < 800) || \
X	(ESQLC_VERSION >= 830 && ESQLC_VERSION < 900) || \
X	(ESQLC_VERSION >= 920)
X#define SQL_NAMELEN	128
X#define SQL_USERLEN	32
X#else
X#define SQL_NAMELEN	18
X#define SQL_USERLEN	8
X#endif
X#define SQL_TABNAMELEN	(3 * SQL_NAMELEN + SQL_USERLEN + sizeof("@:''."))
X#define SQL_COLNAMELEN	(SQL_NAMELEN + 1)
X
X#define loc_mode	lc_union.lc_file.lc_mode
X#define sqlva		sqlvar_struct
X
X/* -- Type Definitions */
X
Xtypedef loc_t	        Blob;
Xtypedef struct decimal	Decimal;
Xtypedef struct dtime	Datetime;
Xtypedef struct intrvl	Interval;
Xtypedef struct sqlca_s	Sqlca;
Xtypedef struct sqlda	Sqlda;
Xtypedef struct sqlva	Sqlva;
X
X#if ESQLC_VERSION >= 900
X
X/* Type for casting dynamic SQL types to LVARCHAR */
Xtypedef void *Lvarchar;
X
X#endif /* ESQLC_VERSION >= 900 */
X
X#ifdef __cplusplus
X}
X#endif /* __cplusplus */
X
X#endif	/* ESQLC_H */
SHAR-EOF
chmod 440 esqlc.h
if [ `wc -c <esqlc.h` -ne 5413 ]
then echo shar: esqlc.h unpacked with wrong size
fi
# end of overwriting check
fi
#--------------------
if [ -f esqllib.h -a "$1" != "-c" ]
then echo shar: esqllib.h already exists
else
echo 'x - esqllib.h (3788 characters)'
sed -e 's/^X//' >esqllib.h <<'SHAR-EOF'
X/*
X@(#)File:            $RCSfile$
X@(#)Version:         $Revision$
X@(#)Last changed:    $Date$
X@(#)Purpose:         ESQL/C Library Function Prototypes
X@(#)Author:          J Leffler
X@(#)Copyright:       (C) JLSS 1992-93,1995-97
X@(#)Product:         :PRODUCT:
X*/
X
X/*TABSTOP=4*/
X
X#ifndef ESQLLIB_H
X#define ESQLLIB_H
X
X#ifdef MAIN_PROGRAM
X#ifndef lint
Xstatic const char esqllib_h[] = "@(#)$Id$";
X#endif	/* lint */
X#endif	/* MAIN_PROGRAM */
X
X#ifdef __cplusplus
Xextern "C" {
X#endif /* __cplusplus */
X
X/* DATE */
Xextern int      rdayofweek(long jdate);
Xextern int      rdefmtdate(long *jdate, char *fmt, char *inp);
Xextern int      rfmtdate(long jdate, char *fmt, char *res);
Xextern int      rjulmdy(long jdate, short mdy[3]);
Xextern int      rleapyear(int year);
Xextern int      rmdyjul(short mdy[3], long *jdate);
Xextern int      rstrdate(char *str, long *jdate);
Xextern int      rdatestr(long jdate, char *str);
Xextern void     rtoday(long *today);
X
X/* DECIMAL */
Xextern int      decadd(dec_t *op1, dec_t *op2, dec_t *res);
Xextern int      deccmp(dec_t *op1, dec_t *op2);
Xextern int      deccpy(dec_t *op1, dec_t *op2);
Xextern int      deccvasc(char *cp, int len, dec_t *np);
Xextern int      deccvdbl(double i, dec_t *np);
Xextern int      deccvint(int i, dec_t *np);
Xextern int      deccvlong(long i, dec_t *np);
Xextern int      decdiv(dec_t *op1, dec_t *op2, dec_t *res);
Xextern char    *dececvt(dec_t *np, int ndigit, int *decpt, int *sign);
Xextern char    *decfcvt(dec_t *np, int ndigit, int *decpt, int *sign);
Xextern int      decmul(dec_t *op1, dec_t *op2, dec_t *res);
Xextern int      decround(dec_t *np, int n);
Xextern int      decsub(dec_t *op1, dec_t *op2, dec_t *res);
Xextern int      dectoasc(dec_t *np, char *cp, int len, int right);
Xextern int      dectodbl(dec_t *np, double *ip);
Xextern int      dectoint(dec_t *np, int *ip);
Xextern int      dectolong(dec_t *np, long *ip);
Xextern int      dectrunc(dec_t *np, int n);
X
X/* FORMAT USING */
Xextern int      rfmtdec(dec_t *np, char *fmt, char *outbuf);
Xextern int      rfmtdouble(double np, char *fmt, char *outbuf);
Xextern int      rfmtlong(long np, char *fmt, char *outbuf);
X
X/* DATETIME/INTERVAL */
Xextern int      dtcvasc(char *str, dtime_t *dt);
Xextern int      dtcvfmtasc(char *str, char *fmt, dtime_t *dt);
Xextern int      dtextend(dtime_t *id, dtime_t *od);
Xextern int      dttoasc(dtime_t *dt, char *str);
Xextern int      incvasc(char *str, intrvl_t *dt);
Xextern int      incvfmtasc(char *str, char *fmt, intrvl_t *dt);
Xextern int      intoasc(intrvl_t *dt, char *str);
Xextern void     dtcurrent(dtime_t *dt);
X
X/* LIBRARY */
Xextern char    *rtypname(int sqltype);
Xextern int      bycmpr(char *b1, char *b2, int len);
Xextern int      byleng(char *fr, int len);
Xextern int      rgetmsg(short msgnum, char *msgstr, short msglen);
Xextern int      risnull(int type, char *ptrvar);
Xextern int      rsetnull(int type, char *ptrvar);
Xextern int      rstod(char *str, double *val);
Xextern int      rstoi(char *str, int *val);
Xextern int      rstol(char *str, long *val);
Xextern int      rtypalign(int pos, int type);
Xextern int      rtypmsize(int sqltype, int sqllen);
Xextern int      rtypwidth(int sqltype, int sqllen);
Xextern int      stcmpr(char *s1, char *s2);
Xextern int      stcopy(char *fr, char *to);
Xextern int      stleng(char *s);
Xextern void     bycopy(char *fr, char *to, int len);
Xextern void     byfill(char *to, int len, char ch);
Xextern void     ldchar(char *fr, int len, char *to);
Xextern void     rdownshift(char *s);
Xextern void     rupshift(char *s);
Xextern void     stcat(char *s, char *dest);
Xextern void     stchar(char *fr, char *to, int cnt);
X
X#ifdef __cplusplus
X}
X#endif /* __cplusplus */
X
X#endif	/* ESQLLIB_H */
SHAR-EOF
chmod 444 esqllib.h
if [ `wc -c <esqllib.h` -ne 3788 ]
then echo shar: esqllib.h unpacked with wrong size
fi
# end of overwriting check
fi
#--------------------
if [ -f esql++.sh -a "$1" != "-c" ]
then echo shar: esql++.sh already exists
else
echo 'x - esql++.sh (5630 characters)'
sed -e 's/^X//' >esql++.sh <<'SHAR-EOF'
X:	"@(#)$Id$"
X#
X#   ESQL/C++ compiler script
X#   For Informix-ESQL/C Versions 4.00, 4.1x, 5.0x, 6.0x, 7.1x or later
X
X#TABSTOP=4
X
X#################################################################
X# Prerequisite (mandatory) change to installed esql script for
X# Version 4.00, 4.1x, 5.0x ESQL/C compiler scripts.
X# diff old.esql new.esql
X# < CC="cc"
X# --
X# > CC=${INFORMIXC:-"cc"}
X# Note that this is not sufficient if you are also using the -g,
X# -G or -nln options.  You then need to find the lines which read
X# CC="cc -g" and change them to CC="${CC} -g", or some equivalent
X# but more complex change.  The '-G' option is a confounded
X# nuisance if you are trying to create shared libraries on
X# SVR4-base Unix.
X#################################################################
X# Additional prerequisite for all versions of ESQL/C:
X# -- The esql*.h headers (esqlc.h, esqllib.h, esql7_20.h, esql5_00.h,
X#    esql4_10.h, esql4_00.h) must be installed in:
X#    * $INFORMIXDIR/incl/esql -- Version 5.00 or later
X#    * $INFORMIXDIR/incl      -- Version 4.1x or 4.00
X#################################################################
X
X: ${ESQL:=esql}
Xif [ -z "$ESQLC_VERSION" ]
Xthen
X	ESQLC_VERSION=`$ESQL -V |
X			sed -n 's/.*Version \([0-9][0-9]*\)\.\([0-9][0-9]\).*/\1\2/p'`
X	export ESQLC_VERSION
Xfi
X
X# Some C++ compilers are strict and insist on .C as the extension for
X# C++ source files; others are lax and accept .C, .c, .cc or .cpp as
X# the extension.  Set ESQLC_CPLUSPLUS_EXTENSION to suit your C++ compiler
X# and local conventions.  The extension generated by the ESQL/C compiler
X# is fixed as '.c' in all current versions of ESQL/C, but this is documented
X# in the variable $G.  This script removes the generated file after editing
X# it into the appropriate C++ source file.
X# NB: if ESQL_CPLUSPLUS_EXTENSION is also '.c', the temporary file is needed.
XX=${ESQLC_CPLUSPLUS_EXTENSION:-'.cc'}
XG='.c'
X
X# Primarily for testing!  Define as ':' to leave C code file around.
X: ${ESQLC_RM_COMMAND:="rm -f"}
X
X# The 7.24 and 9.14 ESQL/C scripts use INFORMIXCPP to specify the C++
X# compiler.  This script unconditionally compiles with the C++ compiler.
XINFORMIXC=${INFORMIXCPP:-${INFORMIXC:-CC}}
Xexport INFORMIXC
X
Xif [ "$X" = "$G" ]
Xthen
X	tmpfile=${TMPDIR:-/tmp}/esql.$$
X	trap "rm -f $tmpfile; exit 1" 0 1 2 3 13 15
Xfi
X
Xesqlc_version="-DESQLC_VERSION=${ESQLC_VERSION}"
Xestat=0
Xarglist=""
Xwhile [ $# -gt 0 ]
Xdo
X	arg=$1
X	case "$arg" in
X	-DESQLC_VERSION=*)
X		esqlc_version=""
X		ESQLC_VERSION=`expr $arg : '-DESQLC_VERSION=\(.*\)'`
X		arglist="$arglist '$arg'"
X		;;
X	*.ec|*.ecpp)
X		case "$arg" in
X		*.ec)	x=ec;;
X		*.ecpp)	x=ecpp;;
X		esac
X		bname=`basename $arg .$x`
X		oname=$bname$X
X		gname=$bname$G
X		if [ "$X" = "$G" ]
X		then outfile=$tmpfile
X		else outfile=$oname
X		fi
X		# Do not use .ecpp extension unless underlying esql script recognizes it!
X		if eval $ESQL $arglist -e $arg && [ -f $gname ]
X		then : OK
X		else estat=1
X		fi
X		# NB: Version 8.xx has never been tested!
X		case $ESQLC_VERSION in
X		[4-7][0-9][0-9])
X			# NB: Version 8.xx has never been tested!
X			# NB: in 4.1x ESQL/C compilers generate code which
X			# contains '(char *)0' where some structure pointer
X			# is required.  In the fully- prototyped C++
X			# environment, leaving all these as just 0 fixes
X			# the problem.
X			# NB: in 5.0x ESQL/C, the calls to _iq functions are
X			# mostly clean, unless you are using SQL descriptors
X			# instead of sqlda structures.  The delete operations
X			# remove the generated, non-prototyped declaration of
X			# _iqnprep().  We also have to remove the nonsense:
X			#
X			#if !defined(__STDC__)
X			#define const
X			#endif
X			#
X			# To do this, exploit the fact that the 4.1x through 7.24
X			# all product a #line 1 directive.  The #line needs to be
X			# printed, as do the cursor and statement name declarations.
X			# The other lines do not need to be printed.
X			{
X			echo '#include "esqlc.h"'
X			sed -e '1,/^#line 1 "/{' \
X				-e '/^static _SQCURSOR /p' \
X				-e '/^static const char/p' \
X				-e '/^#line 1 "/p' \
X				-e 'd' \
X				-e '}' \
X				-e '/_iq[a-z]/s/(char \*) *0/0/g' $gname
X			} > $outfile
X			[ "$X" = "$G" ] && mv $outfile $oname
X			;;
X		9[0-9][0-9])
X			# It appears that 9.10.UC2 ESQL/C includes just <sqlhdr.h> and
X			# that it, in turn, includes almost all the headers that
X			# esqlc.h includes too.  It does not separately include
X			# sqlca.h, nor does it declare _iqnprep().  This makes the
X			# echo/sed script below redundant.  Earlier 9.0x versions may
X			# not be as clean, in which case the script below should be
X			# used instead.  With luck, the edit for _iq[a-z] should be
X			# unnecessary in all 9.0x versions.  The copy of 9.03.UC1
X			# ESQL/C which I was using got removed, unfortunately.
X			#{
X			#echo '#include "esqlc.h"'
X			#sed -e '1d' $gname
X			#} > $outfile
X			#[ "$X" = "$G" ] && mv $outfile $oname
X			[ "$X" = "$G" ] || mv $gname $oname
X			;;
X		*)
X			arg0=`basename $0 .sh`
X			echo "$arg0: unknown ESQLC_VERSION = $ESQLC_VERSION" 1>&2
X			exit 1;;
X		esac
X		${ESQLC_RM_COMMAND} $gname
X		arglist="$arglist $oname"
X		;;
X	*.o | *.c | *.cc | *.cpp | *.C)
X		arglist="$arglist $arg"
X		;;
X	-o)
X		if [ $# -lt 1 ]
X		then error "-o option requires argument"
X		fi
X		shift
X		output="-o $1"
X		;;
X	-V)
X		echo "$0: " 'ESQL++ Version $Revision$ ($Date$)' |
X			sed -e 's/\$[A-Z][a-z]*: \([^\$]*\) \$/\1/g'
X		exec $ESQL $arg
X		;;
X	*)
X		arglist="$arglist '$arg'"
X		;;
X	esac
X	shift
Xdone
X
Xif [ "$X" = "$G" ]
Xthen
X	rm -f $tmpfile
X	trap 0 1 2 3 13 15
Xfi
X
Xif [ $estat != 0 ]
Xthen exit 1
Xfi
X
Xeval exec $ESQL $esqlc_version $arglist $output
SHAR-EOF
chmod 444 esql++.sh
if [ `wc -c <esql++.sh` -ne 5630 ]
then echo shar: esql++.sh unpacked with wrong size
fi
# end of overwriting check
fi
#--------------------
if [ -f esql.v4.patch -a "$1" != "-c" ]
then echo shar: esql.v4.patch already exists
else
echo 'x - esql.v4.patch (1162 characters)'
sed -e 's/^X//' >esql.v4.patch <<'SHAR-EOF'
X#	@(#)$Id$
X#
X# To apply this patch (assuming that this file is called $RCSfile$,
X# and that the current directory is distinct from $INFORMIXDIR/bin):
X#	cp $INFORMIXDIR/bin/esql .
X#	patch < $RCSfile$
X
X*** esql	Fri Jan 24 12:34:17 1997
X--- esql.v4.new	Fri Jan 24 12:35:27 1997
X***************
X*** 25,31 ****
X  : '
X  INFDIR=${INFORMIXDIR=/usr/informix}
X  PREPCC=${INFDIR}/lib/esqlc
X! CC="cc"
X  STAT=0
X  TYPE=unix
X  ANSI=
X--- 25,31 ----
X  : '
X  INFDIR=${INFORMIXDIR=/usr/informix}
X  PREPCC=${INFDIR}/lib/esqlc
X! CC="${INFORMIXC:-cc}"
X  STAT=0
X  TYPE=unix
X  ANSI=
X***************
X*** 94,110 ****
X  	    shift
X  	    ;;
X  	-g )
X! 	    CC="cc -g"
X  	    PA="$PA $1"
X  	    shift
X  	    ;;
X  	-G )
X! 	    CC="cc -g"
X  	    PA="$PA $1"
X  	    shift
X  	    ;;
X  	-nln )
X! 	    CC="cc -g"
X  	    PA="$PA $1"
X  	    shift
X  	    ;;
X--- 94,110 ----
X  	    shift
X  	    ;;
X  	-g )
X! 	    CC="${CC} -g"
X  	    PA="$PA $1"
X  	    shift
X  	    ;;
X  	-G )
X! 	    CC="${CC} -g"
X  	    PA="$PA $1"
X  	    shift
X  	    ;;
X  	-nln )
X! 	    CC="${CC} -g"
X  	    PA="$PA $1"
X  	    shift
X  	    ;;
SHAR-EOF
chmod 444 esql.v4.patch
if [ `wc -c <esql.v4.patch` -ne 1162 ]
then echo shar: esql.v4.patch unpacked with wrong size
fi
# end of overwriting check
fi
#--------------------
if [ -f esql.v5.patch -a "$1" != "-c" ]
then echo shar: esql.v5.patch already exists
else
echo 'x - esql.v5.patch (1168 characters)'
sed -e 's/^X//' >esql.v5.patch <<'SHAR-EOF'
X#	@(#)$Id$
X#
X# To apply this patch (assuming that this file is called $RCSfile$,
X# and that the current directory is distinct from $INFORMIXDIR/bin):
X#	cp $INFORMIXDIR/bin/esql .
X#	patch < $RCSfile$
X
X*** esql	Fri Jan 24 12:34:52 1997
X--- esql.v5.new	Fri Jan 24 12:35:47 1997
X***************
X*** 25,31 ****
X  : '
X  INFDIR=${INFORMIXDIR=/usr/informix}
X  PREPCC=${INFDIR}/lib/esql/esqlc
X! CC="cc"
X  STAT=0
X  TYPE=unix
X  A=
X--- 25,31 ----
X  : '
X  INFDIR=${INFORMIXDIR=/usr/informix}
X  PREPCC=${INFDIR}/lib/esql/esqlc
X! CC="${INFORMIXC:-cc}"
X  STAT=0
X  TYPE=unix
X  A=
X***************
X*** 100,116 ****
X  	    shift
X  	    ;;
X  	-g )
X! 	    CC="cc -g"
X  	    PA="$PA $1"
X  	    shift
X  	    ;;
X  	-G )
X! 	    CC="cc -g"
X  	    PA="$PA $1"
X  	    shift
X  	    ;;
X  	-nln )
X! 	    CC="cc -g"
X  	    PA="$PA $1"
X  	    shift
X  	    ;;
X--- 100,116 ----
X  	    shift
X  	    ;;
X  	-g )
X! 	    CC="${CC} -g"
X  	    PA="$PA $1"
X  	    shift
X  	    ;;
X  	-G )
X! 	    CC="${CC} -g"
X  	    PA="$PA $1"
X  	    shift
X  	    ;;
X  	-nln )
X! 	    CC="${CC} -g"
X  	    PA="$PA $1"
X  	    shift
X  	    ;;
SHAR-EOF
chmod 444 esql.v5.patch
if [ `wc -c <esql.v5.patch` -ne 1168 ]
then echo shar: esql.v5.patch unpacked with wrong size
fi
# end of overwriting check
fi
#--------------------
if [ -f Makefile -a "$1" != "-c" ]
then echo shar: Makefile already exists
else
echo 'x - Makefile (850 characters)'
sed -e 's/^X//' >Makefile <<'SHAR-EOF'
X#	@(#)$Id$
X#
X#	Makefile for ESQL++ Test Application
X
XCPLUSPLUS = g++
XCPP_FLAGS = # -DISO_HEADERS
XCPP_OBJ   = ${CPP_SRC:.cc=.o}
XCPP_SRC   = testcpp.cc
XC_OBJ     = ${C_SRC:.c=.o}
XC_SRC     = testc.c
XDEBRIS    = a.out core *.rej *.orig
XESQL      = INFORMIXC="${CPLUSPLUS}" ${ESQL_CMD}
XESQLJUNK  = ${ESQLSRC:.ec=.c} ${ESQLSRC:.ec=.cc}
XESQLOBJ   = ${ESQLSRC:.ec=.o}
XESQLSRC   = testmain.ec testsub.ec
XESQL_CMD  = esql++
XRM        = rm -f
XTESTAPP   = testapp
X
X.SUFFIXES:
X.SUFFIXES: .ec .cc .c .o
X
X.cc.o:
X	${CC} -c $*.c
X
X.cc.o:
X	${CPLUSPLUS} ${CPP_FLAGS} -c $*.cc
X
X.ec.o:
X	${ESQL} ${CPP_FLAGS} -c $*.ec
X	${RM} $*.c $*.cc
X
Xall: ${TESTAPP}
X
X${TESTAPP}:	${ESQLOBJ} ${CPP_OBJ} ${C_OBJ}
X	${ESQL} -o $@ ${ESQLOBJ} ${CPP_OBJ} ${C_OBJ}
X
Xclean:
X	${RM} ${TESTAPP} ${ESQLOBJ} ${ESQLJUNK} ${DEBRIS} ${CPP_OBJ} ${C_OBJ}
SHAR-EOF
chmod 444 Makefile
if [ `wc -c <Makefile` -ne 850 ]
then echo shar: Makefile unpacked with wrong size
fi
# end of overwriting check
fi
#--------------------
if [ -f testapp.h -a "$1" != "-c" ]
then echo shar: testapp.h already exists
else
echo 'x - testapp.h (532 characters)'
sed -e 's/^X//' >testapp.h <<'SHAR-EOF'
X/*
X@(#)File:            $RCSfile$
X@(#)Version:         $Revision$
X@(#)Last changed:    $Date$
X@(#)Purpose:         Header for ESQL++ Test Application
X@(#)Author:          J Leffler
X@(#)Copyright:       (C) JLSS 1997-98
X@(#)Product:         :PRODUCT:
X*/
X
X/*TABSTOP=4*/
X
X#ifndef TESTAPP_H
X#define TESTAPP_H
X
Xextern void const_tester(const char *);
Xextern void sqlerror(void);
Xextern int sql_select(char *);
X
X#ifdef __cplusplus
Xextern "C"
X#endif
Xint pure_c_function();
X
X#endif	/* TESTAPP_H */
SHAR-EOF
chmod 444 testapp.h
if [ `wc -c <testapp.h` -ne 532 ]
then echo shar: testapp.h unpacked with wrong size
fi
# end of overwriting check
fi
#--------------------
if [ -f testc.c -a "$1" != "-c" ]
then echo shar: testc.c already exists
else
echo 'x - testc.c (1027 characters)'
sed -e 's/^X//' >testc.c <<'SHAR-EOF'
X/*
X@(#)File:            $RCSfile$
X@(#)Version:         $Revision$
X@(#)Last changed:    $Date$
X@(#)Purpose:         Simple test application for ESQL++
X@(#)Author:          J Leffler
X@(#)Copyright:       (C) JLSS 1998
X@(#)Product:         :PRODUCT:
X*/
X
X/*TABSTOP=4*/
X
X#include <stdlib.h>
X
X/*
X** Do not include testapp.h so that if a C++ compiler is used to compile
X** this file, it will fail to compile (because the function is not declared
X** before it is defined), and would fail to link if it was compiled because
X** the name would be mangled by the C++ compiler and the linker would not
X** find the function defined.  Note the GCC set fussy will complain about
X** the lack of a prior declaration and the absence of a prototype (the
X** keyword void would appear in the parentheses if it was an ANSI C
X** prototyped function).
X*/
X
X#ifndef lint
Xstatic const char rcs[] = "@(#)$Id$";
X#endif
X
Xint pure_c_function()
X{
X	return EXIT_SUCCESS;
X}
SHAR-EOF
chmod 444 testc.c
if [ `wc -c <testc.c` -ne 1027 ]
then echo shar: testc.c unpacked with wrong size
fi
# end of overwriting check
fi
#--------------------
if [ -f testcpp.cc -a "$1" != "-c" ]
then echo shar: testcpp.cc already exists
else
echo 'x - testcpp.cc (636 characters)'
sed -e 's/^X//' >testcpp.cc <<'SHAR-EOF'
X/*
X@(#)File:            $RCSfile$
X@(#)Version:         $Revision$
X@(#)Last changed:    $Date$
X@(#)Purpose:         Simple test application for ESQL++
X@(#)Author:          J Leffler
X@(#)Copyright:       (C) JLSS 1997,1999
X@(#)Product:         :PRODUCT:
X*/
X
X/*TABSTOP=4*/
X
X#ifdef ISO14882_HEADERS
X#include <iostream>
X#else
X#include <iostream.h>
X#endif /* ISO14882_HEADERS */
X
X#include "testapp.h"
X
X#ifndef lint
Xstatic const char rcs[] = "@(#)$Id$";
X#endif
X
Xvoid const_tester(const char *arg)
X{
X	cout << "SQL statement: <<" << arg << ">>\n";
X}
SHAR-EOF
chmod 444 testcpp.cc
if [ `wc -c <testcpp.cc` -ne 636 ]
then echo shar: testcpp.cc unpacked with wrong size
fi
# end of overwriting check
fi
#--------------------
if [ -f testmain.ec -a "$1" != "-c" ]
then echo shar: testmain.ec already exists
else
echo 'x - testmain.ec (921 characters)'
sed -e 's/^X//' >testmain.ec <<'SHAR-EOF'
X/*
X@(#)File:            $RCSfile$
X@(#)Version:         $Revision$
X@(#)Last changed:    $Date$
X@(#)Purpose:         Simple test application for ESQL++
X@(#)Author:          J Leffler
X@(#)Copyright:       (C) JLSS 1997-99
X@(#)Product:         :PRODUCT:
X*/
X
X/*TABSTOP=4*/
X
X#ifdef ISO14882_HEADERS
X#include <iostream>
X#else
X#include <iostream.h>
X#endif /* ISO14882_HEADERS */
X
X#include "testapp.h"
X
X#ifndef lint
Xstatic const char rcs[] = "@(#)$Id$";
X#endif
X
Xint main()
X{
X
X	cout << "ESQL/C++ Test Application -- compiled with ESQL/C Version ";
X	cout << ESQLC_VERSION << endl;
X	EXEC SQL WHENEVER ERROR CALL sqlerror;
X	EXEC SQL DATABASE stores;
X
X	int tabid = sql_select("SELECT Tabid FROM Systables"
X							" WHERE TabName = 'syscolumns'");
X
X	cout << "TabID of SysColumns table is " << tabid << endl;
X
X	return(pure_c_function());
X}
SHAR-EOF
chmod 444 testmain.ec
if [ `wc -c <testmain.ec` -ne 921 ]
then echo shar: testmain.ec unpacked with wrong size
fi
# end of overwriting check
fi
#--------------------
if [ -f testsub.ec -a "$1" != "-c" ]
then echo shar: testsub.ec already exists
else
echo 'x - testsub.ec (1211 characters)'
sed -e 's/^X//' >testsub.ec <<'SHAR-EOF'
X/*
X@(#)File:            $RCSfile$
X@(#)Version:         $Revision$
X@(#)Last changed:    $Date$
X@(#)Purpose:         Simple test application for ESQL++
X@(#)Author:          J Leffler
X@(#)Copyright:       (C) JLSS 1997,1999
X@(#)Product:         :PRODUCT:
X*/
X
X/*TABSTOP=4*/
X
X#ifdef ISO14882_HEADERS
X#include <iostream>
X#include <cstdlib>
X#else
X#include <iostream.h>
X#include <stdlib.h>
X#endif /* ISO14882_HEADERS */
X
X#include "testapp.h"
X
X#ifndef lint
Xstatic const char rcs[] = "@(#)$Id$";
X#endif
X
Xvoid sqlerror(void)
X{
X	cerr << "SQL = " << sqlca.sqlcode;
X	if (sqlca.sqlerrd[1] != 0)
X		cerr << ", ISAM = " << sqlca.sqlerrd[1];
X	cerr << endl;
X	exit(1);
X}
X
Xint sql_select(char *txt)
X{
X	EXEC SQL BEGIN DECLARE SECTION;
X	char *stmt = txt;
X	int	retval;
X	EXEC SQL END DECLARE SECTION;
X
X	const_tester(txt);
X	EXEC SQL WHENEVER ERROR CALL sqlerror;
X	EXEC SQL PREPARE p_select FROM :stmt;
X	EXEC SQL DECLARE c_select CURSOR FOR p_select;
X	EXEC SQL OPEN c_select;
X	EXEC SQL FETCH c_select INTO :retval;
X	EXEC SQL CLOSE c_select;
X	EXEC SQL FREE c_select;
X
X#if ESQLC_VERSION >= 600
X	EXEC SQL FREE p_select;
X#endif
X
X	return(retval);
X}
SHAR-EOF
chmod 444 testsub.ec
if [ `wc -c <testsub.ec` -ne 1211 ]
then echo shar: testsub.ec unpacked with wrong size
fi
# end of overwriting check
fi
echo All files extracted
exit 0
