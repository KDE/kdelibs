/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001-2003 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef PPDLOADER_H
#define PPDLOADER_H

#include <qvaluestack.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvariant.h>
#include <qdict.h>

class DrGroup;
class DrBase;
class DrMain;
struct PS_private;

class PPDLoader
{
public:
	PPDLoader();
	~PPDLoader();

	DrMain* readFromFile( const QString& filename );

	bool openUi( const QString& name, const QString& desc, const QString& type );
	bool endUi( const QString& name );
	bool openGroup( const QString& name, const QString& desc );
	bool endGroup( const QString& name );
	bool putStatement( const QString& keyword, const QString& name, const QString& desc, const QStringList& values );
	bool putStatement2( const QString& keyword, const QString& value );
	bool putDefault( const QString& keyword, const QString& value );
	bool putConstraint( const QString& opt1, const QString& opt2, const QString& ch1, const QString& ch2 );
	bool putFooData( const QString& data );
	bool putFooProcessedData( const QVariant& var );
	bool putPaperDimension( const QString& name, const QString& s );
	bool putImageableArea( const QString& name, const QString& s );

	static DrMain* loadDriver( const QString& filename );

private:
	QValueStack<DrGroup*> m_groups;
	DrBase*               m_option;
	QDict<PS_private>     m_ps;
	QStringList           m_fonts;

	friend int kdeprint_ppdparse(void*);
	DrGroup* findOrCreateGroupForOption( const QString& );
	void processPageSizes( DrMain* );
};

#endif /* PPDLOADER_H */
