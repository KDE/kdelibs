/*
    This file is part of the KDE libraries

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
//----------------------------------------------------------------------------
//
// KDE HTML Widget -- Forms
// $Id$

#ifndef __HTMLFORM_H__
#define __HTMLFORM_H__

#include <qwidget.h>
#include <qmultilinedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>

#include "khtmlobj.h"
#include "khtmlfont.h"

//
// External Classes
//
///////////////////

class JSEventHandler;
class QStringList;

//
// Internal Classes
//
///////////////////

class HTMLElement;
class HTMLForm;
class HTMLSelect;
class HTMLButton;
class HTMLTextArea;

//---------------------------------------------------------------------------

class HTMLElement
{
public:
    HTMLElement( HTMLString n )
	{ _name = n; form = 0; }
    virtual ~HTMLElement();
    
    HTMLString elementName() const
	{  return _name; }
    void setElementName( HTMLString n )
	{ _name = n; }
    
    void setForm( HTMLForm *f )
	{ form = f; }
    
    // This function places the element on the page using the
    // absolute coordinates.  Also responsible for showing/hiding
    // non-visible elements
    virtual void position( int /*_x*/, int /*_y*/,
			   int /*_width*/, int /*_height*/ ) {}

    virtual QString encoding()
	{  return QString( "" ); }

    virtual void calcAbsolutePos( int , int ) {}
    
    virtual void resetElement(QString = 0) { }

protected:
    // encode special characters
    QString encodeString( HTMLString e );
    QString decodeString( HTMLString e );

protected:
    QString _encoding;

private:
    HTMLString _name;
    
    HTMLForm *form;
};

class HTMLWidgetElement : public QObject, public HTMLObject, public HTMLElement
{
    Q_OBJECT
protected:
    const HTMLFont *font;
public:
    HTMLWidgetElement( HTMLString n, const HTMLFont *f = 0 ); 
    virtual ~HTMLWidgetElement();
    
    int absX() const
	{ return _absX; }
    int absY() const
	{ return _absY; }
    
    // This function places the element on the page using the
    // absolute coordinates.  Also responsible for showing/hiding
    // non-visible elements
    virtual void position( int _x, int _y, int _width, int _height );
    
    virtual void calcAbsolutePos( int _x, int _y );
    // prints the object
    virtual bool print( QPainter *_painter, int _x, int _y, int _width,
			int _height, int _tx, int _ty, bool toPrinter );
    virtual void print( QPainter *, int, int );

    // recursive function the widget itself and all child widgets
    virtual void paintWidget( QWidget *widget );

    virtual HTMLObject *mouseEvent( int _x, int _y, int button, int state );

    virtual const char * objectName() const { return "HTMLWidgetElement"; };

    void setWidget( QWidget *_w );
    QWidget *widget() { return w; }

    bool isInPixmapMode() { return !showAsWidget; }
    void setPixmapMode( bool flag ) 
	{ showAsWidget = !flag; if (!showAsWidget) w->hide(); }

    QPixmap *pixmap() { return p; }    

private:
    QWidget *w;
    // holds the widgets information...
    QPixmap *p;

    // absolute position of this element in the page
    int _absX;
    int _absY;
    
    // relative position of this element in the page
    int _relX;
    int _relY;
	
    // true if widget is displayed "as is", false if widget is painted
    // into a qpixmap
    bool showAsWidget;
	
    static HTMLWidgetElement *currentFormFocusWidget;
};

//---------------------------------------------------------------------------

class HTMLSelect : public HTMLWidgetElement
{
    Q_OBJECT
public:
    HTMLSelect( QWidget *parent, HTMLString n, int s, bool m,
		const HTMLFont *f );
    virtual ~HTMLSelect() { }
    
    void addOption( HTMLString o, bool sel );
    
    // set text for current option
    void setText( QString text );
    
    const QString &value()
	{ return *_values.at( _item ); }
    const QString &value( int item );
    void setValue( QString v, int item );
    
    virtual QString encoding();
    virtual void resetElement(QString data = 0);
    
