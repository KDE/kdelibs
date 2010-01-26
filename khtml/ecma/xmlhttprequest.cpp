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

#include "xmlhttprequest.h"
#include "xmlhttprequest.lut.h"
#include "kjs_window.h"
#include "kjs_events.h"


#include "dom/dom_doc.h"
#include "dom/dom_exception.h"
#include "dom/dom_string.h"
#include "misc/loader.h"
#include "html/html_documentimpl.h"
#include "xml/dom2_eventsimpl.h"

#include "khtml_part.h"
#include "khtmlview.h"

#include <kio/scheduler.h>
#include <kio/job.h>
#include <QtCore/QObject>
#include <QtCore/QHash>
#include <kdebug.h>

#ifdef APPLE_CHANGES
#include "KWQLoader.h"
#else
#include <kio/netaccess.h>
using KIO::NetAccess;
#endif

using namespace KJS;
using namespace DOM;
//
////////////////////// XMLHttpRequest Object ////////////////////////

/* Source for XMLHttpRequestProtoTable.
@begin XMLHttpRequestProtoTable 7
  abort			XMLHttpRequest::Abort			DontDelete|Function 0
  getAllResponseHeaders	XMLHttpRequest::GetAllResponseHeaders	DontDelete|Function 0
  getResponseHeader	XMLHttpRequest::GetResponseHeader	DontDelete|Function 1
  open			XMLHttpRequest::Open			DontDelete|Function 5
  overrideMimeType	XMLHttpRequest::OverrideMIMEType	DontDelete|Function 1
  send			XMLHttpRequest::Send			DontDelete|Function 1
  setRequestHeader	XMLHttpRequest::SetRequestHeader	DontDelete|Function 2
@end
*/

namespace KJS {

KJS_DEFINE_PROTOTYPE(XMLHttpRequestProto)
KJS_IMPLEMENT_PROTOFUNC(XMLHttpRequestProtoFunc)
KJS_IMPLEMENT_PROTOTYPE("XMLHttpRequest", XMLHttpRequestProto,XMLHttpRequestProtoFunc, ObjectPrototype)


XMLHttpRequestQObject::XMLHttpRequestQObject(XMLHttpRequest *_jsObject)
{
  jsObject = _jsObject;
}

#ifdef APPLE_CHANGES
void XMLHttpRequestQObject::slotData( KIO::Job* job, const char *data, int size )
{
  jsObject->slotData(job, data, size);
}
#else
void XMLHttpRequestQObject::slotData( KIO::Job* job, const QByteArray &data )
{
  jsObject->slotData(job, data);
}
#endif

void XMLHttpRequestQObject::slotFinished( KJob* job )
{
  jsObject->slotFinished(job);
}

void XMLHttpRequestQObject::slotRedirection( KIO::Job* job, const KUrl& url)
{
  jsObject->slotRedirection( job, url );
}

XMLHttpRequestConstructorImp::XMLHttpRequestConstructorImp(ExecState *exec, DOM::DocumentImpl* d)
    : JSObject(exec->lexicalInterpreter()->builtinObjectPrototype()), doc(d)
{
    JSObject* proto = XMLHttpRequestProto::self(exec);
    putDirect(exec->propertyNames().prototype, proto, DontDelete|ReadOnly);
}

bool XMLHttpRequestConstructorImp::implementsConstruct() const
{
  return true;
}

JSObject *XMLHttpRequestConstructorImp::construct(ExecState *exec, const List &)
{
  return new XMLHttpRequest(exec, doc.get());
}

const ClassInfo XMLHttpRequest::info = { "XMLHttpRequest", 0, &XMLHttpRequestTable, 0 };


/* Source for XMLHttpRequestTable.
@begin XMLHttpRequestTable 7
  readyState		XMLHttpRequest::ReadyState		DontDelete|ReadOnly
  responseText		XMLHttpRequest::ResponseText		DontDelete|ReadOnly
  responseXML		XMLHttpRequest::ResponseXML		DontDelete|ReadOnly
  status		XMLHttpRequest::Status			DontDelete|ReadOnly
  statusText		XMLHttpRequest::StatusText		DontDelete|ReadOnly
  onreadystatechange	XMLHttpRequest::Onreadystatechange	DontDelete
  onload		XMLHttpRequest::Onload			DontDelete
@end
*/

bool XMLHttpRequest::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
  return getStaticValueSlot<XMLHttpRequest, DOMObject>(exec, &XMLHttpRequestTable, this, propertyName, slot);
}

