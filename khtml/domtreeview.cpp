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
    m_currentItem = 0;
    part = currentpart;
    connect(((const QObject *)part), SIGNAL(nodeActivated(const DOM::Node &)), this, SLOT(showTree(const DOM::Node &)));
}

DOMTreeView::~DOMTreeView()
{
}

void DOMTreeView::setRootNode(const DOM::Node &pNode)
{
    node = pNode;
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
    m_itemdict.clear();
    DOM::Node nNode = pNode;

    while (!(nNode.ownerDocument().isNull()))
	nNode = nNode.parentNode();
    treeWalker(nNode, pNode);
}

void DOMTreeView::treeWalker(const DOM::Node &pNode, const DOM::Node &sNode)
{
    if(pNode.isNull())
	return;
    QListViewItem *t_item = 0;
    if(m_currentItem == 0)
	t_item = new QListViewItem((QListView *) this, pNode.nodeName().string(), pNode.nodeValue().string());
    else
	t_item = new QListViewItem(m_currentItem, pNode.nodeName().string(), pNode.nodeValue().string());

    if(pNode.childNodes().length() != 0)
	t_item->setExpandable(true);
    DOM::Node t_node = pNode;
    m_itemdict.insert(&t_node, t_item);
    m_currentItem = t_item;
    DOM::Node t_child = pNode.firstChild();
    while(t_child != NULL)
    {
	treeWalker(t_child, sNode);
	t_child = t_child.nextSibling();
    }

    t_item->setOpen(true);
    if (sNode.handle() == pNode.handle())
	t_item->setSelected(true);
    m_currentItem = m_currentItem->parent();
}

void DOMTreeView::showPartTree()
{
    showTree(part->document());
}

void DOMTreeView::updateTree()
{
    clear();
    m_itemdict.clear();
    showTree(node);
}