    virtual const char * objectName() const { return "HTMLSelect"; };

protected slots:
    void slotHighlighted( int indx );

private:
    int _defSelected;
    int _size;
    QList<QString> _values;
    int _item;
};

//---------------------------------------------------------------------------

class HTMLTextArea : public HTMLWidgetElement
{
    Q_OBJECT
public:
    HTMLTextArea( QWidget *parent, HTMLString n, int r, int c,
		  const HTMLFont *f = 0 );
    virtual ~HTMLTextArea() { }
    
    QString value();
    void setText( QString t );
    
    virtual QString encoding();
    virtual void resetElement(QString data = 0);
    
    virtual const char * objectName() const { return "HTMLTextArea"; };

private:
    QString _defText;
};

//---------------------------------------------------------------------------

class HTMLInput : public HTMLWidgetElement
{
    Q_OBJECT
public:
    HTMLInput( HTMLString n, HTMLString v, const HTMLFont *f = 0 );
    virtual ~HTMLInput() { }

    const QString &value() const
	{ return _value; }
    void setValue( QString v )
	{ _value = v; }

    virtual const char * objectName() const { return "HTMLInput"; };

private:
    QString _value;
};

//---------------------------------------------------------------------------

class HTMLCheckBox : public HTMLInput
{
    Q_OBJECT
public:
    HTMLCheckBox( QWidget *parent, HTMLString n, HTMLString v, bool ch,
		  const HTMLFont *f = 0 );
    virtual ~HTMLCheckBox() { }
    
    virtual QString encoding();
    virtual void resetElement(QString data = 0);
    
    virtual const char * objectName() const { return "HTMLCheckBox"; };

private:
    bool _defCheck;
};

//---------------------------------------------------------------------------

class HTMLHidden : public HTMLInput
{
    Q_OBJECT
public:
    HTMLHidden( HTMLString n, HTMLString v );
    virtual ~HTMLHidden() { }

    virtual QString encoding();

    virtual const char * objectName() const { return "HTMLHidden"; };
};

//---------------------------------------------------------------------------

class HTMLRadio : public HTMLInput
{
    Q_OBJECT
	
public:
    HTMLRadio( QWidget *parent, HTMLString n, HTMLString v, bool ch,
	       const HTMLFont *f = 0 );
    virtual ~HTMLRadio() { }
    
    virtual QString encoding();
    virtual void resetElement(QString data = 0);
    
    virtual const char * objectName() const { return "HTMLRadio"; };

public slots:
    void slotRadioSelected( QString n, QString v );

signals:
    void radioSelected( QString n, QString v );

protected slots:
    void slotClicked();

private:
    bool _defCheck;
};

//---------------------------------------------------------------------------

class HTMLReset : public HTMLInput
{
    Q_OBJECT
public:
    HTMLReset( QWidget *parent, HTMLString v, const HTMLFont *f = 0 );
    virtual ~HTMLReset() { }

    virtual const char * objectName() const { return "HTMLReset"; };

protected slots:
    void slotClicked();

signals:
    void resetForm();
};

//---------------------------------------------------------------------------

class HTMLSubmit : public HTMLInput
{
    Q_OBJECT
public:
    HTMLSubmit( QWidget *parent, HTMLString n, HTMLString v,
		const HTMLFont *f = 0 );
    virtual ~HTMLSubmit() { }

    virtual QString encoding();

    virtual const char * objectName() const { return "HTMLSubmit"; };

protected slots:
    void slotClicked();

signals:
    void submitForm();

private:
    bool activated;
};

//---------------------------------------------------------------------------

class HTMLTextInput : public HTMLInput
{
    Q_OBJECT
public:
    HTMLTextInput( QWidget *parent, HTMLString n, HTMLString v, int s,
		   int ml, bool password = false, const HTMLFont *f = 0 );
    virtual ~HTMLTextInput() { }

    virtual QString encoding();
    virtual void resetElement(QString data = 0);

