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
    part = currentpart;
    connect(((const QObject *)part), SIGNAL(sigNodeSelected(const DOM::Node &)), this, SLOT(showTree(const DOM::Node &)));
}

DOMTreeView::~DOMTreeView()
{
}

void DOMTreeView::setRootNode(const DOM::Node &pNode)
{
    document = pNode;
}

void DOMTreeView::selectNode(DOM::Node pNode)
{
    kdDebug() << "Node selected!" << endl;
    QListViewItem *t_selItem = m_itemdict[&pNode];
    setSelected(t_selItem,true);
}

void DOMTreeView::showTree(const DOM::Node &pNode)
{
    clear();
    if (document.isNull() || document.handle()!=pNode.ownerDocument().handle())
    {
	m_itemdict.clear();
	kdDebug()<<"starting new treeview.\n";
	if (!pNode.ownerDocument().isNull())
	    recursive(0, pNode.ownerDocument(), pNode);
	else
	    recursive(0, pNode, 0);
    }
    else
    {
	ensureItemVisible(m_itemdict[pNode.handle()]);
	m_itemdict[pNode.handle()]->setSelected(true);
    }
}

void DOMTreeView::recursive(const DOM::Node &pNode, const DOM::Node &node, const DOM::Node &sNode)
{
    kdDebug()<<"recursing into "<<node.nodeName().string()<<endl;
    QListViewItem *cur_item;
    if(node.ownerDocument().isNull())
    {
	cur_item = new QListViewItem((QListView *) this, node.nodeName().string(), node.nodeValue().string());
	document = pNode.ownerDocument();
    }
    else
	cur_item = new QListViewItem(m_itemdict[pNode.handle()], node.nodeName().string(), node.nodeValue().string());

    if(node.childNodes().length() != 0)
	cur_item->setExpandable(true);

    m_itemdict.insert(node.handle(), cur_item);

    DOM::Node cur_child = node.firstChild();
    while (!cur_child.isNull())
    {
	recursive(node, cur_child, sNode);
	cur_child = cur_child.nextSibling();
    }

    cur_item->setOpen(true);
    if (sNode.handle() == node.handle())
	cur_item->setSelected(true);
}

void DOMTreeView::showPartTree()
{
    showTree(part->document());
}

void DOMTreeView::updateTree()
{
    clear();
    m_itemdict.clear();
    showTree(document);
}
