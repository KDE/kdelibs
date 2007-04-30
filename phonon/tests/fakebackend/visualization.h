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

#ifndef PHONON_FAKE_VISUALIZATION_H
#define PHONON_FAKE_VISUALIZATION_H

#include <QObject>

namespace Phonon
{
namespace Fake
{
    class AudioPath;
    class AbstractVideoOutput;

class Visualization : public QObject
{
    Q_OBJECT
    public:
        Visualization(QObject *parent = 0);

    public Q_SLOTS:
        int visualization() const;
        void setVisualization(int newVisualization);
        void setAudioPath(QObject *audioPath);
        void setVideoOutput(QObject *videoOutput);

    private:
        int m_visualization;
        AudioPath *m_audioPath;
        AbstractVideoOutput *m_videoOutput;
};

}} //namespace Phonon::Fake

#endif // PHONON_FAKE_VISUALIZATION_H
