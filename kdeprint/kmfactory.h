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

#ifndef KMFACTORY_H
#define KMFACTORY_H

#include <qstring.h>
#include <qvaluelist.h>
#include <qstringlist.h>
#include <qptrlist.h>
#include <qobject.h>
#include <dcopobject.h>

#include <sys/types.h>

class KMManager;
class KMJobManager;
class KMUiManager;
class KMVirtualManager;
class KXmlCommandManager;
class KMSpecialManager;
class KPrinterImpl;
class KLibFactory;
class KConfig;
class KPReloadObject;

class KMFactory : public QObject, public DCOPObject
{
	Q_OBJECT
	K_DCOP

public:
	struct PluginInfo
	{
		QString		name;
		QString		comment;
		QStringList	detectUris;
		int		detectPrecedence;
		QStringList	mimeTypes;
		QString		primaryMimeType;
	};

	static KMFactory* self();
        static bool exists();
	static void release();

	KMFactory();
	~KMFactory();

	KMManager* manager();
	KMJobManager* jobManager();
	KMUiManager* uiManager();
	KMVirtualManager* virtualManager();
	KMSpecialManager* specialManager();
	KXmlCommandManager* commandManager();
	KPrinterImpl* printerImplementation();
	KConfig* printConfig(const QString& group = QString::null);
	QString printSystem();
	QValueList<PluginInfo> pluginList();
	PluginInfo pluginInfo(const QString& name);
	void saveConfig();

	void reload(const QString& syst, bool saveSyst = true);
	void registerObject(KPReloadObject*, bool = false);
	void unregisterObject(KPReloadObject*);

	struct Settings
	{
		int	application;
		int	standardDialogPages;
		int	pageSelection;
		int	orientation;
		int	pageSize;
	};
	Settings* settings() const	{ return m_settings; }

k_dcop:
	ASYNC slot_pluginChanged(pid_t);
	ASYNC slot_configChanged();

k_dcop_signals:
	void pluginChanged(pid_t);
	void configChanged();

private:
	void createManager();
	void createJobManager();
	void createUiManager();
	void createPrinterImpl();
	void loadFactory(const QString& syst = QString::null);
	void unload();
	QString autoDetect();

private:
	static KMFactory	*m_self;

	KMManager		*m_manager;
	KMJobManager		*m_jobmanager;
	KMUiManager		*m_uimanager;
	KPrinterImpl		*m_implementation;
	KLibFactory		*m_factory;

	KConfig			*m_printconfig;
	Settings		*m_settings;
	QPtrList<KPReloadObject> m_objects;
};

#endif