JSValue *XMLHttpRequest::getValueProperty(ExecState *exec, int token) const
{
  switch (token) {
  case ReadyState:
    return jsNumber(m_state);
  case ResponseText:
    return ::getStringOrNull(DOM::DOMString(response));
  case ResponseXML:
    if (m_state != XHRS_Loaded) {
      return jsNull();
    }
    if (!createdDocument) {
      QString mimeType = "text/xml";

      if (!m_mimeTypeOverride.isEmpty()) {
	  mimeType = m_mimeTypeOverride;
      } else {
          int dummy;
	  JSValue *header = getResponseHeader("Content-Type", dummy);
	  if (header->type() != UndefinedType)
	      mimeType = header->toString(exec).qstring().split(";")[0].trimmed();
      }

      if (mimeType == "text/xml" || mimeType == "application/xml" || mimeType == "application/xhtml+xml") {
	responseXML = doc->implementation()->createDocument();

	responseXML->open();
	responseXML->setURL(url.url());
	responseXML->write(response);
	responseXML->finishParsing();
	responseXML->close();

	typeIsXML = true;
      } else {
	typeIsXML = false;
      }
      createdDocument = true;
    }

    if (!typeIsXML) {
      return jsNull();
    }

    return getDOMNode(exec,responseXML.get());
  case Status:
    return getStatus();
  case StatusText:
    return getStatusText();
  case Onreadystatechange:
   if (onReadyStateChangeListener && onReadyStateChangeListener->listenerObj()) {
     return onReadyStateChangeListener->listenerObj();
   } else {
     return jsNull();
   }
  case Onload:
   if (onLoadListener && onLoadListener->listenerObj()) {
     return onLoadListener->listenerObj();
   } else {
     return jsNull();
   }
  default:
    kWarning() << "XMLHttpRequest::getValueProperty unhandled token " << token;
    return 0;
  }
}

void XMLHttpRequest::put(ExecState *exec, const Identifier &propertyName, JSValue *value, int attr)
{
  lookupPut<XMLHttpRequest,DOMObject>(exec, propertyName, value, attr, &XMLHttpRequestTable, this );
}

void XMLHttpRequest::putValueProperty(ExecState *exec, int token, JSValue *value, int /*attr*/)
{
  switch(token) {
  case Onreadystatechange:
    if (onReadyStateChangeListener) onReadyStateChangeListener->deref();
    onReadyStateChangeListener = Window::retrieveActive(exec)->getJSEventListener(value, true);
    if (onReadyStateChangeListener) onReadyStateChangeListener->ref();
    break;
  case Onload:
    if (onLoadListener) onLoadListener->deref();
    onLoadListener = Window::retrieveActive(exec)->getJSEventListener(value, true);
    if (onLoadListener) onLoadListener->ref();
    break;
  default:
    kWarning() << "XMLHttpRequest::putValue unhandled token " << token;
  }
}

// Token according to RFC 2616
static bool isValidFieldName(const QString& name)
{
    const QChar* c = name.constData();
    int l = name.length();
    if (l == 0)
        return false;
    for (int i = 0; i < l; ++i, ++c) {
        int u = c->unicode();
        if (u < 32 || u > 126)
            return false;
        switch (u) {
        case '(': case ')': case '<': case '>':
        case '@': case ',': case ';': case ':':
        case '\\': case '"': case '/':
        case '[': case ']': case '?': case '=':
        case '{': case '}': case '\t': case ' ':
	    return false;
        default:
            break;
        }
    }
    return true;
}

static bool isValidFieldValue(const QString& name)
{
    const QChar* c = name.constData();
    int l = name.length();
    if (l == 0)
        return true;

    for (int i = 0; i < l; ++i, ++c) {
        int u = c->unicode();
	if ( u == '\n' || u == '\r' )
	  return false;
    }

    // ### what is invalid?
    return true;
}

