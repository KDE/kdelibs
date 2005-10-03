/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 2000 Peter Kelly (pmk@post.com)
 * Copyright (C) 2003 Apple Computer, Inc.
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
 */


#include "xml_tokenizer.h"
#include "xml/dom_docimpl.h"
#include "xml/dom_textimpl.h"
#include "xml/dom_xmlimpl.h"
#include "html/html_tableimpl.h"
#include "html/html_headimpl.h"
#include "rendering/render_object.h"
#include "misc/htmltags.h"
#include "misc/htmlattrs.h"
#include "misc/loader.h"

#include "khtmlview.h"
#include "khtml_part.h"
#include <qvariant.h>
#include <kdebug.h>
#include <klocale.h>

using namespace DOM;
using namespace khtml;

XMLIncrementalSource::XMLIncrementalSource()
    : QXmlInputSource(), m_pos( 0 ), m_unicode( 0 ),
      m_finished( false )
{
}

void XMLIncrementalSource::fetchData()
{
    //just a dummy to overwrite default behavior
}

QChar XMLIncrementalSource::next()
{
    if ( m_finished )
        return QXmlInputSource::EndOfDocument;
    else if ( m_data.length() <= m_pos )
        return QXmlInputSource::EndOfData;
    else
        return m_unicode[m_pos++];
}

void XMLIncrementalSource::setData( const QString& str )
{
    m_data = str;
    m_unicode = m_data.unicode();
    m_pos = 0;
    if ( !str.isEmpty() )
        m_finished = false;
}
void XMLIncrementalSource::setData( const QByteArray& data )
{
    setData( fromRawData( data, true ) );
}

void XMLIncrementalSource::appendXML( const QString& str )
{
    m_data += str;
    m_unicode = m_data.unicode();
}

QString XMLIncrementalSource::data() const
{
    return m_data;
}

void XMLIncrementalSource::setFinished( bool finished )
{
    m_finished = finished;
}

XMLHandler::XMLHandler(DocumentPtr *_doc, KHTMLView *_view)
    : errorLine(0)
{
    m_doc = _doc;
    if ( m_doc ) m_doc->ref();
    m_view = _view;
    pushNode( _doc->document() );
}

XMLHandler::~XMLHandler()
{
    if ( m_doc ) m_doc->deref();
}

void XMLHandler::pushNode( NodeImpl *node )
{
    m_nodes.push( node );
}

NodeImpl *XMLHandler::popNode()
{
    return m_nodes.pop();
}

NodeImpl *XMLHandler::currentNode() const
{
    return m_nodes.current();
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

    return true;
}


bool XMLHandler::startElement( const QString& namespaceURI, const QString& /*localName*/,
                               const QString& qName, const QXmlAttributes& atts )
{
    if (currentNode()->nodeType() == Node::TEXT_NODE)
        exitText();

    ElementImpl *newElement = m_doc->document()->createElementNS(namespaceURI,qName);
    if (!newElement)
        return false;

    int i;
    for (i = 0; i < atts.length(); i++) {
        int exceptioncode = 0;
        DOMString uri(atts.uri(i));
        DOMString qn(atts.qName(i));
        DOMString val(atts.value(i));
        newElement->setAttributeNS(uri, qn, val, exceptioncode);
        if (exceptioncode) // exception setting attributes
            return false;
    }

    if (newElement->id() == ID_SCRIPT)
        static_cast<HTMLScriptElementImpl *>(newElement)->setCreatedByParser(true);

    //this is tricky. in general the node doesn't have to attach to the one it's in. as far
    //as standards go this is wrong, but there's literally thousands of documents where
    //we see <p><ul>...</ul></p>. the following code is there for those cases.
    //when we can't attach to the currently holding us node we try to attach to its parent
    bool attached = false;
    for ( NodeImpl *current = currentNode(); current; current = current->parent() ) {
        attached = current->addChild( newElement );
        if ( attached )
            break;
    }
    if (attached) {
        if (m_view && !newElement->attached() && !m_doc->document()->hasPendingSheets())
            newElement->attach();
        pushNode( newElement );
        return true;
    }
    else {
        delete newElement;
        return false;
    }

    // ### DOM spec states: "if there is no markup inside an element's content, the text is contained in a
    // single object implementing the Text interface that is the only child of the element."... do we
    // need to ensure that empty elements always have an empty text child?
}


