/***************************************************** vim:set ts=4 sw=4 sts=4:
  kspeech.h
  KTTSD DCOP Interface
  --------------------
  Copyright:
  (C) 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  (C) 2003-2004 by Olaf Schmidt <ojschmidt@kde.org>
  (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: José Pablo Ezequiel "Pupeno" Fernández
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

#ifndef _KSPEECH_H_
#define _KSPEECH_H_

#include <dcopobject.h>

/**
 * @interface kspeech
 *
 * kspeech - the KDE Text-to-speech API.
 *
 * @version 1.0 Draft 2
 *
 * This class defines the DCOP interface for applications desiring to speak text.
 * Applications may speak text by sending DCOP messages to application "kttsd" object "kspeech".
 *
 * %KTTSD -- the KDE Text-to-speech Deamon -- is the program that supplies the services
 * in the KDE Text-to-speech API.
 *
 * @warning The kspeech interface is still being developed and is likely to change in the future.
 *
 * @section Features
 *
 *   - Priority system for warnings and messages, while still playing regular texts.
 *   - Long text is parsed into sentences.  User may backup by sentence or paragraph,
 *     replay, pause, and stop playing.
 *   - Handles multiple speaking applications.  Text messages are treated like print jobs.
 *     Jobs may be created, started, stopped, paused, resumed, and deleted.
 *   - Speak contents of clipboard.
 *
 * @section Requirements
 *
 * KDE 3.2.x and a speech synthesis engine, such as Festival.  Festival can be
 * obtained from 
 * <a href="http://www.cstr.ed.ac.uk/projects/festival/">http://www.cstr.ed.ac.uk/projects/festival/</a>.
 * Festival is distributed with most Linux distros.  Check your distro CDs.  Also works
 * with Hadifax, FreeTTS, or any command that can speak text, such as Festival Lite
 * (flite).
 *
 * @section goals Design Goals
 *
 * The KDE Text-to-speech API is designed with the following goals:
 *
 *   - Support the features enumerated above.
 *   - Plugin-based architecture for support of a wide variety of speech synthesis engines and drivers.
 *   - Permit generation of speech from the command line (or via shell scripts) using the KDE DCOP utilities.
 *   - Provide a lightweight and easily usable interface for applications to generate speech output.
 *   - Applications need not be concerned about contention over the speech device.
 *   - Provide limited support for speech markup languages, such as Sable, Java %Speech Markup Language (JSML),
 *     and %Speech Markup Meta-language (SMML).  Basically, the goal is for %KTTSD to permit pass-thru
 *     of markup to the speech engine.  Generation and transformation of markup is left to higher layers
 *     or individual applications.
 *   - Provide limited support for embedded speech markers.
 *   - Threading-based to prevent system blocking.
 *   - Compatible with original %KTTSD API as developed by José Pablo Ezequiel "Pupeno" Fernández
 *     (avoid breaking existing applications).
 *
 * Architecturally, applications interface with %KTTSD, which performs queueing, speech job managment, and
 * sentence parsing.  %KTTSD interfaces with a %KTTSD speech plugin(s), which then interfaces with the
 * speech engine(s) or driver(s).
 *
   @verbatim
         application
              ^
              |  via DCOP (the KDE Speech-to-text API)
              v
            kttsd
              ^
              |  KTTSD plugin API
              v
         kttsd plugin
              ^
              |
              v
        speech engine
   @endverbatim
 *
 * The %KTTSD Plugin API is documented elsewhere.
 *
 * There is a separate GUI application, called kttsmgr, for providing %KTTSD configuration and job
 * management.
 *
 * @section Using
 *
 * Make sure your speech engine is working.
 *
 * You may need to grant Festival write access to the audio device.
 *
   @verbatim
     chmod a+rw /dev/dsp*
   @endverbatim
 *
 * To configure the speech plugin for %KTTSD, run the KTTS Manager
 *
   @verbatim
     kttsmgr
   @endverbatim
 *
 * If using the Festival or Festival Interactive plugins, you'll need to
 * specify the path to the voices.  On most systems, this will be
 *
   @verbatim
     /usr/share/festival/voices
   @endverbatim
 *
 * or
 *
   @verbatim
    /usr/local/share/festival/voices
   @endverbatim
 * 
 * Be sure to click the Default button after configuring a speech plugin!
 *
 * To run %KTTSD, either check the Enable Text-to-speech System check box, or
 * in a command terminal, enter
 *
   @verbatim
     kttsd
   @endverbatim
 *
 * Click the "jobs" tab.  Click the "Speak File" button and
 * pick a plain text file, then click "Resume" or "Restart" buttons.
 *
 * kttsd maintains three speech queues:
 *   - Warnings
 *   - Messages
 *   - Text Jobs
 *
 * Methods sayWarning and sayMessage place messages into the Warnings and Messages queues respectively.
*  Warnings take priority over messages, which take priority over text jobs.
 * setText and startText place text into the text job queue.  When one job finishes, the next
 * job begins.  Within a text job, the application (and user via the kttsmgr GUI), may back up by
 * sentence or paragraph, advance by sentence or paragraph, or rewind to the beginning.
 * Text jobs may be paused and resumed or deleted from the queue.
 *
 * @section cmdline DCOP Command-line Interface
 *
 * To create a text job to be spoken
 *
   @verbatim
     dcop kttsd kspeech setText <text> <talker>
   @endverbatim
 *
 * where \<text\> is the text to be spoken, and \<talker\> is usually a language code
 * such as "en", "cy", etc.
 *
 * Example.
 *
   @verbatim
     dcop kttsd kspeech setText "This is a test." "en"
   @endverbatim
 *
 * To start speaking the text.
 *
   @verbatim
     dcop kttsd kspeech startText 0
   @endverbatim
 *
 * To stop speaking and rewind to the beginning of the text.
 *
   @verbatim
     dcop kttsd kspeech stopText 0
   @endverbatim
 *
 * Depending upon the speech plugin used, speaking may not immediately stop.
 *
 * To stop and remove a text job.
 *
   @verbatim
     dcop kttsd kspeech removeText 0
   @endverbatim
 *
 * @section programming Calling KTTSD from a Program
 *
 * To make DCOP calls from your program, follow these steps:
 *
 * 1.  Include kspeech_stub.h in your code.  Derive an object from the kspeech_stub interface.
 *     For example, suppose you are developing a KPart and want to call %KTTSD.
 *     Your class declaration might look like this:
 *
   @verbatim
     #include <kspeech_stub.h>
     class MyPart: public KParts::ReadOnlyPart, public kspeech_stub {
   @endverbatim
 *
 * 2.  In your class constructor, initialize DCOPStub, giving it the sender "kttsd", object "kspeech".
 *
   @verbatim
     MyPart::MyPart(QWidget *parent, const char *name) :
        KParts::ReadOnlyPart(parent, name),
        DCOPStub("kttsd", "kspeech") {
   @endverbatim
 *
 * 3.  See if KTTSD is running, and if not, start it.
 *
   @verbatim
     DCOPClient *client = dcopClient();
     client->attach();
     if (!client->isApplicationRegistered("kttsd")) {
         QString error;
         if (KApplication::startServiceByName("KTTSD", QStringList(), &error))
             cout << "Starting KTTSD failed with message " << error << endl;
     }
   @endverbatim
 *
 * 4.  Make calls to KTTSD in your code.
 *
   @verbatim
     uint jobNum = setText("Hello World", "en");
     startText(jobNum);
   @endverbatim
 *
 * 4.  Add libktts to your Makefile.am.
 *
   @verbatim
     mypart_la_LIBADD = libktts
   @endverbatim 
 *
 * @section signals Signals Emitted by KTTSD
 *
 * %KTTSD emits a number of DCOP signals, which provide information about sentences spoken,
 * text jobs started, stopped, paused, resumed, finished, or deleted and markers seen.
 * In general, these signals are broadcast to any application that connects to them.
 * Applications should check the appId argument to determine whether the signal belongs to
 * them or not.
 *
 * To receive %KTTSD DCOP signals, follow these steps:
 *
 * 1.  Include kspeechsink.h in your code.  Derive an object from the KSpeechSink interface
 *     and declare a method for each signal you'd like to receive.  For example, if you were coding a KPart
 *     and wanted to receive the KTTSD signal sentenceStarted:
 *
   @verbatim
     #include <kspeechsink.h>
     class MyPart:
         public KParts::ReadOnlyPart,
         virtual public KSpeechSink
     {
         protected:
            ASYNC sentenceStarted(const QCString& appId, const uint jobNum, const uint seq);
   @endverbatim
 *
 *     You can combine sending and receiving in one object.
 *
   @verbatim
     #include <kspeechsink.h>
     class MyPart: 
         public KParts::ReadOnlyPart,
         public kspeech_stub,
         virtual public KSpeechSink
     {
         protected:
            ASYNC sentenceStarted(const QCString& appId, const uint jobNum, const uint seq);
   @endverbatim
 *
 *     See below for the signals you can declare.
 *
 * 2.  In your class constructor, initialize DCOPObject with the name of your DCOP receiving object.
 *
   @verbatim
     MyPart::MyPart(QWidget *parent, const char *name) :
         KParts::ReadOnlyPart(parent, name),
         DCOPObject("mypart_kspeechsink") {
   @endverbatim
 *
 *     Use any name you like.
 *
 * 3.  Where appropriate (usually in your constructor), make sure your DCOPClient is registered and
 *     connect the %KTTSD DCOP signals to your declared receiving methods.
 *
   @verbatim
     // Register DCOP client.
     DCOPClient *client = kapp->dcopClient();
     if (!client->isRegistered())
     {
         client->attach();
         client->registerAs(kapp->name());    
     }
     // Connect KTTSD DCOP signals to our slots.
     connectDCOPSignal("kttsd", "kspeech",
         "sentenceStarted(QCString,uint,uint)",
         "sentenceStarted(QCString,uint,uint)",
         false);
   @endverbatim
 *
 *     Notice that the argument signatures differ slightly from the actual declarations.  For
 *     example
 *
   @verbatim
     ASYNC sentenceStarted(const QCString& appId, const uint jobNum, const uint seq);
   @endverbatim
 *
 *     becomes
 *
   @verbatim
       "sentenceStarted(QCString,uint,uint)",
   @endverbatim
 *
 *     in the connectDCOPSignal call.
 *
 * 4.  Write the definition for the received signal.  Be sure to check whether the signal
 *     is intended for your application.
 *
   @verbatim
     ASYNC MyPart::sentenceStarted(const QCString& appId, const uint jobNum, const uint seq)
     {
         // Check appId to determine if this is our signal.
         if (appId != dcopClient()->appId) return;
         // Do something here.
     }
   @endverbatim
 *
 * 5.  Add libktts to your Makefile.am LIBADD variable:
 *
   @verbatim
     libmypart_la_LIBADD = libktts
   @endverbatim
 *
 * @section talkers Talkers
 *
 * Many of the methods permit you to specify a desired "talker".  At this time, this
 * should be a language code, such as "en" for English, "sp" for Spanish, etc.
 * Code as NULL to use the default configured talker.
 *
 * In the future, you will be able to configure more than one talker for each language,
 * with different voices, genders, volumes, and talking speeds.
 *
 * @section markup Speech Markup
 *
 * Note: %Speech Markup is not yet implemented in %KTTSD.
 *
 * Each of the three methods for queueing text to be spoken -- @ref setText, @ref sayMessage, and
 * @ref sayWarning -- may contain speech markup,
 * provided that the plugin the user has configured supports that markup.  The markup
 * languages currently supported by plugins are:
 *
 *   - Sable 2.0: Festival
 *   - Java %Speech Markup Language (JSML): Festival (partial)
 *   - %Speech Markup Meta-language (SMML): none at this time
 *
 * TODO: Issue: Do Hadifax and FreeTTS support markup?
 *
 * Before including markup in the text sent to kttsd, the application should
 * query whether the currently-configured plugin 
 * supports the markup language by calling @ref supportsMarkup.
 *
 * @section markers Support for Markers
 *
 * Note: Markers are not yet implemented in %KTTSD.
 *
 * When using a speech markup language, such as Sable, JSML, or SMML, the application may embed
 * named markers into the text.  If the user's chosen speech plugin supports markers, %KTTSD
 * will emit DCOP signal @ref markerSeen when the speech engine encounters the marker.
 * Depending upon the speech engine and plugin, this may occur either when the speech engine
 * encounters the marker during synthesis from text to speech, or when the speech is actually
 * spoken on the audio device.  The calling application can call the @ref supportsMarkers
 * method to determine if the currently configured plugin supports markers or not.
 *
 * @section sentenceparsing Sentence and Paragraph Parsing
 *
 * Not all speech engines provide robust capabilities for stopping speech that is in progress.
 * To compensate for this, %KTTSD parses text jobs given to it by the @ref setText method into
 * sentences and sends the sentences to the speech plugin one at a time.  In this way, should
 * the user wish to stop the speech output, they can do so, and the worst that will happen
 * is that the last sentence will be completed.
 *
 * Sentence parsing also permits the user to rewind by sentences.
 *
 * The default sentence delimiter used for plain text is as follows:
 *
 *   - A period (.), question mark (?), exclamation mark (!), colon (:), or
 *     semi-colon (;) followed by whitespace (including newline), or
 *   - A newline.
 *
 * %KTTSD treats a blank line as a paragraph delimiter.
 *
 * When given text containing speech markup, %KTTSD automatically determines the markup type
 * and parses based on the sentence and paragraph semantics of the markup language.
 * TODO: ISSUE: Can this be reasonably done?
 *
 * An application may change the sentence delimiter by calling @ref setSentenceDelimiter
 * prior to calling @ref setText.
 *
 * Text given to %KTTSD via the @ref sayWarning and @ref sayMessage methods is @e not parsed
 * into sentences.  For this reason, applications should @e not send long messages with
 * these methods.
 *
 * @section festival Using with Festival
 *
 * @bug 17 Mar 2004: The Festival plugin does not work for me.  Crashes as soon
 * as call to festival_initialize is made.  I suspect that the problem is
 * incompatible gcc compilers used to compile kttsd and the libfestival.a
 * static library, which on my system, is version 1.42. If someone can confirm or deny
 * this, I'd be appreciative.. Gary Cramblitt <garycramblitt@comcast.net>.
 * So I added the Festival (Interactive) plugin that interfaces with Festival interactively
 * via pipes ("festival --interactive").  
 *
 * @section festivalcs Using with Festival Client/Server
 *
 * Festival (Client/Server), festivalcs, is still under development.
 *
 * @section flite Using with Festival Lite (flite)
 *
 * Obtain Festival Lite here:
 *
 * <a href="http://www.speech.cs.cmu.edu/flite/index.html">http://www.speech.cs.cmu.edu/flite/index.html</a>.
 *
 * Build and install following the instructions in the README that comes with flite.
 *
 * Start KTTS Manager
 *
   @verbatim
     kttsmgr
   @endverbatim
 *
 * Choose the Command plugin and enter the following as the command
 *
   @verbatim
     flite -t "%t"
   @endverbatim
 *
 * @author José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
 * @author Olaf Schmidt <ojschmidt@kde.org>
 * @author Gary Cramblitt <garycramblitt@comcast.net>
 */

