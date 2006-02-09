/***************************************************** vim:set ts=4 sw=4 sts=4:
  kspeechsink.h
  KTTSD DCOP Signal Sink Interface
  --------------------------------
  Copyright:
  (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

/**
 * @interface KSpeechSink
 *
 * KTTSD DCOP Signal Sink
 *
 * @since KDE 3.4
 *
 * This defines the interface to sink signals emitted by KTTSD, the KDE Text-to-speech Deamon.
 * The DCOP IDL Compiler generates a skeleton file from this interface definition that will
 * marshal the arguments for you.
 *
 * @section Usage
 *
 * See the Signals section of kspeech.h for instructions.
 *
 * @warning The KSpeechSink interface is still being developed and is likely to change in the future.
*/

#ifndef _KSPEECHSINK_H_
#define _KSPEECHSINK_H_

#include <dcopobject.h>

class KSpeechSink : virtual public DCOPObject {
    K_DCOP

    public:
        /**
        * @enum kttsdJobState
        * Job states returned by method getTextJobState.
        */
        enum kttsdJobState
        {
            jsQueued = 0,                /**< Job has been queued but is not yet speakable. */
            jsSpeakable = 1,             /**< Job is speakable, but is not speaking. */
            jsSpeaking = 2,              /**< Job is currently speaking. */
            jsPaused = 3,                /**< Job has been paused. */
            jsFinished = 4               /**< Job is finished and is deleteable. */
        };

        /**
        * @enum kttsdMarkupType
        * %Speech markup language types.
        */
        enum kttsdMarkupType
        {
            mtPlain = 0,                 /**< Plain text */
            mtJsml = 1,                  /**< Java %Speech Markup Language */
            mtSmml = 2,                  /**< %Speech Markup Meta-language */
            mtSable = 3                  /**< Sable 2.0 */
        };

    k_dcop:
        /**
        * This signal is emitted when KTTSD starts or restarts after a call to reinit.
        */
        virtual ASYNC kttsdStarted() { };
        /**
        * This signal is emitted just before KTTSD exits.
        */
        virtual ASYNC kttsdExiting() { };

        /**
         * This signal is emitted when the speech engine/plugin encounters a marker in the text.
         * @param appId          DCOP application ID of the application that queued the text.
         * @param markerName     The name of the marker seen.
         * @see markers
         */
        virtual ASYNC markerSeen(const DCOPCString& appId, const QString& markerName) { Q_UNUSED(appId); Q_UNUSED(markerName); };
        /**
         * This signal is emitted whenever a sentence begins speaking.
         * @param appId          DCOP application ID of the application that queued the text.
         * @param jobNum         Job number of the text job.
         * @param seq            Sequence number of the text.
         * @see getTextCount
         */
        virtual ASYNC sentenceStarted(const DCOPCString& appId, uint jobNum, uint seq) { Q_UNUSED(appId); Q_UNUSED(jobNum); Q_UNUSED(seq); };
        /**
         * This signal is emitted when a sentence has finished speaking.
         * @param appId          DCOP application ID of the application that queued the text.
         * @param jobNum         Job number of the text job.
         * @param seq            Sequence number of the text.
         * @see getTextCount
         */
        virtual ASYNC sentenceFinished(const DCOPCString& appId, uint jobNum, uint seq) { Q_UNUSED(appId); Q_UNUSED(jobNum); Q_UNUSED(seq); };

        /**
         * This signal is emitted whenever a new text job is added to the queue.
         * @param appId          The DCOP senderId of the application that created the job.  NULL if kttsd.
         * @param jobNum         Job number of the text job.
         */
        virtual ASYNC textSet(const DCOPCString& appId, uint jobNum) { Q_UNUSED(appId); Q_UNUSED(jobNum); };

        /**
        * This signal is emitted whenever a new part is appended to a text job.
        * @param appId          The DCOP senderId of the application that created the job.
        * @param jobNum         Job number of the text job.
        * @param partNum        Part number of the new part.  Parts are numbered starting
        *                       at 1.
        */
        virtual ASYNC textAppended(const DCOPCString& appId, uint jobNum, int partNum) { Q_UNUSED(appId); Q_UNUSED(jobNum); Q_UNUSED(partNum); };

        /**
         * This signal is emitted whenever speaking of a text job begins.
         * @param appId          The DCOP senderId of the application that created the job.  NULL if kttsd.
         * @param jobNum         Job number of the text job.
         */
        virtual ASYNC textStarted(const DCOPCString& appId, uint jobNum) { Q_UNUSED(appId); Q_UNUSED(jobNum); };
        /**
         * This signal is emitted whenever a text job is finished.  The job has
         * been marked for deletion from the queue and will be deleted when another
         * job reaches the Finished state. (Only one job in the text queue may be
         * in state Finished at one time.)  If startText or resumeText is
         * called before the job is deleted, it will remain in the queue for speaking.
         * @param appId          The DCOP senderId of the application that created the job.  NULL if kttsd.
         * @param jobNum         Job number of the text job.
         */
        virtual ASYNC textFinished(const DCOPCString& appId, uint jobNum) { Q_UNUSED(appId); Q_UNUSED(jobNum); };
        /**
         * This signal is emitted whenever a speaking text job stops speaking.
         * @param appId          The DCOP senderId of the application that created the job.  NULL if kttsd.
         * @param jobNum         Job number of the text job.
         */
        virtual ASYNC textStopped(const DCOPCString& appId, uint jobNum) { Q_UNUSED(appId); Q_UNUSED(jobNum); };
        /**
         * This signal is emitted whenever a speaking text job is paused.
         * @param appId          The DCOP senderId of the application that created the job.  NULL if kttsd.
         * @param jobNum         Job number of the text job.
         */
        virtual ASYNC textPaused(const DCOPCString& appId, uint jobNum) { Q_UNUSED(appId); Q_UNUSED(jobNum); };
        /**
         * This signal is emitted when a text job, that was previously paused, resumes speaking.
         * @param appId          The DCOP senderId of the application that created the job.  NULL if kttsd.
         * @param jobNum         Job number of the text job.
         */
        virtual ASYNC textResumed(const DCOPCString& appId, uint jobNum) { Q_UNUSED(appId); Q_UNUSED(jobNum); };
        /**
         * This signal is emitted whenever a text job is deleted from the queue.
         * The job is no longer in the queue when this signal is emitted.
         * @param appId          The DCOP senderId of the application that created the job.  NULL if kttsd.
         * @param jobNum         Job number of the text job.
         */
        virtual ASYNC textRemoved(const DCOPCString& appId, uint jobNum) { Q_UNUSED(appId); Q_UNUSED(jobNum); };
};

#endif // _KSPEECHSINK_H_
