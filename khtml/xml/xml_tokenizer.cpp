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

#include "xml_tokenizer.h"
#include "dom_docimpl.h"
#include "dom_node.h"
#include "dom_elementimpl.h"
#include "dom_textimpl.h"
#include "khtmlview.h"
#include <kdebug.h>

using namespace DOM;

XMLHandler::XMLHandler(DocumentImpl *_doc, KHTMLView *_view)
{
  m_doc = _doc;
  m_view = _view;
  m_currentNode = _doc;
}


XMLHandler::~XMLHandler()
{
}


QString XMLHandler::errorProtocol()
{
    return errorProt;
}


bool XMLHandler::startDocument()
{
    // at the beginning of parsing: do some initialization
    errorProt = "";
    state = StateInit;

    return TRUE;
}


bool XMLHandler::startElement( const QString&, const QString&, const QString& qName, const QXmlAttributes& atts )
{
  if (m_currentNode->nodeType() == Node::TEXT_NODE)
    exitText();


  ElementImpl *newElement = m_doc->createElement(qName);

  // ### handle exceptions
  int i;
  for (i = 0; i < atts.length(); i++)
    newElement->setAttribute(atts.localName(i),atts.value(i));
  m_currentNode->addChild(newElement);
  newElement->attach(m_view);
  m_currentNode = newElement;

  return TRUE;

}


bool XMLHandler::endElement( const QString&, const QString&, const QString& qName )
{
  if (m_currentNode->nodeType() == Node::TEXT_NODE)
    exitText();
  if (m_currentNode->parentNode() != 0)
    m_currentNode = m_currentNode->parentNode();
// ###  else error

  return TRUE;
}


bool XMLHandler::characters( const QString& ch )
{
  if (m_currentNode->nodeType() != Node::TEXT_NODE)
    enterText();
  static_cast<TextImpl*>(m_currentNode)->appendData(ch);
  return TRUE;
}


QString XMLHandler::errorString()
{
  return "the document is not in the correct file format";
}


bool XMLHandler::fatalError( const QXmlParseException& exception )
{
    errorProt += QString( "fatal parsing error: %1 in line %2, column %3\n" )
	.arg( exception.message() )
	.arg( exception.lineNumber() )
	.arg( exception.columnNumber() );

    return QXmlDefaultHandler::fatalError( exception );
}

void XMLHandler::enterText()
{
  NodeImpl *newNode = m_doc->createTextNode("");
  m_currentNode->addChild(newNode);
  newNode->attach(m_view);
  m_currentNode = newNode;

}

void XMLHandler::exitText()
{
  if (m_currentNode->parentNode() != 0)
    m_currentNode = m_currentNode->parentNode();
}


//------------------------------------------------------------------------------



XMLTokenizer::XMLTokenizer(DOM::DocumentImpl *_doc, KHTMLView *_view)
{
    m_doc = _doc;
    m_view = _view;
    xmlCode = "";
}

XMLTokenizer::~XMLTokenizer()
{
}


void XMLTokenizer::begin()
{
}

void XMLTokenizer::write( const QString &str )
{
    xmlCode += str;
}

void XMLTokenizer::end()
{
    emit finishedParsing();
}

void XMLTokenizer::finish()
{
  // parse xml file
  XMLHandler handler(m_doc,m_view);
  QXmlInputSource source;
  source.setData(xmlCode);
  QXmlSimpleReader reader;
  reader.setContentHandler( &handler );
  reader.setErrorHandler( &handler );
  bool ok = reader.parse( source );
  if (!ok)
    kdDebug(6036) << "Error during XML parsing: " << handler.errorProtocol() << endl;
  // ### handle exceptions, !ok

  end();
}


