
#ifndef __COLBTN_H__
#define __COLBTN_H__

#include <qpushbt.h>

/**
* This widget can be used to display or allow user selection of a colour.
*
* @see KColorDialog
*
* @short A pushbutton to display or allow user selection of a colour.
* @version $Id$
*/
class KColorButton : public QPushButton
{
	Q_OBJECT
public:
	/**
	* Constructor. Create a KColorButton.
	*/
	KColorButton( QWidget *parent, const char *name = NULL );
	/**
	* Constructor. Create a KColorButton.
	* @param c	The initial colour of the button.
	*/
	KColorButton( const QColor &c, QWidget *parent, const char *name = NULL );
	/**
	* Destructor.
	*/
	virtual ~KColorButton() {}

	/**
	* The current colour.
	* @return The current colour.
	*/
	const QColor color() const
		{	return col; }
	/**
	* Set the current colour.
	*
	* @param c	The colour to set.
	*/
	void setColor( const QColor &c );

signals:
	/**
	* This signal will be emitted when the colour of the widget
	* is changed, either with @ref #setColor or via user selection.
	*/
	void changed( const QColor &newColor );

protected slots:
	/**
	*/
	void slotClicked();

protected:
	/**
	*/
	virtual void drawButtonLabel( QPainter *p );

private:
	QColor col;
};

#endif

