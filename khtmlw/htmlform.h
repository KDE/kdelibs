//----------------------------------------------------------------------------
// khtml widget - forms
//
//

#ifndef __HTMLFORM_H__
#define __HTMLFORM_H__

#include <qwidget.h>
#include "htmlobj.h"

//---------------------------------------------------------------------------

class HTMLElement : public QObject, public HTMLObject
{
	Q_OBJECT
public:
	HTMLElement( const char *n )
		{	_name = n; _absX = 0; _absY = 0; widget = NULL; }
	virtual ~HTMLElement();

	const QString &name() const
		{	return _name; }
	void setName( const char *n )
		{	_name = n; }
	int absX() const
		{	return _absX; }
	int absY() const
		{	return _absY; }

	// This function places the element on the page using the
	// absolute cooridinates.  Also responsible for showing/hiding
	// non-visible elements
	void position( int _x, int _y, int _width, int _height );

	virtual QString encoding()
		{	QString s = ""; return s; }

	virtual void calcAbsolutePos( int _x, int _y );

	virtual void reset() { }

protected:
	// encode special characters
	QString encodeString( const QString &e );

protected:
	QWidget *widget;
	QString _encoding;

private:
	QString _name;

	// absolute position of this element in the page
	int _absX;
	int _absY;
};

//---------------------------------------------------------------------------

class HTMLSelect : public HTMLElement
{
	Q_OBJECT
public:
	HTMLSelect( QWidget *parent, const char *n, int s, bool m );

	void addOption( const char *o, bool sel );

	// set text for current option
	void setText( const char *text );

	const QString &value()
		{	return *_values.at( _item ); }
	const QString &value( int item );
	void setValue( const char *v, int item );

	virtual QString encoding();
	virtual void reset();

protected slots:
	void slotHighlighted( int indx );

private:
	int _defSelected;
	int _size;
	QList<QString> _values;
	int _item;
};

//---------------------------------------------------------------------------

class HTMLTextArea : public HTMLElement
{
	Q_OBJECT
public:
	HTMLTextArea( QWidget *parent, const char *n, int r, int c );

	QString value();
	void setText( const char *t );

	virtual QString encoding();
	virtual void reset();

private:
	QString _defText;
};

//---------------------------------------------------------------------------

class HTMLInput : public HTMLElement
{
	Q_OBJECT
public:
	HTMLInput( const char *n, const char *v );

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
	HTMLCheckBox( QWidget *parent, const char *n, const char *v, bool ch );

	virtual QString encoding();
	virtual void reset();

private:
	bool _defCheck;
};

//---------------------------------------------------------------------------

class HTMLHidden : public HTMLInput
{
	Q_OBJECT
public:
	HTMLHidden( const char *n, const char *v );

	virtual QString encoding();
};

//---------------------------------------------------------------------------

class HTMLRadio : public HTMLInput
{
	Q_OBJECT
public:
	HTMLRadio( QWidget *parent, const char *n, const char *v, bool ch );

	virtual QString encoding();
	virtual void reset();

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
	HTMLReset( QWidget *parent, const char *v );

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
	HTMLSubmit( QWidget *parent, const char *v );

protected slots:
	void slotClicked();

signals:
	void submitForm();
};

//---------------------------------------------------------------------------

class HTMLTextInput : public HTMLInput
{
	Q_OBJECT
public:
	HTMLTextInput( QWidget *parent, const char *n, const char *v, int s );

	virtual QString encoding();
	virtual void reset();

protected slots:
	void slotTextChanged( const char * );

private:
	QString _defText;
};

//---------------------------------------------------------------------------

class HTMLForm : public QObject
{
	Q_OBJECT
public:
	HTMLForm( const char *a, const char *m );
	~HTMLForm();

	void addElement( HTMLElement *e );

	const char *method() const
		{	return _method; }
	const char *action() const
		{	return _action; }

	void position( int _x, int _y, int _width, int _height );

public slots:
	void slotReset();
	void slotSubmit();
	void slotRadioSelected( const char *n, const char *v );

signals:
	void submitted( const char *method, const char *url );
	void radioSelected( const char *n, const char *v );

private:
	QString _method;
	QString _action;

	QList<HTMLElement> elements;
};

//---------------------------------------------------------------------------

#endif

