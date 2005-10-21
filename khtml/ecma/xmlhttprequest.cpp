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
#include <qobject.h>
#include <kdebug.h>

#ifdef APPLE_CHANGES
#include "KWQLoader.h"
#else
#include <kio/netaccess.h>
using KIO::NetAccess;
#endif

#define BANNED_HTTP_HEADERS "authorization,proxy-authorization,"\
                            "content-length,host,connect,copy,move,"\
                            "delete,head,trace,put,propfind,proppatch,"\
                            "mkcol,lock,unlock,options,via"

using namespace KJS;
using khtml::Decoder;

////////////////////// XMLHttpRequest Object ////////////////////////

/* Source for XMLHttpRequestProtoTable.
@begin XMLHttpRequestProtoTable 7
  abort			XMLHttpRequest::Abort			DontDelete|Function 0
  getAllResponseHeaders	XMLHttpRequest::GetAllResponseHeaders	DontDelete|Function 0
  getResponseHeader	XMLHttpRequest::GetResponseHeader	DontDelete|Function 1
  open			XMLHttpRequest::Open			DontDelete|Function 5
  send			XMLHttpRequest::Send			DontDelete|Function 1
  setRequestHeader	XMLHttpRequest::SetRequestHeader	DontDelete|Function 2
@end
*/
DEFINE_PROTOTYPE("XMLHttpRequest",XMLHttpRequestProto)
IMPLEMENT_PROTOFUNC_DOM(XMLHttpRequestProtoFunc)
IMPLEMENT_PROTOTYPE(XMLHttpRequestProto,XMLHttpRequestProtoFunc)