class kspeech : virtual public DCOPObject {
    K_DCOP

    public:
        /**
        * @enum kttsdJobState
        * Job states returned by method @ref getTextJobState.
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
        /** @name DCOP Methods */
        //@{
        
        /**
        * Determine whether the currently-configured speech plugin supports a speech markup language.
        * @param talker         Code for the language to be spoken in.  Example "en".
        *                       If NULL, defaults to the user's default talker.
        * @param markupType     The kttsd code for the desired speech markup language.
        * @return               True if the plugin currently configured for the indicated
        *                       talker supports the indicated speech markup language.
        * @see kttsdMarkupType
        */
        virtual bool supportsMarkup(const QString &talker=NULL, const uint markupType = 0) = 0;
        
        /**
        * Determine whether the currently-configured speech plugin supports markers in speech markup.
        * @param talker         Code for the language to be spoken in.  Example "en".
        *                       If NULL, defaults to the user's default talker.
        * @return               True if the plugin currently configured for the indicated
        *                       talker supports markers.
        */
        virtual bool supportsMarkers(const QString &talker=NULL) = 0;
        
        /**
        * Say a warning.  The warning will be spoken when the current sentence
        * stops speaking and takes precedence over Messages and regular text.  Warnings should only
        * be used for high-priority messages requiring immediate user attention, such as
        * "WARNING. CPU is overheating."
        * @param warning        The warning to be spoken.
        * @param talker         Code for the language to be spoken in.  Example "en".
        *                       If NULL, defaults to the user's default talker.
        *                       If no plugin has been configured for the specified language code,
        *                       defaults to the user's default talker.
        */
        virtual ASYNC sayWarning(const QString &warning, const QString &talker=NULL) = 0;

