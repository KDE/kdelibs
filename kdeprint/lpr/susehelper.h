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

#ifndef susehelper_h
#define susehelper_h

#include <qdict.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

struct SuSEOption;
struct SuSEPrinter;
struct SuSEConfig;
struct SuSEManufacturer;
struct SuSEValue;
class DrMain;
class DrBase;
class QFile;

class SuSEHelper
{
public:
	SuSEHelper(const QString& yastDir);
	~SuSEHelper();

	DrMain* generateDriver(const QString& configname);
	QString generateGsOption(const QString& key, const QString& val, const QMap<QString,QString>&);
	QString generateYaST2Option(const QMap<QString,QString>& opts);
	bool writeUppFile(const QString& filename, const QMap<QString,QString>& opts);
	bool writeYaST2File(const QString& filename, const QMap<QString,QString>& opts);

private:
	void load();
	SuSEOption* parseOption(QFile&);
	SuSEPrinter* parsePrinter(QFile&);
	SuSEConfig* parseConfig(QFile&);
	SuSEManufacturer* parseManufacturer(QFile&);
	SuSEValue parseValue(QFile&);
	QString nextToken(QFile&);
	QString nextValue(QFile&);
	DrBase* createItem(SuSEOption*, const QString&);

private:
	QDict<SuSEManufacturer>	manufacturers;
	QDict<SuSEPrinter>	printers;
	QDict<SuSEConfig>	configs;
	QDict<SuSEOption>	options;
	QString			yastdir;
	bool			loaded;
};

#endif
