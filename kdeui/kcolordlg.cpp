
#include <stdlib.h>
#include <qimage.h>
#include <qpainter.h>
#include <qdrawutl.h>
#include <qevent.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <kconfig.h>
#include "kcolordlg.h"
#include "kcolordlg.moc"
#include "dither.h"

#include <klocale.h>
#include <kapp.h>

#define HSV_X 305
#define RGB_X 385

static QColor *standardPalette = 0;

#define STANDARD_PAL_SIZE 17

void createStandardPalette()
{
    if ( standardPalette )
	return;

    standardPalette = new QColor [STANDARD_PAL_SIZE];

    int i = 0;

    standardPalette[i++] = red;
    standardPalette[i++] = green;
    standardPalette[i++] = blue;
    standardPalette[i++] = cyan;
    standardPalette[i++] = magenta;
    standardPalette[i++] = yellow;
    standardPalette[i++] = darkRed;
    standardPalette[i++] = darkGreen;
    standardPalette[i++] = darkBlue;
    standardPalette[i++] = darkCyan;
    standardPalette[i++] = darkMagenta;
    standardPalette[i++] = darkYellow;
    standardPalette[i++] = white;
    standardPalette[i++] = lightGray;
    standardPalette[i++] = gray;
    standardPalette[i++] = darkGray;
    standardPalette[i++] = black;
}

KHSSelector::KHSSelector( QWidget *parent )
	: KXYSelector( parent )
{
	setRange( 0, 0, 359, 255 );
}

void KHSSelector::resizeEvent( QResizeEvent * )
{
	drawPalette();
}

void KHSSelector::drawContents( QPainter *painter )
{
	painter->drawPixmap( contentsRect().x(), contentsRect().y(), pixmap );
}

void KHSSelector::drawPalette()
{
	int xSize = contentsRect().width(), ySize = contentsRect().height();
	QImage image( xSize, ySize, 32 );
	QColor col;
	int h, s;
	uint *p;

	for ( s = ySize-1; s >= 0; s-- )
	{
		p = (uint *) image.scanLine( ySize - s - 1 );
		for( h = 0; h < xSize; h++ )
		{
			col.setHsv( 359*h/(xSize-1), 255*s/(ySize-1), 192 );
			*p = col.rgb();
			p++;
		}
	}

	if ( QColor::numBitPlanes() <= 8 )
	{
		createStandardPalette();
		kFSDither dither( standardPalette, STANDARD_PAL_SIZE );
		QImage tImage = dither.dither( image );
		pixmap.convertFromImage( tImage );
	}
	else
		pixmap.convertFromImage( image );
}

//-----------------------------------------------------------------------------

KValueSelector::KValueSelector( QWidget *parent )
	: KSelector( KSelector::Vertical, parent )
{
	setRange( 0, 255 );
	pixmap.optimize( TRUE );
}

void KValueSelector::resizeEvent( QResizeEvent * )
{
	drawPalette();
}

void KValueSelector::drawContents( QPainter *painter )
{
	painter->drawPixmap( contentsRect().x(), contentsRect().y(), pixmap );
}

void KValueSelector::drawPalette()
{
	int xSize = contentsRect().width(), ySize = contentsRect().height();
	QImage image( xSize, ySize, 32 );
	QColor col;
	uint *p;
	QRgb rgb;

	for ( int v = 0; v < ySize; v++ )
	{
		p = (uint *) image.scanLine( ySize - v - 1 );
		col.setHsv( hue, sat, 255*v/(ySize-1) );
		rgb = col.rgb();
		for ( int i = 0; i < xSize; i++ )
			*p++ = rgb;
	}

	if ( QColor::numBitPlanes() <= 8 )
	{
		createStandardPalette();
		kFSDither dither( standardPalette, STANDARD_PAL_SIZE );
		QImage tImage = dither.dither( image );
		pixmap.convertFromImage( tImage );
	}
	else
		pixmap.convertFromImage( image );
}

//-----------------------------------------------------------------------------

KColorCells::KColorCells( QWidget *parent, int rows, int cols )
	: QTableView( parent )
{
	setNumRows( rows );
	setNumCols( cols );
	colors = new QColor [ rows * cols ];

	for ( int i = 0; i < rows * cols; i++ )
		colors[i] = backgroundColor();

	selected = 0;
}

