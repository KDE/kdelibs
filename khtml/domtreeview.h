/***************************************************************************
                               domtreeview.cpp
                             -------------------

    copyright            : (C) 2001 - The Kafka Team
    email                : kde-kafka@master.kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DOMTREEVIEW_H
#define DOMTREEVIEW_H

#include <klistview.h>
#include <kdebug.h>
#include <q3listview.h>
#include <q3ptrdict.h>
#include "dom/dom_core.h"

class DOMTreeView : public KListView
{
    Q_OBJECT
    public: 
	DOMTreeView(QWidget *parent, KHTMLPart *part, const char * name = 0);
	~DOMTreeView();

	void recursive(const DOM::Node &pNode, const DOM::Node &node);

    Q_SIGNALS:
	void sigNodeClicked(const DOM::Node &);
	
    public Q_SLOTS:
	void showTree(const DOM::Node &pNode);

    protected Q_SLOTS:
	void slotItemClicked(Q3ListViewItem *);

    private:
	Q3PtrDict<Q3ListViewItem> m_itemdict;
	Q3PtrDict<DOM::Node> m_nodedict;
	DOM::Node document;

	KHTMLPart *part;

};

#endif
