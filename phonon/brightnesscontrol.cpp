/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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

#include "brightnesscontrol.h"
#include "brightnesscontrol_p.h"
#include "factory.h"
#include "effectparameter.h"
#include <klocale.h>

#define PHONON_CLASSNAME BrightnessControl

namespace Phonon
{
PHONON_HEIR_IMPL(VideoEffect)

QList<EffectParameter> BrightnessControl::parameterList() const
{
    QList<EffectParameter> ret;
    EffectParameter par(1, i18n("Brightness"), EffectParameter::IntegerHint, 0,
            lowerBound(), upperBound(), i18n("controls the brightness of the video images"));
    par.setEffect(const_cast<BrightnessControl *>(this));
    return ret;
}

QVariant BrightnessControl::value(int parameterId) const
{
    if (parameterId == 1)
        return brightness();
    return QVariant();
}

void BrightnessControl::setValue(int parameterId, QVariant newValue)
{
    if (parameterId == 1)
        setBrightness(newValue.toInt());
}

PHONON_GETTER(int, brightness, d->brightness)
PHONON_GETTER(int, lowerBound, -1000)
PHONON_GETTER(int, upperBound,  1000)
PHONON_SETTER(setBrightness, brightness, int)

bool BrightnessControlPrivate::aboutToDeleteIface()
{
    if (backendObject)
        pBACKEND_GET(int, brightness, "brightness");
    return true;
}

void BrightnessControl::setupIface()
{
    K_D(BrightnessControl);
    Q_ASSERT(d->backendObject);

    // set up attributes
    BACKEND_CALL1("setBrightness", int, d->brightness);
}
} // namespace Phonon

#include "brightnesscontrol.moc"
#undef PHONON_CLASSNAME
// vim: sw=4 ts=4
