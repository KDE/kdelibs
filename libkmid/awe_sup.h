/*  awe_sup.h  - A wrapper on awe_voice.h
    This file is part of LibKMid 0.9.5
    Copyright (C) 1997,98,99,2000  Antonio Larrosa Jimenez
    LibKMid's homepage : http://www.arrakis.es/~rlarrosa/libkmid.html
 
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
 
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
 
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
 
    Send comments and bug fixes to Antonio Larrosa <larrosa@kde.org>
 
***************************************************************************/
#ifndef _AWE_SUP_H
#define _AWE_SUP_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_AWE_VOICE_H
#include <awe_voice.h>
#define HAVE_AWE32
#elif defined(HAVE_LINUX_AWE_VOICE_H)
#include <linux/awe_voice.h>
#define HAVE_AWE32
#elif defined(HAVE__USR_SRC_SYS_I386_ISA_SOUND_AWE_VOICE_H)
#include "/usr/src/sys/i386/isa/sound/awe_voice.h"
#define HAVE_AWE32
#elif defined(HAVE__USR_SRC_SYS_GNU_I386_ISA_SOUND_AWE_VOICE_H)
#include "/usr/src/sys/gnu/i386/isa/sound/awe_voice.h"
#define HAVE_AWE32
#endif

#ifndef AWE_SET_CHANNEL_MODE
// AWE32 doesn't work if AWE_SET_CHANNEL_MODE isn't defined.
#undef HAVE_AWE32
#endif

#endif
