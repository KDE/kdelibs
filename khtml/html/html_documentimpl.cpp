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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
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

#include "css/cssproperties.h"
#include "css/cssstyleselector.h"
#include "css/css_stylesheetimpl.h"
#include <stdlib.h>
#include <qptrstack.h>

template class QPtrStack<DOM::NodeImpl>;

using namespace DOM;
using namespace khtml;


HTMLDocumentImpl::HTMLDocumentImpl(DOMImplementationImpl *_implementation, KHTMLView *v)
  : DocumentImpl(_implementation, v)
{
//    kdDebug( 6090 ) << "HTMLDocumentImpl constructor this = " << this << endl;
    htmlElement = 0;

    m_doAutoFill = false;
    m_htmlRequested = false;

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

    QCString replyType;
    QByteArray params, reply;
    QDataStream stream(params, IO_WriteOnly);
    stream << URL().url() << windowId;
    if (!kapp->dcopClient()->call("kcookiejar", "kcookiejar",
                                  "findDOMCookies(QString,long int)", params,
                                  replyType, reply))
    {
       kdWarning(6010) << "Can't communicate with cookiejar!" << endl;
       return DOMString();
    }

    QDataStream stream2(reply, IO_ReadOnly);
    if(replyType != "QString") {
         kdError(6010) << "DCOP function findDOMCookies(...) returns "
                       << replyType << ", expected QString" << endl;
         return DOMString();
    }

    QString result;
    stream2 >> result;
    return DOMString(result);
}

