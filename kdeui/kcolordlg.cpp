/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)

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
//-----------------------------------------------------------------------------
// KDE color selection dialog.

// layout managment added Oct 1997 by Mario Weilguni 
// <mweilguni@sime.com>

#include <stdlib.h>
#include <qimage.h>
#include <qpainter.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <kconfig.h>
#include "kcolordlg.h"
#include "kcolordlg.h"

#include <dither.h>
#include <klocale.h>
#include <kapp.h>

#include <kbuttonbox.h>
#include <qlayout.h>
#include <kseparator.h>

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
  if ( colContext )
    QColor::destroyAllocContext( colContext );
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
	QLabel *label;
	QPushButton *button;
	int h, s, v;
	
	selColor = darkCyan;
	selColor.hsv( &h, &s, &v );
	QString st;
	if(kapp->appName().length() > 0)
	  st = kapp->appName() + ": ";
	st =  klocale->translate("Select Color");
	setCaption( st.data() );
	
	// create a toplevel layout
	QGridLayout *tl_layout = new QGridLayout(this, 3, 3, 10);
	tl_layout->addColSpacing(1, 15);
	
	// add a layout for left-side (colors)
	QVBoxLayout *l_left = new QVBoxLayout;
	tl_layout->addLayout(l_left, 0, 0);
	
	// system colors
	label = new QLabel( klocale->translate("System Colors"), this );
	label->setFixedSize(label->sizeHint());
	l_left->addWidget(label, 0, AlignLeft);
	sysColorCells = new KColorCells( this, 3, 6 );
	sysColorCells->setMinimumSize(90, 60);
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
	l_left->addWidget(sysColorCells, 10);
	
	// a little space between
	l_left->addStretch(1);
	
	// add custom colors
	label = new QLabel( klocale->translate("Custom Colors"), this );
	label->setFixedSize(label->sizeHint());
	l_left->addWidget(label, 0, AlignLeft);
	custColorCells = new KColorCells( this, 3, 6 );
	custColorCells->setMinimumSize(90, 60);
	connect( custColorCells, SIGNAL( colorSelected( int ) ),
		 SLOT( slotCustColorSelected( int ) ) );
	l_left->addWidget(custColorCells, 10);
	
	// a little space between
	l_left->addStretch(1);
	
	// add buttom for adding colors
	button = new QPushButton( klocale->translate("Add to Custom Colors"),
				  this );
	button->setMinimumSize(button->sizeHint());
	l_left->addWidget(button, 0);
	connect( button, SIGNAL( clicked() ), 
		 SLOT( slotAddToCustom() ) );
	
	// the more complicated part: the right side
	// add a V-box
	QVBoxLayout *l_right = new QVBoxLayout();
	tl_layout->addLayout(l_right, 0, 2);
	
	// add a H-Box for the XY-Selector and a grid for the
	// entry fields
	QHBoxLayout *l_rtop = new QHBoxLayout();
	l_right->addLayout(l_rtop);
	QGridLayout *l_rbot = new QGridLayout(3, 6);
	l_right->addLayout(l_rbot);  
	
	// the palette and value selector go into the H-box
	palette = new KHSSelector( this );
	palette->setMinimumSize(140, 70);
	l_rtop->addWidget(palette, 8);  
	connect( palette, SIGNAL( valueChanged( int, int ) ),
		 SLOT( slotHSChanged( int, int ) ) );
	
	valuePal = new KValueSelector( this );
	valuePal->setHue( h );
	valuePal->setSaturation( s );
	valuePal->setMinimumSize(26, 70);
	l_rtop->addWidget(valuePal, 1);
	connect( valuePal, SIGNAL( valueChanged( int ) ),
		 SLOT( slotVChanged( int ) ) );
	
	// and now the entry fields and the patch
	patch = new KColorPatch( this );
	l_rbot->addMultiCellWidget(patch, 0, 2, 0, 0, AlignVCenter|AlignLeft);
	patch->setFixedSize(48, 48);
	patch->setColor( selColor );
	
	// add the HSV fields
	label = new QLabel( "H:", this );
	label->setMinimumSize(label->sizeHint());
	label->setAlignment(AlignRight | AlignVCenter);
	l_rbot->addWidget(label, 0, 2);
	hedit = new QLineEdit( this );
	hedit->setFixedHeight(hedit->sizeHint().height());
	l_rbot->addWidget(hedit, 0, 3);
	connect( hedit, SIGNAL( returnPressed() ),
		 SLOT( slotHSVChanged() ) );
	
	label = new QLabel( "S:", this );
	label->setMinimumSize(label->sizeHint());
	label->setAlignment(AlignRight | AlignVCenter);
	l_rbot->addWidget(label, 1, 2);
	sedit = new QLineEdit( this );
	sedit->setFixedHeight(sedit->sizeHint().height());
	l_rbot->addWidget(sedit, 1, 3);
	connect( sedit, SIGNAL( returnPressed() ),
		 SLOT( slotHSVChanged() ) );
	
	label = new QLabel( "V:", this );
	label->setMinimumSize(label->sizeHint());
	label->setAlignment(AlignRight | AlignVCenter);
	l_rbot->addWidget(label, 2, 2);
	vedit = new QLineEdit( this );
	vedit->setFixedHeight(vedit->sizeHint().height());
	l_rbot->addWidget(vedit, 2, 3);
	connect( vedit, SIGNAL( returnPressed() ),
		 SLOT( slotHSVChanged() ) );
	
	
	// add the RGB fields
	label = new QLabel( "R:", this );
	label->setMinimumSize(label->sizeHint());
	label->setAlignment(AlignRight | AlignVCenter);
	l_rbot->addWidget(label, 0, 4);
	redit = new QLineEdit( this );
	redit->setFixedHeight(redit->sizeHint().height());
	l_rbot->addWidget(redit, 0, 5);
	connect( redit, SIGNAL( returnPressed() ),
		 SLOT( slotRGBChanged() ) );
	
	label = new QLabel( "G:", this );
	label->setMinimumSize(label->sizeHint());
	label->setAlignment(AlignRight | AlignVCenter);
	l_rbot->addWidget(label, 1, 4);
	gedit = new QLineEdit( this );
	gedit->setFixedHeight(gedit->sizeHint().height());
	l_rbot->addWidget(gedit, 1, 5);
	connect( gedit, SIGNAL( returnPressed() ),
		 SLOT( slotRGBChanged() ) );
	
	label = new QLabel( "B:", this );
	label->setMinimumSize(label->sizeHint());
	label->setAlignment(AlignRight | AlignVCenter);
	l_rbot->addWidget(label, 2, 4);
	bedit = new QLineEdit( this );
	bedit->setFixedHeight(bedit->sizeHint().height());
	l_rbot->addWidget(bedit, 2, 5);
	connect( bedit, SIGNAL( returnPressed() ),
		 SLOT( slotRGBChanged() ) );
	
	// the entry fields should be wide enought to hold 88888
	int w = hedit->fontMetrics().boundingRect("88888").width();
	hedit->setMinimumWidth(w);
	redit->setMinimumWidth(w);
	
	// the label rows should not expand
	l_rbot->setColStretch(2, 0);
	l_rbot->setColStretch(4, 0);
	
	// the entry rows should expand
	l_rbot->setColStretch(3, 1);
	l_rbot->setColStretch(5, 1);
	
	// a little separator between
	KSeparator *sep = new KSeparator(this);
	tl_layout->addMultiCellWidget(sep, 1, 1, 0, 2);
	
	// the standard buttons
	KButtonBox *bbox = new KButtonBox(this);
        button  = bbox->addButton(klocale->translate("Help"));
          connect( button, SIGNAL(clicked()),
		                   SLOT(getHelp()));                              
        bbox->addStretch(1);
        button = bbox->addButton(klocale->translate("OK"));
	connect( button, SIGNAL( clicked() ), 
		 SLOT( slotOkPressed() ) );
	button = bbox->addButton(klocale->translate("Cancel"));

	connect( button, SIGNAL( clicked() ), 
		 SLOT( reject() ) );
	bbox->layout();
	bbox->setMinimumSize(bbox->sizeHint());

        tl_layout->addMultiCellWidget(bbox, 2, 2, 0, 2);
	tl_layout->setRowStretch(0, 1);
	tl_layout->setRowStretch(1, 0);
	tl_layout->setRowStretch(2, 0);
	tl_layout->activate();
	tl_layout->freeze();
	
	readSettings();
	
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
	KColorDialog dlg( 0L, "Color Selector", TRUE );
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
	QString key;

	KConfig* config = kapp->getConfig();

	QString oldgroup = config->group();
	config->setGroup( "Custom Colors");

	for ( int i = 0; i < custColorCells->numCells(); i++ )
	{
		key.sprintf( "Color%d", i );
		col = config->readColorEntry( key, &lightGray );
		custColorCells->setColor( i, col );
	}
	config->setGroup( oldgroup );
}

void KColorDialog::writeSettings()
{
	QColor color;
	QString key;

	KConfig* config = kapp->getConfig();

	QString oldgroup = config->group();
	config->setGroup( "Custom Colors");

	for ( int i = 0; i < custColorCells->numCells(); i++ )
	{
		color = custColorCells->color( i );
		key.sprintf( "Color%d", i );
		config->writeEntry( key, color, true, true );
	}

	config->setGroup( oldgroup );
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

void KColorDialog::getHelp() {
  if(kapp != 0)
    kapp->invokeHTMLHelp("kcolordialog.html", "");
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
#include "kcolordlg.moc"

