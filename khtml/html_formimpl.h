/**
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999 Lars Knoll (knoll@kde.org)
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
#ifndef HTML_FORMIMPL_H
#define HTML_FORMIMPL_H

// -------------------------------------------------------------------------
#include "dtd.h"
#include "html_elementimpl.h"
#include "khtmlio.h"
#include "khtml.h"

class KHTMLWidget;
class QWidget;
class QPixmap;
class QMultiLineEdit;
#include <qobject.h>

namespace DOM {

class HTMLFormElement;
class DOMString;

class HTMLFormElementImpl : public QObject, public HTMLBlockElementImpl
{
    Q_OBJECT

public:
    HTMLFormElementImpl(DocumentImpl *doc);
    virtual ~HTMLFormElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return FORMStartTag; }
    virtual tagStatus endTag() { return FORMEndTag; }

    long length() const;
    void submit (  );
    void reset (  );

    virtual void parseAttribute(Attribute *attr);

    virtual void attach(KHTMLWidget *w);
    virtual void detach();
 
    void radioClicked( NodeImpl *caller, DOMString ident );

public slots:
    void slotSubmit();
    void slotReset();

protected:
    DOMString url;
    DOMString target;
    bool post;
    KHTMLWidget *view;
};

// -------------------------------------------------------------------------

class HTMLGenericFormElementImpl : public HTMLPositionedElementImpl
{
    friend class HTMLFormElementImpl;

public:
    HTMLGenericFormElementImpl(DocumentImpl *doc);
    virtual ~HTMLGenericFormElementImpl();

    HTMLFormElementImpl *form() { 
	if(!_form) _form = getForm();
	return _form; 
    }

    virtual bool isRendered() { return true; } 
    
    /* 
     * override in derived classes to get the encoded name=value pair
     * for submitting
     */
    virtual QString encoding()
	{ return QString(); }

    virtual void parseAttribute(Attribute *attr);

    // they all add Widgets, and don't render any contents by themself
    virtual void print(QPainter *, int, int, int, int, int, int) {}

protected:
    HTMLFormElementImpl *getForm() const;
    QString encodeString( QString e );
    QString decodeString( QString e );

    DOMString _name;
    HTMLFormElementImpl *_form;
};

// -------------------------------------------------------------------------

class HTMLButtonElementImpl : public HTMLGenericFormElementImpl
{
public:
    HTMLButtonElementImpl(DocumentImpl *doc);

    virtual ~HTMLButtonElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return BUTTONStartTag; }
    virtual tagStatus endTag() { return BUTTONEndTag; }

    bool disabled() const;
    void setDisabled( const bool & );

    long tabIndex() const;
    void setTabIndex( const long & );

    DOMString type() const;
};

// -------------------------------------------------------------------------

class HTMLFieldSetElementImpl : public HTMLGenericFormElementImpl
{
public:
    HTMLFieldSetElementImpl(DocumentImpl *doc);

    virtual ~HTMLFieldSetElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return FIELDSETStartTag; }
    virtual tagStatus endTag() { return FIELDSETEndTag; }
};

// -------------------------------------------------------------------------

class HTMLInputElementImpl : public QObject,
			     public HTMLGenericFormElementImpl, 
			     public HTMLImageRequester
{
    Q_OBJECT

    friend class HTMLFormElementImpl;
public:
    enum typeEnum {
	TEXT,
	PASSWORD,
	CHECKBOX,
	RADIO,
	SUBMIT,
	RESET,
	FILE,
	HIDDEN,
	IMAGE,
	BUTTON
    };

    HTMLInputElementImpl(DocumentImpl *doc);
    virtual ~HTMLInputElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return INPUTStartTag; }
    virtual tagStatus endTag() { return INPUTEndTag; }

    bool defaultChecked() const;
    void setDefaultChecked( const bool & );

    bool checked() const { return _checked; }
    void setChecked(bool b);

    bool disabled() const { return _disabled; }
    void setDisabled( const bool & );

    long maxLength() const { return _maxLen; }
    void setMaxLength( const long & );

    bool readOnly() const;
    void setReadOnly( const bool & );

    long tabIndex() const;
    void setTabIndex( const long & );

    DOMString type() const;

    void blur();
    void focus();
    void select();
    void click();

    virtual void parseAttribute(Attribute *attr);

    virtual void attach(KHTMLWidget *w);
    virtual void detach(); 

    virtual void layout( bool deep = false );

    virtual void setXPos( int xPos );
    virtual void setYPos( int yPos );

    virtual void setPixmap( QPixmap *p );
    virtual void pixmapChanged( QPixmap *p );

    virtual QString encoding();
    virtual void calcMinMaxWidth();