        /**
        * Say a message.  The message will be spoken when the current text paragraph
        * stops speaking.  Messages should be used for one-shot messages that can't wait for
        * normal text messages to stop speaking, such as "You have mail.".
        * @param message        The message to be spoken.
        * @param talker         Code for the language to be spoken in.  Example "en".
        *                       If NULL, defaults to the user's default talker.
        *                       If no talker has been configured for the specified language code,
        *                       defaults to the user's default talker.
        */
        virtual ASYNC sayMessage(const QString &message, const QString &talker=NULL) = 0;

        /**
        * Sets the GREP pattern that will be used as the sentence delimiter.
        * @param delimiter      A valid GREP pattern.
        *
        * The default sentence delimiter is
          @verbatim
              ([\\.\\?\\!\\:\\;])\\s
          @endverbatim
        *
        * Note that backward slashes must be escaped.
        *
        * Changing the sentence delimiter does not affect other applications.
        * @see sentenceparsing
        */
        virtual ASYNC setSentenceDelimiter(const QString &delimiter) = 0;
        
        /**
        * Queue a text job.  Does not start speaking the text.
        * @param text           The message to be spoken.
        * @param talker         Code for the language to be spoken in.  Example "en".
        *                       If NULL, defaults to the user's default plugin.
        *                       If no plugin has been configured for the specified language code,
        *                       defaults to the user's default plugin.
        * @return               Job number.
        *
        * Plain text is parsed into individual sentences using the current sentence delimiter.
        * Call @ref setSentenceDelimiter to change the sentence delimiter prior to calling setText.
        * Call @ref getTextCount to retrieve the sentence count after calling setText.
        *
        * The text may contain speech mark language, such as Sable, JSML, or SMML,
        * provided that the speech plugin/engine support it.  In this case,
        * sentence parsing follows the semantics of the markup language.
        *
        * Call @ref startText to mark the job as speakable and if the
        * job is the first speakable job in the queue, speaking will begin.
        * @see getTextCount
        * @see startText
        */
        virtual uint setText(const QString &text, const QString &talker=NULL) = 0;
        
