/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Matthias Kretz <kretz@kde.org>

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

#ifndef PHONON_DEINTERLACEFILTER_P_H
#define PHONON_DEINTERLACEFILTER_P_H

#include "deinterlacefilter.h"
#include "effect_p.h"

namespace Phonon
{
class DeinterlaceFilterPrivate : public EffectPrivate
{
    Q_DECLARE_PUBLIC(DeinterlaceFilter)
    PHONON_PRIVATECLASS
    protected:
        DeinterlaceFilterPrivate()
        {
            type = Effect::VideoEffect;
            id = -1; // ### check how to get the correct id
            // ############# parameter functions are incorrect
        }
};
}

#endif // PHONON_DEINTERLACEFILTER_P_H