static bool canSetRequestHeader(const QString& name)
{
    static QSet<CaseInsensitiveString> forbiddenHeaders;

    if (forbiddenHeaders.isEmpty()) {
	static const char* const hdrs[] = {
	    "accept-charset",
	    "accept-encoding",
	    "content-length",
	    "connect",
	    "copy",
	    "date",
	    "delete",
	    "expect",
	    "head",
	    "host",
	    "keep-alive",
	    "lock",
	    "mkcol",
	    "move",
	    "options",
	    "put",
	    "propfind",
	    "proppatch",
	    "proxy-authorization",
	    "referer",
	    "te",
	    "trace",
	    "trailer",
	    "transfer-encoding",
	    "unlock",
	    "upgrade",
	    "via"
	};
	for (size_t i = 0; i < sizeof(hdrs)/sizeof(char*); ++i)
	    forbiddenHeaders.insert(CaseInsensitiveString(hdrs[i]));
    }

    return !forbiddenHeaders.contains(name);
}

XMLHttpRequest::XMLHttpRequest(ExecState *exec, DOM::DocumentImpl* d)
  : qObject(new XMLHttpRequestQObject(this)),
    doc(d),
    async(true),
    contentType(QString()),
    job(0),
    m_state(XHRS_Uninitialized),
    onReadyStateChangeListener(0),
    onLoadListener(0),
    decoder(0),
    response(QString::fromLatin1("")),
    createdDocument(false),
    aborted(false)
{
  ref(); // we're a GC point, so refcount pin.
  setPrototype(XMLHttpRequestProto::self(exec));
}

XMLHttpRequest::~XMLHttpRequest()
{
  if (job && method.toUpper() != QLatin1String("POST")) {
        job->kill();
        job = 0;
  }
  if (onLoadListener)
      onLoadListener->deref();
  if (onReadyStateChangeListener)
      onReadyStateChangeListener->deref();
  delete qObject;
  qObject = 0;
  delete decoder;
  decoder = 0;
}

void XMLHttpRequest::changeState(XMLHttpRequestState newState)
{
  // Other engines cancel transfer if the controlling document doesn't
  // exist anymore. Match that, though being paranoid about post
  // (And don't emit any events w/o a doc, if we're kept alive otherwise).
  if (!doc) {
    if (job && method.toUpper() != QLatin1String("POST")) {
      job->kill();
      job = 0;
    }
    return;
  }

  if (m_state != newState) {
    m_state = newState;
    ProtectedPtr<JSObject> ref(this);

    if (onReadyStateChangeListener != 0 && doc->view() && doc->view()->part()) {
      DOM::Event ev = doc->view()->part()->document().createEvent("HTMLEvents");
      ev.initEvent("readystatechange", true, true);
      ev.handle()->setTarget(this);
      ev.handle()->setCurrentTarget(this);
      onReadyStateChangeListener->handleEvent(ev);

      // Make sure the event doesn't point to us, since it can't prevent
      // us from being collecte.
      ev.handle()->setTarget(0);
      ev.handle()->setCurrentTarget(0);
    }

    if (m_state == XHRS_Loaded && onLoadListener != 0 && doc->view() && doc->view()->part()) {
      DOM::Event ev = doc->view()->part()->document().createEvent("HTMLEvents");
      ev.initEvent("load", true, true);
      ev.handle()->setTarget(this);
      ev.handle()->setCurrentTarget(this);
      onLoadListener->handleEvent(ev);
      ev.handle()->setTarget(0);
      ev.handle()->setCurrentTarget(0);
    }
  }
}

bool XMLHttpRequest::urlMatchesDocumentDomain(const KUrl& _url) const
{
  // No need to do work if _url is not valid...
  if (!_url.isValid())
    return false;

  KUrl documentURL(doc->URL());

  // a local file can load anything
  if (documentURL.protocol().toLower() == "file") {
    return true;
  }

  // but a remote document can only load from the same port on the server
  if (documentURL.protocol().toLower() == _url.protocol().toLower() &&
      documentURL.host().toLower() == _url.host().toLower() &&
      documentURL.port() == _url.port()) {
    return true;
  }

  return false;
}

