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

class KMManager;
class KMJobManager;
class KMUiManager;
class KMVirtualManager;
class KPrinterImpl;
class KLibFactory;
class KConfig;

class KMFactory
{
public:
	static KMFactory* self();
	static void release();

	KMFactory();
	~KMFactory();

	KMManager* manager();
	KMJobManager* jobManager();
	KMUiManager* uiManager();
	KMVirtualManager* virtualManager();
	KPrinterImpl* printerImplementation();
	KConfig* printConfig();
	QString printSystem();

private:
	void createManager();
	void createJobManager();
	void createUiManager();
	void createPrinterImpl();
	void loadFactory();

private:
	static KMFactory	*m_self;

	KMManager		*m_manager;
	KMJobManager		*m_jobmanager;
	KMUiManager		*m_uimanager;
	KMVirtualManager	*m_virtualmanager;
	KPrinterImpl		*m_implementation;
	KLibFactory		*m_factory;

	KConfig			*m_printconfig;
};

#endif
