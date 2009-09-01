/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef KCMULTIDIALOG_P_H
#define KCMULTIDIALOG_P_H

#include "kcmultidialog.h"
#include <kpagedialog_p.h>
#include <QtCore/QList>
#include <QtCore/QStringList>

class KCModuleProxy;
class KPageWidgetItem;

class KCMultiDialogPrivate : public KPageDialogPrivate
{
    Q_DECLARE_PUBLIC(KCMultiDialog)
    protected:
        KCMultiDialogPrivate()
            : currentModule(0)
        {
        }

        KCModuleProxy *currentModule;

        struct CreatedModule
        {
            KCModuleProxy *kcm;
            KPageWidgetItem *item;
            QStringList componentNames;
        };

        typedef QList<CreatedModule> ModuleList;
        ModuleList modules;

        void _k_slotCurrentPageChanged(KPageWidgetItem *);
        virtual void _k_clientChanged();
        void _k_dialogClosed();
        void _k_updateHeader(bool use, const QString &message);

        void updateButtons(KCModuleProxy *currentModule);
    private:
        void init();
        void apply();
};

#endif // KCMULTIDIALOG_P_H
// vim: sw=4 sts=4 et tw=100
