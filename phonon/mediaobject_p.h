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

#ifndef MEDIAOBJECT_P_H
#define MEDIAOBJECT_P_H

#include "mediaobject.h"
#include "base_p.h"
#include <QUrl>
#include <QTimer>
#include <QHash>
#include <QString>
#include <QMultiMap>
#include "basedestructionhandler.h"
#include "mediasource.h"
#include <QQueue>

namespace Phonon
{
class KioFallback;
class KioFallbackImpl;
class FrontendInterfacePrivate;

class MediaObjectPrivate : public BasePrivate, private BaseDestructionHandler
{
    friend class KioFallbackImpl;
    friend class AbstractMediaStream;
    friend class AbstractMediaStreamPrivate;
    Q_DECLARE_PUBLIC(MediaObject)
    public:
        QList<FrontendInterfacePrivate *> interfaceList;
    protected:
        virtual bool aboutToDeleteBackendObject();
        virtual void createBackendObject();
        virtual void phononObjectDestroyed(BasePrivate *);
        PHONON_EXPORT void setupBackendObject();

        void streamError(Phonon::ErrorType, const QString &);
        void _k_resumePlay();
        void _k_resumePause();
        void _k_metaDataChanged(const QMultiMap<QString, QString> &);
        void _k_aboutToFinish();
        PHONON_EXPORT void _k_stateChanged(Phonon::State, Phonon::State);

        MediaObjectPrivate()
            : q_ptr(0),
            state(Phonon::LoadingState),
            currentTime(0),
            tickInterval(0),
            videoPaths(),
            audioPaths(),
            currentAudioStream(),
            currentVideoStream(),
            currentSubtitleStream(),
            metaData(),
            errorOverride(false),
            errorString(),
            errorType(Phonon::NormalError),
            prefinishMark(0),
            transitionTime(0), // gapless playback
            kiofallback(0),
            ignoreLoadingToBufferingStateChange(false)
        {
        }

        MediaObject *q_ptr;
        State state;
        qint64 currentTime;
        qint32 tickInterval;
        QList<VideoPath *> videoPaths;
        QList<AudioPath *> audioPaths;
        QHash<AudioPath *, QString> currentAudioStream;
        QHash<VideoPath *, QString> currentVideoStream;
        QHash<VideoPath *, QString> currentSubtitleStream;
        QMultiMap<QString, QString> metaData;
        bool errorOverride;
        QString errorString;
        ErrorType errorType;
        QUrl url;
        QString mediaDevice;
        qint32 prefinishMark;
        qint32 transitionTime;
        AbstractMediaStream *kiofallback;
        bool ignoreLoadingToBufferingStateChange;
        MediaSource mediaSource;
        QQueue<MediaSource> sourceQueue;
};
}

#endif // MEDIAOBJECT_P_H
// vim: sw=4 ts=4 tw=80
