/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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

#include "main.h"
#include <kgenericfactory.h>
#include <kaboutdata.h>
#include <kdeversion.h>
#include <QtGui/QBoxLayout>
#include <kdialog.h>
#include "outputdevicechoice.h"
#include <QtGui/QTabWidget>
#include "backendselection.h"

typedef KGenericFactory<PhononKcm, QWidget> PhononKcmFactory;
K_EXPORT_COMPONENT_FACTORY(kcm_phonon, PhononKcmFactory("kcm_phonon"))

PhononKcm::PhononKcm(QWidget *parent, const QStringList &args)
    : KCModule(PhononKcmFactory::componentData(), parent, args)
{
    KAboutData *about = new KAboutData(
            "kcm_phonon", I18N_NOOP("Phonon Configuration Module"),
            KDE_VERSION_STRING, 0, KAboutData::License_GPL,
            I18N_NOOP("Copyright 2006 Matthias Kretz"));
    about->addAuthor("Matthias Kretz", 0, "kretz@kde.org");
    setAboutData(about);

    setLayout(new QHBoxLayout);
    layout()->setMargin(0);
    layout()->setSpacing(0);

    QTabWidget *tabs = new QTabWidget(this);
    layout()->addWidget(tabs);

    m_outputDeviceWidget = new OutputDeviceChoice;
    tabs->addTab(m_outputDeviceWidget, i18n("Device Preference"));
    m_backendSelection = new BackendSelection;
    tabs->addTab(m_backendSelection, i18n("Backend"));
    load();
    connect(m_backendSelection, SIGNAL(changed()), SLOT(changed()));
    connect(m_outputDeviceWidget, SIGNAL(changed()), SLOT(changed()));
}

void PhononKcm::load()
{
    m_outputDeviceWidget->load();
    m_backendSelection->load();
}

void PhononKcm::save()
{
    m_outputDeviceWidget->save();
    m_backendSelection->save();
}

void PhononKcm::defaults()
{
    m_outputDeviceWidget->defaults();
    m_backendSelection->defaults();
}

#include "main.moc"
// vim: ts=4