        /**
        * Queue a text job from the contents of a file.  Does not start speaking the text.
        * @param filename       Full path to the file to be spoken.  May be a URL.
        * @param talker         Code for the language to be spoken in.  Example "en".
        *                       If NULL, defaults to the user's default talker.
        *                       If no plugin has been configured for the specified language code,
        *                       defaults to the user's default talker.
        * @return               Job number.  0 if an error occurs.
        *
        * Plain text is parsed into individual sentences using the current sentence delimiter.
        * Call @ref setSentenceDelimiter to change the sentence delimiter prior to calling setText.
        * Call @ref getTextCount to retrieve the sentence count after calling setText.
        *
        * The text may contain speech mark language, such as Sable, JSML, or SMML,
        * provided that the speech plugin/engine support it.  In this case,
        * sentence parsing follows the semantics of the markup language.
        *
        * Call @ref startText to mark the job as speakable and if the
        * job is the first speakable job in the queue, speaking will begin.
        * @see getTextCount
        * @see startText
        */
        virtual uint setFile(const QString &filename, const QString &talker=NULL) = 0;
        
        /**
        * Get the number of sentences in a text job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        * @return               The number of sentences in the job.  -1 if no such job.
        *
        * The sentences of a job are given sequence numbers from 1 to the number returned by this
        * method.  The sequence numbers are emitted in the @ref sentenceStarted and
        * @ref sentenceFinished signals.
        */
        virtual int getTextCount(const uint jobNum=0) = 0;

