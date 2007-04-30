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

#include "overlayapi.h"
#include "overlayapi_p.h"
#include "../base_p.h"
#include "../phonondefs_p.h"
#include "../videowidget.h"
#include "../abstractvideooutput_p.h"

#include <QVariant>

namespace Phonon
{
namespace Experimental
{

OverlayApi::OverlayApi(VideoWidget *parent)
    : QObject(parent),
    k_ptr(new OverlayApiPrivate)
{
    K_D(OverlayApi);
    d->q_ptr = this;
    d->videowidget = parent;
}

OverlayApi::~OverlayApi()
{
    delete k_ptr;
}

OverlayApi::OverlayTypes OverlayApi::overlayCapabilities() const
{
    K_D(const OverlayApi);
    QVariant v = d->videowidget->property("_k_overlayCapabilities");
    if (v.isValid()) {
        return static_cast<OverlayType>(v.toInt());
    }
    return OverlayNone;
}

bool OverlayApi::createOverlay(QWidget *widget, OverlayType type)
{
    K_D(OverlayApi);
    QObject *m_backendObject = d->videowidget->k_ptr->backendObject();
    if (!m_backendObject) {
        return false;
    }

    bool result;
    if (pBACKEND_GET2(bool, result, "createOverlay", QWidget *, widget, int, type)) {
        return result;
    }
    return false;
}

void OverlayApi::setBackgroundForOpaqueOverlay(const QImage & /* image */)
{
	// FIXME: implement
}

} // namespace Experimental
} // namespace Phonon

#include "overlayapi.moc"
// vim: sw=4 sts=4 et tw=100
