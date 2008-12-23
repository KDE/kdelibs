/* This file is part of the KDE project
   Copyright (C) 2001 Simon Hausmann <hausmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KPARTS_BROWSERINTERFACE_H
#define KPARTS_BROWSERINTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QVariant>

#include <kparts/kparts_export.h>

namespace KParts
{

/**
 * The purpose of this interface is to allow a direct communication between
 * a KPart and the hosting browser shell (for example Konqueror) . A
 * shell implementing this interface can propagate it to embedded kpart
 * components by using the setBrowserInterface call of the part's
 * KParts::BrowserExtension object.
 *
 * This interface looks not very rich, but the main functionality is
 * implemented using the callMethod method for part->shell
 * communication and using Qt properties for allowing a part to
 * to explicitly query information from the shell.
 *
 * Konqueror in particular, as 'reference' implementation, provides
 * the following functionality through this interface:
 *
 * Qt properties:
 *   Q_PROPERTY( uint historyLength READ historyLength );
 *
 * Callable methods:
 *       void goHistory( int );
 *
 */
class KPARTS_EXPORT BrowserInterface : public QObject
{
    Q_OBJECT
public:
    explicit BrowserInterface( QObject *parent );
    virtual ~BrowserInterface();

    /**
     * Perform a dynamic invocation of a method in the BrowserInterface
     * implementation. Methods are to be implemented as simple Qt slots.
     * You should only include the method name, and not the signature, 
     * in the name argument.
     */
    void callMethod( const char *name, const QVariant &argument );
};

}

#endif
