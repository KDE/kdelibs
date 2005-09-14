/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
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
 * the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "html/html_documentimpl.h"
#include "html/html_imageimpl.h"
#include "html/html_headimpl.h"
#include "html/html_baseimpl.h"
#include "html/htmltokenizer.h"
#include "html/html_miscimpl.h"
#include "html/html_formimpl.h"

#include "khtmlview.h"
#include "khtml_part.h"
#include "khtmlpart_p.h"
#include "khtml_settings.h"
#include "misc/htmlattrs.h"
#include "misc/htmlhashes.h"

#include "xml/xml_tokenizer.h"
#include "xml/dom2_eventsimpl.h"

#include "khtml_factory.h"
#include "rendering/render_object.h"
#include "dom/dom_exception.h"

#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kurl.h>
#include <kglobal.h>
#include <kcharsets.h>
#include <kglobalsettings.h>
#include <ktoolinvocation.h>

#include "css/cssproperties.h"
#include "css/cssstyleselector.h"
#include "css/css_stylesheetimpl.h"
#include <stdlib.h>
#include <q3ptrstack.h>

// Turn off inlining to avoid warning with newer gcc.
#undef __inline
#define __inline
#include "doctypes.cpp"
#undef __inline

template class Q3PtrStack<DOM::NodeImpl>;

using namespace DOM;
using namespace khtml;


HTMLDocumentImpl::HTMLDocumentImpl(DOMImplementationImpl *_implementation, KHTMLView *v)
  : DocumentImpl(_implementation, v)
{
//    kdDebug( 6090 ) << "HTMLDocumentImpl constructor this = " << this << endl;
    htmlElement = 0;

    m_doAutoFill = false;

/* dynamic history stuff to be fixed later (pfeiffer)
    connect( KHTMLFactory::vLinks(), SIGNAL( inserted( const QString& )),
             SLOT( slotHistoryChanged() ));
    connect( KHTMLFactory::vLinks(), SIGNAL( removed( const QString& )),
             SLOT( slotHistoryChanged() ));
*/
    connect( KHTMLFactory::vLinks(), SIGNAL( cleared()),
             SLOT( slotHistoryChanged() ));
}

HTMLDocumentImpl::~HTMLDocumentImpl()
{
}

DOMString HTMLDocumentImpl::referrer() const
{
    if ( view() )
        return view()->part()->pageReferrer();
    return DOMString();
}

DOMString HTMLDocumentImpl::lastModified() const
{
    if ( view() )
        return view()->part()->lastModified();
    return DOMString();
}

DOMString HTMLDocumentImpl::cookie() const
{
    long windowId = 0;
    KHTMLView *v = view ();

    if ( v && v->topLevelWidget() )
      windowId = v->topLevelWidget()->winId();

    DCOPRef   kcookiejar("kcookiejar", "kcookiejar");
    DCOPReply reply = kcookiejar.call("findDOMCookies(QString,long int)",
                  URL().url(), windowId);

    if ( !reply.isValid() )
    {
       kdWarning(6010) << "Can't communicate with cookiejar!" << endl;
       return DOMString();
    }

    QString result;
    if ( !reply.get(result, "QString") ) {
         kdError(6010) << "DCOP function findDOMCookies(...) returns "
                       << reply.type << ", expected QString" << endl;
         return DOMString();
    }

    return DOMString(result);
}

void HTMLDocumentImpl::setCookie( const DOMString & value )
{
    long windowId = 0;
    KHTMLView *v = view ();

    if ( v && v->topLevelWidget() )
      windowId = v->topLevelWidget()->winId();

    QByteArray params;
    QDataStream stream(&params, QIODevice::WriteOnly);
    QByteArray fake_header("Set-Cookie: ");
    fake_header.append(value.string().latin1());
    fake_header.append("\n");
    stream << URL().url() << fake_header << windowId;
    if (!KApplication::dcopClient()->send("kcookiejar", "kcookiejar",
                                  "addCookies(QString,QCString,long int)", params))
    {
         // Maybe it wasn't running (e.g. we're opening local html files)
         KToolInvocation::startServiceByDesktopName( "kcookiejar");
         if (!KApplication::dcopClient()->send("kcookiejar", "kcookiejar",
                                       "addCookies(QString,QCString,long int)", params))
             kdWarning(6010) << "Can't communicate with cookiejar!" << endl;
    }
}