        /**
        * Get the job number of the current text job.
        * @return               Job number of the current text job. 0 if no jobs.
        *
        * Note that the current job may not be speaking. See @ref isSpeakingText.
        * @see getTextJobState.
        * @see isSpeakingText
        */
        virtual uint getCurrentTextJob() = 0;
        
        /**
        * Get the number of jobs in the text job queue.
        * @return               Number of text jobs in the queue.  0 if none.
        */
        virtual uint getTextJobCount() = 0;
        
        /**
        * Get a comma-separated list of text job numbers in the queue.
        * @return               Comma-separated list of text job numbers in the queue.
        */
        virtual QString getTextJobNumbers() = 0;
        
        /**
        * Get the state of a text job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        * @return               State of the job. -1 if invalid job number.
        *
        * @see kttsdJobState
        */
        virtual int getTextJobState(const uint jobNum=0) = 0;
        
        /**
        * Get information about a text job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        * @return               A QDataStream containing information about the job.
        *                       Blank if no such job.
        *
        * The stream contains the following elements:
        *   - int state         Job state.
        *   - QCString appId    DCOP senderId of the application that requested the speech job.
        *   - QString talker    Language code in which to speak the text.
        *   - int seq           Current sentence being spoken.  Sentences are numbered starting at 1.
        *   - int sentenceCount Total number of sentences in the job.
        *
        * The following sample code will decode the stream:
          @verbatim
            QByteArray jobInfo = getTextJobInfo(jobNum);
            QDataStream stream(jobInfo, IO_ReadOnly);
            int state;
            QCString appId;
            QString talker;
            int seq;
            int sentenceCount;
            stream >> state;
            stream >> appId;
            stream >> talker;
            stream >> seq;
            stream >> sentenceCount;
          @endverbatim
        */
        virtual QByteArray getTextJobInfo(const uint jobNum=0) = 0;
        
