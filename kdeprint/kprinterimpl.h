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
#include <qlist.h>

class KPrinter;
class KMPrinter;
class KProcess;
class KPrintProcess;

class KPrinterImpl : public QObject
{
	Q_OBJECT
public:
	KPrinterImpl(QObject *parent = 0, const char *name = 0);
	virtual ~KPrinterImpl();

	virtual bool printFiles(KPrinter*, const QStringList&);
	virtual void preparePrinting(KPrinter*);
	virtual void broadcastOption(const QString& key, const QString& value);

	KMPrinter* filePrinter()				{ return m_fileprinter; }
	void saveOptions(const QMap<QString,QString>& opts)	{ m_options = opts; }
	const QMap<QString,QString>& loadOptions() const 	{ return m_options; }

protected slots:
	void slotProcessExited(KProcess*);

protected:
	bool startPrintProcess(KPrintProcess*, KPrinter*);

protected:
	KMPrinter		*m_fileprinter;
	QMap<QString,QString>	m_options;	// use to save current options
	QList<KPrintProcess>	m_processpool;
};

#endif
