/* This file is part of the KDE project
 *
 * Copyright (C) 2008 Jakub Stachowski <qbast@go2.pl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "servicesaction.h"
#include "servicesaction_p.h"
#include "servicebrowser.h"
#include <QMenu>

namespace DNSSD
{

ServicesAction::ServicesAction(ServiceBrowser* browser, QObject* parent) 
    :  QAction(parent), d(new ServicesActionPrivate(browser, this))
{}

ServicesAction::ServicesAction(ServiceBrowser* browser, const QString& text, QObject* parent) 
    :  QAction(text ,parent), d(new ServicesActionPrivate(browser, this))
{}

ServicesAction::ServicesAction(ServiceBrowser* browser, const QIcon& icon,const QString& text, QObject* parent) 
    :  QAction(icon, text ,parent), d(new ServicesActionPrivate(browser, this))
{}


ServicesAction::~ServicesAction()
{
    delete d;
}

void ServicesActionPrivate::serviceSelected()
{
    QAction* action=static_cast<QAction*>(sender());
    RemoteService::Ptr srv=action->data().value<RemoteService::Ptr>();
    emit m_parent->selected(srv);
}

void ServicesActionPrivate::serviceAdded(RemoteService::Ptr srv)
{
    if (!m_parent->isEnabled()) m_parent->setEnabled(true);
    QAction* act=new QAction(srv->serviceName(), m_parent);
    QVariant v;
    v.setValue(srv);
    act->setData(v);
    connect(act, SIGNAL(triggered(bool)), this, SLOT(serviceSelected()));
    m_menu->addAction(act);
}

void ServicesActionPrivate::serviceRemoved(RemoteService::Ptr srv)
{
    Q_FOREACH(QAction* act, m_menu->actions())   {
	RemoteService::Ptr p=act->data().value<RemoteService::Ptr>();
	if (act->data().value<RemoteService::Ptr>()==srv) {
	    m_menu->removeAction(act);
	    act->deleteLater();
	    break;
	}
    }
    if (m_menu->actions().empty()) m_parent->setEnabled(false);
}

ServicesActionPrivate::ServicesActionPrivate(ServiceBrowser* browser, ServicesAction* parent)  : 
    m_browser(browser), m_menu(new QMenu()), m_parent(parent)
{
    m_browser->setParent(this);
    connect(browser, SIGNAL(serviceAdded(DNSSD::RemoteService::Ptr)), this,
	SLOT(serviceAdded(DNSSD::RemoteService::Ptr)));
    connect(browser, SIGNAL(serviceRemoved(DNSSD::RemoteService::Ptr)), this,
	SLOT(serviceRemoved(DNSSD::RemoteService::Ptr)));
    browser->startBrowse();
    m_parent->setMenu(m_menu);
    m_parent->setEnabled(false);
    Q_FOREACH (RemoteService::Ptr srv, browser->services()) serviceAdded(srv);
}

ServicesActionPrivate::~ServicesActionPrivate()
{
    delete m_menu;
}

}