void XMLHttpRequest::open(const QString& _method, const KUrl& _url, bool _async, int& ec)
{
  abort();
  aborted = false;

  // clear stuff from possible previous load
  m_requestHeaders.clear();
  responseHeaders.clear();
  response = QString::fromLatin1("");
  createdDocument = false;
  responseXML = 0;

  if (!urlMatchesDocumentDomain(_url)) {
    ec = DOMException::SECURITY_ERR;
    return;
  }

  method = _method.toLower();
  url = _url;
  async = _async;

  changeState(XHRS_Open);
}

void XMLHttpRequest::send(const QString& _body, int& ec)
{
  aborted = false;

  if (m_state != XHRS_Open) {
      ec =  DOMException::INVALID_STATE_ERR;
      return;
  }

  const QString protocol = url.protocol().toLower();
  // Abandon the request when the protocol is other than "http",
  // instead of blindly doing a KIO::get on other protocols like file:/.
  if (!protocol.startsWith(QLatin1String("http")) &&
      !protocol.startsWith(QLatin1String("webdav")))
  {
    ec = DOMException::INVALID_ACCESS_ERR;
    abort();
    return;
  }

  if (method == "post") {

    // FIXME: determine post encoding correctly by looking in headers
    // for charset.
    QByteArray buf = _body.toUtf8();

    job = KIO::http_post( url, buf, KIO::HideProgressInfo );
    if(contentType.isNull())
      job->addMetaData( "content-type", "Content-type: text/plain" );
    else
      job->addMetaData( "content-type", contentType );

  }
  else {
    job = KIO::get( url, KIO::NoReload, KIO::HideProgressInfo );
  }

  if (!m_requestHeaders.isEmpty()) {
    QString rh;
    HTTPHeaderMap::ConstIterator begin = m_requestHeaders.constBegin();
    HTTPHeaderMap::ConstIterator end = m_requestHeaders.constEnd();
    for (HTTPHeaderMap::ConstIterator i = begin; i != end; ++i) {
      QString key = i.key().original();
      QString value = i.value();
      if (key.toLower() == "accept") {
        // The HTTP KIO slave supports an override this way
        job->addMetaData("accept", value);
      } else {
        if (!rh.isEmpty())
          rh += "\r\n";
        rh += key + ": " + value;
      }
    }

    job->addMetaData("customHTTPHeader", rh);
  }

  job->addMetaData("PropagateHttpHeader", "true");

  // Set the default referrer. NOTE: the user can still disable
  // this feature at the protocol level (kio_http).
  KUrl documentURL(doc->URL());
  documentURL.setPass(QString());
  documentURL.setUser(QString());
  job->addMetaData("referrer", documentURL.url());
  // kDebug() << "Adding referrer: " << documentURL;

  if (!async) {
    QByteArray data;
    KUrl finalURL;
    QString headers;

#ifdef APPLE_CHANGES
    data = KWQServeSynchronousRequest(khtml::Cache::loader(), doc->docLoader(), job, finalURL, headers);
#else
    QMap<QString, QString> metaData;
    if ( NetAccess::synchronousRun( job, 0, &data, &finalURL, &metaData ) ) {
      headers = metaData[ "HTTP-Headers" ];
    }
#endif
    job = 0;
    processSyncLoadResults(data, finalURL, headers);
    return;
  }

  qObject->connect( job, SIGNAL( result( KJob* ) ),
		    SLOT( slotFinished( KJob* ) ) );
#ifdef APPLE_CHANGES
  qObject->connect( job, SIGNAL( data( KIO::Job*, const char*, int ) ),
		    SLOT( slotData( KIO::Job*, const char*, int ) ) );
#else
  qObject->connect( job, SIGNAL( data( KIO::Job*, const QByteArray& ) ),
		    SLOT( slotData( KIO::Job*, const QByteArray& ) ) );
#endif
  qObject->connect( job, SIGNAL(redirection(KIO::Job*, const KUrl& ) ),
		    SLOT( slotRedirection(KIO::Job*, const KUrl&) ) );

#ifdef APPLE_CHANGES
  KWQServeRequest(khtml::Cache::loader(), doc->docLoader(), job);
#else
  KIO::Scheduler::scheduleJob( job );
#endif
}

