/*  This file is part of the KDE project
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef WEBCAMWATCHER_H
#define WEBCAMWATCHER_H

#include <QObject>
#include <QStringList>

namespace Solid
{
    class Device;
} // namespace Solid

class WebcamWatcher : public QObject
{
    Q_OBJECT
    public:
        WebcamWatcher( QObject * parent = 0 );
        ~WebcamWatcher();
    protected:
        void getDetails( const Solid::Device & dev );
    protected slots:
        void deviceAdded( const QString& udi );
        void deviceRemoved( const QString& udi );
    private:
        QStringList m_videoDevices;
};

#endif