KColorCells::~KColorCells()
{
	delete [] colors;
}

void KColorCells::setColor( int colNum, const QColor &col )
{
	colors[colNum] = col;
	updateCell( colNum/numCols(), colNum%numCols() );
}

void KColorCells::paintCell( QPainter *painter, int row, int col )
{
	QBrush brush;
	qDrawShadePanel( painter, 1, 1, cellWidth()-2, cellHeight()-2, colorGroup(),
	            TRUE, 1, &brush );
	painter->setPen( colors[ row * numCols() + col ] );
	painter->setBrush( QBrush( colors[ row * numCols() + col ] ) );
	painter->drawRect( 2, 2, cellWidth()-4, cellHeight()-4 );

	if ( row * numCols() + col == selected )
		painter->drawWinFocusRect( 2, 2, cellWidth()-4, cellHeight()-4 );
}

void KColorCells::resizeEvent( QResizeEvent * )
{
	setCellWidth( width() / numCols() );
	setCellHeight( height() / numRows() );
}

void KColorCells::mouseReleaseEvent( QMouseEvent *e )
{
	int row = e->pos().y() / cellHeight();
	int col = e->pos().x() / cellWidth();
	int cell = row * numCols() + col;

	if ( selected != cell )
	{
		int prevSel = selected;
		selected = cell;
		updateCell( prevSel/numCols(), prevSel%numCols(), FALSE );
		updateCell( row, col, FALSE );
	}

	emit colorSelected( cell );
}

//-----------------------------------------------------------------------------

KColorPatch::KColorPatch( QWidget *parent ) : QFrame( parent )
{
	setFrameStyle( QFrame::Panel | QFrame::Sunken );
	colContext = 0;
}

KColorPatch::~KColorPatch()
{
}

void KColorPatch::setColor( const QColor &col )
{
	if ( colContext )
		QColor::destroyAllocContext( colContext );
	colContext = QColor::enterAllocContext();
	color.setRgb( col.rgb() );
	color.alloc();
	QColor::leaveAllocContext();

	QPainter painter;

	painter.begin( this );
	drawContents( &painter );
	painter.end();
}

void KColorPatch::drawContents( QPainter *painter )
{
	painter->setPen( color );
	painter->setBrush( QBrush( color ) );
	painter->drawRect( contentsRect() );
}

//-----------------------------------------------------------------------------

