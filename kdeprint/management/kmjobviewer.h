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

#ifndef KMJOBVIEWER_H
#define KMJOBVIEWER_H

#include <kmainwindow.h>
#include "kmprinterpage.h"

class KMJobManager;
class KMJob;
class QListView;
class JobItem;
class QPopupMenu;
class QListViewItem;
class KMPrinter;

class KMJobViewer : public KMainWindow, public KMPrinterPage
{
	Q_OBJECT
public:
	KMJobViewer(QWidget *parent = 0, const char *name = 0);
	~KMJobViewer();

	void addPrinter(const QString& prname);
	void setPrinter(const QString& prname);
	void setPrinter(KMPrinter *p);
	void refresh();
	void selectAll();

signals:
	void jobsShown();

protected slots:
	void slotSelectionChanged();
	void slotHold();
	void slotResume();
	void slotRemove();
	void slotRestart();
	void slotRightClicked(QListViewItem*,const QPoint&,int);
	void slotMove(int prID);
	void slotPrinterToggled(bool);
	void slotAllPrinters();
	void slotRefresh();
	void slotShowMenu();
	void slotHideMenu();
	void slotOnItem(QListViewItem*);
	void slotOnViewport();
	void slotShowCompleted(bool);

protected:
	void init();
	void updateJobs();
	void initActions();
	void initPrinterActions();
	JobItem* findItem(int ID);
	void jobSelection(QList<KMJob>& l);
	void send(int cmd, const QString& name, const QString& arg = QString::null);
	void loadPrinters();

private:
	QListView		*m_view;
	QList<KMJob>		m_jobs;
	QList<JobItem>		m_items;
	QPopupMenu		*m_pop;
	QList<KMPrinter>	m_printers;
};

#endif
