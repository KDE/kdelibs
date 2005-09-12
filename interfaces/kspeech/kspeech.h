/***************************************************** vim:set ts=4 sw=4 sts=4:
  kspeech.h
  KTTSD DCOP Interface
  --------------------
  Copyright:
  (C) 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  (C) 2003-2004 by Olaf Schmidt <ojschmidt@kde.org>
  (C) 2004-2005 by Gary Cramblitt <garycramblitt@comcast.net>
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

#include <Q3CString>
#include <dcopobject.h>
#include <qstringlist.h>

/**
 * @interface KSpeech
 *
 * kspeech - the KDE Text-to-Speech API.
 *
 * @version 1.0 Draft 10
 *
 * @since KDE 3.4
 *
 * This class defines the DCOP interface for applications desiring to speak text.
 * Applications may speak text by sending DCOP messages to application "kttsd" object "KSpeech".
 *
 * %KTTSD -- the KDE Text-to-Speech Deamon -- is the program that supplies the services
 * in the KDE Text-to-Speech API.
 *
 * @warning The KSpeech interface is still being developed and is likely to change in the future.
 *
 * @section Features
 *
 *   - Priority system for Screen Readers, warnings and messages, while still playing
 *     regular texts.
 *   - Long text is parsed into sentences.  User may backup by sentence or part,
 *     replay, pause, and stop playing.
 *   - Handles multiple speaking applications.  Text messages are treated like print jobs.
 *     Jobs may be created, started, stopped, paused, resumed, and deleted.
 *   - Speak contents of clipboard.
 *   - Speak KDE notifications.
 *   - Plugin-based text job filtering permits substitution for misspoken words,
 *     abbreviations, etc., transformation of XML or XHTML to SSML, and automatic
 *     choice of appropriate synthesis engine.
 *
 * @section Requirements
 *
 * You may build any KDE application to use KSpeech, since the interface is in kdelibs, but
 * the kdeaccessibility package must be installed for KTTS to function.
 *
 * You will need a speech synthesis engine, such as Festival.  See the KTTS Handbook
 * for the latest information on installing and configuring speech engines and voices
 * with KTTS.
 *
 * @section goals Design Goals
 *
 * The KDE Text-to-Speech API is designed with the following goals:
 *
 *   - Support the features enumerated above.
 *   - Plugin-based architecture for support of a wide variety of speech synthesis
 *     engines and drivers.
 *   - Permit generation of speech from the command line (or via shell scripts)
 *     using the KDE DCOP utilities.
 *   - Provide a lightweight and easily usable interface for applications to
 *     generate speech output.
 *   - Applications need not be concerned about contention over the speech device.
 *   - Provide limited support for speech markup languages, such as Sable,
 *     Java %Speech Markup Language (JSML), and %Speech Markup Meta-language (SMML).
 *   - Provide limited support for embedded speech markers.
 *   - Asynchronous to prevent system blocking.
 *   - Plugin-based audio architecture.  Currently supports aRts but will support
 *     additional audio engines in the future, such as gstreamer.
 *   - Compatible with original %KTTSD API as developed by José Pablo Ezequiel
 *     "Pupeno" Fernández (avoid breaking existing applications).
 *
 * Architecturally, applications interface with %KTTSD, which performs queueing,
 * speech job managment, plugin management and sentence parsing.  %KTTSD interfaces with a
 * %KTTSD speech plugin(s), which then interfaces with the speech engine(s) or driver(s).
 *
   @verbatim
         application
              ^
              |  via DCOP (the KDE Text-to-Speech API)
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
 * The %KTTSD Plugin API is documented in PluginConf in the kdeaccessibility module.
 *
 * There is a separate GUI application, called kttsmgr, for providing %KTTSD
 * configuration and job management.
 *
 * kttsd maintains 4 types of speech output:
 *   - Screen Reader Output
 *   - Warnings
 *   - Messages
 *   - Text Jobs
 *
 * Method sayScreenReaderOutput speaks Screen Reader output.
 * It pre-empts any other speech in progress,
 * including other Screen Reader outputs, i.e., it is not a queue.
 * This method is reserved for use by Screen Readers.
 *
 * Methods sayWarning and sayMessage place messages into the Warnings and
 * Messages queues respectively.  Warnings take priority over messages, which take priority
 * over text jobs.  Warnings and messages are spoken when the currently-speaking
 * sentence of a text job is finished.
 *
 * setText places text into the text job queue. startText begins speaking jobs.
 * When one job finishes, the next job begins.  Method appendText adds
 * additional parts to a text job.  Within a text job, the application (and user
 * via the kttsmgr GUI), may back up or advance by sentence or part, or rewind
 * to the beginning.
 * See jumpToTextPart and moveRelTextSentence.
 * Text jobs may be paused, stopped, and resumed or deleted from the queue.
 * See pauseText, stopText, resumeText, and removeText.
 *
 * @section cmdline DCOP Command-line Interface
 *
 * To create a text job to be spoken
 *
   @verbatim
     dcop kttsd KSpeech setText <text> <talker>
   @endverbatim
 *
 * where \<text\> is the text to be spoken, and \<talker\> is usually a language code
 * such as "en", "cy", etc.
 *
 * Example.
 *
   @verbatim
     dcop kttsd KSpeech setText "This is a test." "en"
   @endverbatim
 *
 * To start speaking the text.
 *
   @verbatim
     dcop kttsd KSpeech startText 0
   @endverbatim
 *
 * You can combine the setText and startText commands into a single command.
 *
   @verbatim
     dcop kttsd KSpeech sayText <text> <talker>
   @endverbatim
 *
 * @since KDE 3.5
 *
 * To stop speaking and rewind to the beginning of the text.
 *
   @verbatim
     dcop kttsd KSpeech stopText 0
   @endverbatim
 *
 * Depending upon the speech plugin used, speaking may not immediately stop.
 *
 * To stop and remove a text job.
 *
   @verbatim
     dcop kttsd KSpeech removeText 0
   @endverbatim
 *
 * Note: For more information about talker codes, see talkers below.
 *
 * @section programming Calling KTTSD from a Program
 *
 * There are two methods of making DCOP calls from your application to %KTTSD.
 *
 *   - Manually code them using dcopClient object.  See kdebase/konqueror/kttsplugin/khtmlkttsd.cpp
 *     for an example.  This method is recommended if you want to make a few simple calls to KTTSD.
 *   - Use kspeech_stub as described below.  This method generates the marshalling code for you
 *     and is recommended for a more complex speech-enabled applications.  kcmkttsmgr in the
 *     kdeaccessibility module is an example that uses this method.
 *
 * To make DCOP calls from your program using kspeech_stub, follow these steps:
 *
 * 1.  Include kspeech_stub.h in your code.  Derive an object from the KSpeech_stub interface.
 *     For example, suppose you are developing a KPart and want to call %KTTSD.
 *     Your class declaration might look like this:
 *
   @verbatim
     #include <kspeech_stub.h>
     class MyPart: public KParts::ReadOnlyPart, public KSpeech_stub {
   @endverbatim
 *
 * 2.  In your class constructor, initialize DCOPStub, giving it the sender
 *     "kttsd", object "KSpeech".
 *
   @verbatim
     MyPart::MyPart(QWidget *parent) :
        KParts::ReadOnlyPart(parent),
        DCOPStub("kttsd", "KSpeech") {
   @endverbatim
 *
 * 3.  See if KTTSD is running, and if not, start it.
 *
   @verbatim
     DCOPClient *client = dcopClient();
     client->attach();
     if (!client->isApplicationRegistered("kttsd")) {
         QString error;
         if (KApplication::startServiceByDesktopName("kttsd", QStringList(), &error))
             cout << "Starting KTTSD failed with message " << error << endl;
     }
   @endverbatim
 *
 * If you want to detect if KTTSD is installed without starting it, use this code.
 *
   @verbatim
     KTrader::OfferList offers = KTrader::self()->query("DCOP/Text-to-Speech", "Name == 'KTTSD'");
     if (offers.count() > 0)
     {
       // KTTSD is installed.
     }
   @endverbatim
 *
 * Typically, you would do this to hide a menu item or button if KTTSD is not installed.
 *
 * 4.  Make calls to KTTSD in your code.
 *
   @verbatim
     uint jobNum = setText("Hello World", "en");
     startText(jobNum);
   @endverbatim
 *
 * 4.  Add kspeech_DIR and kspeech.stub to your Makefile.am.  Example:
 *
   @verbatim
     kspeech_DIR = $(kde_includes)
     libmypart_la_SOURCES = kspeech.stub
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
 *     and declare a method for each signal you'd like to receive.  For example,
 *     if you were coding a KPart and wanted to receive the KTTSD signal sentenceStarted:
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
         public KSpeech_stub,
         virtual public KSpeechSink
     {
         protected:
            ASYNC sentenceStarted(const QCString& appId, const uint jobNum, const uint seq);
   @endverbatim
 *
 *     See below for the signals you can declare.
 *
 * 2.  In your class constructor, initialize DCOPObject with the name of your DCOP
 *     receiving object.
 *
   @verbatim
     MyPart::MyPart(QWidget *parent) :
         KParts::ReadOnlyPart(parent),
         DCOPObject("mypart_kspeechsink") {
   @endverbatim
 *
 *     Use any name you like.
 *
 * 3.  Where appropriate (usually in your constructor), make sure your DCOPClient
 *     is registered and connect the %KTTSD DCOP signals to your declared receiving
 *     methods.
 *
   @verbatim
     // Register DCOP client.
     DCOPClient *client = KApplication::dcopClient();
     if (!client->isRegistered())
     {
         client->attach();
         client->registerAs(kapp->name());
     }
     // Connect KTTSD DCOP signals to our slots.
     connectDCOPSignal("kttsd", "KSpeech",
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
         if (appId != dcopClient()->appId()) return;
         // Do something here.
     }
   @endverbatim
 *
 * 5.  Add kspeechsink_DIR and kspeechsink.skel to your Makefile.am.  Example for an app
 *     both sending and receiving.
 *
   @verbatim
     kspeech_DIR = $(kde_includes)
     kspeechsink_DIR = $(kde_includes)
     libmypart_la_SOURCES = kspeech.stub kspeechsink.skel
   @endverbatim
 *
 * @section talkers Talkers, Talker Codes, and Plugins
 *
 * Many of the methods permit you to specify a desired "talker".  This
 * may be a simple language code, such as "en" for English, "es" for Spanish, etc.
 * Code as NULL to use the default configured talker.
 *
 * Within KTTSMGR, the user has the ability to configure more than one talker for each language,
 * with different voices, genders, volumes, and talking speeds.
 *
 * Talker codes serve two functions:
 * - They identify configured plugins, and
 * - They provide a way for applications to specify the desired speaking attributes
 *   that influence the choice of plugin to speak text.
 *
 * A Talker Code consists of a series of XML tags and attributes.
 * An example of a full Talker Code with all attributes specified is
 *
 *   <voice lang="en" name="kal" gender="male"/>
 *   <prosody volume="soft" rate="fast"/>
 *   <kttsd synthesizer="Festival" />
 *
 * (The @e voice and @e prosody tags are adapted from the W3C Speech Synthesis
 * Markup Language (SSML) and Java Speech Markup Language (JSML).
 * The @e kttsd tag is an extension to the SMML and JSML languages to support
 * named synthesizers and text encodings.)
 * %KTTS doesn't really care about the @e voice, @e prosody, and @e kttsd tags.  In fact,
 * they may be omitted and just the attributes specified.  The example above then
 * becomes
 *
 *   lang="en" name="kal" gender="male" volume="soft" rate="fast"
 *   synthesizer="Festival"
 *
 * The attributes may be specified in any order.
 *
 * For clarity, the rest of the discussion
 * will omit the @e voice, @e prosody, and @e kttsd tags.
 *
 * The attributes that make up a talker code are:
 *
 * - @e lang.         Language code and optional country code.
 *                    Examples: en, es, en_US, en_GB. Codes
 *                    are case in-sensitive and hyphen (-) or underscore (_) may be
 *                    used to separate the country code from the language code.
 * - @e synthesizer.  The name of the synthesizer (plugin) used to produce the speech.
 * - @e gender.       May be either "male", "female", or "neutral".
 * - @e name.         The name of the voice code.
 *                    The choice of voice codes is synthesizer-specific.
 * - @e volume.       May be "loud", "medium", or "quiet".  A synonym for "quiet" is
 *                    "soft".
 * - @e rate.         May be "fast", "medium", or "slow".
 *
 * Each plugin, once it has been configured by a user in kttsmgr, returns a
 * fully-specified talker code to identify itself.  If the plugin supports it,
 * the user may configure another instance of the plugin with a different set
 * of attributes.  This is the difference between a "plugin" and a "talker".
 * A talker is a configured instance of a plugin.  Each plugin (if it supports it)
 * may be configured as multiple talkers.
 *
 * When the user configures %KTTSD, she configures one or more talkers and then
 * places them in preferred order, top to bottom in kttsmgr.  In effect,
 * she specifies her preferences for each of the talkers.
 *
 * When applications specify a talker code, they need not (and typically do not)
 * give a full specification.  An example of a talker code with only some of the
 * attributes specified might be
 *
 *   lang="en" gender="female"
 *
 * If the talker code is not in XML attribute format, it assumed to be a @e lang
 * attribute.  So the talker code
 *
 *   en
 *
 * is interpreted as
 *
 *   lang="en"
 *
 * When a program requests a talker code in calls to setText, appendText,
 * sayMessage, sayWarning, and sayScreenReaderOutput,
 * %KTTSD tries to match the requested talker code to the closest matching
 * configured talker.
 *
 * The @e lang attribute has highest priority (attempting to speak English with
 * a Spanish synthesizer would likely be unintelligible).  So the language
 * attribute is said to have "priority".
 * If an application does not specify a language attribute, a default one will be assumed.
 * The rest of the attributes are said to be "preferred".  If %KTTSD cannot find
 * a talker with the exact preferred attributes requested, the closest matching
 * talker will likely still be understandable.
 *
 * An application may specify that one or more of the attributes it gives in a talker
 * code have priority by preceeding each priority attribute with an asterisk.
 * For example, the following talker code
 *
 *  lang="en" gender="*female" volume="soft"
 *
 * means that the application wants to use a talker that supports American English language
 * and Female gender.  If there is more than one such talker, one that supports
 * Soft volume would be preferred.  Notice that a talker configured as English, Male,
 * and Soft volume would not be picked as long as an English Female talker is
 * available.
 *
 * The algorithm used by %KTTSD to find a matching talker is as follows:
 *
 * - If language code is not specified by the application, assume default configured
 *   by user.  The primary language code automatically has priority.
 * - (Note: This is not yet implemented.)
 *   If there are no talkers configured in the language, %KTTSD will attempt
 *   to automatically configure one (see automatic configuraton discussion below)
 * - The talker that matches on the most priority attributes wins.
 * - If a tie, the one that matches on the most preferred attributes wins.
 * - If there is still a tie, the one nearest the top of the kttsmgr display
 *   (first configured) will be chosen.
 *
 * Language codes actually consist of two parts, a language code and an optional
 * country code.  For example, en_GB is English (United Kingdom).  The language code is
 * treated as a priority attribute, but the country code (if specified) is treated
 * as preferred.  So for example, if an application requests the following
 * talker code
 *
 *   lang="en_GB" gender="male" volume="medium"
 *
 * then a talker configured as lang="en" gender="male" volume="medium" would be
 * picked over one configured as lang="en_GB" gender="female" volume="soft",
 * since the former matches on two preferred attributes and the latter only on the
 * preferred attribute GB. An application can override this and make the country
 * code priority with an asterisk.  For example,
 *
 *   lang="*en_GB" gender="male" volume="medium"
 *
 * To specify that American English is priority, put an asterisk in front of
 * en_US, like this.
 *
 *   lang="*en_US" gender="male" volume="medium"
 *
 * Here the application is indicating that a talker that speaks American English
 * has priorty over one that speaks a different form of English.
 *
 * (Note: Not yet implemented).
 * If a language code is specified, and no plugin is currently configured
 * with a matching language code, %KTTSD will attempt to automatically
 * load and configure a plugin to support the requested language.  If
 * there is no such plugin, or there is a plugin but it cannot automatically
 * configure itself, %KTTSD will pick one of the configured plugins using the
 * algorithm given above.
 *
 * Notice that %KTTSD will always pick a talker, even if it is a terrible match.
 * (The principle is that something heard is better than nothing at all.  If
 * it sounds terrible, user will change his configuration.)
 * If an attribute is absolutely mandatory -- in other words the application
 * must speak with the attribute or not at all -- the application can determine if
 * there are any talkers configured with the attribute by calling getTalkers,
 * and if there are none, display an error message to the user.
 *
 * Applications can implement their own talker-matching algorithm by
 * calling getTalkers, then finding the desired talker from the returned
 * list.  When the full talker code is passed in, %KKTSD will find an exact
 * match and use the specified talker.
 *
 * If an application requires a configuration that user has not created,
 * it should display a message to user instructing them to run kttsmgr and
 * configure the desired talker.  (This must be done interactively because
 * plugins often need user assistance locating voice files, etc.)
 *
 * The above scheme is designed to balance the needs
 * of applications against user preferences.  Applications are given the control
 * they @e might need, without unnecessarily burdening the application author.
 * If you are an application author, the above discussion might seem overly
 * complicated.  It isn't really all that complicated.  Here are rules of thumb:
 *
 *   - It is legitimate to give a NULL (0) talker code, in which case, the user's default
 *     talker will be used.
 *   - If you know the language code, give that in the talker code, otherwise
 *     leave it out.
 *   - If there is an attribute your application @e requires for proper functioning,
 *     specify that with an asterisk in front of it.  For example, your app might
 *     speak in two different voices, Male and Female.  (Since your
 *     app requires both genders, call getTalkers to determine if both genders
 *     are available, and if not, advise user to configure them.  Better yet,
 *     give the user a choice of available distinquishing attributes
 *     (loud/soft, fast/slow, etc.)
 *   - If there are other attributes you would prefer, specify those without an
 *     asterisk, but leave them out if it doesn't really make any difference
 *     to proper functioning of your application.  Let the user decide them
 *     when they configure %KTTS.
 *
 * One final note about talkers.  %KTTSD does talker matching for each sentence
 * spoken, just before the sentence is sent to a plugin for synthesis.  Therefore,
 * the user can change the effective talker in mid processing of a text job by
 * changing his preferences, or even deleting or adding new talkers to the configuration.
 *
 * @section markup Speech Markup
 *
 * Note: %Speech Markup is not yet fully implemented in %KTTSD.
 *
 * Each of the five methods for queueing text to be spoken -- sayScreenReaderOutput,
 * setText, appendText, sayMessage, and sayWarning -- may contain speech markup,
 * provided that the plugin the user has configured supports that markup.  The markup
 * languages and plugins currently supported are:
 *
 *   - %Speech Synthesis Markup language (SSML): Festival and Hadifix.
 *
 * This may change in the future as synthesizers improve.
 *
 * Before including markup in the text sent to kttsd, the application should
 * query whether the currently-configured plugin
 * supports the markup language by calling supportsMarkup.
 *
 * It it does not support the markup, it will be stripped out of the text.
 *
 * @section markers Support for Markers
 *
 * Note: Markers are not yet implemented in %KTTSD.
 *
 * When using a speech markup language, such as Sable, JSML, or SSML, the application may embed
 * named markers into the text.  If the user's chosen speech plugin supports markers, %KTTSD
 * will emit DCOP signal markerSeen when the speech engine encounters the marker.
 * Depending upon the speech engine and plugin, this may occur either when the speech engine
 * encounters the marker during synthesis from text to speech, or when the speech is actually
 * spoken on the audio device.  The calling application can call the supportsMarkers
 * method to determine if the currently configured plugin supports markers or not.
 *
 * @section sentenceparsing Sentence Parsing
 *
 * Not all speech engines provide robust capabilities for stopping synthesis that is in progress.
 * To compensate for this, %KTTSD parses text jobs given to it by the setText and
 * appendText methods into sentences and sends the sentences to the speech
 * plugin one at a time.  In this way, should the user wish to stop the speech
 * output, they can do so, and the worst that will happen is that the last sentence
 * will be completed.  This is called Sentence Boundary Detection (SBD).
 *
 * Sentence Boundary Detection also permits the user to rewind by sentences.
 *
 * The default sentence delimiter used for plain text is as follows:
 *
 *   - A period (.), question mark (?), exclamation mark (!), colon (:), or
 *     semi-colon (;) followed by whitespace (including newline), or
 *   - Two newlines in a row separated by optional whitespace, or
 *   - The end of the text.
 *
 * When given text containing speech markup, %KTTSD automatically determines the markup type
 * and parses based on the sentence semantics of the markup language.
 *
 * An application may change the sentence delimiter by calling setSentenceDelimiter
 * prior to calling setText.  Changing the delimiter does not affect other
 * applications.
 *
 * Text given to %KTTSD via the sayWarning, sayMessage, and sayScreenReaderOutput
 * methods is @e not parsed into sentences.  For this reason, applications
 * should @e not send long messages with these methods.
 *
 * Sentence Boundary Detection is implemented as a plugin SBD filter.  See
 * filters for more information.
 *
 * @section filters Filters
 *
 * Users may specify filters in the kttsmgr GUI.  Filters are plugins that modify the text
 * to be spoken or change other characteristics of jobs.  Currently, the following filter plugins
 * are available:
 *
 *   - String Replacer.  Permits users to substitute for mispoken words, or vocalize chat
 *     emoticons.
 *   - XML Transformer.  Given a particular XML or XHTML format, permits conversion of the
 *     XML to SSML (Speech Synthesis Markup Language) using XSLT (XML Style Language - Transforms)
 *     stylesheets.
 *   - Talker Chooser.  Permits users to redirect jobs from one configured Talker to another
 *     based on the contents of the job or application that sent it.
 *
 * Additional plugins may be available in the future.
 *
 * In additional to these regular filters, KTTS also implements Sentence Boundary Detection (SBD)
 * as a plugin filter.  See sentenceparsing for more information.
 *
 * Regular filters are applied to Warnings, Messages, and Text jobs.  SBD filters are
 * only applied to regular Text jobs; they are not applied to Warnings and Messages.  Screen
 * Reader Outputs are never filtered.
 *
 * @section authors Authors
 *
 * @author José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
 * @author Gary Cramblitt <garycramblitt@comcast.net>
 * @author Olaf Schmidt <ojschmidt@kde.org>
 * @author Gunnar Schmi Dt <gunnar@schmi-dt.de>
 */

