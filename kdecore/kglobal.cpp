/* This file is part of the KDE libraries
   Copyright (C) 1999 Sirtaj Singh Kanq <taj@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
/*
* kglobal.cpp -- Implementation of class KGlobal.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Sat May  1 02:08:43 EST 1999
*/

#include <qglobal.h>
#include "kglobal.h"

#include <kapp.h>

#include <kconfig.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kiconloader.h>
#include <kstddirs.h>
#include <kinstance.h>

#include <qfont.h>

#ifndef NDEBUG
#define MYASSERT(x) if (!x) \
   qFatal("Fatal error: you need to have a KInstance object before\n" \
         "you do anything that requires it! Examples of this are config\n" \
         "objects, standard directories or translations.");
#else
#define MYASSERT(x) /* nope */
#endif

KStandardDirs *KGlobal::dirs()
{
    MYASSERT(_instance);

    return _instance->dirs();
}

KConfig	*KGlobal::config()
{
    MYASSERT(_instance);

    return _instance->config();
}

KIconLoader *KGlobal::iconLoader()
{
    MYASSERT(_instance);

    return _instance->iconLoader();
}

KInstance *KGlobal::instance()
{
    MYASSERT(_instance);

    return _instance;
}

KLocale	*KGlobal::locale()
{	
    if( _locale == 0 ) {
	if (!_instance)
	   return 0;

        // will set _locale if it works - otherwise 0 is returned
        KLocale::initInstance();
    }

    return _locale;
}

KCharsets *KGlobal::charsets()
{
    if( _charsets == 0 ) {
        _charsets =new KCharsets();
    }

    return _charsets;
}

/*
void KGlobal::init()
{
    if (_instance)
        return;

    debug("using unknown");
    _instance = new KInstance("unknown");
    qAddPostRoutine( freeAll );
}

void KGlobal::freeAll()
{	
    delete _locale;
    _locale = 0;
    delete _charsets;
    _charsets = 0;
    delete _generalFont;
    _generalFont = 0;
    delete _fixedFont;
    _fixedFont = 0;
    delete _instance;
    _instance = 0;
}
*/
	
// The Variables

KInstance       *KGlobal::_instance     = 0;
KInstance       *KGlobal::_activeInstance = 0;
KLocale         *KGlobal::_locale	= 0;
KCharsets       *KGlobal::_charsets	= 0;
