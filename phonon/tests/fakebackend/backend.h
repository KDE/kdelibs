/*  This file is part of the KDE project
    Copyright (C) 2004-2006 Matthias Kretz <kretz@kde.org>

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

#ifndef Phonon_FAKE_BACKEND_H
#define Phonon_FAKE_BACKEND_H

#include <phonon/objectdescription.h>
#include <phonon/backendinterface.h>

#include <QList>
#include <QPointer>
#include <QStringList>

class KUrl;

namespace Phonon
{
namespace Fake
{
    class AudioOutput;

    class Backend : public QObject, public BackendInterface
    {
        Q_OBJECT
        Q_INTERFACES(Phonon::BackendInterface)
        public:
            Backend(QObject *parent, const QStringList &args);
            virtual ~Backend();

            QObject *createObject(BackendInterface::Class, QObject *parent, const QList<QVariant> &args);

            Q_INVOKABLE bool supportsVideo() const;
            Q_INVOKABLE bool supportsOSD() const;
            Q_INVOKABLE bool supportsFourcc(quint32 fourcc) const;
            Q_INVOKABLE bool supportsSubtitles() const;
            Q_INVOKABLE QStringList availableMimeTypes() const;

            void freeSoundcardDevices();

            QSet<int> objectDescriptionIndexes(ObjectDescriptionType type) const;
            QHash<QByteArray, QVariant> objectDescriptionProperties(ObjectDescriptionType type, int index) const;

        Q_SIGNALS:
            void objectDescriptionChanged(ObjectDescriptionType);

        private:
            QStringList m_supportedMimeTypes;
            QList<QPointer<AudioOutput> > m_audioOutputs;
    };
}} // namespace Phonon::Fake

// vim: sw=4 ts=4 tw=80
#endif // Phonon_FAKE_BACKEND_H