// NOTE: kspeech class is now obsolete.  Please use KSpeech instead.

class KSpeech : virtual public DCOPObject {
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
            mtSsml = 2,                  /**< %Speech Synthesis Markup Language */
            mtSable = 3,                 /**< Sable 2.0 */
            mtHtml = 4                   /**< HTML @since 3.5 */
        };

    k_dcop:
        /** @name DCOP Methods */
        //@{

        /**
        * Determine whether the currently-configured speech plugin supports a speech markup language.
        * @param talker         Code for the talker to do the speaking.  Example "en".
        *                       If NULL, defaults to the user's default talker.
        * @param markupType     The kttsd code for the desired speech markup language.
        * @return               True if the plugin currently configured for the indicated
        *                       talker supports the indicated speech markup language.
        * @see kttsdMarkupType
        */
        virtual bool supportsMarkup(const QString &talker, uint markupType = 0) const = 0;

        /**
        * Determine whether the currently-configured speech plugin supports markers in speech markup.
        * @param talker         Code for the talker to do the speaking.  Example "en".
        *                       If NULL, defaults to the user's default talker.
        * @return               True if the plugin currently configured for the indicated
        *                       talker supports markers.
        */
        virtual bool supportsMarkers(const QString &talker) const = 0;

        /**
        * Say a message as soon as possible, interrupting any other speech in progress.
        * IMPORTANT: This method is reserved for use by Screen Readers and should not be used
        * by any other applications.
        * @param msg            The message to be spoken.
        * @param talker         Code for the talker to do the speaking.  Example "en".
        *                       If NULL, defaults to the user's default talker.
        *                       If no plugin has been configured for the specified Talker code,
        *                       defaults to the closest matching talker.
        *
        * If an existing Screen Reader output is in progress, it is stopped and discarded and
        * replaced with this new message.
        */
        virtual ASYNC sayScreenReaderOutput(const QString &msg, const QString &talker) = 0;

        /**
        * Say a warning.  The warning will be spoken when the current sentence
        * stops speaking and takes precedence over Messages and regular text.  Warnings should only
        * be used for high-priority messages requiring immediate user attention, such as
        * "WARNING. CPU is overheating."
        * @param warning        The warning to be spoken.
        * @param talker         Code for the talker to do the speaking.  Example "en".
        *                       If NULL, defaults to the user's default talker.
        *                       If no plugin has been configured for the specified Talker code,
        *                       defaults to the closest matching talker.
        */
        virtual ASYNC sayWarning(const QString &warning, const QString &talker) = 0;

        /**
        * Say a message.  The message will be spoken when the current sentence stops speaking
        * but after any warnings have been spoken.
        * Messages should be used for one-shot messages that can't wait for
        * normal text messages to stop speaking, such as "You have mail.".
        * @param message        The message to be spoken.
        * @param talker         Code for the talker to do the speaking.  Example "en".
        *                       If NULL, defaults to the user's default talker.
        *                       If no talker has been configured for the specified talker code,
        *                       defaults to the closest matching talker.
        */
        virtual ASYNC sayMessage(const QString &message, const QString &talker) = 0;

        /**
        * Sets the GREP pattern that will be used as the sentence delimiter.
        * @param delimiter      A valid GREP pattern.
        *
        * The default sentence delimiter is
          @verbatim
              ([\\.\\?\\!\\:\\;])(\\s|$|(\\n *\\n))
          @endverbatim
        *
        * Note that backward slashes must be escaped.
        * When %KTTSD parses the text, it replaces all tabs, spaces, and formfeeds
        * with a single space, and then replaces the sentence delimiters using
        * the following statement:
          @verbatim
              QString::replace(sentenceDelimiter, "\\1\t");
          @endverbatim
        *
        * which replaces all sentence delimiters with a tab, but
        * preserving the first capture text (first parenthesis).  In other
        * words, the sentence punctuation is preserved.
        * The tab is later used to separate the text into sentences.
        *
        * Changing the sentence delimiter does not affect other applications.
        *
        * @see sentenceparsing
        */
        virtual ASYNC setSentenceDelimiter(const QString &delimiter) = 0;

        /**
        * Queue a text job.  Does not start speaking the text.
        * @param text           The message to be spoken.
        * @param talker         Code for the talker to do the speaking.  Example "en".
        *                       If NULL, defaults to the user's default plugin.
        *                       If no plugin has been configured for the specified Talker code,
        *                       defaults to the closest matching talker.
        * @return               Job number.
        *
        * Plain text is parsed into individual sentences using the current sentence delimiter.
        * Call setSentenceDelimiter to change the sentence delimiter prior to
        * calling setText.
        * Call getTextCount to retrieve the sentence count after calling setText.
        *
        * The text may contain speech mark language, such as Sable, JSML, or SSML,
        * provided that the speech plugin/engine support it.  In this case,
        * sentence parsing follows the semantics of the markup language.
        *
        * Call startText to mark the job as speakable and if the
        * job is the first speakable job in the queue, speaking will begin.
        *
        * @see getTextCount
        * @see startText
        */
        virtual uint setText(const QString &text, const QString &talker) = 0;

        /**
        * Say a plain text job.  This is a convenience method that
        * combines setText and startText into a single call.
        * @param text           The message to be spoken.
        * @param talker         Code for the talker to do the speaking.  Example "en".
        *                       If NULL, defaults to the user's default plugin.
        *                       If no plugin has been configured for the specified Talker code,
        *                       defaults to the closest matching talker.
        * @return               Job number.
        *
        * Plain text is parsed into individual sentences using the current sentence delimiter.
        * Call setSentenceDelimiter to change the sentence delimiter prior to
        * calling setText.
        * Call getTextCount to retrieve the sentence count after calling setText.
        *
        * The text may contain speech mark language, such as Sable, JSML, or SSML,
        * provided that the speech plugin/engine support it.  In this case,
        * sentence parsing follows the semantics of the markup language.
        *
        * The job is marked speakable.
        * If there are other speakable jobs preceeding this one in the queue,
        * those jobs continue speaking and when finished, this job will begin speaking.
        * If there are no other speakable jobs preceeding this one, it begins speaking.
        *
        * @see getTextCount
        *
        * @since KDE 3.5
        */
        virtual uint sayText(const QString &text, const QString &talker) = 0;

        /**
        * Adds another part to a text job.  Does not start speaking the text.
        * @param text           The message to be spoken.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the current job (if any).
        * @return               Part number for the added part.  Parts are numbered starting at 1.
        *
        * The text is parsed into individual sentences.  Call getTextCount to retrieve
        * the sentence count.  Call startText to mark the job as speakable and if the
        * job is the first speakable job in the queue, speaking will begin.
        *
        * @see setText.
        * @see startText.
        */
        virtual int appendText(const QString &text, uint jobNum=0) = 0;

        /**
        * Queue a text job from the contents of a file.  Does not start speaking the text.
        * @param filename       Full path to the file to be spoken.  May be a URL.
        * @param talker         Code for the talker to do the speaking.  Example "en".
        *                       If NULL, defaults to the user's default talker.
        *                       If no plugin has been configured for the specified Talker code,
        *                       defaults to the closest matching talker.
        * @param encoding       Name of the encoding to use when reading the file.  If
        *                       NULL or Empty, uses default stream encoding.
        * @return               Job number.  0 if an error occurs.
        *
        * Plain text is parsed into individual sentences using the current sentence delimiter.
        * Call setSentenceDelimiter to change the sentence delimiter prior to calling setText.
        * Call getTextCount to retrieve the sentence count after calling setText.
        *
        * The text may contain speech mark language, such as Sable, JSML, or SSML,
        * provided that the speech plugin/engine support it.  In this case,
        * sentence parsing follows the semantics of the markup language.
        *
        * Call startText to mark the job as speakable and if the
        * job is the first speakable job in the queue, speaking will begin.
        *
        * @see getTextCount
        * @see startText
        */
        virtual uint setFile(const QString &filename, const QString &talker,
            const QString& encoding) = 0;

        /**
        * Get the number of sentences in a text job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the current job (if any).
        * @return               The number of sentences in the job.  -1 if no such job.
        *
        * The sentences of a job are given sequence numbers from 1 to the number returned by this
        * method.  The sequence numbers are emitted in the sentenceStarted and
        * sentenceFinished signals.
        */
        virtual int getTextCount(uint jobNum=0) = 0;

        /**
        * Get the job number of the current text job.
        * @return               Job number of the current text job. 0 if no jobs.
        *
        * Note that the current job may not be speaking. See isSpeakingText.
        *
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
        *                       but if no such job, applies to the current job (if any).
        * @return               State of the job. -1 if invalid job number.
        *
        * @see kttsdJobState
        */
        virtual int getTextJobState(uint jobNum=0) = 0;

        /**
        * Get information about a text job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the current job (if any).
        * @return               A QDataStream containing information about the job.
        *                       Blank if no such job.
        *
        * The stream contains the following elements:
        *   - int state        - Job state.
        *   - QCString appId   - DCOP senderId of the application that requested the speech job.
        *   - QString talker   - Talker Code requested by application.
        *   - int seq          - Current sentence being spoken.  Sentences are numbered starting at 1.
        *   - int sentenceCount - Total number of sentences in the job.
        *   - int partNum      - Current part of the job begin spoken.  Parts are numbered starting at 1.
        *   - int partCount    - Total number of parts in the job.
        *
        * Note that sequence numbers apply to the entire job.  They do not start from 1 at the beginning of
        * each part.
        *
        * The following sample code will decode the stream:
                @code
                    QByteArray jobInfo = getTextJobInfo(jobNum);
                    QDataStream stream(jobInfo, IO_ReadOnly);
                    int state;
                    QCString appId;
                    QString talker;
                    int seq;
                    int sentenceCount;
                    int partNum;
                    int partCount;
                    stream >> state;
                    stream >> appId;
                    stream >> talker;
                    stream >> seq;
                    stream >> sentenceCount;
                    stream >> partNum;
                    stream >> partCount;
                @endcode
         */
        virtual QByteArray getTextJobInfo(uint jobNum=0) = 0;

        /**
        * Given a Talker Code, returns the Talker ID of the talker that would speak
        * a text job with that Talker Code.
        * @param talkerCode     Talker Code.
        * @return               Talker ID of the talker that would speak the text job.
        */
        virtual QString talkerCodeToTalkerId(const QString& talkerCode) = 0;

        /**
        * Return a sentence of a job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the current job (if any).
        * @param seq            Sequence number of the sentence.
        * @return               The specified sentence in the specified job.  If no such
        *                       job or sentence, returns "".
        */
        virtual QString getTextJobSentence(uint jobNum=0, uint seq=0) = 0;

        /**
        * Determine if kttsd is currently speaking any text jobs.
        * @return               True if currently speaking any text jobs.
        */
        virtual bool isSpeakingText() const = 0;

        /**
        * Remove a text job from the queue.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the current job (if any).
        *
        * The job is deleted from the queue and the textRemoved signal is emitted.
        *
        * If there is another job in the text queue, and it is marked speakable,
        * that job begins speaking.
        */
        virtual ASYNC removeText(uint jobNum=0) = 0;

        /**
        * Start a text job at the beginning.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the current job (if any).
        *
        * Rewinds the job to the beginning.
        *
        * The job is marked speakable.
        * If there are other speakable jobs preceeding this one in the queue,
        * those jobs continue speaking and when finished, this job will begin speaking.
        * If there are no other speakable jobs preceeding this one, it begins speaking.
        *
        * The textStarted signal is emitted when the text job begins speaking.
        * When all the sentences of the job have been spoken, the job is marked for deletion from
        * the text queue and the textFinished signal is emitted.
        */
        virtual ASYNC startText(uint jobNum=0) = 0;

        /**
        * Stop a text job and rewind to the beginning.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the current job (if any).
        *
        * The job is marked not speakable and will not be speakable until startText
        * or resumeText is called.
        *
        * If there are speaking jobs preceeding this one in the queue, they continue speaking.
        *
        * If the job is currently speaking, the textStopped signal is emitted,
        * the job stops speaking, and if the next job in the queue is speakable, it
        * begins speaking.
        *
        * Depending upon the speech engine and plugin used, speech may not stop immediately
        * (it might finish the current sentence).
        */
        virtual ASYNC stopText(uint jobNum=0) = 0;

        /**
        * Pause a text job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the current job (if any).
        *
        * The job is marked as paused and will not be speakable until resumeText or
        * startText is called.
        *
        * If there are speaking jobs preceeding this one in the queue, they continue speaking.
        *
        * If the job is currently speaking, the textPaused signal is emitted and the job
        * stops speaking.  Note that if the next job in the queue is speakable, it does
        * not start speaking as long as this job is paused.
        *
        * Depending upon the speech engine and plugin used, speech may not stop immediately
        * (it might finish the current sentence).
        *
        * @see resumeText
        */
        virtual ASYNC pauseText(uint jobNum=0) = 0;

        /**
        * Start or resume a text job where it was paused.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the current job (if any).
        *
        * The job is marked speakable.
        *
        * If the job is currently speaking, or is waiting to be spoken (speakable
        * state), the resumeText() call is ignored.
        *
        * If the job is currently queued, or is finished, it is the same as calling
        * @see startText .
        *
        * If there are speaking jobs preceeding this one in the queue,
        * those jobs continue speaking and when finished this job will begin
        * speaking where it left off.
        *
        * The textResumed signal is emitted when the job resumes.
        *
        * @see pauseText
        */
        virtual ASYNC resumeText(uint jobNum=0) = 0;

        /**
        * Get a list of the talkers configured in KTTS.
        * @return               A QStringList of fully-specified talker codes, one
        *                       for each talker user has configured.
        *
        * @see talkers
        */
        virtual QStringList getTalkers() = 0;

        /**
        * Change the talker for a text job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the current job (if any).
        * @param talker         New code for the talker to do the speaking.  Example "en".
        *                       If NULL, defaults to the user's default talker.
        *                       If no plugin has been configured for the specified Talker code,
        *                       defaults to the closest matching talker.
        */
        virtual ASYNC changeTextTalker(const QString &talker, uint jobNum=0 ) = 0;

        /**
        * Get the user's default talker.
        * @return               A fully-specified talker code.
        *
        * @see talkers
        * @see getTalkers
        */
        virtual QString userDefaultTalker() = 0;

        /**
        * Move a text job down in the queue so that it is spoken later.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the current job (if any).
        *
        * If the job is currently speaking, it is paused.
        * If the next job in the queue is speakable, it begins speaking.
        */
        virtual ASYNC moveTextLater(uint jobNum=0) = 0;

        /**
        * Jump to the first sentence of a specified part of a text job.
        * @param partNum        Part number of the part to jump to.  Parts are numbered starting at 1.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the current job (if any).
        * @return               Part number of the part actually jumped to.
        *
        * If partNum is greater than the number of parts in the job, jumps to last part.
        * If partNum is 0, does nothing and returns the current part number.
        * If no such job, does nothing and returns 0.
        * Does not affect the current speaking/not-speaking state of the job.
        */
        virtual int jumpToTextPart(int partNum, uint jobNum=0) = 0;

        /**
        * Advance or rewind N sentences in a text job.
        * @param n              Number of sentences to advance (positive) or rewind (negative) in the job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the current job (if any).
        * @return               Sequence number of the sentence actually moved to.  Sequence numbers
        *                       are numbered starting at 1.
        *
        * If no such job, does nothing and returns 0.
        * If n is zero, returns the current sequence number of the job.
        * Does not affect the current speaking/not-speaking state of the job.
        */
        virtual uint moveRelTextSentence(int n, uint jobNum=0) = 0;

        /**
        * Add the clipboard contents to the text queue and begin speaking it.
        */
        virtual ASYNC speakClipboard() = 0;

        /**
        * Displays the %KTTS Manager dialog.  In this dialog, the user may backup or skip forward in
        * any text job by sentence or part, rewind jobs, pause or resume jobs, or
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
        *
        * @see markers
        */
        void markerSeen(const Q3CString& appId, const QString& markerName);
        /**
        * This signal is emitted whenever a sentence begins speaking.
        * @param appId          DCOP application ID of the application that queued the text.
        * @param jobNum         Job number of the text job.
        * @param seq            Sequence number of the text.
        *
        * @see getTextCount
        */
        void sentenceStarted(const Q3CString& appId, uint jobNum, uint seq);
        /**
        * This signal is emitted when a sentence has finished speaking.
        * @param appId          DCOP application ID of the application that queued the text.
        * @param jobNum         Job number of the text job.
        * @param seq            Sequence number of the text.
        *
        * @see getTextCount
        */
        void sentenceFinished(const Q3CString& appId, uint jobNum, uint seq);

        /**
        * This signal is emitted whenever a new text job is added to the queue.
        * @param appId          The DCOP senderId of the application that created the job.
        * @param jobNum         Job number of the text job.
        */
        void textSet(const Q3CString& appId, uint jobNum);

        /**
        * This signal is emitted whenever a new part is appended to a text job.
        * @param appId          The DCOP senderId of the application that created the job.
        * @param jobNum         Job number of the text job.
        * @param partNum        Part number of the new part.  Parts are numbered starting
        *                       at 1.
        */
        void textAppended(const Q3CString& appId, uint jobNum, int partNum);

        /**
        * This signal is emitted whenever speaking of a text job begins.
        * @param appId          The DCOP senderId of the application that created the job.
        * @param jobNum         Job number of the text job.
        */
        void textStarted(const Q3CString& appId, uint jobNum);
        /**
        * This signal is emitted whenever a text job is finished.  The job has
        * been marked for deletion from the queue and will be deleted when another
        * job reaches the Finished state. (Only one job in the text queue may be
        * in state Finished at one time.)  If startText or resumeText is
        * called before the job is deleted, it will remain in the queue for speaking.
        * @param appId          The DCOP senderId of the application that created the job.
        * @param jobNum         Job number of the text job.
        */
        void textFinished(const Q3CString& appId, uint jobNum);
        /**
        * This signal is emitted whenever a speaking text job stops speaking.
        * @param appId          The DCOP senderId of the application that created the job.
        * @param jobNum         Job number of the text job.
        *
        * The signal is only emitted if stopText() is called and the job is currently
        * speaking.
        */
        void textStopped(const Q3CString& appId, uint jobNum);
        /**
        * This signal is emitted whenever a speaking text job is paused.
        * @param appId          The DCOP senderId of the application that created the job.
        * @param jobNum         Job number of the text job.
        */
        void textPaused(const Q3CString& appId, uint jobNum);
        /**
        * This signal is emitted when a text job, that was previously paused, resumes speaking.
        * @param appId          The DCOP senderId of the application that created the job.
        * @param jobNum         Job number of the text job.
        */
        void textResumed(const Q3CString& appId, uint jobNum);
        /**
        * This signal is emitted whenever a text job is deleted from the queue.
        * The job is no longer in the queue when this signal is emitted.
        * @param appId          The DCOP senderId of the application that created the job.
        * @param jobNum         Job number of the text job.
        */
        void textRemoved(const Q3CString& appId, uint jobNum);
        //@}
};

#endif // _KSPEECH_H_