HTMLElementImpl *HTMLDocumentImpl::body()
{
    NodeImpl *de = documentElement();
    if (!de)
        return 0;

    // try to prefer a FRAMESET element over BODY
    NodeImpl* body = 0;
    for (NodeImpl* i = de->firstChild(); i; i = i->nextSibling()) {
        if (i->id() == ID_FRAMESET)
            return static_cast<HTMLElementImpl*>(i);

        if (i->id() == ID_BODY)
            body = i;
    }
    return static_cast<HTMLElementImpl *>(body);
}

void HTMLDocumentImpl::setBody(HTMLElementImpl *_body, int& exceptioncode)
{
    HTMLElementImpl *b = body();
    if ( !_body ) {
        exceptioncode = DOMException::HIERARCHY_REQUEST_ERR;
        return;
    }
    if ( !b )
        documentElement()->appendChild( _body, exceptioncode );
    else
        documentElement()->replaceChild( _body, b, exceptioncode );
}

Tokenizer *HTMLDocumentImpl::createTokenizer()
{
    return new HTMLTokenizer(docPtr(),m_view);
}

// --------------------------------------------------------------------------
// not part of the DOM
// --------------------------------------------------------------------------

bool HTMLDocumentImpl::childAllowed( NodeImpl *newChild )
{
    // ### support comments. etc as a child
    return (newChild->id() == ID_HTML || newChild->id() == ID_COMMENT);
}

ElementImpl *HTMLDocumentImpl::createElement( const DOMString &name, int* pExceptioncode )
{
    ElementImpl *e = createHTMLElement(name);
    if ( e ) {
        e->setHTMLCompat( htmlMode() != XHtml );
        return e;
    }
    return DocumentImpl::createElement(name, pExceptioncode);
}

void HTMLDocumentImpl::slotHistoryChanged()
{
    if ( true || !m_render ) // disabled for now
        return;

    recalcStyle( Force );
    m_render->repaint();
}

HTMLMapElementImpl* HTMLDocumentImpl::getMap(const DOMString& _url)
{
    QString url = _url.string();
    QString s;
    int pos = url.find('#');
    //kdDebug(0) << "map pos of #:" << pos << endl;
    s = QString(_url.unicode() + pos + 1, _url.length() - pos - 1);

    QMap<QString,HTMLMapElementImpl*>::const_iterator it = mapMap.find(s);

    if (it != mapMap.end())
        return *it;
    else
        return 0;
}

void HTMLDocumentImpl::close()
{
    bool doload = !parsing() && m_tokenizer;

    DocumentImpl::close();

    if (doload) {

        if (title().isEmpty()) // ensure setTitle is called at least once
            setTitle( DOMString() );

        // auto fill: walk the tree and try to fill in login credentials
        if (view() && m_doAutoFill) {
            for (NodeImpl* n = this; n; n = n->traverseNextNode())
                if (n->id() == ID_FORM)
                    static_cast<HTMLFormElementImpl*>(n)->doAutoFill();
            m_doAutoFill = false;
        }

        // According to dom the load event must not bubble
        // but other browsers execute in a frameset document
        // the first(IE)/last(Moz/Konq) registered onload on a <frame> and the
        // first(IE)/last(Moz/Konq) registered onload on a <frameset>.

        //kdDebug() << "dispatching LOAD_EVENT on document " << getDocument() << " " << (view()?view()->part()->name():0) << endl;

        //Make sure to flush any pending image events now, as we want them out before the document's load event
        dispatchImageLoadEventsNow();
        getDocument()->dispatchWindowEvent(EventImpl::LOAD_EVENT, false, false);

        // don't update rendering if we're going to redirect anyway
        if ( view() && ( view()->part()->d->m_redirectURL.isNull() ||
                         view()->part()->d->m_delayRedirect > 1 ) )
            updateRendering();
    }
}


const int PARSEMODE_HAVE_DOCTYPE	=	(1<<0);
const int PARSEMODE_HAVE_PUBLIC_ID	=	(1<<1);
const int PARSEMODE_HAVE_SYSTEM_ID	=	(1<<2);
const int PARSEMODE_HAVE_INTERNAL	=	(1<<3);

