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

#ifndef KMTHREADJOB_H
#define KMTHREADJOB_H

#include <qintdict.h>
#include <qobject.h>

class KMJob;
class KMJobManager;

class KMThreadJob : public QObject
{
public:
	KMThreadJob(QObject *parent = 0, const char *name = 0);
	~KMThreadJob();

	KMJob* findJob(int ID);
	KMJob* findJob(const QString& uri);
	bool removeJob(int ID);
	void createJob(KMJob*);
	void updateManager(KMJobManager*);

	static void createJob(int ID, const QString& printer, const QString& name = QString::null, const QString& owner = QString::null, int size = 0);

protected:
	QString jobFile();
	bool loadJobs();
	bool saveJobs();
	bool checkJob(int ID);

private:
	QIntDict<KMJob>	m_jobs;
};

#endif
