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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _XMLHTTPREQUEST_H_
#define _XMLHTTPREQUEST_H_

#include "ecma/kjs_binding.h"
#include "ecma/kjs_dom.h"
#include <kencodingdetector.h>
#include "kio/jobclasses.h"
#include <QPointer>
#include <QHash>

namespace KJS {

  class JSEventListener;
  class XMLHttpRequestQObject;

  class CaseInsensitiveString
  {
  public:
    CaseInsensitiveString(const char* s) : str(QLatin1String(s)) { }
    CaseInsensitiveString(const QString& s) : str(s) { }

    QString original() const { return str; }
    QString toLower() const { return str.toLower(); }

  private:
    QString str;
  };

  inline bool operator==(const CaseInsensitiveString& a,
                         const CaseInsensitiveString& b)
  {
    return a.toLower() == b.toLower();
  }

  inline uint qHash(const CaseInsensitiveString& key)
  {
    return qHash(key.toLower());
  }

  typedef QHash<CaseInsensitiveString, QString> HTTPHeaderMap;

  // these exact numeric values are important because JS expects them
  enum XMLHttpRequestState {
    XHRS_Uninitialized = 0,
    XHRS_Open = 1,
    XHRS_Sent = 2,
    XHRS_Receiving = 3,
    XHRS_Loaded = 4
  };

  class XMLHttpRequestConstructorImp : public ObjectImp {
  public:
    XMLHttpRequestConstructorImp(ExecState *exec, DOM::DocumentImpl* d);
    virtual bool implementsConstruct() const;
    virtual ObjectImp *construct(ExecState *exec, const List &args);
  private:
    SharedPtr<DOM::DocumentImpl> doc;
  };

  class XMLHttpRequest : public DOMObject {
  public:
    XMLHttpRequest(ExecState *, DOM::DocumentImpl* d);
    ~XMLHttpRequest();

    bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
    ValueImp *getValueProperty(ExecState *exec, int token) const;
    virtual void put(ExecState *exec, const Identifier &propertyName, ValueImp *value, int attr = None);
    void putValueProperty(ExecState *exec, int token, ValueImp *value, int /*attr*/);
    virtual bool toBoolean(ExecState *) const { return true; }
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Onload, Onreadystatechange, ReadyState, ResponseText, ResponseXML, Status, StatusText, Abort,
           GetAllResponseHeaders, GetResponseHeader, Open, Send, SetRequestHeader };

  private:
    friend class XMLHttpRequestProtoFunc;
    friend class XMLHttpRequestQObject;

    ValueImp *getStatusText() const;
    ValueImp *getStatus() const;
    bool urlMatchesDocumentDomain(const KUrl&) const;

    XMLHttpRequestQObject *qObject;

#ifdef APPLE_CHANGES
    void slotData( KIO::Job* job, const char *data, int size );
#else
    void slotData( KIO::Job* job, const QByteArray &data );
#endif
    void slotFinished( KJob* );
    void slotRedirection( KIO::Job*, const KUrl& );

    void processSyncLoadResults(const QByteArray &data, const KUrl &finalURL, const QString &headers);

    void open(const QString& _method, const KUrl& _url, bool _async);
    void send(const QString& _body);
    void abort();
    void setRequestHeader(const QString& name, const QString &value);
    ValueImp *getAllResponseHeaders() const;
    ValueImp *getResponseHeader(const QString& name) const;

    void changeState(XMLHttpRequestState newState);

    QPointer<DOM::DocumentImpl> doc;

    KUrl url;
    QString method;
    bool async;
    HTTPHeaderMap m_requestHeaders;
    QString contentType;

    KIO::TransferJob * job;

    XMLHttpRequestState state;
    JSEventListener *onReadyStateChangeListener;
    JSEventListener *onLoadListener;

    KEncodingDetector *decoder;
    QString encoding;
    QString responseHeaders;

    QString response;
    mutable bool createdDocument;
    mutable bool typeIsXML;
    mutable SharedPtr<DOM::DocumentImpl> responseXML;

    bool aborted;
  };


  class XMLHttpRequestQObject : public QObject {
    Q_OBJECT

  public:
    XMLHttpRequestQObject(XMLHttpRequest *_jsObject);

  public Q_SLOTS:
    void slotData( KIO::Job* job, const QByteArray &data );
    void slotFinished( KJob* job );
    void slotRedirection( KIO::Job* job, const KUrl& url);

  private:
    XMLHttpRequest *jsObject;
  };

} // namespace

#endif
