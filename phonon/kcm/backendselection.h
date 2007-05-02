/*  This file is part of the KDE project
    Copyright (C) 2004,2006 Matthias Kretz <kretz@kde.org>

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

#ifndef BACKENDSELECTION_H
#define BACKENDSELECTION_H

#include "ui_backendselection.h"
#include <QtGui/QWidget>
#include <QtCore/QHash>


#include <kservice.h>
class KCModuleProxy;

class BackendSelection : public QWidget, private Ui::BackendSelection
{
    Q_OBJECT
    public:
        BackendSelection(QWidget *parent = 0);

        void load();
        void save();
        void defaults();

    private Q_SLOTS:
        void selectionChanged();
        void up();
        void down();

 Q_SIGNALS:
        void changed();

    private:
        void showBackendKcm(const KService::Ptr &backendService);
        void loadServices(const KService::List &offers);
        QHash<QString, KService::Ptr> m_services;
        QHash<QString, KCModuleProxy *> m_kcms;
        int m_emptyPage;
};

// vim: sw=4 ts=4 tw=80
#endif // BACKENDSELECTION_H
