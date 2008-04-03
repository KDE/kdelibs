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

#ifndef AUDIONODE_H
#define AUDIONODE_H

namespace Phonon
{
namespace Fake
{

class AudioNode
{
    public:
        AudioNode() : m_hasInput(false) {}
        virtual ~AudioNode() {}
        virtual void processBuffer(QVector<float> &buffer) = 0;
        bool hasInput() const { return m_hasInput; }
        void setHasInput(bool x) { m_hasInput = x; }
    private:
        bool m_hasInput;
};

} // namespace Fake
} // namespace Phonon

Q_DECLARE_INTERFACE(Phonon::Fake::AudioNode, "FakeAudioNode.phonon.kde.org")

#endif // AUDIONODE_H
