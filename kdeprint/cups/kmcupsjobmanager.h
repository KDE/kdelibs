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

#ifndef KMCUPSJOBMANAGER_H
#define KMCUPSJOBMANAGER_H

#include "kmjobmanager.h"
#include "ipprequest.h"
#include <qstringlist.h>

class KMPrinter;

class KMCupsJobManager : public KMJobManager
{
	Q_OBJECT

public:
	KMCupsJobManager(QObject *parent = 0, const char *name = 0);
	virtual ~KMCupsJobManager();

	int actions();
	QValueList<KAction*> createPluginActions(KActionCollection*);
	void validatePluginActions(KActionCollection*, const QPtrList<KMJob>&);
	bool doPluginAction(int, const QPtrList<KMJob>&);

protected:
	bool jobIppReport(KMJob*);
	bool changePriority(const QPtrList<KMJob>&, bool);
	bool editJobAttributes(KMJob*);

protected:
	bool listJobs(const QString&, JobType, int = 0);
	bool sendCommandSystemJob(const QPtrList<KMJob>& jobs, int action, const QString& arg = QString::null);
	void parseListAnswer(IppRequest& req, KMPrinter *pr);
};

#endif
