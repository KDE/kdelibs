/* This file is part of the KDE project
   Copyright (C) 2001 Ian Reinhart Geiser <geiseri@yahoo.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the Lesser GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kappdcopiface.h"
#include <kapplication.h>
#include <dcopclient.h>
#include <kglobal.h>
#include <kconfig.h>


KAppDCOPInterface::KAppDCOPInterface(KApplication * theKApp)
	: DCOPObject( "MainApplication-Interface")
{
	m_KApplication = theKApp;
}

KAppDCOPInterface::~KAppDCOPInterface()
{
}

void KAppDCOPInterface::disableSessionManagement()
{

}

QByteArray KAppDCOPInterface::startupId()
{
	return "";
}

QByteArray KAppDCOPInterface::caption()
{
	return "";
}

void KAppDCOPInterface::quit()
{
	m_KApplication->quit();
}

void KAppDCOPInterface::updateUserTimestamp( quint32 time )
{
	m_KApplication->updateUserTimestamp( time );
}

void KAppDCOPInterface::reparseConfiguration()
{
	KGlobal::config()->reparseConfiguration();
}