static int parseDocTypePart(const QString& buffer, int index)
{
    while (true) {
        QChar ch = buffer[index];
        if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
            ++index;
        else if (ch == '-') {
            int tmpIndex=index;
            if (buffer[index+1] == '-' &&
                ((tmpIndex=buffer.find("--", index+2)) != -1))
                index = tmpIndex+2;
            else
                return index;
        }
        else
            return index;
    }
}

static bool containsString(const char* str, const QString& buffer, int offset)
{
    QString startString(str);
    if (offset + startString.length() > buffer.length())
        return false;

    QString bufferString = buffer.mid(offset, startString.length()).lower();
    QString lowerStart = startString.lower();

    return bufferString.startsWith(lowerStart);
}

static bool parseDocTypeDeclaration(const QString& buffer,
                                    int* resultFlags,
                                    QString& publicID,
                                    QString& systemID)
{
    bool haveDocType = false;
    *resultFlags = 0;

    // Skip through any comments and processing instructions.
    int index = 0;
    do {
        index = buffer.find('<', index);
        if (index == -1) break;
        QChar nextChar = buffer[index+1];
        if (nextChar == '!') {
            if (containsString("doctype", buffer, index+2)) {
                haveDocType = true;
                index += 9; // Skip "<!DOCTYPE"
                break;
            }
            index = parseDocTypePart(buffer,index);
            index = buffer.find('>', index);
        }
        else if (nextChar == '?')
            index = buffer.find('>', index);
        else
            break;
    } while (index != -1);

    if (!haveDocType)
        return true;
    *resultFlags |= PARSEMODE_HAVE_DOCTYPE;

    index = parseDocTypePart(buffer, index);
    if (!containsString("html", buffer, index))
        return false;

    index = parseDocTypePart(buffer, index+4);
    bool hasPublic = containsString("public", buffer, index);
    if (hasPublic) {
        index = parseDocTypePart(buffer, index+6);

        // We've read <!DOCTYPE HTML PUBLIC (not case sensitive).
        // Now we find the beginning and end of the public identifers
        // and system identifiers (assuming they're even present).
        QChar theChar = buffer[index];
        if (theChar != '\"' && theChar != '\'')
            return false;

        // |start| is the first character (after the quote) and |end|
        // is the final quote, so there are |end|-|start| characters.
        int publicIDStart = index+1;
        int publicIDEnd = buffer.find(theChar, publicIDStart);
        if (publicIDEnd == -1)
            return false;
        index = parseDocTypePart(buffer, publicIDEnd+1);
        QChar next = buffer[index];
        if (next == '>') {
            // Public identifier present, but no system identifier.
            // Do nothing.  Note that this is the most common
            // case.
        }
        else if (next == '\"' || next == '\'') {
            // We have a system identifier.
            *resultFlags |= PARSEMODE_HAVE_SYSTEM_ID;
            int systemIDStart = index+1;
            int systemIDEnd = buffer.find(next, systemIDStart);
            if (systemIDEnd == -1)
                return false;
            systemID = buffer.mid(systemIDStart, systemIDEnd - systemIDStart);
        }
        else if (next == '[') {
            // We found an internal subset.
            *resultFlags |= PARSEMODE_HAVE_INTERNAL;
        }
        else
            return false; // Something's wrong.

        // We need to trim whitespace off the public identifier.
        publicID = buffer.mid(publicIDStart, publicIDEnd - publicIDStart);
        publicID = publicID.stripWhiteSpace();
        *resultFlags |= PARSEMODE_HAVE_PUBLIC_ID;
    } else {
        if (containsString("system", buffer, index)) {
            // Doctype has a system ID but no public ID
            *resultFlags |= PARSEMODE_HAVE_SYSTEM_ID;
            index = parseDocTypePart(buffer, index+6);
            QChar next = buffer[index];
            if (next != '\"' && next != '\'')
                return false;
            int systemIDStart = index+1;
            int systemIDEnd = buffer.find(next, systemIDStart);
            if (systemIDEnd == -1)
                return false;
            systemID = buffer.mid(systemIDStart, systemIDEnd - systemIDStart);
            index = parseDocTypePart(buffer, systemIDEnd+1);
        }

        QChar nextChar = buffer[index];
        if (nextChar == '[')
            *resultFlags |= PARSEMODE_HAVE_INTERNAL;
        else if (nextChar != '>')
            return false;
    }

    return true;
}

