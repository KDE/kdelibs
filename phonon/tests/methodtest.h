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

#ifndef PHONON_METHODTEST_H
#define PHONON_METHODTEST_H

#include <QObject>
#include "loadfakebackend.h"

class MethodTest : public QObject
{
    Q_OBJECT
    private Q_SLOTS:
        void initTestCase() { Phonon::loadFakeBackend(); }
        void checkBackendInterface();

        void checkAudioDataOutputMethods_data();
        void checkAudioDataOutputMethods();
        void checkAudioEffectMethods_data();
        void checkAudioEffectMethods();
        void checkAudioOutputMethods_data();
        void checkAudioOutputMethods();
        void checkAudioPathMethods_data();
        void checkAudioPathMethods();
        void checkAvCaptureMethods_data();
        void checkAvCaptureMethods();
        void checkBackendMethods_data();
        void checkBackendMethods();
        void checkBrightnessControlMethods_data();
        void checkBrightnessControlMethods();
        void checkByteStreamMethods_data();
        void checkByteStreamMethods();
        void checkMediaObjectMethods_data();
        void checkMediaObjectMethods();
        void checkMediaQueueMethods_data();
        void checkMediaQueueMethods();
        void checkVideoDataOutputMethods_data();
        void checkVideoDataOutputMethods();
        void checkVideoEffectMethods_data();
        void checkVideoEffectMethods();
        void checkVideoPathMethods_data();
        void checkVideoPathMethods();
        void checkVisualizationMethods_data();
        void checkVisualizationMethods();
        void checkVolumeFaderEffectMethods_data();
        void checkVolumeFaderEffectMethods();
        void checkByteStreamInterfaces();
        void checkMediaObjectInterfaces();
        void checkAvCapturetInterfaces();
        void checkAudioOutputInterface();

    private:
        void addColumns();
        void addMethod(const char *returnType, const char *signature, bool optional = false);
        void addSignal(const char *signature);
        void checkMethods(QObject *backendObject);
};
#endif // PHONON_METHODTEST_H

// vim: ts=4
