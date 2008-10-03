/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include <wtf/Platform.h>

#include "HTMLMediaElement.h"
#include "HTMLDocument.h"
#include <misc/htmlhashes.h>
#include <rendering/render_object.h>
#include "MediaError.h"
#include "TimeRanges.h"

const double doubleMax = 999999999.8; // ### numeric_limits<double>::max()
const double doubleInf = 999999999.0; // ### numeric_limits<double>::infinity()

using namespace DOM;

namespace khtml {

HTMLMediaElement::HTMLMediaElement(Document* doc)
    : HTMLElement(doc)
    , m_defaultPlaybackRate(1.0f)
    , m_networkState(EMPTY)
    , m_readyState(DATA_UNAVAILABLE)
    , m_begun(false)
    , m_loadedFirstFrame(false)
    , m_autoplaying(true)
    , m_currentLoop(0)
    , m_volume(0.5f)
    , m_muted(false)
    , m_paused(true)
    , m_seeking(false)
    , m_currentTimeDuringSeek(0)
    , m_previousProgress(0)
    , m_previousProgressTime(doubleMax)
    , m_sentStalledEvent(false)
    , m_bufferingRate(0)
    , m_player(0)
{
}

HTMLMediaElement::~HTMLMediaElement()
{
}

void HTMLMediaElement::attributeChanged(NodeImpl::Id attrId)
{
    HTMLElement::attributeChanged(attrId);

    if (attrId == ATTR_SRC) {
        // 3.14.9.2.
        // change to src attribute triggers load()
        if (inDocument() && m_networkState == EMPTY)
            scheduleLoad();
    } if (attrId == ATTR_CONTROLS) {
        if (!isVideo() && attached() && (controls() != (renderer() != 0))) {
            detach();
            attach();
        }
        if (renderer())
            renderer()->updateFromElement();
    }
}

void HTMLMediaElement::scheduleLoad()
{
    // ###
}

String serializeTimeOffset(float time)
{
    QString timeString = QString::number(time);
    // FIXME serialize time offset values properly (format not specified yet)
    timeString.append("s");
    return timeString;
}

float parseTimeOffset(const String& timeString, bool* ok = 0)
{
#if 0
    const UChar* characters = timeString.characters();
    unsigned length = timeString.length();
    
    if (length && characters[length - 1] == 's')
        length--;
    
    // FIXME parse time offset values (format not specified yet)
    float val = charactersToFloat(characters, length, ok);
    return val;
#else
    return timeString.string().toFloat(ok);
#endif
}

float HTMLMediaElement::getTimeOffsetAttribute(NodeImpl::Id name, float valueOnError) const
{
    bool ok;
    String timeString = getAttribute(name);
    float result = parseTimeOffset(timeString, &ok);
    if (ok)
        return result;
    return valueOnError;
}

void HTMLMediaElement::setTimeOffsetAttribute(NodeImpl::Id name, float value)
{
    setAttribute(name, serializeTimeOffset(value));
}

PassRefPtr<MediaError> HTMLMediaElement::error() const 
{
    return m_error;
}

String HTMLMediaElement::src() const
{
    return document()->completeURL(getAttribute(ATTR_SRC).string());
}

void HTMLMediaElement::setSrc(const String& url)
{
    setAttribute(ATTR_SRC, url);
}

String HTMLMediaElement::currentSrc() const
{
    return m_currentSrc;
}

HTMLMediaElement::NetworkState HTMLMediaElement::networkState() const
{
    return m_networkState;
}

float HTMLMediaElement::bufferingRate()
{
    if (!m_player)
        return 0;
    return m_bufferingRate;
}

void HTMLMediaElement::load(ExceptionCode&)
{
    // ###
}

void HTMLMediaElement::setReadyState(ReadyState state)
{
    // 3.14.9.6. The ready states
    if (m_readyState == state)
        return;
    
    // ###

    updatePlayState();
}

HTMLMediaElement::ReadyState HTMLMediaElement::readyState() const
{
    return m_readyState;
}

bool HTMLMediaElement::seeking() const
{
    return m_seeking;
}

// playback state
float HTMLMediaElement::currentTime() const
{
    if (!m_player)
        return 0;
    if (m_seeking)
        return m_currentTimeDuringSeek;
    return m_player->currentTime();
}

void HTMLMediaElement::setCurrentTime(float time, ExceptionCode& ec)
{
    //    seek(time, ec);
}

float HTMLMediaElement::duration() const
{
    return m_player ? m_player->duration() : 0;
}

bool HTMLMediaElement::paused() const
{
    return m_paused;
}

float HTMLMediaElement::defaultPlaybackRate() const
{
    return m_defaultPlaybackRate;
}

void HTMLMediaElement::setDefaultPlaybackRate(float rate, ExceptionCode& ec)
{
    if (rate == 0.0f) {
        ec = DOMException::NOT_SUPPORTED_ERR;
        return;
    }
    if (m_defaultPlaybackRate != rate) {
        m_defaultPlaybackRate = rate;
        // ### dispatchEventAsync(ratechangeEvent);
    }
}

float HTMLMediaElement::playbackRate() const
{
    return m_player ? m_player->rate() : 0;
}

void HTMLMediaElement::setPlaybackRate(float rate, ExceptionCode& ec)
{
    if (rate == 0.0f) {
        ec = DOMException::NOT_SUPPORTED_ERR;
        return;
    }
    if (m_player && m_player->rate() != rate) {
        m_player->setRate(rate);
        // ### dispatchEventAsync(ratechangeEvent);
    }
}

bool HTMLMediaElement::ended() const
{
    return endedPlayback();
}

bool HTMLMediaElement::autoplay() const
{
    return hasAttribute(ATTR_AUTOPLAY);
}

void HTMLMediaElement::setAutoplay(bool b)
{
    //   setBooleanAttribute(ATTR_AUTOPLAY, b);
}

void HTMLMediaElement::play(ExceptionCode& ec)
{
    // 3.14.9.7. Playing the media resource
    if (!m_player || networkState() == EMPTY) {
        ec = 0;
        load(ec);
        if (ec)
            return;
    }
    ExceptionCode unused;
    if (endedPlayback()) {
        m_currentLoop = 0;
        // ### seek(effectiveStart(), unused);
    }
    setPlaybackRate(defaultPlaybackRate(), unused);
    
    if (m_paused) {
        m_paused = false;
        // ### dispatchEventAsync(playEvent);
    }

    m_autoplaying = false;
    
    updatePlayState();
}

void HTMLMediaElement::pause(ExceptionCode& ec)
{
    // 3.14.9.7. Playing the media resource
    if (!m_player || networkState() == EMPTY) {
        ec = 0;
        load(ec);
        if (ec)
            return;
    }

    if (!m_paused) {
        m_paused = true;
        // ### dispatchEventAsync(timeupdateEvent);
        // ### dispatchEventAsync(pauseEvent);
    }

    m_autoplaying = false;
    
    updatePlayState();
}

unsigned HTMLMediaElement::playCount() const
{
    String val = getAttribute(ATTR_PLAYCOUNT);
    int count = val.toInt();
    return qMax(count, 1); 
}

void HTMLMediaElement::setPlayCount(unsigned count, ExceptionCode& ec)
{
    if (!count) {
        ec = DOMException::INDEX_SIZE_ERR;
        return;
    }
    setAttribute(ATTR_PLAYCOUNT, QString::number(count));
    checkIfSeekNeeded();
}

float HTMLMediaElement::start() const 
{ 
    return getTimeOffsetAttribute(ATTR_START, 0); 
}

void HTMLMediaElement::setStart(float time) 
{ 
    setTimeOffsetAttribute(ATTR_START, time); 
    checkIfSeekNeeded();
}

float HTMLMediaElement::end() const 
{ 
    return getTimeOffsetAttribute(ATTR_END, doubleInf); 
}

void HTMLMediaElement::setEnd(float time) 
{ 
    setTimeOffsetAttribute(ATTR_END, time); 
    checkIfSeekNeeded();
}

float HTMLMediaElement::loopStart() const 
{ 
    return getTimeOffsetAttribute(ATTR_LOOPSTART, 0); 
}

void HTMLMediaElement::setLoopStart(float time) 
{
    setTimeOffsetAttribute(ATTR_LOOPSTART, time); 
    checkIfSeekNeeded();
}

float HTMLMediaElement::loopEnd() const 
{ 
    return getTimeOffsetAttribute(ATTR_LOOPEND, doubleInf); 
}

void HTMLMediaElement::setLoopEnd(float time) 
{ 
    setTimeOffsetAttribute(ATTR_LOOPEND, time); 
    checkIfSeekNeeded();
}

unsigned HTMLMediaElement::currentLoop() const
{
    return m_currentLoop;
}

void HTMLMediaElement::setCurrentLoop(unsigned currentLoop)
{
    m_currentLoop = currentLoop;
}

bool HTMLMediaElement::controls() const
{
    return hasAttribute(ATTR_CONTROLS);
}

void HTMLMediaElement::setControls(bool b)
{
    // setBooleanAttribute(ATTR_CONTROLS, b);
}

float HTMLMediaElement::volume() const
{
    return m_volume;
}

void HTMLMediaElement::setVolume(float vol, ExceptionCode& ec)
{
    if (vol < 0.0f || vol > 1.0f) {
        ec = DOMException::INDEX_SIZE_ERR;
        return;
    }
    
    if (m_volume != vol) {
        m_volume = vol;
        updateVolume();
        // ### dispatchEventAsync(volumechangeEvent);
    }
}

bool HTMLMediaElement::muted() const
{
    return m_muted;
}

void HTMLMediaElement::setMuted(bool muted)
{
    if (m_muted != muted) {
        m_muted = muted;
        updateVolume();
        // ### dispatchEventAsync(volumechangeEvent);
    }
}

void HTMLMediaElement::checkIfSeekNeeded()
{
    // ###
}

PassRefPtr<TimeRanges> HTMLMediaElement::buffered() const
{
    // FIXME real ranges support
    if (!m_player || !m_player->maxTimeBuffered())
        return new TimeRanges;
    return new TimeRanges(0, m_player->maxTimeBuffered());
}

PassRefPtr<TimeRanges> HTMLMediaElement::played() const
{
    // FIXME track played
    return new TimeRanges;
}

PassRefPtr<TimeRanges> HTMLMediaElement::seekable() const
{
    // FIXME real ranges support
    if (!m_player || !m_player->maxTimeSeekable())
        return new TimeRanges;
    return new TimeRanges(0, m_player->maxTimeSeekable());
}

bool HTMLMediaElement::endedPlayback() const
{
#if 0
    return networkState() >= LOADED_METADATA && currentTime() >= effectiveEnd() && currentLoop() == playCount() - 1;
#endif
    return false;
}

void HTMLMediaElement::updateVolume()
{
    if (!m_player)
        return;

    m_player->setVolume(m_muted ? 0 : m_volume);
    
    if (renderer())
        renderer()->updateFromElement();
}

void HTMLMediaElement::updatePlayState()
{
    if (!m_player)
        return;

    // ###
}

}
