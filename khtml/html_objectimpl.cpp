

#include "html_objectimpl.h"
#include "dom_nodeimpl.h"
#include "java/kjavaappletwidget.h"
#include "dom_string.h"
#include "khtmlattrs.h"
#include "khtmltags.h"
#include "khtml.h"
#include <qstring.h>

#include <stdio.h>

using namespace DOM;

// -------------------------------------------------------------------------

HTMLAppletElementImpl::HTMLAppletElementImpl(DocumentImpl *doc)
  : HTMLPositionedElementImpl(doc)
{
    applet = 0;
    base = 0;
    code = 0;
    name = 0;
}

HTMLAppletElementImpl::~HTMLAppletElementImpl()
{
    if(base) base->deref();
    if(code) code->deref();
    if(name) name->deref();
}

const DOMString HTMLAppletElementImpl::nodeName() const
{
    return "APPLET";
}

ushort HTMLAppletElementImpl::id() const
{
    return ID_APPLET;
}

void HTMLAppletElementImpl::parseAttribute(Attribute *attr)
{
    switch( attr->id )
    {
    case ATTR_CODEBASE:
    	base = attr->val();
	base->ref();
	break;	
    case ATTR_ARCHIVE:
	break;
    case ATTR_CODE:
	code = attr->val();
	code->ref();
	break;	
    case ATTR_OBJECT:
	break;
    case ATTR_ALT:
	break;
    case ATTR_NAME:
	name = attr->val();
	name->ref();
	break;
    case ATTR_WIDTH:
	width = attr->val()->toInt();
	break;
    case ATTR_HEIGHT:
	ascent = attr->val()->toInt();
	break;
    default:
	HTMLPositionedElementImpl::parseAttribute(attr);
    }
}

void HTMLAppletElementImpl::layout( bool deep )
{

    if(!applet) return;

    applet->create();

    printf("initializing java widget II\n");
    NodeImpl *child = firstChild();
    while(child)
    {
	if(child->id() == ID_PARAM)
	{
	    HTMLParamElementImpl *p = static_cast<HTMLParamElementImpl *>(child);
	    QString n = QConstString(p->name->s, p->name->l).string();
	    QString v = QConstString(p->value->s, p->value->l).string();
	    applet->setParameter(n, v);
	}
	child = child->nextSibling();
    }

    applet->show();
}

void HTMLAppletElementImpl::attach(KHTMLWidget *_view)
{
    printf("initializing java widget I\n");
    view = _view;
    applet = new KJavaAppletWidget(view->viewport());

    applet->resize(width, getHeight());
    QString tmp = QConstString(base->s, base->l).string();
    applet->setBaseURL(tmp);
    tmp = QConstString(code->s, code->l).string();
    applet->setAppletClass(tmp);
    tmp = QConstString(name->s, name->l).string();
    applet->setAppletName(tmp);
}

void HTMLAppletElementImpl::detach()
{
    view = 0;
}

void HTMLAppletElementImpl::setXPos( int xPos )
{
    printf("setXPos I\n");
    x = xPos;
    int absX, absY;
    getAbsolutePosition(absX, absY);
    absY -= ascent;
    if(view && applet)
    {
	view->addChild(applet, absX, absY);
    }
}

void HTMLAppletElementImpl::setYPos( int yPos )
{
    printf("setYPos\n");
    y = yPos;
    int absX, absY;
    getAbsolutePosition(absX, absY);
    absY -= ascent;
    if(view && applet)
    {
             view->addChild(applet, absX, absY);
    }
}

// -------------------------------------------------------------------------

HTMLObjectElementImpl::HTMLObjectElementImpl(DocumentImpl *doc) : HTMLElementImpl(doc)
{
}

HTMLObjectElementImpl::~HTMLObjectElementImpl()
{
}

const DOMString HTMLObjectElementImpl::nodeName() const
{
    return "OBJECT";
}

ushort HTMLObjectElementImpl::id() const
{
    return ID_OBJECT;
}

HTMLFormElement HTMLObjectElementImpl::form() const
{
}

bool HTMLObjectElementImpl::declare() const
{
}

void HTMLObjectElementImpl::setDeclare( const bool & )
{
}

long HTMLObjectElementImpl::tabIndex() const
{
}

void HTMLObjectElementImpl::setTabIndex( const long & )
{
}

// -------------------------------------------------------------------------

HTMLParamElementImpl::HTMLParamElementImpl(DocumentImpl *doc) : HTMLElementImpl(doc)
{
    name = 0;
    value = 0;
}

HTMLParamElementImpl::~HTMLParamElementImpl()
{
    if(name) delete name;
    if(value) delete value;
}

const DOMString HTMLParamElementImpl::nodeName() const
{
    return "PARAM";
}

ushort HTMLParamElementImpl::id() const
{
    return ID_PARAM;
}

void HTMLParamElementImpl::parseAttribute(Attribute *attr)
{
    switch( attr->id )
    {
    case ATTR_NAME:
	name = attr->val();
	name->ref();
	break;
    case ATTR_VALUE:
	value = attr->val();
	value->ref();
	break;
    }
}