void HTMLDocumentImpl::setCookie( const DOMString & value )
{
    long windowId = 0;
    KHTMLView *v = view ();

    if ( v && v->topLevelWidget() )
      windowId = v->topLevelWidget()->winId();

    QByteArray params;
    QDataStream stream(params, IO_WriteOnly);
    QCString fake_header("Set-Cookie: ");
    fake_header.append(value.string().latin1());
    fake_header.append("\n");
    stream << URL().url() << fake_header << windowId;
    if (!kapp->dcopClient()->send("kcookiejar", "kcookiejar",
                                  "addCookies(QString,QCString,long int)", params))
    {
         // Maybe it wasn't running (e.g. we're opening local html files)
         KApplication::startServiceByDesktopName( "kcookiejar");
         if (!kapp->dcopClient()->send("kcookiejar", "kcookiejar",
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

    QMapConstIterator<QString,HTMLMapElementImpl*> it = mapMap.find(s);

    if (it != mapMap.end())
        return *it;
    else
        return 0;
}

static bool isTransitional(const QString &spec, int start)
{
    if((spec.find("TRANSITIONAL", start, false ) != -1 ) ||
       (spec.find("LOOSE", start, false ) != -1 ) ||
       (spec.find("FRAMESET", start, false ) != -1 ) ||
       (spec.find("LATIN1", start, false ) != -1 ) ||
       (spec.find("SYMBOLS", start, false ) != -1 ) ||
       (spec.find("SPECIAL", start, false ) != -1 ) ) {
        return true;
    }
    return false;
}

void HTMLDocumentImpl::close()
{
    bool doload = !parsing() && m_tokenizer;

    DocumentImpl::close();

    HTMLElementImpl* b = body();
    if (b && doload) {

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
        getDocument()->dispatchWindowEvent(EventImpl::LOAD_EVENT, false, false);

        // don't update rendering if we're going to redirect anyway
        if ( view() && ( view()->part()->d->m_redirectURL.isNull() ||
                         view()->part()->d->m_delayRedirect > 1 ) )
            updateRendering();
    }
}


void HTMLDocumentImpl::determineParseMode( const QString &str )
{
    //kdDebug() << "DocumentImpl::determineParseMode str=" << str<< endl;
    // determines the parse mode for HTML
    // quite some hints here are inspired by the mozilla code.
    int oldPMode = pMode;

    // default parsing mode is Loose
    pMode = Compat;
    hMode = Html3;

    ParseMode systemId = Unknown;
    ParseMode publicId = Unknown;

    int pos = 0;
    int doctype = str.find("!doctype", 0, false);
    if( doctype > 2 ) {
        pos = doctype - 2;
        // Store doctype name
        int start = doctype + 9;
        while ( start < (int)str.length() && str[start].isSpace() )
            start++;
        int espace = str.find(' ',start);
        QString name = str.mid(start,espace-start);
        //kdDebug() << "DocumentImpl::determineParseMode setName: " << name << endl;
        m_doctype->setName( name );
    }

    // get the first tag (or the doctype tag)
    int start = str.find('<', pos);
    int stop = str.find('>', pos);
    if( start > -1 && stop > start ) {
        QString spec = str.mid( start + 1, stop - start - 1 );
        //kdDebug() << "DocumentImpl::determineParseMode dtd=" << spec<< endl;
        start = 0;
        int quote = -1;
        if( doctype != -1 ) {
            while( (quote = spec.find( "\"", start )) != -1 ) {
                int quote2 = spec.find( "\"", quote+1 );
                if(quote2 < 0) quote2 = spec.length();
                QString val = spec.mid( quote+1, quote2 - quote-1 );
                //kdDebug() << "DocumentImpl::determineParseMode val = " << val << endl;
                // find system id
                pos = val.find("http://www.w3.org/tr/", 0, false);
                if ( pos != -1 ) {
                    // loose or strict dtd?
                    if ( val.find("strict.dtd", pos, false) != -1 )
                        systemId = Strict;
                    else if (isTransitional(val, pos))
                        systemId = Transitional;
                }

                // find public id
                pos = val.find("//dtd", 0, false );
                if ( pos != -1 ) {
                    if( val.find( "xhtml", pos+6, false ) != -1 ) {
                        hMode = XHtml;
                        publicId = isTransitional(val, pos) ? Transitional : Strict;
                    } else if ( val.find( "15445:1999", pos+6 ) != -1 ) {
                        hMode = Html4;
                        publicId = Strict;
                    } else {
                        int tagPos = val.find( "html", pos+6, false );
                        if( tagPos == -1 )
                            tagPos = val.find( "hypertext markup", pos+6, false );
                        if ( tagPos != -1 ) {
                            tagPos = val.find(QRegExp("[0-9]"), tagPos );
                            int version = val.mid( tagPos, 1 ).toInt();
                            if( version > 3 ) {
                                hMode = Html4;
                                publicId = isTransitional( val, tagPos ) ? Transitional : Strict;
                            }
                        }
                    }
                }
                start = quote2 + 1;
            }
        }

        if( systemId && systemId == publicId ) { // not unknown and both agree
            pMode = publicId;
        }
        else if ( systemId == Unknown )
            pMode = hMode == Html3 ? Compat : publicId;
        else if ( ( publicId == Transitional && systemId == Strict ) ||
                  ( publicId == Strict && systemId == Transitional ) ) {
            pMode = hMode == Html3 ? Compat : Transitional;
        } else {
            pMode = Compat;
        }

        if ( hMode == XHtml )
            pMode = publicId;

        // This needs to be done last, see tests/parser/compatmode_xhtml_mixed.html
        if ( m_htmlRequested && hMode == XHtml )
            hMode = Html4; // make all tags uppercase when served as text/html (#86446)
    }
    // kdDebug() << "DocumentImpl::determineParseMode: publicId =" << publicId << " systemId = " << systemId << endl;
    // kdDebug() << "DocumentImpl::determineParseMode: htmlMode = " << hMode<< endl;
    if( pMode == Strict )
        kdDebug(6020) << " using strict parseMode" << endl;
    else if (pMode == Compat )
        kdDebug(6020) << " using compatibility parseMode" << endl;
    else
        kdDebug(6020) << " using transitional parseMode" << endl;

    if ( pMode != oldPMode && styleSelector() )
        recalcStyleSelector();
}

#include "html_documentimpl.moc"