namespace KJS {

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

void XMLHttpRequestQObject::slotFinished( KIO::Job* job )
{
  jsObject->slotFinished(job);
}

void XMLHttpRequestQObject::slotRedirection( KIO::Job* job, const KURL& url)
{
  jsObject->slotRedirection( job, url );
}

XMLHttpRequestConstructorImp::XMLHttpRequestConstructorImp(ExecState *, const DOM::Document &d)
    : ObjectImp(), doc(d)
{
}

bool XMLHttpRequestConstructorImp::implementsConstruct() const
{
  return true;
}

Object XMLHttpRequestConstructorImp::construct(ExecState *exec, const List &)
{
  return Object(new XMLHttpRequest(exec, doc));
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

Value XMLHttpRequest::tryGet(ExecState *exec, const Identifier &propertyName) const
{
  return DOMObjectLookupGetValue<XMLHttpRequest,DOMObject>(exec, propertyName, &XMLHttpRequestTable, this);
}

Value XMLHttpRequest::getValueProperty(ExecState *exec, int token) const
{
  switch (token) {
  case ReadyState:
    return Number(state);
  case ResponseText:
    return getString(DOM::DOMString(response));
  case ResponseXML:
    if (state != Completed) {
      return Undefined();
    }
    if (!createdDocument) {
      QString mimeType = "text/xml";

      Value header = getResponseHeader("Content-Type");
      if (header.type() != UndefinedType) {
	mimeType = QStringList::split(";", header.toString(exec).qstring())[0].stripWhiteSpace();
      }

      if (mimeType == "text/xml" || mimeType == "application/xml" || mimeType == "application/xhtml+xml") {
	responseXML = DOM::Document(doc->implementation()->createDocument());

	DOM::DocumentImpl *docImpl = static_cast<DOM::DocumentImpl *>(responseXML.handle());

	docImpl->open();
	docImpl->write(response);
	docImpl->finishParsing();
	docImpl->close();

	typeIsXML = true;
      } else {
	typeIsXML = false;
      }
      createdDocument = true;
    }

    if (!typeIsXML) {
      return Undefined();
    }

    return getDOMNode(exec,responseXML);
  case Status:
    return getStatus();
  case StatusText:
    return getStatusText();
  case Onreadystatechange:
   if (onReadyStateChangeListener && onReadyStateChangeListener->listenerObjImp()) {
     return onReadyStateChangeListener->listenerObj();
   } else {
     return Null();
   }
  case Onload:
   if (onLoadListener && onLoadListener->listenerObjImp()) {
     return onLoadListener->listenerObj();
   } else {
    return Null();
   }
  default:
    kdWarning() << "XMLHttpRequest::getValueProperty unhandled token " << token << endl;
    return Value();
  }
}

void XMLHttpRequest::tryPut(ExecState *exec, const Identifier &propertyName, const Value& value, int attr)
{
  DOMObjectLookupPut<XMLHttpRequest,DOMObject>(exec, propertyName, value, attr, &XMLHttpRequestTable, this );
}

void XMLHttpRequest::putValueProperty(ExecState *exec, int token, const Value& value, int /*attr*/)
{
  switch(token) {
  case Onreadystatechange:
    onReadyStateChangeListener = Window::retrieveActive(exec)->getJSEventListener(value, true);
    if (onReadyStateChangeListener) onReadyStateChangeListener->ref();
    break;
  case Onload:
    onLoadListener = Window::retrieveActive(exec)->getJSEventListener(value, true);
    if (onLoadListener) onLoadListener->ref();
    break;
  default:
    kdWarning() << "XMLHttpRequest::putValue unhandled token " << token << endl;
  }
}

XMLHttpRequest::XMLHttpRequest(ExecState *exec, const DOM::Document &d)
  : DOMObject(XMLHttpRequestProto::self(exec)),
    qObject(new XMLHttpRequestQObject(this)),
    doc(static_cast<DOM::DocumentImpl*>(d.handle())),
    async(true),
    contentType(QString::null),
    job(0),
    state(Uninitialized),
    onReadyStateChangeListener(0),
    onLoadListener(0),
    decoder(0),
    createdDocument(false),
    aborted(false)
{
}

XMLHttpRequest::~XMLHttpRequest()
{
  delete qObject;
  qObject = 0;
  delete decoder;
  decoder = 0;
}

void XMLHttpRequest::changeState(XMLHttpRequestState newState)
{
  if (state != newState) {
    state = newState;

    if (onReadyStateChangeListener != 0 && doc->view() && doc->view()->part()) {
      DOM::Event ev = doc->view()->part()->document().createEvent("HTMLEvents");
      ev.initEvent("readystatechange", true, true);
      onReadyStateChangeListener->handleEvent(ev);
    }

    if (state == Completed && onLoadListener != 0 && doc->view() && doc->view()->part()) {
      DOM::Event ev = doc->view()->part()->document().createEvent("HTMLEvents");
      ev.initEvent("load", true, true);
      onLoadListener->handleEvent(ev);
    }
  }
}

bool XMLHttpRequest::urlMatchesDocumentDomain(const KURL& _url) const
{
  // No need to do work if _url is not valid...
  if (!_url.isValid())
    return false;

  KURL documentURL(doc->URL());

  // a local file can load anything
  if (documentURL.protocol().lower() == "file") {
    return true;
  }

  // but a remote document can only load from the same port on the server
  if (documentURL.protocol().lower() == _url.protocol().lower() &&
      documentURL.host().lower() == _url.host().lower() &&
      documentURL.port() == _url.port()) {
    return true;
  }

  return false;
}

void XMLHttpRequest::open(const QString& _method, const KURL& _url, bool _async)
{
  abort();
  aborted = false;

  // clear stuff from possible previous load
  requestHeaders.clear();
  responseHeaders = QString();
  response = QString();
  createdDocument = false;
  responseXML = DOM::Document();

  changeState(Uninitialized);

  if (aborted) {
    return;
  }

  if (!urlMatchesDocumentDomain(_url)) {
    return;
  }


  method = _method.lower();
  url = _url;
  async = _async;

  changeState(Loading);
}

void XMLHttpRequest::send(const QString& _body)
{
  aborted = false;

  if (method == "post") {
    QString protocol = url.protocol().lower();

    // Abondon the request when the protocol is other than "http",
    // instead of blindly changing it to a "get" request.
    if (!protocol.startsWith("http") && !protocol.startsWith("webdav"))
    {
      abort();
      return;
    }

    // FIXME: determine post encoding correctly by looking in headers
    // for charset.
    QByteArray buf;
    buf.duplicate(_body.utf8().data(), _body.length());

    job = KIO::http_post( url, buf, false );
    if(contentType.isNull())
      job->addMetaData( "content-type", "Content-type: text/plain" );
    else
      job->addMetaData( "content-type", contentType );
  }
  else {
    job = KIO::get( url, false, false );
  }

  if (!requestHeaders.isEmpty()) {
    QString rh;
    QMap<QString, QString>::ConstIterator begin = requestHeaders.begin();
    QMap<QString, QString>::ConstIterator end = requestHeaders.end();
    for (QMap<QString, QString>::ConstIterator i = begin; i != end; ++i) {
      if (i != begin)
        rh += "\r\n";
      rh += i.key() + ": " + i.data();
    }

    job->addMetaData("customHTTPHeader", rh);
  }

  job->addMetaData("PropagateHttpHeader", "true");

  // Set the default referrer if one is not already supplied
  // through setRequestHeader. NOTE: the user can still disable
  // this feature at the protocol level (kio_http).
  if (requestHeaders.find("Referer") == requestHeaders.end()) {
    KURL documentURL(doc->URL());
    documentURL.setPass(QString::null);
    documentURL.setUser(QString::null);
    job->addMetaData("referrer", documentURL.url());
    // kdDebug() << "Adding referrer: " << documentURL << endl;
  }

  if (!async) {
    QByteArray data;
    KURL finalURL;
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

  qObject->connect( job, SIGNAL( result( KIO::Job* ) ),
		    SLOT( slotFinished( KIO::Job* ) ) );
#ifdef APPLE_CHANGES
  qObject->connect( job, SIGNAL( data( KIO::Job*, const char*, int ) ),
		    SLOT( slotData( KIO::Job*, const char*, int ) ) );
#else
  qObject->connect( job, SIGNAL( data( KIO::Job*, const QByteArray& ) ),
		    SLOT( slotData( KIO::Job*, const QByteArray& ) ) );
#endif
  qObject->connect( job, SIGNAL(redirection(KIO::Job*, const KURL& ) ),
		    SLOT( slotRedirection(KIO::Job*, const KURL&) ) );

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
}

void XMLHttpRequest::setRequestHeader(const QString& _name, const QString &value)
{
  QString name = _name.lower().stripWhiteSpace();

  // Content-type needs to be set seperately from the other headers
  if(name == "content-type") {
    contentType = "Content-type: " + value;
    return;
  }

  // Sanitize the referrer header to protect against spoofing...
  if(name == "referer") {
    KURL referrerURL(value);
    if (urlMatchesDocumentDomain(referrerURL))
      requestHeaders[name] = referrerURL.url();
    return;
  }

  // Sanitize the request headers below and handle them as if they are
  // calls to open. Otherwise, we will end up ignoring them all together!
  // TODO: Do something about "put" which kio_http sort of supports and
  // the webDAV headers such as PROPFIND etc...
  if (name == "get"  || name == "post") {
    KURL reqURL (doc->URL(), value.stripWhiteSpace());
    open(name, reqURL, async);
    return;
  }

  // Reject all banned headers. See BANNED_HTTP_HEADERS above.
  qDebug("Banned HTTP Headers: %s", BANNED_HTTP_HEADERS);
  QStringList bannedHeaders = QStringList::split(',',
                                  QString::fromLatin1(BANNED_HTTP_HEADERS));

  if (bannedHeaders.contains(name))
    return;   // Denied

  requestHeaders[name] = value.stripWhiteSpace();
}

Value XMLHttpRequest::getAllResponseHeaders() const
{
  if (responseHeaders.isEmpty()) {
    return Undefined();
  }

  int endOfLine = responseHeaders.find("\n");

  if (endOfLine == -1) {
    return Undefined();
  }

  return String(responseHeaders.mid(endOfLine + 1) + "\n");
}

Value XMLHttpRequest::getResponseHeader(const QString& name) const
{
  if (responseHeaders.isEmpty()) {
    return Undefined();
  }

  QRegExp headerLinePattern(name + ":", false);

  int matchLength;
  int headerLinePos = headerLinePattern.search(responseHeaders, 0);
  matchLength = headerLinePattern.matchedLength();
  while (headerLinePos != -1) {
    if (headerLinePos == 0 || responseHeaders[headerLinePos-1] == '\n') {
      break;
    }

    headerLinePos = headerLinePattern.search(responseHeaders, headerLinePos + 1);
    matchLength = headerLinePattern.matchedLength();
  }


  if (headerLinePos == -1) {
    return Undefined();
  }

  int endOfLine = responseHeaders.find("\n", headerLinePos + matchLength);

  return String(responseHeaders.mid(headerLinePos + matchLength, endOfLine - (headerLinePos + matchLength)).stripWhiteSpace());
}

static Value httpStatus(const QString& response, bool textStatus = false)
{
  if (response.isEmpty()) {
    return Undefined();
  }

  int endOfLine = response.find("\n");
  QString firstLine = (endOfLine == -1) ? response : response.left(endOfLine);
  int codeStart = firstLine.find(" ");
  int codeEnd = firstLine.find(" ", codeStart + 1);

  if (codeStart == -1 || codeEnd == -1) {
    return Undefined();
  }

  if (textStatus) {
    QString statusText = firstLine.mid(codeEnd + 1, endOfLine - (codeEnd + 1)).stripWhiteSpace();
    return String(statusText);
  }

  QString number = firstLine.mid(codeStart + 1, codeEnd - (codeStart + 1));

  bool ok = false;
  int code = number.toInt(&ok);
  if (!ok) {
    return Undefined();
  }

  return Number(code);
}

Value XMLHttpRequest::getStatus() const
{
  return httpStatus(responseHeaders);
}

Value XMLHttpRequest::getStatusText() const
{
  return httpStatus(responseHeaders, true);
}

void XMLHttpRequest::processSyncLoadResults(const QByteArray &data, const KURL &finalURL, const QString &headers)
{
  if (!urlMatchesDocumentDomain(finalURL)) {
    abort();
    return;
  }

  responseHeaders = headers;
  changeState(Loaded);
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

void XMLHttpRequest::slotFinished(KIO::Job *)
{
  if (decoder) {
    response += decoder->flush();
  }

  // make sure to forget about the job before emitting completed,
  // since changeState triggers JS code, which might e.g. call abort.
  job = 0;
  changeState(Completed);

  delete decoder;
  decoder = 0;
}

void XMLHttpRequest::slotRedirection(KIO::Job*, const KURL& url)
{
  if (!urlMatchesDocumentDomain(url)) {
    abort();
  }
}

#ifdef APPLE_CHANGES
void XMLHttpRequest::slotData( KIO::Job*, const char *data, int len )
#else
void XMLHttpRequest::slotData(KIO::Job*, const QByteArray &_data)
#endif
{
  if (state < Loaded ) {
    responseHeaders = job->queryMetaData("HTTP-Headers");

    // NOTE: Replace a 304 response with a 200! Both IE and Mozilla do this.
    // Problem first reported through bug# 110272.
    int codeStart = responseHeaders.find("304");
    if ( codeStart != -1) {
      int codeEnd = responseHeaders.find("\n", codeStart+3);
      if (codeEnd != -1)
        responseHeaders.replace(codeStart, (codeEnd-codeStart), "200 OK");
    }

    changeState(Loaded);
  }

#ifndef APPLE_CHANGES
  const char *data = (const char *)_data.data();
  int len = (int)_data.size();
#endif

  if ( decoder == NULL ) {
    int pos = responseHeaders.find("content-type:", 0, false);

    if ( pos > -1 ) {
      pos += 13;
      int index = responseHeaders.find('\n', pos);
      QString type = responseHeaders.mid(pos, (index-pos));
      index = type.find (';');
      if (index > -1)
        encoding = type.mid( index+1 ).remove(QRegExp("charset[ ]*=[ ]*", false)).stripWhiteSpace();
    }

    decoder = new Decoder;
    if (!encoding.isNull())
      decoder->setEncoding(encoding.latin1(), Decoder::EncodingFromHTTPHeader);
    else {
      // FIXME: Inherit the default encoding from the parent document?
    }
  }
  if (len == 0)
    return;

  if (len == -1)
    len = strlen(data);

  QString decoded = decoder->decode(data, len);

  response += decoded;

  if (!aborted) {
    changeState(Interactive);
  }
}

Value XMLHttpRequestProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  if (!thisObj.inherits(&XMLHttpRequest::info)) {
    Object err = Error::create(exec,TypeError);
    exec->setException(err);
    return err;
  }

  XMLHttpRequest *request = static_cast<XMLHttpRequest *>(thisObj.imp());
  switch (id) {
  case XMLHttpRequest::Abort:
    request->abort();
    return Undefined();
  case XMLHttpRequest::GetAllResponseHeaders:
    if (args.size() != 0) {
    return Undefined();
    }

    return request->getAllResponseHeaders();
  case XMLHttpRequest::GetResponseHeader:
    if (args.size() != 1) {
    return Undefined();
    }

    return request->getResponseHeader(args[0].toString(exec).qstring());
  case XMLHttpRequest::Open:
    {
      if (args.size() < 2 || args.size() > 5) {
        return Undefined();
      }

      QString method = args[0].toString(exec).qstring();
      KHTMLPart *part = ::qt_cast<KHTMLPart *>(Window::retrieveActive(exec)->part());
      if (!part)
        return Undefined();
      KURL url = KURL(part->document().completeURL(args[1].toString(exec).qstring()).string());

      bool async = true;
      if (args.size() >= 3) {
	async = args[2].toBoolean(exec);
      }

      if (args.size() >= 4) {
	url.setUser(args[3].toString(exec).qstring());
      }

      if (args.size() >= 5) {
	url.setPass(args[4].toString(exec).qstring());
      }

      request->open(method, url, async);

      return Undefined();
    }
  case XMLHttpRequest::Send:
    {
      if (args.size() > 1) {
        return Undefined();
      }

      if (request->state != Loading) {
	return Undefined();
      }

      QString body;

      if (args.size() >= 1) {
	Object obj = Object::dynamicCast(args[0]);
	if (obj.isValid() && obj.inherits(&DOMDocument::info)) {
	  DOM::Node docNode = static_cast<KJS::DOMDocument *>(obj.imp())->toNode();
	  DOM::DocumentImpl *doc = static_cast<DOM::DocumentImpl *>(docNode.handle());

	  try {
	    body = doc->toString().string();
	    // FIXME: also need to set content type, including encoding!

	  } catch(DOM::DOMException& e) {
	     Object err = Error::create(exec, GeneralError, "Exception serializing document");
	     exec->setException(err);
	  }
	} else {
	  body = args[0].toString(exec).qstring();
	}
      }

      request->send(body);

      return Undefined();
    }
  case XMLHttpRequest::SetRequestHeader:
    if (args.size() != 2) {
      return Undefined();
    }

    request->setRequestHeader(args[0].toString(exec).qstring(), args[1].toString(exec).qstring());

    return Undefined();
  }

  return Undefined();
}

} // end namespace

#include "xmlhttprequest.moc"
