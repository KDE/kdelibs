/* This file is part of the KDE project
 *
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#include "kcarddb.h"
#include <kconfig.h>
#include <kprocess.h>
#include <qstring.h>



KCardDB::KCardDB() {
	cfg = new KConfig("ksmartcarddb", false, false);
}


KCardDB::~KCardDB() {
	delete cfg;
}


const QString KCardDB::getModuleName(const QString ATR) {

    cfg->reparseConfiguration();
    if (cfg->hasGroup(ATR)) {
        cfg->setGroup(ATR);
        return cfg->readEntry("Handler", QString::null);
    }

    return QString::null;
}


int KCardDB::removeByHandler(const QString module) {
	QStringList g = cfg->groupList();

	for (QStringList::Iterator i = g.begin(); i != g.end(); ++i) {
		cfg->setGroup(*i);
		if (module == cfg->readEntry("Handler", QString::null))
			cfg->deleteGroup(*i);
	}

	cfg->sync();
	return 0;
}


int KCardDB::removeCard(const QString ATR) {
	cfg->deleteGroup(ATR);
	cfg->sync();
	return 0;
}


int KCardDB::addHandler(const QString ATR, const QString module) {
	cfg->setGroup(ATR);
	cfg->writeEntry("Handler", module);
	cfg->sync();
	return 0;
}


int KCardDB::launchSelector(const QString slot, const QString ATR=QString::null) {
	KProcess p;
	p << "kcardchooser" << "--slot" << slot << "--atr" << ATR;
	p.start(KProcess::DontCare);
	return 0;
}


QString KCardDB::guessHandler(const QString ATR) {
	return QString::null;
}



