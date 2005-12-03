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

#ifndef FOOMATIC2LOADER_H
#define FOOMATIC2LOADER_H

#include <qvariant.h>

#include <kdelibs_export.h>

class QIODevice;
class DrBase;
class DrMain;

class KDEPRINT_EXPORT Foomatic2Loader
{
public:
	Foomatic2Loader();
	~Foomatic2Loader();

	bool read( QIODevice *d );
	bool readFromBuffer( const QString& buffer );
	bool readFromFile( const QString& filename );
	QMap<QString,QVariant> data() const;
	DrMain* buildDriver() const;
	DrMain* modifyDriver( DrMain* ) const;

	static DrMain* loadDriver( const QString& filename );

private:
	QMap<QString,QVariant> m_foodata;

	friend int kdeprint_foomatic2parse( void* );
	DrBase* createValue( const QString&, const QMap<QString,QVariant>& ) const;
	DrBase* createOption( const QMap<QString,QVariant>& ) const;
};

inline QMap<QString,QVariant> Foomatic2Loader::data() const
{ return m_foodata; }

#endif /* FOOMATIC2LOADER_H */
