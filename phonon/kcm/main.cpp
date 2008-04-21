/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Matthias Kretz <kretz@kde.org>

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

#include "main.h"
#include <kgenericfactory.h>
#include <ktabwidget.h>
#include <kaboutdata.h>
#include <kdeversion.h>
#include <QtGui/QBoxLayout>
#include <kdialog.h>
#include "devicepreference.h"
#include "backendselection.h"

K_PLUGIN_FACTORY(PhononKcmFactory, registerPlugin<PhononKcm>();)
K_EXPORT_PLUGIN(PhononKcmFactory("kcm_phonon"))

PhononKcm::PhononKcm(QWidget *parent, const QVariantList &args)
    : KCModule(PhononKcmFactory::componentData(), parent, args)
{
    KAboutData *about = new KAboutData(
            "kcm_phonon", 0, ki18n("Phonon Configuration Module"),
            KDE_VERSION_STRING, KLocalizedString(), KAboutData::License_GPL,
            ki18n("Copyright 2006 Matthias Kretz"));
    about->addAuthor(ki18n("Matthias Kretz"), KLocalizedString(), "kretz@kde.org");
    setAboutData(about);

    setLayout(new QHBoxLayout);
    layout()->setMargin(0);
    layout()->setSpacing(0);

    KTabWidget *tabs = new KTabWidget(this);
    layout()->addWidget(tabs);

    m_devicePreferenceWidget = new DevicePreference;
    tabs->addTab(m_devicePreferenceWidget, i18n("Device Preference"));
    m_backendSelection = new BackendSelection;
    tabs->addTab(m_backendSelection, i18n("Backend"));
    load();
    connect(m_backendSelection, SIGNAL(changed()), SLOT(changed()));
    connect(m_devicePreferenceWidget, SIGNAL(changed()), SLOT(changed()));
    setButtons( KCModule::Default|KCModule::Apply );
}

void PhononKcm::load()
{
    m_devicePreferenceWidget->load();
    m_backendSelection->load();
}

void PhononKcm::save()
{
    m_devicePreferenceWidget->save();
    m_backendSelection->save();
}

void PhononKcm::defaults()
{
    m_devicePreferenceWidget->defaults();
    m_backendSelection->defaults();
}

#include "main.moc"
// vim: ts=4