void XMLHttpRequest::abort()
{
  if (job) {
    job->kill();
    job = 0;
  }
  delete decoder;
  decoder = 0;
  aborted = true;
  changeState(XHRS_Uninitialized);
}

void XMLHttpRequest::overrideMIMEType(const QString& override)
{
    m_mimeTypeOverride = override;
}

void XMLHttpRequest::setRequestHeader(const QString& _name, const QString& _value, int& ec)
{
  // throw exception if connection is not open or the send flag is set
  if (m_state != XHRS_Open || job != 0) {
      ec = DOMException::INVALID_STATE_ERR;
      return;
  }

  if (!isValidFieldName(_name) || !isValidFieldValue(_value)) {
      ec = DOMException::SYNTAX_ERR;
      return;
  }
  QString name = _name.toLower();
  QString value = _value.trimmed();
  if (value.isEmpty())
      return;

  // Content-type needs to be set separately from the other headers
  if(name == "content-type") {
    contentType = "Content-type: " + value;
    return;
  }

  // Sanitize the referrer header to protect against spoofing...
  if(name == "referer") {
    KUrl referrerURL(value);
    if (urlMatchesDocumentDomain(referrerURL))
      m_requestHeaders[name] = referrerURL.url();
    return;
  }

  // Sanitize the request headers below and handle them as if they are
  // calls to open. Otherwise, we will end up ignoring them all together!
  // TODO: Do something about "put" which kio_http sort of supports and
  // the webDAV headers such as PROPFIND etc...
  if (name == "get"  || name == "post") {
    KUrl reqURL (doc->URL(), value.trimmed());
    open(name, reqURL, async, ec);
    return;
  }

  // Reject all banned headers.
  if (!canSetRequestHeader(_name)) {
      kWarning(6070) << "Refusing to set unsafe XMLHttpRequest header "
                     << name << endl;
      return;
  }

  m_requestHeaders[_name] = value;
}

JSValue *XMLHttpRequest::getAllResponseHeaders(int& ec) const
{
  if (m_state < XHRS_Receiving) {
      ec = DOMException::INVALID_STATE_ERR;
      return jsString("");
  }

  // ### test error flag, return jsNull

  if (responseHeaders.isEmpty()) {
    return jsUndefined();
  }

  int endOfLine = responseHeaders.indexOf("\n");

  if (endOfLine == -1) {
    return jsUndefined();
  }

  return jsString(responseHeaders.mid(endOfLine + 1) + '\n');
}

JSValue *XMLHttpRequest::getResponseHeader(const QString& name, int& ec) const
{
  if (m_state < XHRS_Receiving) {
      ec = DOMException::INVALID_STATE_ERR;
      return jsString("");
  }

  if (!isValidFieldName(name)) {
    return jsString("");
  }

  // ### test error flag, return jsNull

  if (responseHeaders.isEmpty()) {
    return jsUndefined();
  }

  QRegExp headerLinePattern(name + ':', Qt::CaseInsensitive);

  int matchLength;
  int headerLinePos = headerLinePattern.indexIn(responseHeaders, 0);
  matchLength = headerLinePattern.matchedLength();
  while (headerLinePos != -1) {
    if (headerLinePos == 0 || responseHeaders[headerLinePos-1] == '\n') {
      break;
    }

    headerLinePos = headerLinePattern.indexIn(responseHeaders, headerLinePos + 1);
    matchLength = headerLinePattern.matchedLength();
  }


  if (headerLinePos == -1) {
    return jsNull();
  }

  int endOfLine = responseHeaders.indexOf("\n", headerLinePos + matchLength);

  return jsString(responseHeaders.mid(headerLinePos + matchLength, endOfLine - (headerLinePos + matchLength)).trimmed());
}