    virtual const char * objectName() const { return "HTMLTextInput"; };

protected slots:
    void slotTextChanged( const QString &);
    void slotReturnPressed();

signals:
    void submitForm();

private:
    QString _defText;
};

//---------------------------------------------------------------------------

class HTMLImageInput : public QObject, public HTMLImage, public HTMLElement
{
    Q_OBJECT
public:
    HTMLImageInput( KHTMLWidget *widget, HTMLString url, HTMLString name );
    virtual ~HTMLImageInput() {}

    virtual QString encoding();

    virtual HTMLObject *mouseEvent( int, int, int, int );

    virtual const char * objectName() const { return "HTMLImageInput"; };

signals:
    void submitForm();

private:
    int  _xp;
    int  _yp;
    bool pressed;
    bool activated;
};

//---------------------------------------------------------------------------

class HTMLForm : public QObject
{
    Q_OBJECT
public:
    HTMLForm( QString a, QString m, HTMLString target );
    virtual ~HTMLForm();

    void addElement( HTMLElement *e );
    // We keep hidden elements here, not in HTMLObject hierarchy
    void addHidden( HTMLHidden *he );
    void removeElement( HTMLElement *e );

    QString method() const
	{ return _method; }
    QString action() const
	{ return _action; }

    void position( int _x, int _y, int _width, int _height );

    void saveForm(QStringList *saveList);
    void restoreForm(QStringList *saveList);

public slots:
    void slotReset();
    void slotSubmit();
    void slotRadioSelected( QString n, QString v );

signals:
    void submitted( QString method, QString url, 
		    const char *data, QString target );
    void radioSelected( QString n, QString v );

private:
    QString _method;
    QString _action;
    HTMLString _target;

    QList<HTMLElement> elements;
    QList<HTMLHidden>  hidden;
};

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

class HTMLButton : public HTMLInput
{
    Q_OBJECT
public:
    HTMLButton( KHTMLWidget *_parent, HTMLString _name, HTMLString v, 
		QList<JSEventHandler> *_handlers, const HTMLFont *f = 0 );
    virtual ~HTMLButton();
    
    virtual const char * objectName() const { return "HTMLButton"; };

protected slots:
    void slotClicked();
    
protected:
    KHTMLWidget *view;
    QList<JSEventHandler> *eventHandlers;
};

//---------------------------------------------------------------------------

class HTMLMultiLineEditWidget : public QMultiLineEdit
{
    Q_OBJECT
public:
    HTMLMultiLineEditWidget( HTMLWidgetElement *htmlParent, QWidget *parent = 0, const char *name = 0);
   ~HTMLMultiLineEditWidget();

protected:
    void paintEvent( QPaintEvent *pe );
       
private:
    HTMLWidgetElement *widgetElement;
};

//---------------------------------------------------------------------------

class HTMLPushButtonWidget : public QPushButton
{
    Q_OBJECT
public:
    HTMLPushButtonWidget( HTMLWidgetElement *htmlParent, QWidget *parent = 0, const char *name = 0);
   ~HTMLPushButtonWidget();

protected:
    virtual void paintEvent( QPaintEvent *pe );   
   
private:
    HTMLWidgetElement *widgetElement;
};

//---------------------------------------------------------------------------

class HTMLRadioButtonWidget : public QRadioButton
{
    Q_OBJECT
public:
    HTMLRadioButtonWidget( HTMLWidgetElement *htmlParent, QWidget *parent = 0, const char *name = 0);
   ~HTMLRadioButtonWidget();
   
protected:
    virtual void paintEvent( QPaintEvent *pe );
    
private:
    HTMLWidgetElement *widgetElement;
};

//---------------------------------------------------------------------------

class HTMLCheckBoxWidget : public QCheckBox
{
    Q_OBJECT
public:
    HTMLCheckBoxWidget( HTMLWidgetElement *htmlParent, QWidget *parent = 0, const char *name = 0);
   ~HTMLCheckBoxWidget();
   
protected:
    virtual void paintEvent( QPaintEvent *pe );
    
private:
    HTMLWidgetElement *widgetElement;
};
           
#endif

