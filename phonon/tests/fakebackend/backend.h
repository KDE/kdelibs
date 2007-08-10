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

#include <QtCore/QList>
#include <QtCore/QPointer>
#include <QtCore/QStringList>

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
            Backend(QObject *parent = 0, const QStringList & = QStringList());
            virtual ~Backend();

            QObject *createObject(BackendInterface::Class, QObject *parent, const QList<QVariant> &args);

            bool supportsVideo() const;
            bool supportsOSD() const;
            bool supportsFourcc(quint32 fourcc) const;
            bool supportsSubtitles() const;
            QStringList availableMimeTypes() const;

            void freeSoundcardDevices();

            QSet<int> objectDescriptionIndexes(ObjectDescriptionType type) const;
            QHash<QByteArray, QVariant> objectDescriptionProperties(ObjectDescriptionType type, int index) const;

            bool startConnectionChange(QSet<QObject *>);
            bool connectNodes(QObject *, QObject *);
            bool disconnectNodes(QObject *, QObject *);
            bool endConnectionChange(QSet<QObject *>);

        Q_SIGNALS:
            void objectDescriptionChanged(ObjectDescriptionType);

        private:
            QStringList m_supportedMimeTypes;
            QList<QPointer<AudioOutput> > m_audioOutputs;
    };
}} // namespace Phonon::Fake

// vim: sw=4 ts=4 tw=80
#endif // Phonon_FAKE_BACKEND_H