bool XMLHandler::endElement( const QString& /*namespaceURI*/, const QString& /*localName*/, const QString& /*qName*/ )
{
    if (currentNode()->nodeType() == Node::TEXT_NODE)
        exitText();

    NodeImpl *node = popNode();
    if ( node ) {
        node->close();
        while ( currentNode()  && currentNode()->implicitNode() ) //for the implicit HTMLTableSectionElementImpl
            popNode()->close();
    } else
        return false;

    return true;
}


bool XMLHandler::startCDATA()
{
    if (currentNode()->nodeType() == Node::TEXT_NODE)
        exitText();

    NodeImpl *newNode = m_doc->document()->createCDATASection(new DOMStringImpl(""));
    if (currentNode()->addChild(newNode)) {
        if (m_view && !newNode->attached() && !m_doc->document()->hasPendingSheets())
            newNode->attach();
        pushNode( newNode );
        return true;
    }
    else {
        delete newNode;
        return false;
    }

}

bool XMLHandler::endCDATA()
{
    popNode();
    Q_ASSERT( currentNode() );
    return currentNode();
}

bool XMLHandler::characters( const QString& ch )
{
    //this is needed for xhtml parsing. otherwise we try to attach
    //"\n\t" to html, head and other nodes which don't accept textchildren
    if ( ch.trimmed().isEmpty() )
        return true;

    if (currentNode()->nodeType() == Node::TEXT_NODE ||
        currentNode()->nodeType() == Node::CDATA_SECTION_NODE ||
        enterText()) {
        int exceptioncode = 0;
        static_cast<TextImpl*>(currentNode())->appendData(ch,exceptioncode);
        if (exceptioncode)
            return false;
        return true;
    }
    else
        return false;

}

bool XMLHandler::comment(const QString & ch)
{
    if (currentNode()->nodeType() == Node::TEXT_NODE)
        exitText();
    // ### handle exceptions
    currentNode()->addChild(m_doc->document()->createComment(new DOMStringImpl(ch.unicode(), ch.length())));
    return true;
}

bool XMLHandler::processingInstruction(const QString &target, const QString &data)
{
    if (currentNode()->nodeType() == Node::TEXT_NODE)
        exitText();
    // ### handle exceptions
    ProcessingInstructionImpl *pi =
        m_doc->document()->createProcessingInstruction(target, new DOMStringImpl(data.unicode(), data.length()));
    currentNode()->addChild(pi);
    pi->checkStyleSheet();
    return true;
}


QString XMLHandler::errorString() const
{
    return i18n("the document is not in the correct file format");
}


bool XMLHandler::fatalError( const QXmlParseException& exception )
{
    errorProt += i18n( "fatal parsing error: %1 in line %2, column %3" )
        .arg( exception.message() )
        .arg( exception.lineNumber() )
        .arg( exception.columnNumber() );

    errorLine = exception.lineNumber();
    errorCol = exception.columnNumber();

    return false;
}

bool XMLHandler::enterText()
{
    NodeImpl *newNode = m_doc->document()->createTextNode("");
    if (currentNode()->addChild(newNode)) {
        pushNode( newNode );
        return true;
    }
    else {
        delete newNode;
        return false;
    }
}

void XMLHandler::exitText()
{
    if ( m_view && !currentNode()->attached() && !m_doc->document()->hasPendingSheets() )
        currentNode()->attach();
    popNode();
}

bool XMLHandler::attributeDecl(const QString &/*eName*/, const QString &/*aName*/, const QString &/*type*/,
                               const QString &/*valueDefault*/, const QString &/*value*/)
{
    // qt's xml parser (as of 2.2.3) does not currently give us values for type, valueDefault and
    // value. When it does, we can store these somewhere and have default attributes on elements
    return true;
}

bool XMLHandler::externalEntityDecl(const QString &/*name*/, const QString &/*publicId*/, const QString &/*systemId*/)
{
    // ### insert these too - is there anything special we have to do here?
    return true;
}

bool XMLHandler::internalEntityDecl(const QString &name, const QString &value)
{
    EntityImpl *e = new EntityImpl(m_doc,name);
    // ### further parse entities inside the value and add them as separate nodes (or entityreferences)?
    e->addChild(m_doc->document()->createTextNode(new DOMStringImpl(value.unicode(), value.length())));
     if (m_doc->document()->doctype())
         static_cast<GenericRONamedNodeMapImpl*>(m_doc->document()->doctype()->entities())->addNode(e);
    return true;
}