void HTMLDocumentImpl::determineParseMode( const QString &str )
{
    //kdDebug() << "DocumentImpl::determineParseMode str=" << str<< endl;
    int oldPMode = pMode;

    // This code more or less mimics Mozilla's implementation (specifically the
    // doctype parsing implemented by David Baron in Mozilla's nsParser.cpp).
    //
    // There are three possible parse modes:
    // COMPAT - quirks mode emulates WinIE
    // and NS4.  CSS parsing is also relaxed in this mode, e.g., unit types can
    // be omitted from numbers.
    // ALMOST STRICT - This mode is identical to strict mode
    // except for its treatment of line-height in the inline box model.  For
    // now (until the inline box model is re-written), this mode is identical
    // to STANDARDS mode.
    // STRICT - no quirks apply.  Web pages will obey the specifications to
    // the letter.

    QString systemID, publicID;
    int resultFlags = 0;
    if (parseDocTypeDeclaration(str, &resultFlags, publicID, systemID)) {
        if (resultFlags & PARSEMODE_HAVE_DOCTYPE) {
            m_doctype->setName("HTML");
            m_doctype->setPublicId(publicID);
            m_doctype->setSystemId(systemID);
        }
        if (!(resultFlags & PARSEMODE_HAVE_DOCTYPE)) {
            // No doctype found at all.  Default to quirks mode and Html4.
            pMode = Compat;
            hMode = Html4;
        }
        else if ((resultFlags & PARSEMODE_HAVE_INTERNAL) ||
                 !(resultFlags & PARSEMODE_HAVE_PUBLIC_ID)) {
            // Internal subsets always denote full standards, as does
            // a doctype without a public ID.
            pMode = Strict;
            hMode = Html4;
        }
        else {
            // We have to check a list of public IDs to see what we
            // should do.
            QString lowerPubID = publicID.lower();
            const char* pubIDStr = lowerPubID.latin1();

            // Look up the entry in our gperf-generated table.
            const PubIDInfo* doctypeEntry = findDoctypeEntry(pubIDStr, publicID.length());
            if (!doctypeEntry) {
                // The DOCTYPE is not in the list.  Assume strict mode.
                pMode = Strict;
                hMode = Html4;
                return;
            }

            switch ((resultFlags & PARSEMODE_HAVE_SYSTEM_ID) ?
                    doctypeEntry->mode_if_sysid :
                    doctypeEntry->mode_if_no_sysid)
            {
                case PubIDInfo::eQuirks3:
                    pMode = Compat;
                    hMode = Html3;
                    break;
                case PubIDInfo::eQuirks:
                    pMode = Compat;
                    hMode = Html4;
                    break;
                case PubIDInfo::eAlmostStandards:
                    pMode = Transitional;
                    hMode = Html4;
                    break;
                 default:
                    assert(false);
            }
        }
    }
    else {
        // Malformed doctype implies quirks mode.
        pMode = Compat;
        hMode = Html3;
    }

    // This needs to be done last, see tests/parser/compatmode_xhtml_mixed.html
    if ( hMode == Html4 && !m_htmlRequested ) {
        // this part is still debatable and possibly UA dependent
        hMode = XHtml;
        pMode = Transitional;
    }

    m_styleSelector->strictParsing = !inCompatMode();

    // kdDebug() << "DocumentImpl::determineParseMode: publicId =" << publicID << " systemId = " << systemID << endl;
    // kdDebug() << "DocumentImpl::determineParseMode: htmlMode = " << hMode<< endl;
    if( pMode == Strict )
        kdDebug(6030) << " using strict parseMode" << endl;
    else if (pMode == Compat )
        kdDebug(6030) << " using compatibility parseMode" << endl;
    else
        kdDebug(6030) << " using transitional parseMode" << endl;

    // not sure this is needed
    if ( pMode != oldPMode && styleSelector() )
        recalcStyleSelector();

}


#include "html_documentimpl.moc"
