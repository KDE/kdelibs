
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qpainter.h>
#include "kcolordlg.h"
#include "kcolorbtn.h"

#include "kcolorbtn.moc"

KColorButton::KColorButton( QWidget *parent, const char *name )
	: QPushButton( parent, name )
{
	connect( this, SIGNAL( clicked() ), SLOT( slotClicked() ) );
}

KColorButton::KColorButton( const QColor &c, QWidget *parent, const char *name )
	: QPushButton( parent, name )
{
	connect( this, SIGNAL( clicked() ), SLOT( slotClicked() ) );
	col = c;
}

void KColorButton::setColor( const QColor &c )
{
	col = c;
	repaint( false );
}

void KColorButton::slotClicked()
{
	if ( KColorDialog::getColor( col ) == QDialog::Rejected )
		return;

	repaint( false );

	emit changed( col );
}

void KColorButton::drawButtonLabel( QPainter *painter )
{
	if ( style() == WindowsStyle && isDown() )
		painter->fillRect( 6, 6, width() - 10, height() - 10, col );
	else
		painter->fillRect( 5, 5, width() - 10, height() - 10, col );
}


