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

#ifndef KPFILTERPAGE_H
#define KPFILTERPAGE_H

#include "kprintdialogpage.h"

#include <qdict.h>
#include <qstringlist.h>

class KListView;
class KXmlCommand;
class QListViewItem;
class QPushButton;
class QTextBrowser;

class KPFilterPage : public KPrintDialogPage
{
	Q_OBJECT
public:
	KPFilterPage(QWidget *parent = 0, const char *name = 0);
	~KPFilterPage();

	void setOptions(const QMap<QString,QString>& opts);
	void getOptions(QMap<QString,QString>& opts, bool incldef = false);
	bool isValid(QString& msg);

protected slots:
	void slotAddClicked();
	void slotRemoveClicked();
	void slotUpClicked();
	void slotDownClicked();
	void slotConfigureClicked();
	void slotItemSelected(QListViewItem*);

protected:
	KXmlCommand* currentFilter();
	void checkFilterChain();
	void updateInfo();
	QStringList activeList();
    void updateButton();
private:
	KListView		*m_view;
	QStringList		m_filters;	// <idname,description> pairs
	QDict<KXmlCommand>	m_activefilters;
	QPushButton		*m_add, *m_remove, *m_up, *m_down, *m_configure;
	bool			m_valid;
	QTextBrowser		*m_info;
};

#endif
