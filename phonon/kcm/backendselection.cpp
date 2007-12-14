/*  This file is part of the KDE project
    Copyright (C) 2004-2007 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) version 3.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "backendselection.h"

#include <kio/krun.h>
#include <kservicetypeprofile.h>
#include <kservicetypetrader.h>
#include <kconfig.h>
#include <QtCore/QStringList>
#include <QtGui/QListWidget>
#include <kapplication.h>
#include <kicon.h>
#include <QtCore/QList>
#include <QtDBus/QtDBus>
#include <kcmoduleproxy.h>

BackendSelection::BackendSelection(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    m_down->setIcon(KIcon("go-down"));
    m_up->setIcon(KIcon("go-up"));
    m_comment->setWordWrap(true);

    m_emptyPage = stackedWidget->addWidget(new QWidget());

    connect(m_select, SIGNAL(itemSelectionChanged()),
            SLOT(selectionChanged()));
    //connect(m_website, SIGNAL(leftClickedUrl(const QString  &)),
            //kapp, SLOT(invokeBrowser(const QString  &)));
    connect(m_up, SIGNAL(clicked()), SLOT(up()));
    connect(m_down, SIGNAL(clicked()), SLOT(down()));
}

void BackendSelection::load()
{
    const KService::List offers = KServiceTypeTrader::self()->query("PhononBackend",
            "Type == 'Service' and [X-KDE-PhononBackendInfo-InterfaceVersion] == 1");
    // the offers are already sorted for preference
    loadServices(offers);
    foreach (KCModuleProxy *proxy, m_kcms) {
        if (proxy) {
            proxy->load();
        }
    }
}

void BackendSelection::showBackendKcm(const KService::Ptr &backendService)
{
    QString parentComponent = backendService->library();
    if (!m_kcms.contains(parentComponent)) {
        const KService::List offers = KServiceTypeTrader::self()->query("KCModule",
                QString("'%1' in [X-KDE-ParentComponents]").arg(parentComponent));
        if (offers.isEmpty()) {
            m_kcms.insert(parentComponent, 0);
        } else {
            KCModuleProxy *proxy = new KCModuleProxy(offers.first());
            connect(proxy, SIGNAL(changed(bool)), SIGNAL(changed()));
            m_kcms.insert(parentComponent, proxy);
            stackedWidget->addWidget(proxy);
        }
    }
    QWidget *w = m_kcms.value(parentComponent);
    if (w) {
        stackedWidget->setCurrentWidget(w);
    } else {
        stackedWidget->setCurrentIndex(m_emptyPage);
    }
}

void BackendSelection::loadServices(const KService::List &offers)
{
    m_services.clear();
    m_select->clear();

    KService::List::const_iterator it = offers.begin();
    const KService::List::const_iterator end = offers.end();
    for (; it != end; ++it)
    {
        KService::Ptr service = *it;
        m_select->addItem(service->name());
        m_services[service->name()] = service;
    }
    m_select->setItemSelected(m_select->item(0), true);
}

void BackendSelection::save()
{
    // save embedded KCMs
    foreach (KCModuleProxy *proxy, m_kcms) {
        if (proxy) {
            proxy->save();
        }
    }

    // save to servicetype profile
    KService::List services;
    unsigned int count = m_select->count();
    for (unsigned int i = 0; i < count; ++i)
    {
        QListWidgetItem *item = m_select->item(i);
        KService::Ptr service = m_services[item->text()];
        services.append(service);
    }

    // get the currently used list
    const KService::List offers = KServiceTypeTrader::self()->query("PhononBackend",
            "Type == 'Service' and [X-KDE-PhononBackendInfo-InterfaceVersion] == 1");

    // we have to compare the lists manually as KService::Ptr::operator== is not what we want for
    // comparison
    if (offers.size() == services.size()) {
        bool equal = true;
        for (int i = 0; i < offers.size(); ++i) {
            if (offers[i]->entryPath() != services[i]->entryPath()) {
                equal = false;
                break;
            }
        }
        if (equal) {
            return;
        }
    }

    // be very conservative with this signal as it interrupts all playback:
    if (offers != services) {
        KServiceTypeProfile::writeServiceTypeProfile("PhononBackend", services);

        QDBusMessage signal = QDBusMessage::createSignal("/", "org.kde.Phonon.Factory", "phononBackendChanged");
        QDBusConnection::sessionBus().send(signal);
    }
}

void BackendSelection::defaults()
{
    foreach (KCModuleProxy *proxy, m_kcms) {
        if (proxy) {
            proxy->defaults();
        }
    }

    loadServices(KServiceTypeTrader::self()->defaultOffers("PhononBackend"));
}

void BackendSelection::selectionChanged()
{
    KService::Ptr service;
    foreach (QListWidgetItem *item, m_select->selectedItems()) {
        service = m_services[item->text()];
        m_up->setEnabled(m_select->row(item) > 0);
        m_down->setEnabled(m_select->row(item) < m_select->count() - 1);
        break;
    }
    if(service) {
        m_icon->setPixmap(KIcon(service->icon()).pixmap(128));
        m_name->setText(QString());//service->name());
        m_comment->setText(service->comment());
        const QString website = service->property("X-KDE-PhononBackendInfo-Website").toString();
        m_website->setText(QString("<a href=\"%1\">%1</a>").arg(website));
        connect(m_website, SIGNAL(linkActivated(const QString &)), SLOT(openWebsite(const QString &)));
        m_version->setText(service->property("X-KDE-PhononBackendInfo-Version").toString());
        showBackendKcm(service);
    } else {
        m_up->setEnabled(false);
        m_down->setEnabled(false);
    }
}

void BackendSelection::openWebsite(const QString &url)
{
    new KRun(KUrl(url), window());
}

void BackendSelection::up()
{
    QList<QListWidgetItem *> selectedList = m_select->selectedItems();
    foreach (QListWidgetItem *selected, selectedList)
    {
        int row = m_select->row(selected);
        if (row > 0)
        {
            QListWidgetItem *taken = m_select->takeItem(row - 1);
            m_select->insertItem(row, taken);
            emit changed();
            selectionChanged();
        }
    }
}

void BackendSelection::down()
{
    QList<QListWidgetItem *> selectedList = m_select->selectedItems();
    foreach (QListWidgetItem *selected, selectedList)
    {
        int row = m_select->row(selected);
        if (row + 1 < m_select->count())
        {
            QListWidgetItem *taken = m_select->takeItem(row + 1);
            m_select->insertItem(row, taken);
            emit changed();
            selectionChanged();
        }
    }
}

#include "backendselection.moc"

// vim: sw=4 ts=4
