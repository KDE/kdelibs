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

#ifndef KPRINTFILTER_H
#define KPRINTFILTER_H

#include <qobject.h>
#include <qstring.h>
#include <qmap.h>

class DrMain;

class KPrintFilter : public QObject
{
friend class FilterHandler;
public:
	KPrintFilter(const QString& idname, QObject *parent = 0, const char *name = 0);
	virtual ~KPrintFilter();

	void setXmlTemplate(const QString& filename);
	bool readXmlTemplate(const QString& filename);
	QString buildCommand(const QMap<QString,QString>& opts, bool pipein = true, bool pipeout = true);
	bool configure(QWidget *parent = 0);
	void setOptions(const QMap<QString,QString>& opts);
	void getOptions(QMap<QString,QString>& opts, bool incldef = false);
	QString idName() const;
	QString description() const;

private:
	struct KPrintFilterIO;
	void clean();

private:
	QString		m_idname;
	QString		m_description;
	QString		m_command;
	KPrintFilterIO	*m_input, *m_output;
	DrMain		*m_driver;
	bool		m_read;
};

inline void KPrintFilter::setXmlTemplate(const QString& filename)
{ m_idname = filename; }

inline QString KPrintFilter::idName() const
{ return m_idname; }

inline QString KPrintFilter::description() const
{ return m_description; }

#endif
