/*
 Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>
 
 Win32 port:
 Copyright (C) 2004 Jaroslaw Staniek <js@iidea.pl>
 
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
 */

#include <config.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>

#include "kkeyserver.h"

namespace KKeyServer {
//---------------------------------------------------------------------
// Array Structures
//---------------------------------------------------------------------
    
struct ModInfo
{
	int modQt;
	const char* psName;
	QString* sLabel; // this struct is used in static objects, so must use a pointer here.
};

//---------------------------------------------------------------------
// Arrays
//---------------------------------------------------------------------

static ModInfo g_rgModInfo[4] =
{
	{ Qt::SHIFT,   I18N_NOOP("Shift"), 0 },
	{ Qt::CTRL,    I18N_NOOP("Ctrl"), 0 },
	{ Qt::ALT,     I18N_NOOP("Alt"), 0 },
	{ Qt::META,    I18N_NOOP("Meta"), 0 }
};

//---------------------------------------------------------------------
// Initialization
//---------------------------------------------------------------------
static bool g_bInitializedKKeyLabels;
static bool g_bMacLabels;

static void intializeKKeyLabels()
{
	KConfigGroup cg( KGlobal::config(), "Keyboard" );
	g_rgModInfo[0].sLabel = new QString( cg.readEntry( "Label Shift", i18n(g_rgModInfo[0].psName) ) );
	g_rgModInfo[1].sLabel = new QString( cg.readEntry( "Label Ctrl", i18n(g_rgModInfo[1].psName) ) );
	g_rgModInfo[2].sLabel = new QString( cg.readEntry( "Label Alt", i18n(g_rgModInfo[2].psName) ) );
	g_rgModInfo[3].sLabel = new QString( cg.readEntry( "Label Win", i18n(g_rgModInfo[3].psName) ) );
	g_bMacLabels = (*g_rgModInfo[2].sLabel == "Command");
	g_bInitializedKKeyLabels = true;
    
}

//---------------------------------------------------------------------
// Public functions
//---------------------------------------------------------------------

static QString modToString( uint mod, bool bUserSpace )
{
	if( bUserSpace && !g_bInitializedKKeyLabels )
		intializeKKeyLabels();
    
	QString s;
	for( int i = 3; i >= 0; i-- ) {
		if( mod & g_rgModInfo[i].modQt ) {
			if( !s.isEmpty() )
				s += '+';
			s += (bUserSpace)
                ? *g_rgModInfo[i].sLabel
                : QString(g_rgModInfo[i].psName);
		}
	}
	return s;
}

QString modToStringUser( uint mod )
{
	return modToString( mod, true );
}

uint stringUserToMod( const QString& mod )
{
	QString s;
	for( int i = 3; i >= 0; i-- ) {
		if( mod.toLower() == g_rgModInfo[i].sLabel->toLower())
			return g_rgModInfo[i].modQt;
	}
	return 0;
}


}
