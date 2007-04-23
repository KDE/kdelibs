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

#ifndef BACKENDCHOOSER_H
#define BACKENDCHOOSER_H

#include "ui_backendchooser.h"

#include <QWidget>
#include <QHash>
#include <QString>

#include <kservice.h>

class BackendChooser : public QWidget
{
    Q_OBJECT
public:
    BackendChooser(QWidget *parent, const QString &backendType);

    void load();
    void save();
    void defaults();

private Q_SLOTS:
    void slotSelectionChanged();
    void slotUpClicked();
    void slotDownClicked();

Q_SIGNALS:
    void changed(bool state);

private:
    void loadServicesInView(const KService::List &offers);
    KService::List servicesFromView();

    Ui::BackendChooser m_ui;
    KService::List m_initServices;

    QString m_backendType;
    QHash<QString, KService::Ptr> m_services;
};

#endif