KColorDialog::KColorDialog( QWidget *parent, const char *name, bool modal )
	: QDialog( parent, name, modal )
{
	selColor = darkCyan;
	int h, s, v;
	selColor.hsv( &h, &s, &v );

	setCaption( klocale->translate("Select Color") );

	QLabel *label;
	QPushButton *button;

	label = new QLabel( klocale->translate("System Colors"), this );
	label->setGeometry( 15, 15, 150, 20 );

	sysColorCells = new KColorCells( this, 3, 6 );
	sysColorCells->setGeometry( 15, 35, 180, 60 );
	sysColorCells->setColor( 0, red );
	sysColorCells->setColor( 1, green );
	sysColorCells->setColor( 2, blue );
	sysColorCells->setColor( 3, cyan );
	sysColorCells->setColor( 4, magenta );
	sysColorCells->setColor( 5, yellow );
	sysColorCells->setColor( 6, darkRed );
	sysColorCells->setColor( 7, darkGreen );
	sysColorCells->setColor( 8, darkBlue );
	sysColorCells->setColor( 9, darkCyan );
	sysColorCells->setColor( 10, darkMagenta );
	sysColorCells->setColor( 11, darkYellow );
	sysColorCells->setColor( 12, white );
	sysColorCells->setColor( 13, lightGray );
	sysColorCells->setColor( 14, gray );
	sysColorCells->setColor( 15, darkGray );
	sysColorCells->setColor( 16, black );
	connect( sysColorCells, SIGNAL( colorSelected( int ) ),
			SLOT( slotSysColorSelected( int ) ) );

	label = new QLabel( klocale->translate("Custom Colors"), this );
	label->setGeometry( 15, 105, 150, 20 );

	custColorCells = new KColorCells( this, 3, 6 );
	custColorCells->setGeometry( 15, 125, 180, 60 );
	connect( custColorCells, SIGNAL( colorSelected( int ) ),
			SLOT( slotCustColorSelected( int ) ) );

	readSettings();

	label = new QLabel( "H:", this );
	label->setGeometry( HSV_X-15, 155, 15, 20 );
	hedit = new QLineEdit( this );
	hedit->setGeometry( HSV_X, 155, 40, 20 );
	connect( hedit, SIGNAL( returnPressed() ),
		SLOT( slotHSVChanged() ) );

	label = new QLabel( "S:", this );
	label->setGeometry( HSV_X-15, 180, 15, 20 );
	sedit = new QLineEdit( this );
	sedit->setGeometry( HSV_X, 180, 40, 20 );
	connect( sedit, SIGNAL( returnPressed() ),
		SLOT( slotHSVChanged() ) );

	label = new QLabel( "V:", this );
	label->setGeometry( HSV_X-15, 205, 15, 20 );
	vedit = new QLineEdit( this );
	vedit->setGeometry( HSV_X, 205, 40, 20 );
	connect( vedit, SIGNAL( returnPressed() ),
		SLOT( slotHSVChanged() ) );

	label = new QLabel( "R:", this );
	label->setGeometry( RGB_X-15, 155, 15, 20 );
	redit = new QLineEdit( this );
	redit->setGeometry( RGB_X, 155, 40, 20 );
	connect( redit, SIGNAL( returnPressed() ),
		SLOT( slotRGBChanged() ) );

	label = new QLabel( "G:", this );
	label->setGeometry( RGB_X-15, 180, 15, 20 );
	gedit = new QLineEdit( this );
	gedit->setGeometry( RGB_X, 180, 40, 20 );
	connect( gedit, SIGNAL( returnPressed() ),
		SLOT( slotRGBChanged() ) );

	label = new QLabel( "B:", this );
	label->setGeometry( RGB_X-15, 205, 15, 20 );
	bedit = new QLineEdit( this );
	bedit->setGeometry( RGB_X, 205, 40, 20 );
	connect( bedit, SIGNAL( returnPressed() ),
		SLOT( slotRGBChanged() ) );

	patch = new KColorPatch( this );
	patch->setGeometry( 210, 185, 60, 40 );
	patch->setColor( selColor );

	palette = new KHSSelector( this );
	palette->setGeometry( 210, 15, 180, 128 );
	connect( palette, SIGNAL( valueChanged( int, int ) ),
			SLOT( slotHSChanged( int, int ) ) );
	
	valuePal = new KValueSelector( this );
	valuePal->setHue( h );
	valuePal->setSaturation( s );
	valuePal->setGeometry( 400, 12, 26, 134 );
	connect( valuePal, SIGNAL( valueChanged( int ) ),
			SLOT( slotVChanged( int ) ) );

	button = new QPushButton( klocale->translate("Ok"), this );
	button->setGeometry( 300, 245, 50, 25 );
	connect( button, SIGNAL( clicked() ), SLOT( slotOkPressed() ) );

	button = new QPushButton( klocale->translate("Cancel"), this );
	button->setGeometry( 375, 245, 50, 25 );
	connect( button, SIGNAL( clicked() ), SLOT( reject() ) );

	button = new QPushButton( klocale->translate("Add to Custom Colors"),
				  this );
	button->setGeometry( 15, 200, 180, 25 );
	connect( button, SIGNAL( clicked() ), SLOT( slotAddToCustom() ) );

	setRgbEdit();
	setHsvEdit();

	palette->setValues( h, s );
	valuePal->setValue( v );
}

void KColorDialog::setColor( const QColor &col )
{
	selColor = col;

	setRgbEdit();
	setHsvEdit();

	int h, s, v;
	selColor.hsv( &h, &s, &v );
	palette->setValues( h, s );
	valuePal->setHue( h );
	valuePal->setSaturation( s );
	valuePal->drawPalette();
	valuePal->repaint( FALSE );
	valuePal->setValue( v );
	patch->setColor( selColor );
}

