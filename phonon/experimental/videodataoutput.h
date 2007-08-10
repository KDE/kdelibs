/*  This file is part of the KDE project
    Copyright (C) 2005-2007 Matthias Kretz <kretz@kde.org>

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

#ifndef PHONON_VIDEODATAOUTPUT_H
#define PHONON_VIDEODATAOUTPUT_H

#include "export.h"
#include "../abstractvideooutput.h"
#include "../phonondefs.h"
#include <QtCore/QObject>

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template<typename T> class QVector;
template<typename Key, typename T> class QMap;
#endif
class QSize;

namespace Phonon
{
namespace Experimental
{
    class VideoDataOutputPrivate;
    struct VideoFrame;

    /**
     * \short This class gives you the video data.
     *
     * This class implements a special AbstractVideoOutput that gives your
     * application the video data.
     *
     * You can also use the video data for further processing (e.g. encoding and
     * saving to a file).
     *
     * \author Matthias Kretz <kretz@kde.org>
     */
    class PHONONEXPERIMENTAL_EXPORT VideoDataOutput : public QObject, public AbstractVideoOutput
    {
        Q_OBJECT
        K_DECLARE_PRIVATE(VideoDataOutput)
        /**
         * This property retrieves the nominal latency of the
         * backend.
         */
        Q_PROPERTY(int latency READ latency)

        /**
         * This property indicates the state of the data output.
         */
        Q_PROPERTY(bool running READ isRunning WRITE setRunning)

        PHONON_HEIR(VideoDataOutput)
    public:
        int latency() const;

        bool isRunning() const;

        Phonon::Experimental::VideoFrame frameForTime(qint64 timestamp);

    public slots:
        void setRunning(bool running);
        void start();
        void stop();

    Q_SIGNALS:
        /**
         * Fixme: I don't think this makes sense, but I've been wrong before.
         *
         * Emitted whenever another dataSize number of samples are ready and
         * format is set to IntegerFormat.
         *
         * If format is set to FloatFormat the signal is not emitted at all.
         *
         * \param frame An object of class VideoFrame holding the video data
         * and some additional information.
         * void frameReady(const Phonon::Experimental::VideoFrame &frame);
         */

        /**
         * The signal is emitted whenever a frame should be displayed.
         * nowStamp is the current time, outStamp tells the users
         * what time the frame should be displayed with.
         *
         * The relevant frames should be fetched and displayed using frameForTime
         * method.
         */
        void displayFrame(qint64 nowStamp, qint64 outStamp);

        /**
         * This signal is emitted after the last frameReady signal of a
         * media is emitted.
         */
        void endOfMedia();
    };

} // namespace Experimental
} //namespace Phonon

// vim: sw=4 ts=4 tw=80
#endif // PHONON_VIDEODATAOUTPUT_H
