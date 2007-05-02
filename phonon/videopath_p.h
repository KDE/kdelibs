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

#ifndef VIDEOPATH_P_H
#define VIDEOPATH_P_H

#include "videopath.h"
#include "base_p.h"
#include <QtCore/QList>

namespace Phonon
{
class VideoPathPrivate : public BasePrivate, private BaseDestructionHandler
{
    Q_DECLARE_PUBLIC(VideoPath)
    PHONON_PRIVATECLASS
    protected:
        VideoPath *q_ptr;
        VideoPathPrivate()
            : q_ptr(0)
        {
        }

        QList<AbstractVideoOutput *> outputs;
        QList<Effect *> effects;

    private:
        virtual void phononObjectDestroyed(BasePrivate *);
};
} //namespace Phonon

#endif // VIDEOPATH_P_H
// vim: sw=4 ts=4 tw=80