bool XMLHandler::notationDecl(const QString &/*name*/, const QString &/*publicId*/, const QString &/*systemId*/)
{
// ### FIXME
//     if (m_doc->document()->doctype()) {
//         NotationImpl *n = new NotationImpl(m_doc,name,publicId,systemId);
//         static_cast<GenericRONamedNodeMapImpl*>(m_doc->document()->doctype()->notations())->addNode(n);
//     }
    return true;
}

bool XMLHandler::unparsedEntityDecl(const QString &/*name*/, const QString &/*publicId*/,
                                    const QString &/*systemId*/, const QString &/*notationName*/)
{
    // ###
    return true;
}


//------------------------------------------------------------------------------

XMLTokenizer::XMLTokenizer(DOM::DocumentPtr *_doc, KHTMLView *_view)
    : m_handler(_doc,_view)
{
    m_doc = _doc;
    if ( m_doc ) m_doc->ref();
    m_view = _view;
    m_scriptsIt = 0;
    m_cachedScript = 0;
    m_noErrors = true;
    m_reader.setContentHandler( &m_handler );
    m_reader.setLexicalHandler( &m_handler );
    m_reader.setErrorHandler( &m_handler );
    m_reader.setDeclHandler( &m_handler );
    m_reader.setDTDHandler( &m_handler );
    m_reader.setFeature("http://xml.org/sax/features/namespace-prefixes", true);
}

XMLTokenizer::~XMLTokenizer()
{
    if (m_doc) m_doc->deref();
    if (m_scriptsIt)
        delete m_scriptsIt;
    if (m_cachedScript)
        m_cachedScript->deref(this);
}


void XMLTokenizer::begin()
{
    // parse xml file
    m_reader.parse( &m_source, true );
}

void XMLTokenizer::write( const TokenizerString &str, bool appendData )
{
    if ( !m_noErrors && appendData )
        return;
    if ( appendData ) {
        m_source.appendXML( str.toString() );

    } else {
        m_source.setData( str.toString() );
    }
    m_noErrors = m_reader.parseContinue();
}

void XMLTokenizer::end()
{
    m_source.setFinished( true );
    //if ( m_noErrors )
    //m_noErrors = m_reader.parseContinue();
    emit finishedParsing();
}

void XMLTokenizer::finish()
{
    m_source.setFinished( true );
    if (!m_noErrors) {
        // An error occurred during parsing of the code. Display an error page to the user (the DOM
        // tree is created manually and includes an excerpt from the code where the error is located)

        // ### for multiple error messages, display the code for each (can this happen?)

        // Clear the document
        int exceptioncode = 0;
        while (m_doc->document()->hasChildNodes())
            static_cast<NodeImpl*>(m_doc->document())->removeChild(m_doc->document()->firstChild(),exceptioncode);

        QString line, errorLocPtr;
        if ( m_handler.errorLine ) {
            QString xmlCode = m_source.data();
            QTextIStream stream(&xmlCode);
            for (unsigned long lineno = 0; lineno < m_handler.errorLine-1; lineno++)
                stream.readLine();
            line = stream.readLine();

            for (unsigned long colno = 0; colno < m_handler.errorCol-1; colno++)
                errorLocPtr += " ";
            errorLocPtr += "^";
        }

        // Create elements for display
        DocumentImpl *doc = m_doc->document();
        NodeImpl *html = doc->createElementNS(XHTML_NAMESPACE,"html");
        NodeImpl   *body = doc->createElementNS(XHTML_NAMESPACE,"body");
        NodeImpl     *h1 = doc->createElementNS(XHTML_NAMESPACE,"h1");
        NodeImpl       *headingText = doc->createTextNode(i18n("XML parsing error"));
        NodeImpl     *errorText = doc->createTextNode(m_handler.errorProtocol());
        NodeImpl     *hr = 0;
        NodeImpl     *pre = 0;
        NodeImpl     *lineText = 0;
        NodeImpl     *errorLocText = 0;
        if ( !line.isNull() ) {
            hr = doc->createElementNS(XHTML_NAMESPACE,"hr");
            pre = doc->createElementNS(XHTML_NAMESPACE,"pre");
            lineText = doc->createTextNode(line+"\n");
            errorLocText = doc->createTextNode(errorLocPtr);
        }

        // Construct DOM tree. We ignore exceptions as we assume they will not be thrown here (due to the
        // fact we are using a known tag set)
        doc->appendChild(html,exceptioncode);
        html->appendChild(body,exceptioncode);
        if ( body )
            body->appendChild(h1,exceptioncode);
        h1->appendChild(headingText,exceptioncode);
        body->appendChild(errorText,exceptioncode);
        body->appendChild(hr,exceptioncode);
        body->appendChild(pre,exceptioncode);
        if ( pre ) {
            pre->appendChild(lineText,exceptioncode);
            pre->appendChild(errorLocText,exceptioncode);
        }

        // Close the renderers so that they update their display correctly
        // ### this should not be necessary, but requires changes in the rendering code...
        h1->close();
        if ( pre ) pre->close();
        body->close();

        m_doc->document()->recalcStyle( NodeImpl::Inherit );
        m_doc->document()->updateRendering();

        end();
    }
    else {
        // Parsing was successful. Now locate all html <script> tags in the document and execute them
        // one by one
        addScripts(m_doc->document());
        m_scriptsIt = new Q3PtrListIterator<HTMLScriptElementImpl>(m_scripts);
        executeScripts();
    }

}