static JSValue *httpStatus(const QString& response, bool textStatus = false)
{
  if (response.isEmpty()) {
    return jsUndefined();
  }

  int endOfLine = response.indexOf("\n");
  QString firstLine = (endOfLine == -1) ? response : response.left(endOfLine);
  int codeStart = firstLine.indexOf(" ");
  int codeEnd = firstLine.indexOf(" ", codeStart + 1);

  if (codeStart == -1 || codeEnd == -1) {
    return jsUndefined();
  }

  if (textStatus) {
    QString statusText = firstLine.mid(codeEnd + 1, endOfLine - (codeEnd + 1)).trimmed();
    return jsString(statusText);
  }

  QString number = firstLine.mid(codeStart + 1, codeEnd - (codeStart + 1));

  bool ok = false;
  int code = number.toInt(&ok);
  if (!ok) {
    return jsUndefined();
  }

  return jsNumber(code);
}

JSValue *XMLHttpRequest::getStatus() const
{
  return httpStatus(responseHeaders);
}

JSValue *XMLHttpRequest::getStatusText() const
{
  return httpStatus(responseHeaders, true);
}

void XMLHttpRequest::processSyncLoadResults(const QByteArray &data, const KUrl &finalURL, const QString &headers)
{
  if (!urlMatchesDocumentDomain(finalURL)) {
    abort();
    return;
  }

  responseHeaders = headers;
  changeState(XHRS_Sent);
  if (aborted) {
    return;
  }

#ifdef APPLE_CHANGES
  const char *bytes = (const char *)data.data();
  int len = (int)data.size();

  slotData(0, bytes, len);
#else
  slotData(0, data);
#endif

  if (aborted) {
    return;
  }

  slotFinished(0);
}

void XMLHttpRequest::slotFinished(KJob *)
{
  if (decoder) {
    response += decoder->flush();
  }

  // make sure to forget about the job before emitting completed,
  // since changeState triggers JS code, which might e.g. call abort.
  job = 0;
  changeState(XHRS_Loaded);

  delete decoder;
  decoder = 0;
}

void XMLHttpRequest::slotRedirection(KIO::Job*, const KUrl& url)
{
  if (!urlMatchesDocumentDomain(url)) {
    abort();
  }
}

static QString encodingFromContentType(const QString& type)
{
    QString encoding;
    int index = type.indexOf(';');
    if (index > -1)
        encoding = type.mid( index+1 ).remove(QRegExp("charset[ ]*=[ ]*", Qt::CaseInsensitive)).trimmed();
    return encoding;
}

#ifdef APPLE_CHANGES
void XMLHttpRequest::slotData( KIO::Job*, const char *data, int len )
#else
void XMLHttpRequest::slotData(KIO::Job*, const QByteArray &_data)
#endif
{
  if (m_state < XHRS_Sent ) {
    responseHeaders = job->queryMetaData("HTTP-Headers");

    // NOTE: Replace a 304 response with a 200! Both IE and Mozilla do this.
    // Problem first reported through bug# 110272.
    int codeStart = responseHeaders.indexOf("304");
    if ( codeStart != -1) {
      int codeEnd = responseHeaders.indexOf("\n", codeStart+3);
      if (codeEnd != -1)
        responseHeaders.replace(codeStart, (codeEnd-codeStart), "200 OK");
    }

    changeState(XHRS_Sent);
  }

#ifndef APPLE_CHANGES
  const char *data = (const char *)_data.data();
  int len = (int)_data.size();
#endif

  if ( decoder == NULL ) {
    if (!m_mimeTypeOverride.isEmpty())
        encoding = encodingFromContentType(m_mimeTypeOverride);

    if (encoding.isEmpty()) {
      int pos = responseHeaders.indexOf(QLatin1String("content-type:"), 0, Qt::CaseInsensitive);
      if ( pos > -1 ) {
        pos += 13;
        int index = responseHeaders.indexOf('\n', pos);
        QString type = responseHeaders.mid(pos, (index-pos));
        encoding = encodingFromContentType(type);
      }
    }

    decoder = new KEncodingDetector;

    if (!encoding.isEmpty())
      decoder->setEncoding(encoding.toLatin1().constData(), KEncodingDetector::EncodingFromHTTPHeader);
    else
      decoder->setEncoding("UTF-8", KEncodingDetector::DefaultEncoding);
  }
  if (len == 0)
    return;

  if (len == -1)
    len = strlen(data);

  QString decoded = decoder->decodeWithBuffering(data, len);

  response += decoded;

  if (!aborted) {
    changeState(XHRS_Receiving);
  }
}

