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
 * $Id$
 */
#include "dom_docimpl.h"

#include "dom_node.h"
#include "dom_elementimpl.h"
#include "dom_textimpl.h"
#include "dom_exception.h"
#include "dom2_rangeimpl.h"
#include "dom2_traversalimpl.h"

#include "css/cssstyleselector.h"
#include "css/css_stylesheetimpl.h"

#include <qstring.h>
#include <qstack.h>
#include "misc/htmlhashes.h"
#include "misc/loader.h"
#include <kdebug.h>

#include "html_baseimpl.h"
#include "html_blockimpl.h"
#include "html_documentimpl.h"
#include "html_elementimpl.h"
#include "html_formimpl.h"
#include "html_headimpl.h"
#include "html_imageimpl.h"
#include "html_inlineimpl.h"
#include "html_listimpl.h"
#include "html_miscimpl.h"
#include "html_tableimpl.h"
#include "html_objectimpl.h"
#include "htmltokenizer.h"
#include "xml_tokenizer.h"

#include "rendering/render_object.h"
#include "rendering/render_root.h"
#include "rendering/render_style.h"

#include "khtmlview.h"
#include "khtml_part.h"

#include <kglobal.h>
#include <kcharsets.h>
#include <kglobalsettings.h>
#include "khtml_settings.h"
#include <iostream.h>

using namespace DOM;
using namespace khtml;

//template class QStack<DOM::NodeImpl>; // needed ?


DOMImplementationImpl::DOMImplementationImpl()
{
}

DOMImplementationImpl::~DOMImplementationImpl()
{
}

bool DOMImplementationImpl::hasFeature ( const DOMString &feature, const DOMString &version )
{
    // no valid implementation at the moment... ;-)
    if(feature == "HTML" && version == "1") return true;

    return false;
}

// ------------------------------------------------------------------------

DocumentImpl::DocumentImpl() : NodeBaseImpl(0)
{
    m_styleSelector = 0;
    m_view = 0;
    m_style = 0;
    m_docLoader = new DocLoader();
    visuallyOrdered = false;
    m_loadingSheet = false;
    m_sheet = 0;
    m_elemSheet = 0;
    tokenizer = 0;
}

DocumentImpl::DocumentImpl(KHTMLView *v) : NodeBaseImpl(0)
{
    m_styleSelector = 0;
    m_view = v;
    m_style = 0;
    m_docLoader = new DocLoader();
    visuallyOrdered = false;
    m_loadingSheet = false;
    m_sheet = 0;
    m_elemSheet = 0;
    tokenizer = 0;
}

DocumentImpl::~DocumentImpl()
{
    delete m_sheet;
    delete m_styleSelector;
    delete m_style;
    delete m_docLoader;
    if (m_elemSheet )
	m_elemSheet->deref();
    if (tokenizer)
	delete tokenizer;
}

const DOMString DocumentImpl::nodeName() const
{
    return "#document";
}

unsigned short DocumentImpl::nodeType() const
{
    return Node::DOCUMENT_NODE;
}

ElementImpl *DocumentImpl::documentElement() const
{
    if(!firstChild() || firstChild()->nodeType() != Node::ELEMENT_NODE)
       return 0;
    return (ElementImpl *)firstChild();
}

