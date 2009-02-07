/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2006 Germain Garand <germain@ebooksfrance.org>
 *  Copyright (C) 2007 Matthias Kretz <kretz@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "ecma/kjs_audio.h"
#include "ecma/kjs_window.h"
#include "ecma/kjs_events.h"
#include "khtmlview.h"

#include <phonon/mediaobject.h>
#include <phonon/path.h>
#include <phonon/audiooutput.h>

#include <dom/html_document.h>

#include "kjs_audio.lut.h"

namespace KJS {

AudioConstructorImp::AudioConstructorImp(ExecState* exec, DOM::DocumentImpl* d)
    : JSObject(exec->lexicalInterpreter()->builtinObjectPrototype()), doc(d)
{
}

bool AudioConstructorImp::implementsConstruct() const
{
    return true;
}

JSObject *AudioConstructorImp::construct(ExecState *exec, const List &list)
{
    QString url;
    if (list.size() > 0) {
        url = list.at(0)->toString(exec).qstring();
    }
    return new Audio( exec, doc.get(), url );
}

////////////////////// Audio Object ////////////////////////

/* Source for AudioProtoTable.
@begin AudioProtoTable 3
  play		Audio::Play		DontDelete|Function 0
  loop		Audio::Loop		DontDelete|Function 1
  stop		Audio::Stop		DontDelete|Function 0
@end
*/

KJS_DEFINE_PROTOTYPE(AudioProto)
KJS_IMPLEMENT_PROTOFUNC(AudioProtoFunc)
KJS_IMPLEMENT_PROTOTYPE("Audio", AudioProto, AudioProtoFunc, ObjectPrototype)

const ClassInfo Audio::info = { "Audio", 0, &AudioTable, 0 };

/* Source for AudioTable.
@begin AudioTable 3
    onerror	Audio::Onerror		DontDelete
    onload	Audio::Onload		DontDelete
@end
*/

Audio::Audio(ExecState *exec, DOM::DocumentImpl* d, const QString& url)
 :  m_doc(d), 
    m_cs(0),
    m_qObj(new AudioQObject(this)),
    m_onLoadListener(0), 
    m_onErrorListener(0)
{
    setPrototype(AudioProto::self(exec));
    KHTMLPart *part = qobject_cast<KHTMLPart*>(Window::retrieveActive(exec)->part());
    if (part)
        m_url = part->htmlDocument().completeURL(url).string();
    m_cs = m_doc->docLoader()->requestSound( DOM::DOMString(m_url) );
}

Audio::~Audio()
{
    if (m_cs) 
        m_cs->deref(this);
    if (m_onErrorListener)
        m_onErrorListener->deref();
    if (m_onLoadListener)
        m_onLoadListener->deref();
    delete m_qObj;
}

bool Audio::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<Audio, DOMObject>(exec, &AudioTable, this, propertyName, slot);
}

JSValue *Audio::getValueProperty(ExecState * /*exec*/, int token) const
{
    switch (token) {
      case Onerror:
        if (m_onErrorListener && m_onErrorListener->listenerObj()) {
            return m_onErrorListener->listenerObj();
        } else {
            return jsNull();
        }
      case Onload:
        if (m_onLoadListener && m_onLoadListener->listenerObj()) {
            return m_onLoadListener->listenerObj();
        } else {
            return jsNull();
        }
      default:
        kWarning() << "Audio::getValueProperty unhandled token " << token;
        break;
    }
    return 0;
}

void Audio::put(ExecState *exec, const Identifier &propertyName, JSValue *value, int attr)
{
    lookupPut<Audio,DOMObject>(exec, propertyName, value, attr, &AudioTable, this );
}

void Audio::putValueProperty(ExecState *exec, int token, JSValue *value, int /*attr*/)
{
    switch(token) {
      case Onerror:
        if (m_onErrorListener)
            m_onErrorListener->deref();
        m_onErrorListener = Window::retrieveActive(exec)->getJSEventListener(value, true);
        if (m_onErrorListener) 
            m_onErrorListener->ref();
        break;
      case Onload:
        if (m_onLoadListener)
            m_onLoadListener->deref();
        m_onLoadListener = Window::retrieveActive(exec)->getJSEventListener(value, true);
        if (m_onLoadListener)
            m_onLoadListener->ref();
        break;
      default:
        kWarning() << "Audio::putValue unhandled token " << token;
    }
}

