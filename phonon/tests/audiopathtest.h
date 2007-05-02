/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.

*/

#ifndef AUDIOPATHTEST_H
#define AUDIOPATHTEST_H

#include <QtCore/QObject>
#include <kurl.h>
#include <phonon/mediaobject.h>
#include <QtTest/QSignalSpy>
#include <phonon/audiopath.h>
#include <phonon/audiooutput.h>

class AudioPathTest : public QObject
{
    Q_OBJECT

    private Q_SLOTS:
        void initTestCase();
        void checkForDefaults();

        void addOutputs();
        void addEffects();

        void cleanupTestCase();

    private:
        KUrl m_url;
        Phonon::MediaObject *m_media;
        Phonon::AudioPath *m_path;
        Phonon::AudioOutput *m_output;
};

// vim: sw=4 ts=4
#endif // AUDIOPATHTEST_H
