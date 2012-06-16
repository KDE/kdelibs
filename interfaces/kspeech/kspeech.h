/***************************************************** vim:set ts=4 sw=4 sts=4:
  KSpeech
  
  The KDE Text-to-Speech API.
  ------------------------------
  Copyright:
  (C) 2006 by Gary Cramblitt <garycramblitt@comcast.net>
  (C) 2009 by Jeremy Whiting <jpwhiting@kde.org>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 ******************************************************************************/

#ifndef KSPEECH_H
#define KSPEECH_H

// Qt includes
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QByteArray>

namespace KSpeech
{
    /**
    * @enum JobPriority
    * Determines the priority of jobs submitted by @ref say.
    * maps directly to SPDPriority
    */
    enum JobPriority
    {
        jpAll                = 0,    /**< All priorities.  Used for information retrieval only. */
        jpScreenReaderOutput = 1,    /**< Screen Reader job. SPD_IMPORTANT */
        jpWarning            = 2,    /**< Warning job. SPD_NOTIFICATION */
        jpMessage            = 3,    /**< Message job.SPD_MESSAGE */
        jpText               = 4,    /**< Text job. SPD_TEXT */
        jpProgress           = 5     /**< Progress report. SPD_PROGRESS added KDE 4.4 */ 
    };
    
    /**
    * @enum JobState
    * Job states returned by method @ref getJobState.
    */
    enum JobState
    {
        jsQueued      = 0,  /**< Job has been queued but is not yet speakable. */
        jsFiltering   = 1,  /**< Job is being filtered. */
        jsSpeakable   = 2,  /**< Job is speakable, but is not speaking. */
        jsSpeaking    = 3,  /**< Job is currently speaking. */
        jsPaused      = 4,  /**< Job is paused. */
        jsInterrupted = 5,  /**< Job is paused because it has been interrupted by another job. */
        jsFinished    = 6,  /**< Job is finished and is deleteable. */
        jsDeleted     = 7   /**< Job is deleted from the queue. */
    };
    
    /**
    * @enum SayOptions
    * Hints about text content when sending via @ref say.
    */
    enum SayOptions
    {
        soNone      = 0x0000,   /**< No options specified.  Autodetected. */
        soPlainText = 0x0001,   /**< The text contains plain text. */
        soHtml      = 0x0002,   /**< The text contains HTML markup. */
        soSsml      = 0x0004,   /**< The text contains SSML markup. */
        soChar      = 0x0008,   /**< The text should be spoken as individual characters. */
        soKey       = 0x0010,   /**< The text contains a keyboard symbolic key name. */
        soSoundIcon = 0x0020    /**< The text is the name of a sound icon. */
    };
    
    /**
    * @enum TalkerCapabilities1
    * Flags for synthesizer/talker capabilities.
    * All items marked FALSE are hard-coded off at this time.
    */
    enum TalkerCapabilities1
    {
        tcCanListVoices                         = 0x00000001,
        tcCanSetVoiceByProperties               = 0x00000002,
        tcCanGetCurrentVoice                    = 0x00000004,
        tcCanSetRateRelative                    = 0x00000008, /**< FALSE */
        tcCanSetRateAbsolute                    = 0x00000010,
        tcCanGetRateDefault                     = 0x00000020,
        tcCanSetPitchRelative                   = 0x00000040, /**< FALSE */
        tcCanSetPitchAbsolute                   = 0x00000080,
        tcCanGetPitchDefault                    = 0x00000100,
        tcCanSetPitchRangeRelative              = 0x00000200, /**< FALSE */
        tcCanSetPitchRangeAbsolute              = 0x00000400, /**< FALSE */
        tcCanGetPitchRangeDefault               = 0x00000800, /**< FALSE */
        tcCanSetVolumeRelative                  = 0x00001000, /**< FALSE */
        tcCanSetVolumeAbsolute                  = 0x00002000,
        tcCanGetVolumeDefault                   = 0x00004000,
        tcCanSetPunctuationModeAll              = 0x00008000, /**< FALSE */
        tcCanSetPunctuationModeNone             = 0x00010000, /**< FALSE */
        tcCanSetPunctuationModeSome             = 0x00020000, /**< FALSE */
        tcCanSetPunctuationDetail               = 0x00040000, /**< FALSE */
        tcCanSetCapitalLettersModeSpelling      = 0x00080000, /**< FALSE */
        tcCanSetCapitalLettersModeIcon          = 0x00100000, /**< FALSE */
        tcCanSetCapitalLettersModePitch         = 0x00200000, /**< FALSE */
        tcCanSetNumberGrouping                  = 0x00400000, /**< FALSE */
        tcCanSayTextFromPosition                = 0x00800000, /**< FALSE */
        tcCanSayChar                            = 0x01000000, /**< FALSE */
        tcCanSayKey                             = 0x02000000, /**< FALSE */
        tcCanSayIcon                            = 0x04000000, /**< FALSE */
        tcCanSetDictionary                      = 0x08000000, /**< FALSE */
        tcCanRetrieveAudio                      = 0x10000000, /**< FALSE */
        tcCanPlayAudio                          = 0x20000000  /**< FALSE */
    };
    
    /**
    * @enum TalkerCapabilities2
    * All items marked FALSE are hard-coded off at this time.
    */
    enum TalkerCapabilities2
    {
        tcCanReportEventsBySentences            = 0x00000001,
        tcCanReportEventsByWords                = 0x00000002, /**< FALSE */
        tcCanReportCustomIndexMarks             = 0x00000004, /**< FALSE */
        tcHonorsPerformanceGuidelines1          = 0x00000008, /**< FALSE */
        tcHonorsPerformanceGuidelines2          = 0x00000010, /**< FALSE */
        tcHonorsPerformanceGuidelines           = 0x00000018, /**< FALSE */
        tcCanDeferMessage                       = 0x00000020, /**< FALSE */
        tcCanParseSsml                          = 0x00000040,
        tcSupportsMultilingualUtterances        = 0x00000080, /**< FALSE */
        tcCanParseHtml                          = 0x00000100
    };

    /**
    * @enum MarkerType
    * Types of markers emitted by @ref marker signal.
    */
    enum MarkerType
    {
        mtSentenceBegin = 0,
        mtSentenceEnd   = 1,
        mtWordBegin     = 2,
        mtPhonemeBegin  = 3,
        mtCustom        = 4
    };

};

#endif // KSPEECH_H
