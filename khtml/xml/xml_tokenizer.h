/**
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

#include "html/htmltokenizer.h"
#include <qxml.h>

namespace DOM {
    class DocumentImpl;
    class NodeImpl;
}



class XMLHandler : public QXmlDefaultHandler
{
public:
    XMLHandler(DOM::DocumentImpl *_doc, KHTMLView *_view);
    virtual ~XMLHandler();

    // return the error protocol if parsing failed
    QString errorProtocol();

    // overloaded handler functions
    bool startDocument();
    bool startElement( const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts );
    bool endElement( const QString& namespaceURI, const QString& localName, const QString& qName );
    bool characters( const QString& ch );

    void enterText();
    void exitText();

    QString errorString();

    bool fatalError( const QXmlParseException& exception );

private:
    QString errorProt;
    DOM::DocumentImpl *m_doc;
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





class XMLTokenizer : public Tokenizer
{
    Q_OBJECT
public:
    XMLTokenizer(DOM::DocumentImpl *, KHTMLView * = 0);
    virtual ~XMLTokenizer();
    virtual void begin();
    virtual void write( const QString &str );
    virtual void end();
    virtual void finish();

protected:
    DocumentImpl *m_doc;
    KHTMLView *m_view;

    QString xmlCode;

};

#endif
