/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)

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

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include "kcolordialog.h"
#include <kconfig.h>
#include <klocale.h>

int main( int argc, char *argv[] )
{
	KLocale::setMainCatalog("kdelibs");
	QApplication::setColorMode( QApplication::CustomColor );
	KAboutData about("KColorDialogTest", "KColorDialogTest", "version");
	KCmdLineArgs::init(argc, argv, &about);
	KApplication::disableAutoDcopRegistration();

	KApplication a;

	KConfig aConfig;
	aConfig.setGroup( "KColorDialog-test" );

	QColor color = aConfig.readColorEntry( "Chosen" );
	int nRet = KColorDialog::getColor( color, Qt::red /*testing default color*/ );
	aConfig.writeEntry( "Chosen", color );

	return nRet;
}

