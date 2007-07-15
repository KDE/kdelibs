/*  This file is part of the KDE project
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library.  If not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this library
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KDE_NETWORKSTATUS_INDICATOR_H
#define KDE_NETWORKSTATUS_INDICATOR_H

#include <QtGui/QWidget>
#include <kio/kio_export.h>
#include <solid/networking.h>

/**
 * Widget indicating network connection status using an icon and tooltip.  This widget uses
 * Solid::Networking internally to automatically show and hide itself as required.
 *
 * @code
 * KStatusBarOfflineIndicator * indicator = new KStatusBarOfflineIndicator( this );
 * statusBar()->addWidget( indicator, 0, false );
 * @endcode
 *
 * @author Will Stephenson <wstephenson@kde.org>
 */
class KDE_EXPORT KStatusBarOfflineIndicator : public QWidget
{
Q_OBJECT
public:
    /**
     * Default constructor.
     * @param parent the widget's parent
     */
    explicit KStatusBarOfflineIndicator( QWidget * parent );
    ~KStatusBarOfflineIndicator();
protected Q_SLOTS:
    /**
     * Hides or shows the widget, depending on the current state of the network service.
     */
    void networkStatusChanged( Solid::Networking::Status status );
    void initialize();
private:
};

#endif

