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

#ifndef KCMSOLID_H
#define KCMSOLID_H

#include <kcmodule.h>

class BackendChooser;

class KcmSolid : public KCModule
{
    Q_OBJECT
public:
    KcmSolid(QWidget *parent, const QStringList &);

    virtual void load();
    virtual void save();
    virtual void defaults();

private Q_SLOTS:
    void slotChooserChanged(bool state);

private:
    int m_changedChooser;

    BackendChooser *m_powerChooser;
    BackendChooser *m_networkChooser;
    BackendChooser *m_bluetoothChooser;
};

#endif
