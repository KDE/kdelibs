
#ifndef __COLBTN_H__
#define __COLBTN_H__

#include <qpushbt.h>

class KColorButton : public QPushButton
{
	Q_OBJECT
public:
	KColorButton( QWidget *parent, const char *name = NULL );
	KColorButton( const QColor &c, QWidget *parent, const char *name = NULL );
	virtual ~KColorButton() {}

	const QColor color() const
		{	return col; }
	void setColor( const QColor &c );

signals:
	void changed( const QColor &newColor );

protected slots:
	void slotClicked();

protected:
	virtual void drawButtonLabel( QPainter *p );

private:
	QColor col;
};

#endif

