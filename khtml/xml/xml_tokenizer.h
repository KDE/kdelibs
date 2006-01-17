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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef _XML_Tokenizer_h_
#define _XML_Tokenizer_h_

#include <qxml.h>
#include <q3ptrlist.h>
#include <q3ptrstack.h>
#include <qobject.h>
#include "misc/loader_client.h"
#include "misc/stringit.h"

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

    QString errorString() const;

    bool fatalError( const QXmlParseException& exception );

    unsigned long errorLine;
    unsigned long errorCol;

private:
    void pushNode( DOM::NodeImpl *node );
    DOM::NodeImpl *popNode();
    DOM::NodeImpl *currentNode() const;
private:
    QString errorProt;
    DOM::DocumentPtr *m_doc;
    KHTMLView *m_view;
    Q3PtrStack<DOM::NodeImpl> m_nodes;
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
    virtual void write( const TokenizerString &str, bool appendData) = 0;
    virtual void end() = 0;
    virtual void finish() = 0;
    virtual void setOnHold(bool /*_onHold*/) {}
    virtual bool isWaitingForScripts() const = 0;
    virtual bool isExecutingScript() const = 0;
    virtual void abort() {}
    virtual void setAutoClose(bool b=true) = 0;

Q_SIGNALS:
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
    virtual QString data() const;

    void appendXML( const QString& str );
    void setFinished( bool );

private:
    QString      m_data;
    int          m_pos;
    const QChar *m_unicode;
    bool         m_finished;
};

class XMLTokenizer : public Tokenizer, public khtml::CachedObjectClient
{
public:
    XMLTokenizer(DOM::DocumentPtr *, KHTMLView * = 0);
    virtual ~XMLTokenizer();
    virtual void begin();
    virtual void write( const TokenizerString &str, bool );
    virtual void end();
    virtual void finish();
    virtual void setAutoClose(bool b=true) { qWarning("XMLTokenizer::setAutoClose: stub."); (void)b; }

    // from CachedObjectClient
    void notifyFinished(khtml::CachedObject *finishedObj);

    virtual bool isWaitingForScripts() const;
    virtual bool isExecutingScript() const { return false; }

protected:
    DOM::DocumentPtr *m_doc;
    KHTMLView *m_view;

    void executeScripts();
    void addScripts(DOM::NodeImpl *n);

    Q3PtrList<DOM::HTMLScriptElementImpl> m_scripts;
    Q3PtrListIterator<DOM::HTMLScriptElementImpl> *m_scriptsIt;
    khtml::CachedScript *m_cachedScript;

    XMLHandler m_handler;
    QXmlSimpleReader m_reader;
    XMLIncrementalSource m_source;
    bool m_noErrors;
};

} // end namespace

#endif
