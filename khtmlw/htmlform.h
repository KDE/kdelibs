/* This file is part of the KDE libraries
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
// khtml widget - forms
//
//

#ifndef __HTMLFORM_H__
#define __HTMLFORM_H__

class HTMLElement;
class HTMLForm;
class HTMLSelect;
class HTMLButton;
class HTMLTextArea;

#include <qwidget.h>
#include <qtimer.h>

#include "htmlobj.h"

class HTMLForm;

//---------------------------------------------------------------------------

class HTMLElement
{
public:
	HTMLElement( const char *n )
		{ _name = n; form = 0; }
	virtual ~HTMLElement();

	const QString &elementName() const
		{	return _name; }
	void setElementName( const char *n )
		{	_name = n; }

	void setForm( HTMLForm *f )
		{	form = f; }

	// This function places the element on the page using the
	// absolute coordinates.  Also responsible for showing/hiding
	// non-visible elements
	virtual void position( int /*_x*/, int /*_y*/,
			       int /*_width*/, int /*_height*/ ) {}

	virtual bool positionChanged( int /*_x*/, int /*_y*/,
			       int /*_width*/, int /*_height*/ ) { return true; }

	virtual QString encoding()
		{	return QString( "" ); }

	virtual void calcAbsolutePos( int , int ) {}

	virtual void resetElement() { }

    virtual void hideElement() {}

protected:
	// encode special characters
	QString encodeString( const QString &e );

protected:
	QString _encoding;

private:
	QString _name;

	HTMLForm *form;
};

class HTMLWidgetElement : public QObject, public HTMLObject, public HTMLElement
{
	Q_OBJECT
public:
	HTMLWidgetElement( const char *n, const HTMLFont *f = 0 ) 
	     : HTMLElement( n )
	    { _absX = 0; _absY = 0; widget = 0; font = f; }
	virtual ~HTMLWidgetElement();

	int absX() const
		{	return _absX; }
	int absY() const
		{	return _absY; }

	// This function places the element on the page using the
	// absolute coordinates.  Also responsible for showing/hiding
	// non-visible elements
	virtual void position( int _x, int _y, int _width, int _height );
	virtual bool positionChanged( int _x, int _y, int _width, int _height );

	virtual void calcAbsolutePos( int _x, int _y );

    virtual void hideElement();

protected:
	QWidget *widget;
	const HTMLFont *font;

private:
	// absolute position of this element in the page
	int _absX;
	int _absY;
};

//---------------------------------------------------------------------------

class HTMLSelect : public HTMLWidgetElement
{
	Q_OBJECT
public:
	HTMLSelect( QWidget *parent, const char *n, int s, bool m,
		    const HTMLFont *f = 0 );
	virtual ~HTMLSelect() { }

	void addOption( const char *o, bool sel );

	// set text for current option
	void setText( const char *text );

	const QString &value()
		{	return *_values.at( _item ); }
	const QString &value( int item );
	void setValue( const char *v, int item );

	virtual QString encoding();
	virtual void resetElement();

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
	HTMLTextArea( QWidget *parent, const char *n, int r, int c,
		      const HTMLFont *f = 0 );
	virtual ~HTMLTextArea() { }

	QString value();
	void setText( const char *t );

	virtual QString encoding();
	virtual void resetElement();

private:
	QString _defText;
};

//---------------------------------------------------------------------------

class HTMLInput : public HTMLWidgetElement
{
	Q_OBJECT
public:
	HTMLInput( const char *n, const char *v, const HTMLFont *f = 0 );
	virtual ~HTMLInput() { }

	const QString &value() const
		{	return _value; }
	void setValue( const char *v )
		{	_value = v; }

private:
	QString   _value;
};

//---------------------------------------------------------------------------

class HTMLCheckBox : public HTMLInput
{
	Q_OBJECT
public:
	HTMLCheckBox( QWidget *parent, const char *n, const char *v, bool ch,
		      const HTMLFont *f = 0 );
	virtual ~HTMLCheckBox() { }

	virtual QString encoding();
	virtual void resetElement();

private:
	bool _defCheck;
};

//---------------------------------------------------------------------------

class HTMLHidden : public HTMLInput
{
	Q_OBJECT
public:
	HTMLHidden( const char *n, const char *v );
	virtual ~HTMLHidden() { }

	virtual QString encoding();
};

//---------------------------------------------------------------------------

class HTMLRadio : public HTMLInput
{
	Q_OBJECT
public:
	HTMLRadio( QWidget *parent, const char *n, const char *v, bool ch,
		   const HTMLFont *f = 0 );
	virtual ~HTMLRadio() { }

	virtual QString encoding();
	virtual void resetElement();

public slots:
	void slotRadioSelected( const char *n, const char *v );

signals:
	void radioSelected( const char *n, const char *v );

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
	HTMLReset( QWidget *parent, const char *v, const HTMLFont *f = 0 );
	virtual ~HTMLReset() { }

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
	HTMLSubmit( QWidget *parent, const char *n, const char *v,
		    const HTMLFont *f = 0 );
	virtual ~HTMLSubmit() { }

	virtual QString encoding();

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
	HTMLTextInput( QWidget *parent, const char *n, const char *v, int s,
		    int ml, bool password = false, const HTMLFont *f = 0 );
	virtual ~HTMLTextInput() { }

	virtual QString encoding();
	virtual void resetElement();

protected slots:
	void slotTextChanged( const char * );
	void slotReturnPressed();

signals:
	void submitForm();

private:
	QString _defText;
};

//---------------------------------------------------------------------------

class HTMLImageInput : public HTMLImage, public HTMLElement
{
    Q_OBJECT
public:
    HTMLImageInput( KHTMLWidget *widget, const char *, int mw, const char *n );
    virtual ~HTMLImageInput() {}

    virtual QString encoding();

    virtual HTMLObject *mouseEvent( int, int, int, int );

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
	HTMLForm( const char *a, const char *m );
	virtual ~HTMLForm();

	void addElement( HTMLElement *e );
	// We keep hidden elements here, not in HTMLObject hierarchy
	void addHidden( HTMLHidden *he );
	void removeElement( HTMLElement *e );

	const char *method() const
		{	return _method; }
	const char *action() const
		{	return _action; }

	void position( int _x, int _y, int _width, int _height );

public slots:
	void slotReset();
	void slotSubmit();
	void slotRadioSelected( const char *n, const char *v );

protected slots:
    void slotTimeout();

signals:
	void submitted( const char *method, const char *url, const char *data );
	void radioSelected( const char *n, const char *v );

private:
	QString _method;
	QString _action;

	QList<HTMLElement> elements;
	QList<HTMLHidden>  hidden;

    QTimer *timer;
    int dx;
    int dy;
    int width;
    int height;
};

//---------------------------------------------------------------------------

#include "jscript.h"

//---------------------------------------------------------------------------

class HTMLButton : public HTMLInput
{
    Q_OBJECT
public:
    HTMLButton( KHTMLWidget *_parent, const char *_name, const char *v, 
		QList<JSEventHandler> *_handlers, const HTMLFont *f = 0 );
    virtual ~HTMLButton();
    
protected slots:
    void slotClicked();
    
protected:
    KHTMLWidget *view;
    QList<JSEventHandler> *eventHandlers;
};

#endif