        /**
        * Return a sentence of a job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        * @param seq            Sequence number of the sentence.
        * @return               The specified sentence in the specified job.  If not such
        *                       job or sentence, returns "".
        */
        virtual QString getTextJobSentence(const uint jobNum=0, const uint seq=0) = 0;
       
        /**
        * Determine if kttsd is currently speaking any text jobs.
        * @return               True if currently speaking any text jobs.
        */
        virtual bool isSpeakingText() = 0;
        
        /**
        * Remove a text job from the queue.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        *
        * The job is deleted from the queue and the @ref textRemoved signal is emitted.
        *
        * If there is another job in the text queue, and it is marked speakable,
        * that job begins speaking.
        */
        virtual ASYNC removeText(const uint jobNum=0) = 0;

        /**
        * Start a text job at the beginning.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        *
        * Rewinds the job to the beginning.
        *
        * The job is marked speakable.
        * If there are other speakable jobs preceeding this one in the queue,
        * those jobs continue speaking and when finished, this job will begin speaking.
        * If there are no other speakable jobs preceeding this one, it begins speaking.
        *
        * The @ref textStarted signal is emitted when the text job begins speaking.
        * When all the sentences of the job have been spoken, the job is marked for deletion from
        * the text queue and the @ref textFinished signal is emitted.
        */
        virtual ASYNC startText(const uint jobNum=0) = 0;

        /**
        * Stop a text job and rewind to the beginning.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        *
        * The job is marked not speakable and will not be speakable until @ref startText or @ref resumeText
        * is called.
        *
        * If there are speaking jobs preceeding this one in the queue, they continue speaking.
        * If the job is currently speaking, the @ref textStopped signal is emitted and the job stops speaking.
        * Depending upon the speech engine and plugin used, speeking may not stop immediately
        * (it might finish the current sentence).
        */
        virtual ASYNC stopText(const uint jobNum=0) = 0;

        /**
        * Pause a text job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        *
        * The job is marked as paused and will not be speakable until @ref resumeText or
        * @ref startText is called.
        *
        * If there are speaking jobs preceeding this one in the queue, they continue speaking.
        * If the job is currently speaking, the @ref textPaused signal is emitted and the job stops speaking.
        * Depending upon the speech engine and plugin used, speeking may not stop immediately
        * (it might finish the current sentence).
        * @see resumeText
        */
        virtual ASYNC pauseText(const uint jobNum=0) = 0;

        /**
        * Start or resume a text job where it was paused.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        *
        * The job is marked speakable.
        *
        * If the job was not paused, it is the same as calling @ref startText.
        *
        * If there are speaking jobs preceeding this one in the queue, those jobs continue speaking and,
        * when finished this job will begin speaking where it left off.
        *
        * The @ref textResumed signal is emitted when the job resumes.
        * @see pauseText
        */
        virtual ASYNC resumeText(const uint jobNum=0) = 0;
        
        /**
        * Change the talker for a text job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        * @param talker         New code for the language to be spoken in.  Example "en".
        *                       If NULL, defaults to the user's default talker.
        *                       If no plugin has been configured for the specified language code,
        *                       defaults to the user's default talker.
        */
        virtual ASYNC changeTextTalker(const uint jobNum=0, const QString &talker=NULL) = 0;
        
        /**
        * Move a text job down in the queue so that it is spoken later.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        *
        * If the job is currently speaking, it is paused.
        * If the next job in the queue is speakable, it begins speaking.
        */
        virtual ASYNC moveTextLater(const uint jobNum=0) = 0;

        /**
        * Go to the previous paragraph in a text job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        */
        virtual ASYNC prevParText(const uint jobNum=0) = 0;

