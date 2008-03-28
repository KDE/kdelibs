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
#ifndef PHONON_EXPERIMENTAL_FACTORY_P_H
#define PHONON_EXPERIMENTAL_FACTORY_P_H

#include <QtCore/QObject>
#include "export.h"

namespace Phonon
{
namespace Experimental
{
namespace Factory
{
    /**
     * Emits signals for Phonon::Experimental::Factory.
     */
    class Sender : public QObject
    {
        Q_OBJECT
        Q_SIGNALS:
            /**
             * \copydoc Phonon::Experimental::BackendCapabilities::Notifier::availableVideoCaptureDevicesChanged
             */
            void availableVideoCaptureDevicesChanged();
    };

    PHONONEXPERIMENTAL_EXPORT Sender *sender();

    /**
     * Create a new backend object for a VideoDataOutput.
     *
     * \return a pointer to the VideoDataOutput the backend provides.
     */
    PHONONEXPERIMENTAL_EXPORT QObject *createVideoDataOutput(QObject *parent = 0);

} // namespace Factory
} // namespace Experimental
} // namespace Phonon

#endif // PHONON_EXPERIMENTAL_FACTORY_P_H
