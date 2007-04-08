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

#ifndef PHONON_OVERLAYAPI_H
#define PHONON_OVERLAYAPI_H

#include <QObject>
#include "export.h"

namespace Phonon
{
class VideoWidget;
namespace Experimental
{
class OverlayApiPrivate;

class PHONONEXPERIMENTAL_EXPORT OverlayApi : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(OverlayApi)
    Q_ENUMS(OverlayType)
    public:
        OverlayApi(VideoWidget *parent);

        ~OverlayApi();

        /**
         * Defines the different overlay types.
         */
        enum OverlayType {
            /**
             * No overlay.
             */
            OverlayNone = 0,
            /**
             * Scaled overlay (with semi-transparency).
             */
            OverlayScaled = 1,
            /**
             * Unscaled overlay (without semi-transparency).
             */
            OverlayOpaque = 2,
            /**
             * Unscaled overlay (with semi-transparency).
             */
            OverlayFull = 4
        };
        Q_DECLARE_FLAGS(OverlayTypes, OverlayType)

        /**
         * Query the overlay capabilities of the VideoWidget.
         */
        OverlayTypes overlayCapabilities() const;

        /**
         * Creates an overlay (takes ownership of the widget).
         * Note that you can only have one overlay per video widget.
         * You can remove the overlay either by deleting or by reparenting
         * the widget.
         * @return whether the creation was successful
         */
        bool createOverlay(QWidget *widget, OverlayType type);

    protected:
        OverlayApiPrivate *const d_ptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(OverlayApi::OverlayTypes)

} // namespace Experimental
} // namespace Phonon
#endif // PHONON_OVERLAYAPI_H