        /**
        * Go to the previous sentence in the queue.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        */
        virtual ASYNC prevSenText(const uint jobNum=0) = 0;

        /**
        * Go to next sentence in a text job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        */
        virtual ASYNC nextSenText(const uint jobNum=0) = 0;

        /**
        * Go to next paragraph in a text job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        */
        virtual ASYNC nextParText(const uint jobNum=0) = 0;
        
        /**
        * Add the clipboard contents to the text queue and begin speaking it.
        */
        virtual ASYNC speakClipboard() = 0;
        
        /**
        * Displays the %KTTS Manager dialog.  In this dialog, the user may backup or skip forward in
        * any text job by sentence or paragraph, rewind jobs, pause or resume jobs, or
        * delete jobs.
        */
        virtual void showDialog() = 0;

        /**
        * Stop the service.
        */
        virtual void kttsdExit() = 0;

        /**
        * Re-start %KTTSD.
        */
        virtual void reinit() = 0;
        //@}
        
    k_dcop_signals:
        void ignoreThis();
    
        /** @name DCOP Signals */
        //@{
        
        /**
        * This signal is emitted when KTTSD starts or restarts after a call to reinit.
        */
        void kttsdStarted();
        /**
        * This signal is emitted just before KTTSD exits.
        */
        void kttsdExiting();
        /**
        * This signal is emitted when the speech engine/plugin encounters a marker in the text.
        * @param appId          DCOP application ID of the application that queued the text.
        * @param markerName     The name of the marker seen.
        * @see markers
        */
        void markerSeen(const QCString& appId, const QString& markerName);
        /**
        * This signal is emitted whenever a sentence begins speaking.
        * @param appId          DCOP application ID of the application that queued the text.
        * @param jobNum         Job number of the text job.
        * @param seq            Sequence number of the text.
        * @see getTextCount
        */
        void sentenceStarted(const QCString& appId, const uint jobNum, const uint seq);
        /**
        * This signal is emitted when a sentence has finished speaking.
        * @param appId          DCOP application ID of the application that queued the text.
        * @param jobNum         Job number of the text job.
        * @param seq            Sequence number of the text.
        * @see getTextCount
        */        
        void sentenceFinished(const QCString& appId, const uint jobNum, const uint seq);
        
        /**
        * This signal is emitted whenever a new text job is added to the queue.
        * @param appId          The DCOP senderId of the application that created the job.
        * @param jobNum         Job number of the text job.
        */
        void textSet(const QCString& appId, const uint jobNum);
    
        /**
        * This signal is emitted whenever speaking of a text job begins.
        * @param appId          The DCOP senderId of the application that created the job.
        * @param jobNum         Job number of the text job.
        */
        void textStarted(const QCString& appId, const uint jobNum);
        /**
        * This signal is emitted whenever a text job is finished.  The job has
        * been marked for deletion from the queue and will be deleted when another
        * job reaches the Finished state. (Only one job in the text queue may be
        * in state Finished at one time.)  If @ref startText or @ref resumeText is
        * called before the job is deleted, it will remain in the queue for speaking.
        * @param appId          The DCOP senderId of the application that created the job.
        * @param jobNum         Job number of the text job.
        */
        void textFinished(const QCString& appId, const uint jobNum);
        /**
        * This signal is emitted whenever a speaking text job stops speaking.
        * @param appId          The DCOP senderId of the application that created the job.
        * @param jobNum         Job number of the text job.
        */
        void textStopped(const QCString& appId, const uint jobNum);
        /**
        * This signal is emitted whenever a speaking text job is paused.
        * @param appId          The DCOP senderId of the application that created the job.
        * @param jobNum         Job number of the text job.
        */
        void textPaused(const QCString& appId, const uint jobNum);
        /**
        * This signal is emitted when a text job, that was previously paused, resumes speaking.
        * @param appId          The DCOP senderId of the application that created the job.
        * @param jobNum         Job number of the text job.
        */
        void textResumed(const QCString& appId, const uint jobNum);
        /**
        * This signal is emitted whenever a text job is deleted from the queue.
        * The job is no longer in the queue when this signal is emitted.
        * @param appId          The DCOP senderId of the application that created the job.
        * @param jobNum         Job number of the text job.
        */
        void textRemoved(const QCString& appId, const uint jobNum);
        //@}
};
#endif // _KSPEECH_H_
