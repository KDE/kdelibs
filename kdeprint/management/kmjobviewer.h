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
#include "kpreloadobject.h"

class KMJobManager;
class KMJob;
class KListView;
class JobItem;
class QPopupMenu;
class QListViewItem;
class KMPrinter;
class QTimer;
class QLineEdit;

class KMJobViewer : public KMainWindow, public KMPrinterPage, public KPReloadObject
{
	Q_OBJECT
public:
	KMJobViewer(QWidget *parent = 0, const char *name = 0);
	~KMJobViewer();

	void setPrinter(const QString& prname);
	void setPrinter(KMPrinter *p);
	void refresh(bool reload = false);
	QString printer() const;

signals:
	void jobsShown(KMJobViewer*, bool hasJobs);
	void refreshClicked();
	void printerChanged(KMJobViewer*, const QString& prname);
	void viewerDestroyed(KMJobViewer*);

public slots:
	void pluginActionActivated(int);

protected slots:
	void slotSelectionChanged();
	void slotHold();
	void slotResume();
	void slotRemove();
	void slotRestart();
	void slotRightClicked(QListViewItem*,const QPoint&,int);
	void slotMove(int prID);
	void slotPrinterSelected(int);
	void slotShowCompleted(bool);
	void slotRefresh();
	void slotClose();
	void slotShowMoveMenu();
	void slotShowPrinterMenu();
	void slotUserOnly(bool);
	void slotUserChanged();

protected:
	void init();
	void updateJobs();
	void initActions();
	JobItem* findItem(const QString& uri);
	void jobSelection(QPtrList<KMJob>& l);
	void send(int cmd, const QString& name, const QString& arg = QString::null);
	void loadPrinters();
	void loadPluginActions();
	void removePluginActions();
	void reload();
	//void aboutToReload();
	void closeEvent(QCloseEvent*);
	void triggerRefresh();
	void addToManager();
	void removeFromManager();
	void buildPrinterMenu(QPopupMenu *menu, bool use_all = false);
	void updateCaption();

private:
	KListView		*m_view;
	QPtrList<KMJob>		m_jobs;
	QPtrList<JobItem>		m_items;
	QPopupMenu		*m_pop;
	QPtrList<KMPrinter>	m_printers;
	QString	m_prname;
	int	m_type;
	QString	m_username;
	QLineEdit	*m_userfield;
};

inline QString KMJobViewer::printer() const
{ return m_prname; }

#endif