void Audio::notifyFinished(khtml::CachedObject * /*co*/)
{
    if (!m_cs) return;

    m_qObj->setSound( m_cs->sound() );
    m_cs->deref(this);
    m_cs = 0;

    m_qObj->setupPlayer();

    if (m_onLoadListener && m_doc->view() && m_doc->view()->part()) {
        DOM::Event ev = m_doc->view()->part()->document().createEvent("Event");
        ev.initEvent("load", true, true);
        m_onLoadListener->handleEvent(ev);
    }
}

void Audio::error(int, const QString &)
{
    if (m_cs) {
        m_cs->deref(this);
        m_cs = 0;
    }

    if (m_onErrorListener && m_doc->view() && m_doc->view()->part()) {
        DOM::Event ev = m_doc->view()->part()->document().createEvent("Event");
        ev.initEvent("error", true, true);
        m_onErrorListener->handleEvent(ev);
    }
}

void Audio::refLoader()
{
    if (m_cs) m_cs->ref(this);
}

void Audio::play()
{
    m_qObj->play();
}

void Audio::stop()
{
    m_qObj->stop();
}

void Audio::loop(int n)
{
    m_qObj->loop(n);
}


JSValue *AudioProtoFunc::callAsFunction(ExecState *exec, JSObject *thisObj, const List &args)
{  
    if (!thisObj->inherits(&Audio::info)) {
        return throwError(exec, TypeError);
    }
    Audio *audio = static_cast<Audio*>(thisObj);
    switch (id) {
      case Audio::Play:
          audio->play();
          break;
      case Audio::Loop:
          audio->loop( args.size() > 0 ? args[0]->toInt32(exec) : -1 );
          break;
      case Audio::Stop:
          audio->stop();
          break;
      default:
          break;      
    }
    return jsUndefined();
}

using namespace Phonon;

AudioQObject::AudioQObject(Audio* jObj)
  : m_jObj( jObj ),
    m_media(0),
    m_offset(0),
    m_playCount(0),
    m_stopping(false)
{
    // sound might be immediately available, so delay ref'ing until
    // listeners have had a chance to register.
    QTimer::singleShot( 0, this, SLOT(refLoader()));
}

AudioQObject::~AudioQObject()
{
}

void AudioQObject::refLoader()
{
    m_jObj->refLoader(); 
}

void AudioQObject::setupPlayer()
{
    m_media = new MediaObject( this );
    AudioOutput *audioOutput = new AudioOutput(Phonon::MusicCategory, m_media);

    m_media->setCurrentSource(this);

    Phonon::createPath(m_media, audioOutput);

    setStreamSeekable( true );
    setStreamSize( m_sound.size()-1 ); // why -1?

    connect(m_media, SIGNAL(finished()), this, SLOT(finished()));

    m_playCount = 1;
}

void AudioQObject::finished()
{
    --m_playCount;
    if (m_playCount > 0) {
        m_media->play();
    }
}

void AudioQObject::reset()
{
    m_offset = 0;
}

void AudioQObject::needData()
{
    if (m_offset > 0) {
        writeData(m_sound.right(m_sound.size() - m_offset));
        m_offset = 0;
    } else {
        writeData(m_sound);
    }
    endOfData();
}

void AudioQObject::enoughData()
{
    // nothing to do
}

void AudioQObject::seekStream(qint64 offset)
{
    m_offset = offset;
}

void AudioQObject::play()
{
    loop(1);
}

void AudioQObject::stop()
{
    m_media->stop();
    m_playCount = 0;
}

void AudioQObject::loop(int n)
{
    m_playCount = n;
    if (!m_media || m_playCount <= 0 || m_media->state() == PlayingState)
        return;
    m_media->play();
}

} // namespace KJS

#include "kjs_audio.moc"
