/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Michael Matz <matz@kde.org>
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

#include <qglobal.h>

#if QT_VERSION < 300

#include <qobjectdict.h>
#include <kapplication.h>

#define _UNLOAD(p) \
static void unload_##p (const char **list) \
{ \
    if (!objectDict || !kapp) return; \
    /*qDebug("removing meta classes for %s", #p);*/ \
    const char **n = list; \
    for (; *n; n++) { \
        /*qDebug("  removing metaclass %s", *n);*/ \
	/* Because objectDict was set to AutoDelete the following does the \
           right thing. */ \
        while (objectDict->remove(*n)) ; \
    } \
    /*qDebug("removing done");*/ \
} \
 \
class _KUnloadMetaClass_Helper##p { \
public: \
    _KUnloadMetaClass_Helper##p () {} \
    ~_KUnloadMetaClass_Helper##p () { \
        unload_##p (_metalist_##p); \
    } \
}; \
 \
static _KUnloadMetaClass_Helper##p _helper_object_##p;

#else
#define _UNLOAD(p) 
#endif
