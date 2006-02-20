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
#ifndef LPDTOOLS_H
#define LPDTOOLS_H

#include <qstring.h>
#include <qmap.h>
#include <qlist.h>
#include <qtextstream.h>

class KMLpdManager;
class DrMain;
class KMPrinter;

class PrintcapEntry
{
friend class KMLpdManager;
public:
	bool readLine(const QString& line);
	void writeEntry(QTextStream&);
	KMPrinter* createPrinter();
	QString arg(const QString& key) const 	{ return m_args[key]; }
	QString comment(int i);
private:
	QString			m_name;
	QString			m_comment;
	QMap<QString,QString>	m_args;
};

//*****************************************************************************************************

struct Resolution
{
	int 	xdpi, ydpi;
	QString	comment;
};

struct BitsPerPixel
{
	QString	bpp;
	QString	comment;
};

class PrinttoolEntry
{
friend class KMLpdManager;
public:
	bool readEntry(QTextStream& t);
	DrMain* createDriver();
private:
	QString			m_name, m_gsdriver, m_description, m_about;
	QList<Resolution*>	m_resolutions;
	QList<BitsPerPixel*>	m_depths;
};

//*****************************************************************************************************

QString getPrintcapLine(QTextStream& t, QString *lastcomment = NULL);

#endif // LPDTOOLS_H
