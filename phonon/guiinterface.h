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

#ifndef GUIINTERFACE_H
#define GUIINTERFACE_H

#include "phonon_export.h"
#include <QStringList>
#include <QString>
#include <QVariant>

class QObject;

namespace Phonon
{

class KioFallback;
class MediaObject;

class PHONONCORE_EXPORT GuiInterface
{
    public:
        virtual ~GuiInterface();

        static GuiInterface *instance();
        virtual QVariant icon(const QString &name) { Q_UNUSED(name); return QVariant(); }

        virtual void notification(const char *notificationName, const QString &text,
                const QStringList &actions = QStringList(), QObject *receiver = 0,
                const char *actionSlot = 0);

        virtual KioFallback *newKioFallback(MediaObject *parent);

    private:
        static GuiInterface *s_instance;
};
extern PHONONCORE_EXPORT GuiInterface *_kde_internal_GuiInterface;

} // namespace Phonon

#endif // GUIINTERFACE_H
