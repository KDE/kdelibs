/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2006 Germain Garand <germain@ebooksfrance.org>
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

#ifndef KJS_AUDIO_H
#define KJS_AUDIO_H

#include "ecma/kjs_dom.h"
#include "ecma/kjs_binding.h"
#include <kjs/object.h>
#include <QtCore/QPointer>
#include <phonon/phononnamespace.h>
#include <phonon/abstractmediastream.h>

#include "misc/loader.h"

namespace Phonon {
    class AudioPath;
    class AudioOutput;
}

namespace KJS {

  class AudioConstructorImp : public JSObject {
  public:
    AudioConstructorImp(ExecState *exec, DOM::DocumentImpl* d);
    virtual bool implementsConstruct() const;
    using KJS::JSObject::construct;
    virtual JSObject* construct(ExecState *exec, const List &args);
  private:
    SharedPtr<DOM::DocumentImpl> doc;
  };

  ////////////////////// Audio Object ////////////////////////

  class JSEventListener;
  class AudioQObject;

  class Audio : public DOMObject, private khtml::CachedObjectClient {
  public:
    Audio(ExecState *, DOM::DocumentImpl* d, const QString& url);
    virtual ~Audio();

    using KJS::JSObject::getOwnPropertySlot;
    bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
    JSValue *getValueProperty(ExecState *exec, int token) const;
    using KJS::JSObject::put;
    virtual void put(ExecState *exec, const Identifier &propertyName, JSValue *value, int attr = None);
    void putValueProperty(ExecState *exec, int token, JSValue *value, int /*attr*/);
    virtual bool toBoolean(ExecState *) const { return true; }
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Onload, Onerror, Loop, Play, Stop };

    void play();
    void stop();
    void loop(int n=-1);

  private:
    friend class AudioProtoFunc;
    friend class AudioQObject;

    // from CachedObjectClient
    virtual void notifyFinished(khtml::CachedObject *);
    virtual void error(int err, const QString &text);

    void refLoader();

    QString m_url;
    QPointer<DOM::DocumentImpl> m_doc;
    khtml::CachedSound* m_cs;

    AudioQObject* m_qObj;

    JSEventListener *m_onLoadListener;
    JSEventListener *m_onErrorListener;
  };

  // Needs a separate class as QObjects confuse the hell out of
  // KJS's garbage collector
  class AudioQObject: public Phonon::AbstractMediaStream {
      Q_OBJECT
  public:
      AudioQObject(Audio* jObj);
      virtual ~AudioQObject();

      void setSound( const QByteArray& s ) { m_sound = s; }
      void setupPlayer();

      void play();
      void stop();
      void loop(int n=-1);

  protected:
      void reset();
      void needData();
      void enoughData();
      void seekStream(qint64 offset);

  protected Q_SLOTS:
    void refLoader();
    void finished();

  private:
    Audio* m_jObj;

    Phonon::MediaObject* m_media;
    QByteArray m_sound;
    qint64 m_offset;
    int m_playCount;
    bool m_stopping;

    static Phonon::AudioPath*   s_audioPath;
    static Phonon::AudioOutput* s_audioOutput;
    static int s_refs;
};

} // namespace KJS

#endif
