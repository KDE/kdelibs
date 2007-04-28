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

#include "kcmsolid.h"

#include <kgenericfactory.h>

#include <kaboutdata.h>
#include <kdeversion.h>

#include <QVBoxLayout>

#include "backendchooser.h"


typedef KGenericFactory<KcmSolid, QWidget> KcmSolidFactory;
K_EXPORT_COMPONENT_FACTORY(kcm_solid, KcmSolidFactory("kcm_solid"))


KcmSolid::KcmSolid(QWidget *parent, const QStringList &args)
    : KCModule(KcmSolidFactory::componentData(), parent, args),
      m_changedChooser(0)
{
    KAboutData *about = new KAboutData(
        "kcm_solid", I18N_NOOP("Solid Configuration Module"),
        KDE_VERSION_STRING, 0, KAboutData::License_GPL,
        I18N_NOOP("Copyright 2006 Kevin Ottens"));
    about->addAuthor("Kevin Ottens", 0, "ervin@kde.org");
    setAboutData(about);

    setLayout(new QVBoxLayout);
    layout()->setMargin(0);
    layout()->setSpacing(0);

    m_powerChooser = new BackendChooser(this, "SolidPowerManager");
    m_networkChooser = new BackendChooser(this, "SolidNetworkManager");
    m_bluetoothChooser = new BackendChooser(this, "SolidBluetoothManager");

    layout()->addWidget(m_powerChooser);
    layout()->addWidget(m_networkChooser);
    layout()->addWidget(m_bluetoothChooser);

    load();

    connect(m_powerChooser, SIGNAL(changed(bool)),
             this, SLOT(slotChooserChanged(bool)));
    connect(m_networkChooser, SIGNAL(changed(bool)),
             this, SLOT(slotChooserChanged(bool)));
    connect(m_bluetoothChooser, SIGNAL(changed(bool)),
             this, SLOT(slotChooserChanged(bool)));

}

void KcmSolid::load()
{
    m_powerChooser->load();
    m_networkChooser->load();
    m_bluetoothChooser->load();
}

void KcmSolid::save()
{
    m_powerChooser->save();
    m_networkChooser->save();
    m_bluetoothChooser->save();
}

void KcmSolid::defaults()
{
    m_powerChooser->defaults();
    m_networkChooser->defaults();
    m_bluetoothChooser->defaults();
}

void KcmSolid::slotChooserChanged(bool state)
{
    if (state)
    {
        m_changedChooser++;
    }
    else
    {
        m_changedChooser--;
    }


    emit changed(m_changedChooser!= 0);
}

#include "kcmsolid.moc"
