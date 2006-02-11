/*
   This file is part of the KDE libraries
   Copyright (C) 2004 Jaroslaw Staniek <js@iidea.pl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <io.h> /* to avoid #includes */

#define KPATH_SEPARATOR ';'
#define popen _popen
#define pclose _pclose

#ifndef KDECORE_EXPORT
# if defined(MAKE_KDECORE_LIB) || defined(MAKE_KDEFAKES_LIB)
#  define KDECORE_EXPORT KDE_EXPORT
# else
#  define KDECORE_EXPORT KDE_IMPORT //for apps and other libs
# endif
#endif

#ifndef KDEUI_EXPORT
# ifdef MAKE_KDEUI_LIB
#  define KDEUI_EXPORT KDE_EXPORT
# else
#  define KDEUI_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KDEFX_EXPORT
# ifdef MAKE_KDEFX_LIB
#  define KDEFX_EXPORT  KDE_EXPORT
# else
#  define KDEFX_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KDEPRINT_EXPORT
# ifdef MAKE_KDEPRINT_LIB
#  define KDEPRINT_EXPORT  KDE_EXPORT
# else
#  define KDEPRINT_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KDEPRINT_MANAGEMENT_EXPORT
# ifdef MAKE_KDEPRINT_MANAGEMENT_LIB
#  define KDEPRINT_MANAGEMENT_EXPORT  KDE_EXPORT
# else
#  define KDEPRINT_MANAGEMENT_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KDEPRINT_MANAGEMENT_MODULE_EXPORT
# ifdef MAKE_KDEPRINT_MANAGEMENT_MODULE_LIB
#  define KDEPRINT_MANAGEMENT_MODULE_EXPORT  KDE_EXPORT
# else
#  define KDEPRINT_MANAGEMENT_MODULE_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KIO_EXPORT
# ifdef MAKE_KIO_LIB
#  define KIO_EXPORT KDE_EXPORT
# else
#  define KIO_EXPORT KDE_IMPORT
# endif
#endif

#ifndef DCOP_EXPORT
# ifdef MAKE_DCOP_LIB
#  define DCOP_EXPORT KDE_EXPORT
# else
#  define DCOP_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KPARTS_EXPORT
# ifdef MAKE_KPARTS_LIB
#  define KPARTS_EXPORT KDE_EXPORT
# else
#  define KPARTS_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KTEXTEDITOR_EXPORT
# ifdef MAKE_KTEXTEDITOR_LIB
#  define KTEXTEDITOR_EXPORT KDE_EXPORT
# else
#  define KTEXTEDITOR_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KABC_EXPORT
# ifdef MAKE_KABC_LIB
#  define KABC_EXPORT KDE_EXPORT
# else
#  define KABC_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KVCARD_EXPORT
# ifdef MAKE_KVCARD_LIB
#  define KVCARD_EXPORT KDE_EXPORT
# else
#  define KVCARD_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KRESOURCES_EXPORT
# ifdef MAKE_KRESOURCES_LIB
#  define KRESOURCES_EXPORT KDE_EXPORT
# else
#  define KRESOURCES_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KCMRESOURCES_EXPORT
# ifdef MAKE_KCM_KRESOURCES_LIB
#  define KCM_KRESOURCES_EXPORT KDE_EXPORT
# else
#  define KCM_KRESOURCES_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KDESU_EXPORT
# ifdef MAKE_KDESU_LIB
#  define KDESU_EXPORT KDE_EXPORT
# else
#  define KDESU_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KSTYLE_EXPORT
# ifdef MAKE_KSTYLE_LIB
#  define KSTYLE_EXPORT KDE_EXPORT
# else
#  define KSTYLE_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KMDI_EXPORT
# ifdef MAKE_KMDI_LIB
#  define KMDI_EXPORT  KDE_EXPORT
# else
#  define KMDI_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KUTILS_EXPORT
# ifdef MAKE_KUTILS_LIB
#  define KUTILS_EXPORT  KDE_EXPORT
# else
#  define KUTILS_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KATEPARTINTERFACES_EXPORT
# ifdef MAKE_KATEPARTINTERFACES_LIB
#  define KATEPARTINTERFACES_EXPORT KDE_EXPORT
# else
#  define KATEPARTINTERFACES_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KATEPART_EXPORT
# ifdef MAKE_KATEPART_LIB
#  define KATEPART_EXPORT  KDE_EXPORT
# else
#  define KATEPART_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KDE3SUPPORT_EXPORT
# ifdef MAKE_KDE3SUPPORT_LIB
#  define KDE3SUPPORT_EXPORT KDE_EXPORT
# else
#  define KDE3SUPPORT_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KHTML_EXPORT
# ifdef MAKE_KHTML_LIB
#  define KHTML_EXPORT KDE_EXPORT
# else
#  define KHTML_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KICE_EXPORT
# ifdef MAKE_KICE_LIB
#  define KICE_EXPORT KDE_EXPORT
# else
#  define KICE_EXPORT KDE_IMPORT
# endif 
#endif 

#ifndef KJS_EXPORT
# ifdef MAKE_KJS_LIB
#  define KJS_EXPORT KDE_EXPORT
# else
#  define KJS_EXPORT KDE_IMPORT
# endif 
#endif 

#ifndef KWALLETCLIENT_EXPORT
# ifdef MAKE_KWALLETCLIENT_LIB
#  define KWALLETCLIENT_EXPORT KDE_EXPORT
# else
#  define KWALLETCLIENT_EXPORT KDE_IMPORT
# endif 
#endif 

#ifndef KWALLETBACKEND_EXPORT
# ifdef MAKE_KWALLETBACKEND_LIB
#  define KWALLETBACKEND_EXPORT KDE_EXPORT
# else
#  define KWALLETBACKEND_EXPORT KDE_IMPORT
# endif 
#endif 

#ifndef KDNSSD_EXPORT
# ifdef MAKE_KDNSSD_LIB
#  define KDNSSD_EXPORT KDE_EXPORT
# else
#  define KDNSSD_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KDED_EXPORT
# ifdef MAKE_KDED_LIB
#  define KDED_EXPORT KDE_EXPORT
# else
#  define KDED_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KDOCUMENT_EXPORT
# ifdef MAKE_KDOCUMENT_LIB
#  define KDOCUMENT_EXPORT KDE_EXPORT
# else
#  define KDOCUMENT_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KNEWSTUFF_EXPORT
# ifdef MAKE_KNEWSTUFF_LIB
#  define KNEWSTUFF_EXPORT KDE_EXPORT
# else
#  define KNEWSTUFF_EXPORT KDE_IMPORT
# endif
#endif

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif
