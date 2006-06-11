#ifndef _KSPEECHDEF_H_
#define _KSPEECHDEF_H_

namespace KSpeech {
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
            mtSsml = 2,                  /**< %Speech Synthesis Markup Language */
            mtSable = 3,                 /**< Sable 2.0 */
            mtHtml = 4                   /**< HTML @since 3.5 */
        };
}

#endif
