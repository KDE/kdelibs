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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef PPDLOADER_H
#define PPDLOADER_H

#include <q3valuestack.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvariant.h>
#include <q3dict.h>

#include <kdelibs_export.h>

class DrGroup;
class DrBase;
class DrMain;
struct PS_private;

class KDEPRINT_EXPORT PPDLoader
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

	void setErrorMsg( const QString& msg );
	QString errorMsg() const;

	static DrMain* loadDriver( const QString& filename, QString* msg = NULL );

private:
	Q3ValueStack<DrGroup*> m_groups;
	DrBase*               m_option;
	Q3Dict<PS_private>     m_ps;
	QStringList           m_fonts;
	QString               m_errormsg;

	friend int kdeprint_ppdparse(void*);
	DrGroup* findOrCreateGroupForOption( const QString& );
	void processPageSizes( DrMain* );
};

#endif /* PPDLOADER_H */
