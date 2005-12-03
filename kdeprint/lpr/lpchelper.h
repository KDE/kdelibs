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

#ifndef LPCHELPER_H
#define LPCHELPER_H

#include <qobject.h>
#include <qmap.h>
#include <qtextstream.h>
#include "kmprinter.h"

class KMJob;

class LpcHelper : public QObject
{
public:
	LpcHelper(QObject *parent = 0);
	~LpcHelper();

	KMPrinter::PrinterState state(const QString&) const;
	KMPrinter::PrinterState state(KMPrinter*) const;
	void updateStates();

	bool enable(KMPrinter*, bool, QString&);
	bool start(KMPrinter*, bool, QString&);
	bool removeJob(KMJob*, QString&);
	bool changeJobState(KMJob*, int, QString&);

	bool restart(QString&);

protected:
	bool changeState(const QString&, const QString&, QString&);
	void parseStatusLPR(QTextStream&);
	void parseStatusLPRng(QTextStream&);
	int parseStateChangeLPR(const QString&, const QString&);
	int parseStateChangeLPRng(const QString&, const QString&);

private:
	QMap<QString, KMPrinter::PrinterState>	m_state;
	QString	m_exepath, m_lprmpath, m_checkpcpath;
};

#endif
