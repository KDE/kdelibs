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
#include <qlistview.h>
#include <qptrdict.h>
#include <dom/dom_core.h>

class DOMTreeView : public KListView
{
    Q_OBJECT
    public: 
	DOMTreeView(QWidget *parent, KHTMLPart *part, const char * name = 0);
	~DOMTreeView();

	void recursive(const DOM::Node &pNode, const DOM::Node &node);
	
    public slots:
	void showTree(const DOM::Node &pNode);

    private:
	QPtrDict<QListViewItem> m_itemdict;
	DOM::Node document;

	KHTMLPart *part;

};

#endif