// static function to display dialog and return color
int KColorDialog::getColor( QColor &theColor )
{
	KColorDialog dlg( NULL, "Color Selector", TRUE );
	if ( theColor.isValid() )
		dlg.setColor( theColor );
	int result = dlg.exec();

	if ( result == Accepted )
		theColor = dlg.color();

	return result;
}

void KColorDialog::slotOkPressed()
{
	writeSettings();
	accept();
}

void KColorDialog::slotRGBChanged()
{
	int red = atoi( redit->text() );
	int grn = atoi( gedit->text() );
	int blu = atoi( bedit->text() );

	if ( red > 255 || red < 0 ) return;
	if ( grn > 255 || grn < 0 ) return;
	if ( blu > 255 || blu < 0 ) return;

	selColor.setRgb( red, grn, blu );
	patch->setColor( selColor );

	setRgbEdit();
	setHsvEdit();

	int h, s, v;
	selColor.hsv( &h, &s, &v );
	palette->setValues( h, s );
	valuePal->setHue( h );
	valuePal->setSaturation( s );
	valuePal->drawPalette();
	valuePal->repaint( FALSE );
	valuePal->setValue( v );

	emit colorSelected( selColor );
}

void KColorDialog::slotHSVChanged()
{
	int hue = atoi( hedit->text() );
	int sat = atoi( sedit->text() );
	int val = atoi( vedit->text() );

	if ( hue > 359 || hue < 0 ) return;
	if ( sat > 255 || sat < 0 ) return;
	if ( val > 255 || val < 0 ) return;

	selColor.setHsv( hue, sat, val );
	patch->setColor( selColor );

	setRgbEdit();
	setHsvEdit();

	palette->setValues( hue, sat );
	valuePal->setHue( hue );
	valuePal->setSaturation( sat );
	valuePal->drawPalette();
	valuePal->repaint( FALSE );
	valuePal->setValue( val );

	emit colorSelected( selColor );
}

void KColorDialog::slotHSChanged( int h, int s )
{
	selColor.setHsv( h, s, valuePal->value() );

	valuePal->setHue( h );
	valuePal->setSaturation( s );
	valuePal->drawPalette();
	valuePal->repaint( FALSE );

	patch->setColor( selColor );

	setRgbEdit();
	setHsvEdit();

	emit colorSelected( selColor );
}

void KColorDialog::slotVChanged( int v )
{
	selColor.setHsv( palette->xValue(), palette->yValue(), v );
	patch->setColor( selColor );

	setRgbEdit();
	setHsvEdit();

	emit colorSelected( selColor );
}

void KColorDialog::slotSysColorSelected( int col )
{
	selColor = sysColorCells->color( col );

	patch->setColor( selColor );

	setRgbEdit();
	setHsvEdit();

	int h, s, v;
	selColor.hsv( &h, &s, &v );
	palette->setValues( h, s );
	valuePal->setHue( h );
	valuePal->setSaturation( s );
	valuePal->drawPalette();
	valuePal->repaint( FALSE );
	valuePal->setValue( v );

	emit colorSelected( selColor );
}

void KColorDialog::slotCustColorSelected( int col )
{
	QColor color = custColorCells->color( col );

	// if a color has not been assigned to this cell, don't change current col
	if ( color == lightGray )
		return;

	selColor = color;

	patch->setColor( selColor );

	setRgbEdit();
	setHsvEdit();

	int h, s, v;
	selColor.hsv( &h, &s, &v );
	palette->setValues( h, s );
	valuePal->setHue( h );
	valuePal->setSaturation( s );
	valuePal->drawPalette();
	valuePal->repaint( FALSE );
	valuePal->setValue( v );

	emit colorSelected( selColor );
}

void KColorDialog::slotAddToCustom()
{
	custColorCells->setColor( custColorCells->getSelected(), selColor );
}

void KColorDialog::readSettings()
{
	QColor col;
	QString str, key;

	// use a global config file
	KConfig config;

	config.setGroup( "Custom Colors");

	for ( int i = 0; i < custColorCells->numCells(); i++ )
	{
		key.sprintf( "Color%d", i );
		str = config.readEntry( key );
		if ( !str.isNull() )
			col.setNamedColor( str );
		else
			col = lightGray;
		custColorCells->setColor( i, col );
	}
}

