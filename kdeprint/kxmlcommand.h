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

#ifndef KXMLCOMMAND_H
#define KXMLCOMMAND_H

#include <qdom.h>
#include <qmap.h>
#include <qobject.h>

class DrMain;
class DrGroup;
class DrBase;

class KXmlCommand : public QObject
{
	friend class KXmlCommandManager;

public:
	~KXmlCommand();

	QString name() const;
	void setName(const QString&);
	QString command();
	void setCommand(const QString&);
	DrMain* driver();
	DrMain* takeDriver();
	void setDriver(DrMain*);
	QString io(bool io_input = true, bool io_pipe = false);
	void setIo(const QString&, bool io_input = true, bool io_pipe = false);
	QString description();
	void setDescription(const QString&);
	QString mimeType();
	void setMimeType(const QString&);
	bool acceptMimeType(const QString&);
	QStringList inputMimeTypes();
	void setInputMimeTypes(const QStringList&);
	QStringList requirements();
	void setRequirements(const QStringList&);
	QString comment();
	void setComment( const QString& );
	bool isValid();

	QString buildCommand(const QMap<QString,QString>& opts, bool pipein = true, bool pipeout = true);
	void setOptions(const QMap<QString,QString>& opts);
	void getOptions(QMap<QString,QString>& opts, bool incldef = false);

protected:
	void init();
	void loadXml();
	void saveXml();
	void loadDesktop();
	void saveDesktop();
	void check(bool use_xml = false);
	DrGroup* parseGroup(const QDomElement& e, DrGroup *grp = 0);
	DrBase* parseArgument(const QDomElement& e);
	void parseIO(const QDomElement& e, int n);
	QDomElement createIO(QDomDocument&, int, const QString&);
	QDomElement createGroup(QDomDocument&, DrGroup*);
	QDomElement createElement(QDomDocument&, DrBase*);

	// use protected constructor to only allow the manager to
	// create KXmlCommand object.
	KXmlCommand(const QString& xmlId = QString::null);

private:
	class KXmlCommandPrivate;
	KXmlCommandPrivate	*d;
};

class KXmlCommandManager : public QObject
{
public:
	enum IO_CheckType	{ None = 0, Basic, Advanced };

	KXmlCommandManager();
	~KXmlCommandManager();

	KXmlCommand* loadCommand(const QString& xmlId, bool check = false);
	void saveCommand(KXmlCommand *xmlCmd);
	QStringList commandList();
	QStringList commandListWithDescription();
	QString selectCommand(QWidget *parent = 0);

	QStringList autoConvert(const QString& mimesrc, const QString& mimedest);
	int insertCommand(QStringList& list, const QString& filtername, bool defaultToStart = true);
	bool checkCommand(const QString&, int inputCheck = Advanced, int outputCheck = Advanced, QString *msg = 0);
	bool configure(KXmlCommand*, QWidget *parent = 0);
	void cleanUp();

	static KXmlCommandManager* self();

protected:
	void preload();
	KXmlCommand* command(const QString&) const;

private:
	class KXmlCommandManagerPrivate;
	KXmlCommandManagerPrivate	*d;
	static KXmlCommandManager	*m_self;
};

#endif
