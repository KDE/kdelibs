/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Timo Hummel (timo.hummel@sap.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

/*
 * This file is used to catch signals which would normally
 * crash the application (like segmentation fault, floating
 * point exception and such).
 */

#ifndef __KCRASH_H
#define __KCRASH_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <qstring.h>

void defaultCrashHandler (int signal);
void setCrashHandler (void (*crashHandler)(int));
void setEmergencySaveFunction (void (*saveFunction)(int));
void resetCrashRecursion (void);

extern "C" {
 // Taken from signal.h
 typedef void(*kdesignal_t) (int);

const kdesignal_t KDE_CRASH_DEFAULT  = ((kdesignal_t)0x1);
const kdesignal_t KDE_CRASH_INTERNAL = ((kdesignal_t)0x2);
const kdesignal_t KDE_SAVE_NONE      = ((kdesignal_t)0x3);
}

struct kcrashargs {
 	char *crashObjName;
 	char *crashObjValue;
};

#endif

