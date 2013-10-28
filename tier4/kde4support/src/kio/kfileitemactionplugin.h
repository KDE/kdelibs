/* This file is part of the KDE project
   Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>
   Based on konq_popupmenuplugin.h Copyright 2008 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2 of the License or
   ( at your option ) version 3 or, at the discretion of KDE e.V.
   ( which shall act as a proxy as in section 14 of the GPLv3 ), any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KFILEITEMACTION_PLUGIN_H
#define KFILEITEMACTION_PLUGIN_H

#include <kde4support_export.h>
#include <QtCore/QObject>

class QAction;
class QMenu;
class QWidget;
class KFileItemListProperties;

/**
 * @deprecated use KAbstractFileItemActionPlugin instead
 * (introduced in kdelibs-4.6.1 in order to remove the erroneous
 * "const" in the actions method)
 */
class KDE4SUPPORT_DEPRECATED_EXPORT KFileItemActionPlugin : public QObject
{
    Q_OBJECT

public:
    KFileItemActionPlugin(QObject *parent);

    virtual ~KFileItemActionPlugin();

    virtual QList<QAction*> actions(const KFileItemListProperties &fileItemInfos,
                                    QWidget *parentWidget) const = 0;
};

#endif
