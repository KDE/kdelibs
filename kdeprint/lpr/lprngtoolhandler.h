/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
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

#ifndef LPRNGTOOLHANDLER_H
#define LPRNGTOOLHANDLER_H

#include "lprhandler.h"
#include <qmap.h>
#include <qstringlist.h>
#include <qpair.h>

class LPRngToolHandler : public LprHandler
{
public:
	LPRngToolHandler(KMManager *mgr = 0);

	bool validate(PrintcapEntry*);
	bool completePrinter(KMPrinter*, PrintcapEntry*, bool = true);
	DrMain* loadDriver(KMPrinter*, PrintcapEntry*, bool = false);
	DrMain* loadDbDriver(const QString&);
	PrintcapEntry* createEntry(KMPrinter*);
	bool savePrinterDriver(KMPrinter*, PrintcapEntry*, DrMain*, bool* = 0);

protected:
	QMap<QString,QString> parseXferOptions(const QString&);
	void loadAuthFile(const QString&, QString&, QString&);
	QValueList< QPair<QString,QStringList> > loadChoiceDict(const QString&);
	QMap<QString,QString> parseZOptions(const QString&);
	QString filterDir();


private:
	QValueList< QPair<QString,QStringList> >	m_dict;
	QString	m_filterdir;
};

#endif
