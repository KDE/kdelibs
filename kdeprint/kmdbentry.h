/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
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

#ifndef KMDBENTRY_H
#define KMDBENTRY_H

#include <qstring.h>
#include <q3ptrlist.h>

#include <kdelibs_export.h>

class KDEPRINT_EXPORT KMDBEntry
{
	public:
	// the file location of the driver
	QString	file;
	// normal information
	QString	manufacturer;
	QString	model;
	QString	modelname;
	// information used for auto-detection
	QString	pnpmanufacturer;
	QString	pnpmodel;
	// short driver description (if any)
	QString	description;
	// tell whether this is the recommended driver
	bool recommended;
	QString	drivercomment;

	KMDBEntry();
	bool validate(bool checkIt = true);
};

typedef Q3PtrList<KMDBEntry>		KMDBEntryList;
typedef Q3PtrListIterator<KMDBEntry>	KMDBEntryListIterator;

#endif
