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

#ifndef KCMUTILS_KCMODULEPROXY_P_H
#define KCMUTILS_KCMODULEPROXY_P_H

#include "kcmoduleinfo.h"
#include "kcmoduleproxy.h"
#include <QtGui/QLabel>
class QVBoxLayout;

class KCModuleProxyPrivate
{
    Q_DECLARE_PUBLIC(KCModuleProxy)
    protected:
        KCModuleProxyPrivate(KCModuleProxy *_parent, const KCModuleInfo &info, const QStringList &_args)
            : args(_args), kcm(0), topLayout(0), rootInfo(0), modInfo(info),
            changed(false), bogusOccupier(false), parent(_parent)
        {
        }

        ~KCModuleProxyPrivate()
        {
            delete rootInfo; // Delete before embedWidget!
            delete kcm;
        }

        void loadModule();

        /**
         * Makes sure the proper variables is set and signals are emitted.
         */
        void _k_moduleChanged(bool);

        /**
         * Zeroes d->kcm
         */
        void _k_moduleDestroyed();

        /**
         * Gets called by DCOP when an application closes.
         * Is used to (try to) reload a KCM which previously
         * was loaded.
         */
        void _k_ownerChanged(const QString &service, const QString &oldOwner, const QString &newOwner);

        QStringList args;
        KCModule *kcm;
        QVBoxLayout *topLayout; /* Contains QScrollView view, and root stuff */
        QLabel *rootInfo;
        QString dbusService;
        QString dbusPath;
        KCModuleInfo modInfo;
        bool changed;
        bool bogusOccupier;
        KCModuleProxy *parent;
        KCModuleProxy *q_ptr;
};

#endif // KCMUTILS_KCMODULEPROXY_P_H
// vim: sw=4 sts=4 et tw=100