void KColorDialog::writeSettings()
{
	QColor color;
	QString str, key;

	// use a global config file
	KConfig config;

	config.setGroup( "Custom Colors");

	for ( int i = 0; i < custColorCells->numCells(); i++ )
	{
		color = custColorCells->color( i );
		key.sprintf( "Color%d", i );
		str.sprintf("#%02x%02x%02x", color.red(), color.green(), color.blue());
		config.writeEntry( key, str );
		str.detach();
	}
}

void KColorDialog::setRgbEdit()
{
	int r, g, b;
	selColor.rgb( &r, &g, &b );
	QString num;

	num.setNum( r );
	redit->setText( num );
	num.setNum( g );
	gedit->setText( num );
	num.setNum( b );
	bedit->setText( num );
}

void KColorDialog::setHsvEdit()
{
	int h, s, v;
	selColor.hsv( &h, &s, &v );
	QString num;

	num.setNum( h );
	hedit->setText( num );
	num.setNum( s );
	sedit->setText( num );
	num.setNum( v );
	vedit->setText( num );
}

//----------------------------------------------------------------------------

KColorCombo::KColorCombo( QWidget *parent, const char *name )
	: QComboBox( parent, name )
{
	customColor.setRgb( 255, 255, 255 );
	color.setRgb( 255, 255, 255 );

	createStandardPalette();

	addColors();

	connect( this, SIGNAL( activated(int) ), SLOT( slotActivated(int) ) );
	connect( this, SIGNAL( highlighted(int) ), SLOT( slotHighlighted(int) ) );
}

void KColorCombo::setColor( const QColor &col )
{
	color = col;

	addColors();
}

void KColorCombo::resizeEvent( QResizeEvent *re )
{
	QComboBox::resizeEvent( re );

	addColors();
}

void KColorCombo::slotActivated( int index )
{
	if ( index == 0 )
	{
	    if ( KColorDialog::getColor( customColor ) == QDialog::Accepted )
		{
			QRect rect( 0, 0, width(), 20 );
			QPixmap pixmap( rect.width(), rect.height() );
			QPainter painter;
			QPen pen;

			if ( qGray( customColor.rgb() ) < 128 )
				pen.setColor( white );
			else
				pen.setColor( black );

			painter.begin( &pixmap );
			QBrush brush( customColor );
			painter.fillRect( rect, brush );
			painter.setPen( pen );
			painter.drawText( 2, 18, 
					  klocale->translate("Custom...") );
			painter.end();

			changeItem( pixmap, 0 );
			pixmap.detach();
		}

		color = customColor;
	}
	else
		color = standardPalette[ index - 1 ];

	emit activated( color );
}

void KColorCombo::slotHighlighted( int index )
{
	if ( index == 0 )
		color = customColor;
	else
		color = standardPalette[ index - 1 ];

	emit highlighted( color );
}

void KColorCombo::addColors()
{
	QRect rect( 0, 0, width(), 20 );
	QPixmap pixmap( rect.width(), rect.height() );
	QPainter painter;
	QPen pen;
	int i;

	clear();

	createStandardPalette();

	for ( i = 0; i < STANDARD_PAL_SIZE; i++ )
		if ( standardPalette[i] == color ) break;

	if ( i == STANDARD_PAL_SIZE )
		customColor = color;

	if ( qGray( customColor.rgb() ) < 128 )
		pen.setColor( white );
	else
		pen.setColor( black );

	painter.begin( &pixmap );
	QBrush brush( customColor );
	painter.fillRect( rect, brush );
	painter.setPen( pen );
	painter.drawText( 2, 18, klocale->translate("Custom...") );
	painter.end();

	insertItem( pixmap );
	pixmap.detach();
	
	for ( i = 0; i < STANDARD_PAL_SIZE; i++ )
	{
		painter.begin( &pixmap );
		QBrush brush( standardPalette[i] );
		painter.fillRect( rect, brush );
		painter.end();

		insertItem( pixmap );
		pixmap.detach();

		if ( standardPalette[i] == color )
			setCurrentItem( i + 1 );
	}
}


