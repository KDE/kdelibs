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

#include "khtml_part.h"
 
#include "domtreeview.moc"

DOMTreeView::DOMTreeView(QWidget *parent, KHTMLPart *currentpart, const char * name) : KListView(parent, name)
{
    setCaption(name);
    setRootIsDecorated(true);
    addColumn("Name");
    addColumn("Value");
    setSorting(-1);
    part = currentpart;
    connect(((const QObject *)part), SIGNAL(sigNodeActivated(const DOM::Node &)), this, SLOT(showTree(const DOM::Node &)));
}

DOMTreeView::~DOMTreeView()
{
    disconnect(part);
}

void DOMTreeView::showTree(const DOM::Node &pNode)
{
    if (pNode.isNull())
    {
	kdDebug()<<"Null node selected!"<<endl;
	return;
    }
    if (document != pNode.ownerDocument())
    {
	kdDebug()<<"document has changed! "<<endl;
	kdDebug()<<"node at:"<<pNode.handle()<<endl;
	clear();
	m_itemdict.clear();
	if (!pNode.ownerDocument().isNull())
	    recursive(0, pNode.ownerDocument());
	else
	    recursive(0, pNode);
    }
    setCurrentItem(m_itemdict[pNode.handle()]);
    ensureItemVisible(m_itemdict[pNode.handle()]);
}

void DOMTreeView::recursive(const DOM::Node &pNode, const DOM::Node &node)
{
    QListViewItem *cur_item;
    if(node.ownerDocument().isNull())
    {
	cur_item = new QListViewItem((QListView *) this, node.nodeName().string(), node.nodeValue().string());
	document = node;
    }
    else
	cur_item = new QListViewItem(m_itemdict[pNode.handle()], node.nodeName().string(), node.nodeValue().string());

    m_itemdict.insert(node.handle(), cur_item);

    DOM::Node cur_child = node.lastChild();
    while (!cur_child.isNull())
    {
	recursive(node, cur_child);
	cur_child = cur_child.previousSibling();
    }
}