ElementImpl *DocumentImpl::createElement( const DOMString &name )
{
    uint id = khtml::getTagID( name.string().lower().latin1(), name.string().length() );

    // ### preserve case-sensitivity of xml element names
    if (!id) {
	// ### only allow for non-HTML documents ???
        return new XMLElementImpl(this,name.implementation());
    }

    ElementImpl *n = 0;
    switch(id)
    {
    case ID_HTML:
	n = new HTMLHtmlElementImpl(this);
	break;
    case ID_HEAD:
	n = new HTMLHeadElementImpl(this);
	break;
    case ID_BODY:
	n = new HTMLBodyElementImpl(this);
	break;

// head elements
    case ID_BASE:
	n = new HTMLBaseElementImpl(this);
	break;
    case ID_LINK:
	n = new HTMLLinkElementImpl(this);
	break;
    case ID_META:
	n = new HTMLMetaElementImpl(this);
	break;
    case ID_STYLE:
	n = new HTMLStyleElementImpl(this);
	break;
    case ID_TITLE:
	n = new HTMLTitleElementImpl(this);
	break;

// frames
    case ID_FRAME:
	n = new HTMLFrameElementImpl(this);
	break;
    case ID_FRAMESET:
	n = new HTMLFrameSetElementImpl(this);
	break;
    case ID_IFRAME:
	n = new HTMLIFrameElementImpl(this);
	break;

// form elements
// ### FIXME: we need a way to set form dependency after we have made the form elements
    case ID_FORM:
	    n = new HTMLFormElementImpl(this);
	break;
    case ID_BUTTON:
            n = new HTMLButtonElementImpl(this);
	break;
    case ID_FIELDSET:
            n = new HTMLFieldSetElementImpl(this);
	break;
    case ID_INPUT:
            n = new HTMLInputElementImpl(this);
	break;
    case ID_ISINDEX:
	    n = new HTMLIsIndexElementImpl(this);
	break;
    case ID_LABEL:
            n = new HTMLLabelElementImpl(this);
	break;
    case ID_LEGEND:
            n = new HTMLLegendElementImpl(this);
	break;
    case ID_OPTGROUP:
            n = new HTMLOptGroupElementImpl(this);
	break;
    case ID_OPTION:
            n = new HTMLOptionElementImpl(this);
	break;
    case ID_SELECT:
            n = new HTMLSelectElementImpl(this);
	break;
    case ID_TEXTAREA:
            n = new HTMLTextAreaElementImpl(this);
	break;

// lists
    case ID_DL:
	n = new HTMLDListElementImpl(this);
	break;
    case ID_DD:
	n = new HTMLGenericElementImpl(this, id);
	break;
    case ID_DT:
	n = new HTMLGenericElementImpl(this, id);
	break;
    case ID_UL:
	n = new HTMLUListElementImpl(this);
	break;
    case ID_OL:
	n = new HTMLOListElementImpl(this);
	break;
    case ID_DIR:
	n = new HTMLDirectoryElementImpl(this);
	break;
    case ID_MENU:
	n = new HTMLMenuElementImpl(this);
	break;
    case ID_LI:
	n = new HTMLLIElementImpl(this);
	break;

// formatting elements (block)
    case ID_BLOCKQUOTE:
	n = new HTMLBlockquoteElementImpl(this);
	break;
    case ID_DIV:
	n = new HTMLDivElementImpl(this);
	break;
    case ID_H1:
    case ID_H2:
    case ID_H3:
    case ID_H4:
    case ID_H5:
    case ID_H6:
	n = new HTMLHeadingElementImpl(this, id);
	break;
    case ID_HR:
	n = new HTMLHRElementImpl(this);
	break;
    case ID_P:
	n = new HTMLParagraphElementImpl(this);
	break;
    case ID_PRE:
	n = new HTMLPreElementImpl(this);
	break;

// font stuff
    case ID_BASEFONT:
	n = new HTMLBaseFontElementImpl(this);
	break;
    case ID_FONT:
	n = new HTMLFontElementImpl(this);
	break;

// ins/del
    case ID_DEL:
    case ID_INS:
	n = new HTMLModElementImpl(this, id);
	break;

// anchor
    case ID_A:
	n = new HTMLAnchorElementImpl(this);
	break;

// images
    case ID_IMG:
	n = new HTMLImageElementImpl(this);
	break;
    case ID_MAP:
	n = new HTMLMapElementImpl(this);
	/*n = map;*/
	break;
    case ID_AREA:
	n = new HTMLAreaElementImpl(this);
	break;

// objects, applets and scripts
    case ID_APPLET:
	n = new HTMLAppletElementImpl(this);
	break;
    case ID_OBJECT:
	n = new HTMLObjectElementImpl(this);
	break;
    case ID_PARAM:
	n = new HTMLParamElementImpl(this);
	break;
    case ID_SCRIPT:
	n = new HTMLScriptElementImpl(this);
	break;

// tables
    case ID_TABLE:
	n = new HTMLTableElementImpl(this);
	break;
    case ID_CAPTION:
	n = new HTMLTableCaptionElementImpl(this);
	break;
    case ID_COLGROUP:
    case ID_COL:
	n = new HTMLTableColElementImpl(this, id);
	break;
    case ID_TR:
	n = new HTMLTableRowElementImpl(this);
	break;
    case ID_TD:
    case ID_TH:
	n = new HTMLTableCellElementImpl(this, id);
	break;
    case ID_THEAD:
    case ID_TBODY:
    case ID_TFOOT:
	n = new HTMLTableSectionElementImpl(this, id);
	break;

// inline elements
    case ID_BR:
	n = new HTMLBRElementImpl(this);
	break;
    case ID_Q:
	n = new HTMLQuoteElementImpl(this);
	break;

// elements with no special representation in the DOM

// block:
    case ID_ADDRESS:
    case ID_CENTER:
	n = new HTMLGenericElementImpl(this, id);
	break;
// inline
	// %fontstyle
    case ID_TT:
    case ID_U:
    case ID_B:
    case ID_I:
    case ID_S:
    case ID_STRIKE:
    case ID_BIG:
    case ID_SMALL:

	// %phrase
    case ID_EM:
    case ID_STRONG:
    case ID_DFN:
    case ID_CODE:
    case ID_SAMP:
    case ID_KBD:
    case ID_VAR:
    case ID_CITE:
    case ID_ABBR:
    case ID_ACRONYM:

	// %special
    case ID_SUB:
    case ID_SUP:
    case ID_SPAN:
	n = new HTMLGenericElementImpl(this, id);
	break;

    case ID_BDO:
	break;

// text
    case ID_TEXT:
        kdDebug( 6020 ) << "Use document->createTextNode()" << endl;
	break;

    default:
	kdDebug( 6020 ) << "Unknown tag " << id << "!" << endl;
    }
    return n;
}