JSValue *XMLHttpRequestProtoFunc::callAsFunction(ExecState *exec, JSObject *thisObj, const List &args)
{
  if (!thisObj->inherits(&XMLHttpRequest::info)) {
    return throwError(exec, TypeError);
  }

  XMLHttpRequest *request = static_cast<XMLHttpRequest *>(thisObj);

  if (!request->doc) {
    setDOMException(exec, DOMException::INVALID_STATE_ERR);
    return jsUndefined();
  }

  int ec = 0;

  switch (id) {
  case XMLHttpRequest::Abort:
    request->abort();
    return jsUndefined();
  case XMLHttpRequest::GetAllResponseHeaders:
    {
      JSValue *ret = request->getAllResponseHeaders(ec);
      setDOMException(exec, ec);
      return ret;
    }
  case XMLHttpRequest::GetResponseHeader:
    {
      if (args.size() < 1)
          return throwError(exec, SyntaxError, "Not enough arguments");
      JSValue *ret = request->getResponseHeader(args[0]->toString(exec).qstring(), ec);
      setDOMException(exec, ec);
      return ret;
    }
  case XMLHttpRequest::Open:
    {
      if (args.size() < 2)
          return throwError(exec, SyntaxError, "Not enough arguments");

      QString method = args[0]->toString(exec).qstring();
      KHTMLPart *part = qobject_cast<KHTMLPart*>(Window::retrieveActive(exec)->part());
      if (!part)
        return jsUndefined();
      KUrl url = KUrl(part->document().completeURL(args[1]->toString(exec).qstring()).string());

      bool async = true;
      if (args.size() >= 3) {
	async = args[2]->toBoolean(exec);
      }

      if (args.size() >= 4) {
	url.setUser(args[3]->toString(exec).qstring());
      }

      if (args.size() >= 5) {
	url.setPass(args[4]->toString(exec).qstring());
      }

      request->open(method, url, async, ec);
      setDOMException(exec, ec);
      return jsUndefined();
    }
  case XMLHttpRequest::Send:
    {
      QString body;
      if (args.size() >= 1) {
        DOM::NodeImpl* docNode = toNode(args[0]);
        if (docNode && docNode->isDocumentNode()) {
          DOM::DocumentImpl *doc = static_cast<DOM::DocumentImpl *>(docNode);

          try {
            body = doc->toString().string();
            // FIXME: also need to set content type, including encoding!

          } catch(DOM::DOMException&) {
            return throwError(exec, GeneralError, "Exception serializing document");
          }
        } else {
          body = args[0]->toString(exec).qstring();
        }
      }

      request->send(body, ec);
      setDOMException(exec, ec);
      return jsUndefined();
    }
  case XMLHttpRequest::SetRequestHeader:
      {
      if (args.size() < 2)
          return throwError(exec, SyntaxError, "Not enough arguments");
      JSValue* keyArgument = args[0];
      JSValue* valArgument = args[1];
      QString key, val;
      if (!keyArgument->isUndefined() && !keyArgument->isNull())
          key = keyArgument->toString(exec).qstring();
      if (!valArgument->isUndefined() && !valArgument->isNull())
          val = valArgument->toString(exec).qstring();
      request->setRequestHeader(key, val, ec);
      setDOMException(exec, ec);
      return jsUndefined();
      }

  case XMLHttpRequest::OverrideMIMEType:
      if (args.size() < 1)
          return throwError(exec, SyntaxError, "Not enough arguments");

      request->overrideMIMEType(args[0]->toString(exec).qstring());
      return jsUndefined();
  }

  return jsUndefined();
}

} // end namespace

#include "xmlhttprequest.moc"
