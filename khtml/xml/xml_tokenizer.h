/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 2000 Peter Kelly (pmk@post.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */

#ifndef _XML_Tokenizer_h_
#define _XML_Tokenizer_h_

#include <qxml.h>
#include <qptrlist.h>
#include <qobject.h>
#include "misc/loader_client.h"

class KHTMLView;

namespace khtml {
    class CachedObject;
    class CachedScript;
}

namespace DOM {
    class DocumentImpl;
    class NodeImpl;
    class HTMLScriptElementImpl;
    class DocumentPtr;
    class HTMLScriptElementImpl;
}

namespace khtml {

class XMLHandler : public QXmlDefaultHandler
{
public:
    XMLHandler(DOM::DocumentPtr *_doc, KHTMLView *_view);
    virtual ~XMLHandler();

    // return the error protocol if parsing failed
    QString errorProtocol();

    // overloaded handler functions
    bool startDocument();
    bool startElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts);
    bool endElement(const QString& namespaceURI, const QString& localName, const QString& qName);
    bool startCDATA();
    bool endCDATA();
    bool characters(const QString& ch);
    bool comment(const QString & ch);
    bool processingInstruction(const QString &target, const QString &data);


    // from QXmlDeclHandler
    bool attributeDecl(const QString &eName, const QString &aName, const QString &type, const QString &valueDefault, const QString &value);
    bool externalEntityDecl(const QString &name, const QString &publicId, const QString &systemId);
    bool internalEntityDecl(const QString &name, const QString &value);

    // from QXmlDTDHandler
    bool notationDecl(const QString &name, const QString &publicId, const QString &systemId);
    bool unparsedEntityDecl(const QString &name, const QString &publicId, const QString &systemId, const QString &notationName);

    bool enterText();
    void exitText();

    QString errorString();

    bool fatalError( const QXmlParseException& exception );

    unsigned long errorLine;
    unsigned long errorCol;

private:
    QString errorProt;
    DOM::DocumentPtr *m_doc;
    KHTMLView *m_view;
    DOM::NodeImpl *m_currentNode;
    DOM::NodeImpl *m_rootNode;

    enum State {
        StateInit,
        StateDocument,
        StateQuote,
        StateLine,
        StateHeading,
        StateP
    };
    State state;
};

class Tokenizer : public QObject
{
    Q_OBJECT
public:
    virtual void begin() = 0;
    // script output must be prepended, while new data
    // received during executing a script must be appended, hence the
    // extra bool to be able to distinguish between both cases. document.write()
    // always uses false, while khtmlpart uses true
    virtual void write( const QString &str, bool appendData) = 0;
    virtual void end() = 0;
    virtual void finish() = 0;
    virtual void setOnHold(bool /*_onHold*/) {}

signals:
    void finishedParsing();

};

class XMLIncrementalSource : public QXmlInputSource
{
public:
    XMLIncrementalSource();
    virtual void fetchData();
    virtual QChar next();
    virtual void setData( const QString& str );
    virtual void setData( const QByteArray& data );
    virtual QString data();

    void appendXML( const QString& str );
    void setFinished( bool );

private:
    QString      m_data;
    uint         m_pos;
    const QChar *m_unicode;
    bool         m_finished;
};

class XMLTokenizer : public Tokenizer, public khtml::CachedObjectClient
{
public:
    XMLTokenizer(DOM::DocumentPtr *, KHTMLView * = 0);
    virtual ~XMLTokenizer();
    virtual void begin();
    virtual void write( const QString &str, bool );
    virtual void end();
    virtual void finish();

    // from CachedObjectClient
    void notifyFinished(khtml::CachedObject *finishedObj);

protected:
    DOM::DocumentPtr *m_doc;
    KHTMLView *m_view;

    void executeScripts();
    void addScripts(DOM::NodeImpl *n);

    QPtrList<DOM::HTMLScriptElementImpl> m_scripts;
    QPtrListIterator<DOM::HTMLScriptElementImpl> *m_scriptsIt;
    khtml::CachedScript *m_cachedScript;

    XMLHandler m_handler;
    QXmlSimpleReader m_reader;
    XMLIncrementalSource m_source;
    bool m_noErrors;
};

} // end namespace

#endif