StyleSheetListImpl *DocumentImpl::styleSheets()
{
    // ### implement for xml
    return 0;
}


void DocumentImpl::createSelector()
{
    applyChanges();
}

// Used to maintain list of all forms in document
QString DocumentImpl::registerElement(ElementImpl *e)
{
   m_registeredElements.append(e);
   QString state;
   if (!m_state.isEmpty())
   {
      state = m_state.first();
      m_state.remove(m_state.begin());
   }
   return state;
}

// Used to maintain list of all forms in document
void DocumentImpl::removeElement(ElementImpl *e)
{
   m_registeredElements.removeRef(e);
}

QStringList DocumentImpl::state()
{
   QStringList s;
   for( ElementImpl *e = m_registeredElements.first();
        e; e = m_registeredElements.next())
   {
       s.append(e->state());
   }
   return s;
}


RangeImpl *DocumentImpl::createRange()
{
    return new RangeImpl(this);
}

NodeIteratorImpl *DocumentImpl::createNodeIterator(NodeImpl *, unsigned long /*whatToShow*/,
						   NodeFilterImpl */*filter*/, bool /*entityReferenceExpansion*/)
{
 // ###
    return new NodeIteratorImpl;
//    return 0;
}

/*TreeWalker DocumentImpl::createTreeWalker(Node root, unsigned long whatToShow, NodeFilter filter,
                                bool entityReferenceExpansion)
{
    return 0; // ###
}*/

void DocumentImpl::applyChanges(bool,bool force)
{
    if(m_styleSelector) delete m_styleSelector;
    m_styleSelector = new CSSStyleSelector(this);
    if(!m_render) return;

    recalcStyle();

    // a style change can influence the children, so we just go
    // through them and trigger an appplyChanges there too
    NodeImpl *n = _first;
    while(n) {
        n->applyChanges(false,force || changed());
        n = n->nextSibling();
    }

    // force a relayout of this part of the document
    m_render->layout();
    // force a repaint of this part.
    // ### if updateSize() changes any size, it will already force a
    // repaint, so we might do double work here...
    m_render->repaint();
    setChanged(false);
}

void DocumentImpl::setChanged(bool b)
{
    if (b)
	changedNodes.append(this);
    NodeBaseImpl::setChanged(b);
}

void DocumentImpl::recalcStyle()
{
    QTime qt;
    qt.start();
    if( !m_render ) return;
    if( m_style ) delete m_style;
    m_style = new RenderStyle();
    m_style->setDisplay(BLOCK);
    m_style->setVisuallyOrdered( visuallyOrdered );
    // ### make the font stuff _really_ work!!!!
    const KHTMLSettings *settings = m_view->part()->settings();
    QValueList<int> fs = settings->fontSizes();
    int size = fs[3];
    if(size < settings->minFontSize())
        size = settings->minFontSize();
    QFont f = KGlobalSettings::generalFont();
    f.setFamily(settings->stdFontName());
    f.setPointSize(size);
    //kdDebug() << "HTMLDocumentImpl::attach: setting to charset " << settings->charset() << endl;
    KGlobal::charsets()->setQFont(f, settings->charset());
    m_style->setFont(f);

    m_style->setHtmlHacks(true); // enable html specific rendering tricks
    if(m_render)
	m_render->setStyle(m_style);

    NodeImpl *n;
    for (n = _first; n; n = n->nextSibling())
	n->recalcStyle();
    kdDebug( ) << "TIME: recalcStyle() dt=" << qt.elapsed() << endl;
}



// ------------------------------------------------------------------------

DocumentFragmentImpl *DocumentImpl::createDocumentFragment(  )
{
    return new DocumentFragmentImpl(this);
}

TextImpl *DocumentImpl::createTextNode( const DOMString &data )
{
    return new TextImpl(this, data);
}

CommentImpl *DocumentImpl::createComment ( const DOMString &data )
{
    return new CommentImpl(this,data);
}

AttrImpl *DocumentImpl::createAttribute( const DOMString &name )
{
    return new AttrImpl(this, name);
}

