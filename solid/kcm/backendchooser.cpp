/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.

*/

#include "backendchooser.h"

#include <kservicetypeprofile.h>
#include <kservicetype.h>
#include <kservicetypetrader.h>
#include <kconfig.h>
#include <QStringList>
#include <QListWidgetItem>
#include <kapplication.h>
#include <kicon.h>
#include <QList>


static bool equals(const KService::List &list1, const KService::List &list2)
{
    if (list1.size()!=list2.size())
    {
        return false;
    }

    for (int i=0; i<list1.size(); ++i)
    {
        if (list1[i]->name()!=list2[i]->name())
        {
            return false;
        }
    }

    return true;
}


BackendChooser::BackendChooser(QWidget *parent, const QString &backendType)
    : QWidget(parent), m_backendType(backendType)
{
    m_ui.setupUi(this);

    connect(m_ui.listView, SIGNAL(itemSelectionChanged()),
             this, SLOT(slotSelectionChanged()));
    connect(m_ui.upButton, SIGNAL(clicked()),
             this, SLOT(slotUpClicked()));
    connect(m_ui.downButton, SIGNAL(clicked()),
             this, SLOT(slotDownClicked()));

    KServiceType::Ptr service_type = KServiceType::serviceType(backendType);

    QString title = service_type->comment();

    if (title.isEmpty())
    {
        title = backendType;
    }

    m_ui.groupBox->setTitle(title);
}

void BackendChooser::load()
{
    m_initServices = KServiceTypeTrader::self()->query(m_backendType, "Type == 'Service'");

    loadServicesInView(m_initServices);
}

void BackendChooser::loadServicesInView(const KService::List &services)
{
    m_services.clear();
    m_ui.listView->clear();

    foreach (KService::Ptr service, services)
    {
        m_ui.listView->addItem(service->name());
        m_services[service->name()] = service;
    }

    m_ui.listView->setItemSelected(m_ui.listView->item(0), true);
}

KService::List BackendChooser::servicesFromView()
{
    KService::List services;

    int count = m_ui.listView->count();

    for (int i=0; i<count; ++i)
    {
        QListWidgetItem *item = m_ui.listView->item(i);
        KService::Ptr service = m_services[item->text()];
        services.append(service);
    }

    return services;
}

void BackendChooser::save()
{
    if (!equals(m_initServices, servicesFromView()))
    {
        m_initServices = servicesFromView();
        KServiceTypeProfile::writeServiceTypeProfile(m_backendType, m_initServices);
        emit changed(false);
    }
}

void BackendChooser::defaults()
{
    KService::List currentServices = servicesFromView();
    KService::List defaultServices = KServiceTypeTrader::self()->defaultOffers(m_backendType, "Type == 'Service'");

    if (!equals(currentServices, defaultServices))
    {
        loadServicesInView(KServiceTypeTrader::self()->defaultOffers(m_backendType, "Type == 'Service'"));
        emit changed(!equals(m_initServices, servicesFromView()));
    }
}

void BackendChooser::slotSelectionChanged()
{
    KService::Ptr service;

    for (int i=0; i<m_ui.listView->count(); ++i)
    {
        QListWidgetItem *item = m_ui.listView->item(i);

        if (m_ui.listView->isItemSelected(item))
        {
            service = m_services[item->text()];
            break;
        }
    }

    if (service)
    {
        m_ui.iconLabel->setPixmap(KIcon(service->icon()).pixmap(32));
        m_ui.nameLabel->setText(service->name());
        m_ui.commentLabel->setText(service->comment());
        m_ui.versionLabel->setText(service->property("X-KDE-SolidBackendInfo-Version").toString());
    }
}

void BackendChooser::slotUpClicked()
{
    QList<QListWidgetItem *> selectedList = m_ui.listView->selectedItems();

    foreach (QListWidgetItem *selected, selectedList)
    {
        int row = m_ui.listView->row(selected);

        if (row > 0)
        {
            QListWidgetItem *taken = m_ui.listView->takeItem(row - 1);
            m_ui.listView->insertItem(row, taken);

            emit changed(!equals(m_initServices, servicesFromView()));
            return;
        }
    }
}

void BackendChooser::slotDownClicked()
{
    QList<QListWidgetItem *> selectedList = m_ui.listView->selectedItems();

    foreach (QListWidgetItem *selected, selectedList)
    {
        int row = m_ui.listView->row(selected);

        if (row + 1 < m_ui.listView->count())
        {
            QListWidgetItem *taken = m_ui.listView->takeItem(row + 1);
            m_ui.listView->insertItem(row, taken);

            emit changed(!equals(m_initServices, servicesFromView()));
            return;
        }
    }
}

#include "backendchooser.moc"
