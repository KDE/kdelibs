/* This file is part of the KDE project
   Copyright (C) 2003-2005 Jaroslaw Staniek <js@iidea.pl>

   Global definitions for KDElibs/win32 (win32)

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifdef __cplusplus 
# include <qglobal.h>
#endif

/*! Bootstrap */
#define kdemain \
kde_start(int argc, char **argv); \
KDE_IMPORT void kde_bootstrap(); \
KDE_IMPORT void kde_destroy(); \
int main(int argc, char **argv) \
{ kde_bootstrap(); const int retcode = kde_start(argc, argv); kde_destroy(); return retcode; } \
int kde_start


#ifndef WIN32
# define WIN32
#endif

#ifndef HAVE_CONFIG_H
# define HAVE_CONFIG_H 1
#endif

#ifndef HAVE_SYS_STAT_H
# define HAVE_SYS_STAT_H 1
#endif

#ifndef HAVE_SYS_PARAM_H
# define HAVE_SYS_PARAM_H 1
#endif

#ifndef M_PI
# define M_PI 3.14159265358979323846
#endif

#ifndef NULL
# define NULL 0
#endif

/*
#ifdef __cplusplus
#include <qstring.h>

# ifndef KDEDIR
#  define KDEDIR QString(KDEWIN32_DIR)
# endif
# ifndef __KDE_BINDIR 
#  define __KDE_BINDIR QString(KDEWIN32_DIR "/bin")
# endif
#endif
*/
/*
# ifndef __pid_t_defined
  //typedef __pid_t pid_t;
  typedef unsigned int pid_t;
#  define __pid_t_defined
# endif
*/
# ifndef __uid_t_defined
//typedef __uid_t uid_t;
  typedef unsigned int __uid_t;
  typedef __uid_t uid_t;
#  define __uid_t_defined
# endif

# ifndef __gid_t_defined
//typedef __uid_t uid_t;
  typedef unsigned int __gid_t;
  typedef __gid_t gid_t;
#  define __gid_t_defined
# endif


#define snprintf _snprintf
#define vsnprintf _vsnprintf


#if defined(_MSC_VER)

# if (_MSC_VER <= 1200)
 /* Force Conformance for For Loop Scope (/Zc:forScope) is only allowed for 
    MSVC++ version > 6.0
   -this option controls the scope of variables
  defined as part of the For loop control statement. Traditionally, Microsoft 
  extended the scope of loop control variables defined in the loop control statement 
  beyond the scope of the loop. Such behavior violates the ANSI C/C++ standard 
  and may cause compiler errors when compiling ANSI C/C++ code with Microsoft C++.
  Simple (dirty) macro trick to fix the compile problem for msvc6
  ( see http://q12.org/pipermail/ode/2003-January/002811.html
    and http://support.microsoft.com/default.aspx?scid=kb;EN-US;167748 )
 */
#  define for  if (0); else for
# endif

//operator (char*) caused errors
//#define QT_NO_ASCII_CAST

/* warning C4150: deletion of pointer to incomplete type 'type'; no destructor called

The delete operator was called to delete the given type, which was 
declared but not defined. The compiler was unable to find any destructors for the given type.
*/
#pragma warning( disable: 4150 )


/* warning C4521: qmultiple copy constructors specified
*/
#pragma warning( disable: 4521 )

/* warning C4251: 'identifier' : class 'type' needs to have dll-interface 
   to be used by clients of class 'type2'
*/
#pragma warning( disable: 4251 )

/* identifier was truncated to '255'.. */
# pragma warning(disable: 4786)

/* warning C4250: ...: inherits ... via dominanceidentifier */
# pragma warning(disable: 4250)

# pragma warning(disable: 4006) /*libxxx : warning LNK4006: 
         ".... already defined in kaction.obj; second definition ignored" */

/* disable C4100: unreferenced formal parameter */
# pragma warning(disable: 4100)

/* disable C4101: unreferenced local variable */
# pragma warning(disable: 4101)

/* disable C4189: local variable is initialized but not referenced */
# pragma warning(disable: 4189)

# if (_MSC_VER >= 1300)
/* disable C4273: inconsistent DLL linkage. dllexport assumed. */
#  pragma warning(disable: 4273)

/* disable C4288: nonstandard extension used : 'i' : loop control variable 
 declared in the for-loop is used outside the for-loop scope; 
 it conflicts with the declaration in the outer scope */
#  pragma warning(disable: 4288)

/* disable 4996: warnings for deprecated symbols */
#  pragma warning(disable: 4996)

/* temporary disable for Qt4 BUG: "no suitable definition provided for explicit template" */
#  pragma warning(disable: 4661)

# endif

#endif

/*#if (_MSC_VER >= 1300) && (WINVER < 0x0500)
#ifdef __cplusplus
extern "C" {
#endif
*/
/* VC7 or later, building with pre-VC7 runtime libraries */
/*extern "C"*/ long _ftol( double ); /* defined by VC6 C libs */
/*extern "C" KDEWIN32_EXPORT long _ftol2( double dblSource ); */
/*#ifdef __cplusplus
inline long _cdecl _ftol2( double dblSource ) { return _ftol( dblSource ); }
}
#endif*/