NodeListImpl *DocumentImpl::getElementsByTagName( const DOMString &tagname )
{
    return new TagNodeListImpl( this, tagname );
}

void DocumentImpl::updateRendering()
{
    QListIterator<NodeImpl> it(changedNodes);
    for (; it.current(); ++it) {
	if( it.current()->changed() )
	    it.current()->applyChanges( true, true );
    }
     changedNodes.clear();
}

void DocumentImpl::setReloading()
{
    m_docLoader->reloading = true;
}

void DocumentImpl::attach(KHTMLView *w)
{
    m_view = w;
    if(!m_styleSelector) createSelector();
    m_render = new RenderRoot(w);
    recalcStyle();

    NodeBaseImpl::attach(w);
}

void DocumentImpl::slotFinishedParsing()
{
    emit finishedParsing();
}

void DocumentImpl::setVisuallyOrdered()
{
    visuallyOrdered = true;
    if(!m_style) return;
    m_style->setVisuallyOrdered(true);
}

void DocumentImpl::setSelection(NodeImpl* s, int sp, NodeImpl* e, int ep)
{
    static_cast<RenderRoot*>(m_render)
        ->setSelection(s->renderer(),sp,e->renderer(),ep);
}

void DocumentImpl::clearSelection()
{
    static_cast<RenderRoot*>(m_render)
        ->clearSelection();
}

Tokenizer *DocumentImpl::createTokenizer()
{
    return new XMLTokenizer(this,m_view);
}

void DocumentImpl::open(  )
{
    clear();
    tokenizer = createTokenizer();
    connect(tokenizer,SIGNAL(finishedParsing()),this,SLOT(slotFinishedParsing()));
    tokenizer->begin();
}

void DocumentImpl::close(  )
{
    if (m_render)
        m_render->close();

    if(tokenizer) delete tokenizer;
    tokenizer = 0;
}

void DocumentImpl::write( const DOMString &text )
{
    if(tokenizer)
        tokenizer->write(text.string());
}

void DocumentImpl::write( const QString &text )
{
    if(tokenizer)
        tokenizer->write(text);
}

void DocumentImpl::writeln( const DOMString &text )
{
    write(text);
    write(DOMString("\n"));
}

void DocumentImpl::finishParsing (  )
{
    if(tokenizer)
        tokenizer->finish();
}

void DocumentImpl::clear()
{
    if(tokenizer) delete tokenizer;
    tokenizer = 0;

    // #### clear tree
}

ElementImpl *DocumentImpl::getElementById( const DOMString &elementId )
{
    QStack<NodeImpl> nodeStack;
    NodeImpl *current = _first;

    while(1)
    {
        if(!current)
        {
            if(nodeStack.isEmpty()) break;
            current = nodeStack.pop();
            current = current->nextSibling();
        }
        else
        {
            if(current->isElementNode())
            {
                ElementImpl *e = static_cast<ElementImpl *>(current);
                if(e->getAttribute(ATTR_ID) == elementId)
                    return e;
            }

            NodeImpl *child = current->firstChild();
            if(child)
            {
                nodeStack.push(current);
                current = child;
            }
            else
            {
                current = current->nextSibling();
            }
        }
    }

    return 0;
}

DOMString DocumentImpl::baseURL() const
{
    if(!view()->part()->baseURL().isEmpty()) return view()->part()->baseURL().url();
    return url;
}

void DocumentImpl::setStyleSheet(const DOM::DOMString &url, const DOM::DOMString &sheet)
{
    kdDebug( 6030 ) << "HTMLDocument::setStyleSheet()" << endl;
    m_sheet = new CSSStyleSheetImpl(this, url);
    m_sheet->ref();
    m_sheet->parseString(sheet);
    m_loadingSheet = false;

    createSelector();
}

CSSStyleSheetImpl* DocumentImpl::elementSheet()
{
    if (!m_elemSheet) {
        m_elemSheet = new CSSStyleSheetImpl(this, baseURL());
	m_elemSheet->ref();
    }
    return m_elemSheet;
}

// ----------------------------------------------------------------------------

DocumentFragmentImpl::DocumentFragmentImpl(DocumentImpl *doc) : NodeBaseImpl(doc)
{
}

DocumentFragmentImpl::DocumentFragmentImpl(const DocumentFragmentImpl &other)
    : NodeBaseImpl(other)
{
}

const DOMString DocumentFragmentImpl::nodeName() const
{
  return "#document-fragment";
}

unsigned short DocumentFragmentImpl::nodeType() const
{
    return Node::DOCUMENT_FRAGMENT_NODE;
}

bool DocumentFragmentImpl::childAllowed( NodeImpl */*newChild*/ )
{
    return true;
}



