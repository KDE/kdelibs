// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2003 Apple Computer, Inc.
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _XMLHTTPREQUEST_H_
#define _XMLHTTPREQUEST_H_

#include "ecma/kjs_binding.h"
#include "ecma/kjs_dom.h"
#include "misc/decoder.h"
#include "kio/jobclasses.h"

namespace KJS {

  class JSEventListener;
  class XMLHttpRequestQObject;

  // these exact numeric values are important because JS expects them
  enum XMLHttpRequestState {
    Uninitialized = 0,
    Loading = 1,
    Loaded = 2,
    Interactive = 3,
    Completed = 4
  };

  class XMLHttpRequestConstructorImp : public ObjectImp {
  public:
    XMLHttpRequestConstructorImp(ExecState *exec, const DOM::Document &d);
    virtual bool implementsConstruct() const;
    virtual Object construct(ExecState *exec, const List &args);
  private:
    DOM::Document doc;
  };

  class XMLHttpRequest : public DOMObject {
  public:
    XMLHttpRequest(ExecState *, const DOM::Document &d);
    ~XMLHttpRequest();
    virtual Value tryGet(ExecState *exec, const Identifier &propertyName) const;
    Value getValueProperty(ExecState *exec, int token) const;
    virtual void tryPut(ExecState *exec, const Identifier &propertyName, const Value& value, int attr = None);
    void putValueProperty(ExecState *exec, int token, const Value& value, int /*attr*/);
    virtual bool toBoolean(ExecState *) const { return true; }
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Onload, Onreadystatechange, ReadyState, ResponseText, ResponseXML, Status, StatusText, Abort, GetAllResponseHeaders, GetResponseHeader, Open, Send, SetRequestHeader };

  private:
    friend class XMLHttpRequestProtoFunc;
    friend class XMLHttpRequestQObject;

    Value getStatusText() const;
    Value getStatus() const;
    bool urlMatchesDocumentDomain(const KURL&) const;

    XMLHttpRequestQObject *qObject;

#ifdef APPLE_CHANGES
    void slotData( KIO::Job* job, const char *data, int size );
#else
    void slotData( KIO::Job* job, const QByteArray &data );
#endif
    void slotFinished( KIO::Job* );
    void slotRedirection( KIO::Job*, const KURL& );

    void processSyncLoadResults(const QByteArray &data, const KURL &finalURL, const QString &headers);

    void open(const QString& _method, const KURL& _url, bool _async);
    void send(const QString& _body);
    void abort();
    void setRequestHeader(const QString& name, const QString &value);
    Value getAllResponseHeaders() const;
    Value getResponseHeader(const QString& name) const;

    void changeState(XMLHttpRequestState newState);

    QGuardedPtr<DOM::DocumentImpl> doc;

    KURL url;
    QString method;
    bool async;
    QString requestHeaders;

    KIO::TransferJob * job;

    XMLHttpRequestState state;
    JSEventListener *onReadyStateChangeListener;
    JSEventListener *onLoadListener;

    khtml::Decoder *decoder;
    QString encoding;
    QString responseHeaders;

    QString response;
    mutable bool createdDocument;
    mutable bool typeIsXML;
    mutable DOM::Document responseXML;

    bool aborted;
  };


  class XMLHttpRequestQObject : public QObject {
    Q_OBJECT

  public:
    XMLHttpRequestQObject(XMLHttpRequest *_jsObject);

  public slots:
    void slotData( KIO::Job* job, const QByteArray &data );
    void slotFinished( KIO::Job* job );
    void slotRedirection( KIO::Job* job, const KURL& url);

  private:
    XMLHttpRequest *jsObject;
  };

} // namespace

#endif