void XMLTokenizer::addScripts(NodeImpl *n)
{
    // Recursively go through the entire document tree, looking for html <script> tags. For each of these
    // that is found, add it to the m_scripts list from which they will be executed

    if (n->id() == ID_SCRIPT) {
        m_scripts.append(static_cast<HTMLScriptElementImpl*>(n));
    }

    NodeImpl *child;
    for (child = n->firstChild(); child; child = child->nextSibling())
        addScripts(child);
}

void XMLTokenizer::executeScripts()
{
    // Iterate through all of the html <script> tags in the document. For those that have a src attribute,
    // start loading the script and return (executeScripts() will be called again once the script is loaded
    // and continue where it left off). For scripts that don't have a src attribute, execute the code
    // inside the tag
    while (m_scriptsIt->current()) {
        DOMString scriptSrc = m_scriptsIt->current()->getAttribute(ATTR_SRC);
        QString charset = m_scriptsIt->current()->getAttribute(ATTR_CHARSET).string();

        if (!scriptSrc.isEmpty()) {
            // we have a src attribute
            m_cachedScript = m_doc->document()->docLoader()->requestScript(scriptSrc, charset);
            ++(*m_scriptsIt);
            m_cachedScript->ref(this); // will call executeScripts() again if already cached
            return;
        }
        else {
            // no src attribute - execute from contents of tag
            QString scriptCode = "";
            NodeImpl *child;
            for (child = m_scriptsIt->current()->firstChild(); child; child = child->nextSibling()) {
                if ( ( child->nodeType() == Node::TEXT_NODE || child->nodeType() == Node::CDATA_SECTION_NODE) &&
                     static_cast<TextImpl*>(child)->string() )
                    scriptCode += QConstString(static_cast<TextImpl*>(child)->string()->s,
                                               static_cast<TextImpl*>(child)->string()->l).string();
            }
            // the script cannot do document.write until we support incremental parsing
            // ### handle the case where the script deletes the node or redirects to
            // another page, etc. (also in notifyFinished())
            // ### the script may add another script node after this one which should be executed
            if (m_view) {
                m_view->part()->executeScript(DOM::Node(), scriptCode);
            }
            ++(*m_scriptsIt);
        }
    }

    // All scripts have finished executing, so calculate the style for the document and close
    // the last element
    m_doc->document()->updateStyleSelector();

    // We are now finished parsing
    end();
}

void XMLTokenizer::notifyFinished(CachedObject *finishedObj)
{
    // This is called when a script has finished loading that was requested from executeScripts(). We execute
    // the script, and then call executeScripts() again to continue iterating through the list of scripts in
    // the document
    if (finishedObj == m_cachedScript) {
        DOMString scriptSource = m_cachedScript->script();
        m_cachedScript->deref(this);
        m_cachedScript = 0;
        m_view->part()->executeScript(DOM::Node(), scriptSource.string());
        executeScripts();
    }
}

bool XMLTokenizer::isWaitingForScripts() const
{
    return m_cachedScript != 0;
}

#include "xml_tokenizer.moc"

