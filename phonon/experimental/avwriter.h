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

#ifndef PHONON_AVWRITER_H
#define PHONON_AVWRITER_H

#include "export.h"
#include "../abstractaudiooutput.h"
#include "../abstractvideooutput.h"

namespace Phonon
{
namespace Experimental
{

class AvWriterPrivate;

//TODO:
// Container formats can support multiple audio, video and subtitle streams, a
// control track (menus) and chapters. Anything else?
// How should those features be mapped to this API? Multiple audio and video
// streams should be covered already. Are the subtitle streams implicit
// depending on the source material?
// Chapters: call a method when to add a chapter? How does that map to a
// specific frame/sample? Chapter support is probably overkill for Phonon.
class PHONONEXPERIMENTAL_EXPORT AvWriter : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(AvWriter)
    Q_PROPERTY(ContainerFormat containerFormat READ containerFormat WRITE setContainerFormat)
    Q_PROPERTY(KUrl url READ url WRITE setUrl)
    public:
        /**
         * Standard QObject constructor.
         *
         * \param parent QObject parent
         */
        AvWriter(QObject *parent);
        
        KUrl url() const;
        setUrl(const KUrl &url);

        /**
         * Creates a new AudioWriter object to be used for sending the audio
         * data to this file
         *
         * \param streamName A name identifying the stream. Often this name is
         * used for the language identifier.
         *
         * \return Returns the new AudioWriter object or 0 if the container
         * format does not support multiple audio streams.
         */
        AudioWriter *addAudioStream(const QString &streamName);
        VideoWriter *addVideoStream(const QString &streamName);

        ContainerFormat containerFormat() const;

    public Q_SLOTS:
        void setContainerFormat(ContainerFormat format);
};

} // namespace Experimental
} // namespace Phonon

#endif // PHONON_AVWRITER_H
// vim: sw=4 ts=4 tw=80