public slots:
    void slotTextChanged( const QString & );
    void slotReturnPressed();
    void slotSubmit();
    void slotClicked();
    // ### add all clicked/pressed/etc... signals for HTML events...

protected:
    typeEnum _type;
    DOMString _value;
    QString currValue;
    bool _checked;
    bool _disabled;
    int _maxLen;
    int _size;
    DOMString _src;
    QPixmap *_pixmap;
    bool _clicked;

    KHTMLWidget *view;
    QWidget *w;
};

// -------------------------------------------------------------------------

class HTMLLabelElementImpl : public HTMLGenericFormElementImpl
{
public:
    HTMLLabelElementImpl(DocumentImpl *doc);
    virtual ~HTMLLabelElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return LABELStartTag; }
    virtual tagStatus endTag() { return LABELEndTag; }
};

// -------------------------------------------------------------------------

class HTMLLegendElementImpl : public HTMLGenericFormElementImpl
{
public:
    HTMLLegendElementImpl(DocumentImpl *doc);
    virtual ~HTMLLegendElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return LEGENDStartTag; }
    virtual tagStatus endTag() { return LEGENDEndTag; }
};

// -------------------------------------------------------------------------

class HTMLSelectElementImpl : public HTMLGenericFormElementImpl
{
public:
    HTMLSelectElementImpl(DocumentImpl *doc);
    virtual ~HTMLSelectElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return SELECTStartTag; }
    virtual tagStatus endTag() { return SELECTEndTag; }

    DOMString type() const;

    long selectedIndex() const;
    void setSelectedIndex( const long & );

    long length() const;

    bool disabled() const;
    void setDisabled( const bool & );

    bool multiple() const { return _multiple; }
    void setMultiple( const bool & );

    long size() const { return _size; }
    void setSize( const long & );

    long tabIndex() const;
    void setTabIndex( const long & );

    void add ( const HTMLElement &element, const HTMLElement &before );
    void remove ( const long &index );
    void blur (  );
    void focus (  );

    virtual void parseAttribute(Attribute *attr);

    virtual void attach(KHTMLWidget *w);
    virtual void detach(); 

    virtual void layout( bool deep = false );

    virtual void setXPos( int xPos );
    virtual void setYPos( int yPos );
    virtual void calcMinMaxWidth();

    virtual void close();

protected:
    QWidget *w;
    KHTMLWidget *view;

    int _size;
    bool _disabled;
    bool _multiple;
};

// -------------------------------------------------------------------------

class HTMLOptGroupElementImpl : public HTMLGenericFormElementImpl
{
public:
    HTMLOptGroupElementImpl(DocumentImpl *doc);
    virtual ~HTMLOptGroupElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return OPTGROUPStartTag; }
    virtual tagStatus endTag() { return OPTGROUPEndTag; }

    bool disabled() const;

    void setDisabled( const bool & );
};

// ---------------------------------------------------------------------------

class HTMLOptionElementImpl : public HTMLGenericFormElementImpl
{
public:
    HTMLOptionElementImpl(DocumentImpl *doc);
    virtual ~HTMLOptionElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return OPTIONStartTag; }
    virtual tagStatus endTag() { return OPTIONEndTag; }

    bool defaultSelected() const;
    void setDefaultSelected( const bool & );

    DOMString text() const;

    long index() const;
    void setIndex( const long & );

    bool disabled() const;
    void setDisabled( const bool & );

    bool selected() const;
};

// -------------------------------------------------------------------------

class HTMLTextAreaElementImpl : public HTMLGenericFormElementImpl
{
public:
    HTMLTextAreaElementImpl(DocumentImpl *doc);
    virtual ~HTMLTextAreaElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return TEXTAREAStartTag; }
    virtual tagStatus endTag() { return TEXTAREAEndTag; }

    long cols() const;
    void setCols( const long & );

    bool disabled() const;
    void setDisabled( const bool & );

    bool readOnly() const;
    void setReadOnly( const bool & );

    long rows() const;
    void setRows( const long & );

    long tabIndex() const;
    void setTabIndex( const long & );

    DOMString type() const;

    void blur (  );
    void focus (  );
    void select (  );

    virtual void parseAttribute(Attribute *attr);

    virtual void attach(KHTMLWidget *w);
    virtual void detach(); 

    virtual void layout( bool deep = false );

    virtual void setXPos( int xPos );
    virtual void setYPos( int yPos );
    virtual void calcMinMaxWidth();

protected:
    QMultiLineEdit *edit;
    KHTMLWidget *view;

    int _rows;
    int _cols;
    bool _disabled;
};

}; //namespace

#endif
