// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2002 David Faure (david@mandrakesoft.com)
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

// maximum global call stack size. Protects against accidental or
// malicious infinite recursions. Define to -1 if you want no limit.
#define KJS_MAX_STACK 1000

// we don't want any padding between UChars (ARM processor)
#if defined(__GNUC__) || defined(__INTEL_COMPILER)
#define KJS_PACKED __attribute__((__packed__))
#else
#define KJS_PACKED
#endif

#undef __KDE_HAVE_GCC_VISIBILITY

#ifdef __KDE_HAVE_GCC_VISIBILITY
#define KJS_EXPORT __attribute__ ((visibility("default")))
#else
#define KJS_EXPORT
#endif

#ifndef NDEBUG // protection against problems if committing with KJS_VERBOSE on

// Uncomment this to enable very verbose output from KJS
//#define KJS_VERBOSE
// Uncomment this to debug memory allocation and garbage collection
//#define KJS_DEBUG_MEM

#endif

#endif
