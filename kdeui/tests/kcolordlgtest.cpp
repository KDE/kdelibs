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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kapplication.h>
#include "kcolordialog.h"
#include <kconfig.h>
#include <klocale.h>

int main( int argc, char *argv[] )
{
	QColor color;

	KLocale::setMainCatalogue("kdelibs");
	QApplication::setColorMode( QApplication::CustomColors );
	KApplication a( argc, argv, "KColorDialogTest" );
        KConfig aConfig;
        aConfig.setGroup( "KColorDialog-test" );
    
	color = aConfig.readColorEntry( "Chosen" );
	int nRet = KColorDialog::getColor( color, Qt::red /*testing default color*/ );
	aConfig.writeEntry( "Chosen", color );
	
	return nRet;
}

