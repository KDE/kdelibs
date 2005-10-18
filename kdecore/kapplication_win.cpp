/*
   This file is part of the KDE libraries
   Copyright (C) 2004 Jaroslaw Staniek <js@iidea.pl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <kapplication.h>
#include <kstandarddirs.h>
#include <klocale.h>

#include <QTranslator>

/**
 * MS Windows-related actions for KApplication startup.
 * 
 * - Use Qt translation which will be usable for QFileDialog 
 *    and other Qt-only GUIs. The "qt_<language>.qm" file should be stored
 *    in the same place as .po files for a given language.
 *
 * @internal
*/
void KApplication_init_windows()
{
	QString qt_transl_file = ::locate( "locale", KGlobal::locale()->language() 
		+ "/LC_MESSAGES/qt_" + KGlobal::locale()->language() + ".qm" );
	QTranslator *qt_transl = new QTranslator();
	if (qt_transl->load( qt_transl_file, ""))
		kapp->installTranslator( qt_transl );
	else
		delete qt_transl;
}
