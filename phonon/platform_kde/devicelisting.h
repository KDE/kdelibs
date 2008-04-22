/*  This file is part of the KDE project
    Copyright (C) 2008 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) version 3.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef DEVICELISTING_H
#define DEVICELISTING_H

#include <QtCore/QBasicTimer>
#include <QtCore/QByteArray>
#include <QtCore/QHash>
#include <QtCore/QMap>
#include <QtCore/QMultiMap>
#include <QtCore/QObject>
#include <QtCore/QVariant>

#include "../objectdescription.h"

namespace Phonon
{

class AudioDevice;

class DeviceListing : public QObject
{
    Q_OBJECT
    public:
        DeviceListing();
        ~DeviceListing();

        QList<int> objectDescriptionIndexes(Phonon::ObjectDescriptionType type);
        QHash<QByteArray, QVariant> objectDescriptionProperties(Phonon::ObjectDescriptionType type, int index);

    public slots:
        Q_SCRIPTABLE void ossSettingChanged(bool useOss);

    signals:
        void objectDescriptionChanged(ObjectDescriptionType);

    protected:
        void timerEvent(QTimerEvent *e);

    private slots:
        void devicePlugged(const Phonon::AudioDevice &);
        void deviceUnplugged(const Phonon::AudioDevice &);

    private:
        void checkAudioOutputs();
        void checkAudioInputs();
        QMultiMap<int, int> m_sortedOutputIndexes;
        QMultiMap<int, int> m_sortedInputIndexes;
        QHash<int, QHash<QByteArray, QVariant> > m_outputInfos;
        QHash<int, QHash<QByteArray, QVariant> > m_inputInfos;
        QBasicTimer m_signalTimer;
        bool m_useOss : 1;
};

} // namespace Phonon

#endif // DEVICELISTING_H
