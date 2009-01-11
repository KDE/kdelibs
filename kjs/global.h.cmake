// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2002 David Faure (faure@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef KJS_GLOBAL_H
#define KJS_GLOBAL_H

#include <wtf/Platform.h>

// we don't want any padding between UChars (ARM processor)
#if defined(__GNUC__) || defined(__INTEL_COMPILER)
#define KJS_PACKED __attribute__((__packed__))
#else
#define KJS_PACKED
#endif

#if defined(_MSC_VER)
#pragma warning(disable: 4355)
#endif

#cmakedefine __KDE_HAVE_GCC_VISIBILITY

#ifndef KJS_EXPORT
# ifdef __KDE_HAVE_GCC_VISIBILITY
#  define KJS_EXPORT __attribute__ ((visibility("default")))
# elif defined(_WIN32) || defined(_WIN64)
#  ifdef MAKE_KJS_LIB
#   define KJS_EXPORT __declspec(dllexport)
#  else
#   define KJS_EXPORT __declspec(dllimport)
#  endif
# else
#  define KJS_EXPORT
# endif
#endif

// some methods are declared inside kjs and defined outside (currently only in khtml)
// KJS_EXTERNAL_EXPORT should be prefixed to each related method
#ifndef KJS_EXTERNAL_EXPORT
# if defined(MAKE_KJS_LIB) 
#  define KJS_EXTERNAL_EXPORT
# elif (defined(_WIN32) || defined(_WIN64)) && defined(__GNUC__)
#  if defined(MAKE_KHTML_LIB) || defined(MAKE_KJSEMBED_LIB) || defined(MAKE_KATEPART_LIB)
#   define KJS_EXTERNAL_EXPORT __declspec(dllexport)
#  else 
#   define KJS_EXTERNAL_EXPORT 
#  endif
# else 
#   define KJS_EXTERNAL_EXPORT
# endif
#endif

#ifndef NDEBUG // protection against problems if committing with KJS_VERBOSE on

// Uncomment this to enable very verbose output from KJS
//#define KJS_VERBOSE
// Uncomment this to debug memory allocation and garbage collection
//#define KJS_DEBUG_MEM

#endif

// Apple feature which we don't use
#define KJS_MULTIPLE_THREADS 0
// Debugging features, turned off by default
#define DEBUG_COLLECTOR 0

#ifndef HAVE_STDINT_H
#cmakedefine HAVE_STDINT_H 1
#endif

/* Valgrind memcheck presence */
#cmakedefine HAVE_VALGRIND_MEMCHECK_H 1

#endif
