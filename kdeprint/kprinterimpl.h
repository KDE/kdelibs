/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  $Id$
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

#ifndef KPRINTERIMPL_H
#define KPRINTERIMPL_H

#include <qobject.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qptrlist.h>

class KPrinter;
class KMPrinter;

class KPrinterImpl : public QObject
{
	Q_OBJECT
public:
	KPrinterImpl(QObject *parent = 0, const char *name = 0);
	virtual ~KPrinterImpl();

	virtual bool setupCommand(QString& cmd, KPrinter*);
	virtual void preparePrinting(KPrinter*);
	virtual void broadcastOption(const QString& key, const QString& value);

	bool printFiles(KPrinter*, const QStringList&, bool removeflag = false);
	// result:
	//	-1	->	error
	//	0	->	nothing happened
	//	1	->	files filterd
	int filterFiles(KPrinter*, QStringList&, bool removeflag = false);
	int autoConvertFiles(KPrinter*, QStringList&, bool removeflag = false);
	void saveOptions(const QMap<QString,QString>& opts);
	const QMap<QString,QString>& loadOptions() const 	{ return m_options; }
	QString tempFile();
	QString quote(const QString&);
	void statusMessage(const QString&, KPrinter* = 0);

protected:
	bool startPrinting(const QString& cmd, KPrinter *printer, const QStringList& files, bool removeflag = false);
	int dcopPrint(const QString& cmd, const QStringList& files, bool removeflag = false);
	bool setupSpecialCommand(QString&, KPrinter*, const QStringList&);
	int doFilterFiles(KPrinter* pr, QStringList& files, const QStringList& flist, const QMap<QString,QString>& opts, bool removeflag = false);
	void loadAppOptions();
	void saveAppOptions();

protected:
	QMap<QString,QString>	m_options;	// use to save current options
};

#endif
