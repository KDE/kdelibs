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

#ifndef PHONON_FAKE_ABSTRACTVIDEOOUTPUT_H
#define PHONON_FAKE_ABSTRACTVIDEOOUTPUT_H

#include <phonon/experimental/videoframe.h>

namespace Phonon
{
namespace Fake
{

class AbstractVideoOutput
{
    public:
        virtual ~AbstractVideoOutput() {}
        virtual void processFrame(Phonon::Experimental::VideoFrame &frame) = 0;
        virtual void *internal1(void * = 0) { return 0; }
};

}} //namespace Phonon::Fake

Q_DECLARE_INTERFACE(Phonon::Fake::AbstractVideoOutput, "org.kde.Phonon.Fake.AbstractVideoOutput/0.1")

#endif // PHONON_FAKE_ABSTRACTVIDEOOUTPUT_H
// vim: sw=4 ts=4
