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
//
// 1999-09-27 Espen Sand <espensa@online.no>
// KColorDialog is now subclassed from KDialogBase. I have also extended
// KColorDialog::getColor() so that in contains a parent argument. This
// improves centering capability.
//
// layout managment added Oct 1997 by Mario Weilguni
// <mweilguni@sime.com>
//

#include <stdio.h>
#include <stdlib.h>

#include <qdrawutil.h>
#include <qevent.h>
#include <qfile.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qtimer.h>

#include <kapp.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klistbox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kseparator.h>
#include <kpalette.h>
#include <kimageeffect.h>

#include "kcolordlg.h"
#include "kcolordrag.h"
#include "kstaticdeleter.h"
#include <config.h>

#include <X11/Xlib.h>

// defined in qapplication_x11.cpp
typedef int (*QX11EventFilter) (XEvent*);
extern QX11EventFilter qt_set_x11_event_filter (QX11EventFilter filter);

#define HSV_X 305
#define RGB_X 385

static const char *recentColors = "Recent_Colors";
static const char *customColors = "Custom_Colors";


#define STANDARD_PAL_SIZE 17

KColor::KColor()
: QColor()
{
  r = 0; g = 0; b = 0; h = 0; s = 0; v = 0;
};

KColor::KColor( const KColor &col)
: QColor( col )
{
  h = col.h; s = col.s; v = col.v;
  r = col.r; g = col.g; b = col.b;
};

KColor::KColor( const QColor &col)
: QColor( col )
{
  QColor::rgb(&r, &g, &b);
  QColor::hsv(&h, &s, &v);
};

bool KColor::operator==(const KColor& col) const
{
  return (h == col.h) && (s == col.s) && (v == col.v) &&
         (r == col.r) && (g == col.g) && (b == col.b);
}

KColor& KColor::operator=(const KColor& col)
{
  *(QColor *)this = col;
  h = col.h; s = col.s; v = col.v;
  r = col.r; g = col.g; b = col.b;
  return *this;
}

void
KColor::setHsv(int _h, int _s, int _v)
{
  h = _h; s = _s; v = _v;
  QColor::setHsv(h, s, v);
  QColor::rgb(&r, &g, &b);
};

void
KColor::setRgb(int _r, int _g, int _b)
{
  r = _r; g = _g; b = _b;
  QColor::setRgb(r, g, b);
  QColor::hsv(&h, &s, &v);
};

void
KColor::rgb(int *_r, int *_g, int *_b)
{
  *_r = r; *_g = g; *_b = b;
};

void
KColor::hsv(int *_h, int *_s, int *_v)
{
  *_h = h; *_s = s; *_v = v;
};


static QColor *standardPalette = 0;
static KStaticDeleter<QColor> spd;

static void createStandardPalette()
{
    if ( standardPalette )
	return;

    standardPalette = spd.setObject(new QColor [STANDARD_PAL_SIZE], true);

    int i = 0;

    standardPalette[i++] = Qt::red;
    standardPalette[i++] = Qt::green;
    standardPalette[i++] = Qt::blue;
    standardPalette[i++] = Qt::cyan;
    standardPalette[i++] = Qt::magenta;
    standardPalette[i++] = Qt::yellow;
    standardPalette[i++] = Qt::darkRed;
    standardPalette[i++] = Qt::darkGreen;
    standardPalette[i++] = Qt::darkBlue;
    standardPalette[i++] = Qt::darkCyan;
    standardPalette[i++] = Qt::darkMagenta;
    standardPalette[i++] = Qt::darkYellow;
    standardPalette[i++] = Qt::white;
    standardPalette[i++] = Qt::lightGray;
    standardPalette[i++] = Qt::gray;
    standardPalette[i++] = Qt::darkGray;
    standardPalette[i++] = Qt::black;
}


KHSSelector::KHSSelector( QWidget *parent, const char *name )
	: KXYSelector( parent, name )
{
	setRange( 0, 0, 359, 255 );
}

void KHSSelector::updateContents()
{
	drawPalette(&pixmap);
}

void KHSSelector::resizeEvent( QResizeEvent * )
{
	updateContents();
}

void KHSSelector::drawContents( QPainter *painter )
{
	painter->drawPixmap( contentsRect().x(), contentsRect().y(), pixmap );
}

void KHSSelector::drawPalette( QPixmap *pixmap )
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
		KImageEffect::dither( image, standardPalette, STANDARD_PAL_SIZE );
	}
	pixmap->convertFromImage( image );
}


//-----------------------------------------------------------------------------

KValueSelector::KValueSelector( QWidget *parent, const char *name )
	: KSelector( KSelector::Vertical, parent, name ), _hue(0), _sat(0)
{
	setRange( 0, 255 );
	pixmap.setOptimization( QPixmap::BestOptim );
}

KValueSelector::KValueSelector(Orientation o, QWidget *parent, const char *name )
	: KSelector( o, parent, name), _hue(0), _sat(0)
{
	setRange( 0, 255 );
	pixmap.setOptimization( QPixmap::BestOptim );
}

void KValueSelector::updateContents()
{
	drawPalette(&pixmap);
}

void KValueSelector::resizeEvent( QResizeEvent * )
{
	updateContents();
}

void KValueSelector::drawContents( QPainter *painter )
{
	painter->drawPixmap( contentsRect().x(), contentsRect().y(), pixmap );
}

void KValueSelector::drawPalette( QPixmap *pixmap)
{
	int xSize = contentsRect().width(), ySize = contentsRect().height();
	QImage image( xSize, ySize, 32 );
	QColor col;
	uint *p;
	QRgb rgb;

	if ( orientation() == KSelector::Horizontal )
	{
		for ( int v = 0; v < ySize; v++ )
		{
			p = (uint *) image.scanLine( ySize - v - 1 );

			for( int x = 0; x < xSize; x++ )
			{
				col.setHsv( _hue, _sat, 255*x/(xSize-1) );
				rgb = col.rgb();
				*p++ = rgb;
			}
		}
	}

	if( orientation() == KSelector::Vertical )
	{
		for ( int v = 0; v < ySize; v++ )
		{
			p = (uint *) image.scanLine( ySize - v - 1 );
			col.setHsv( _hue, _sat, 255*v/(ySize-1) );
			rgb = col.rgb();
			for ( int i = 0; i < xSize; i++ )
				*p++ = rgb;
		}
	}

	if ( QColor::numBitPlanes() <= 8 )
	{
		createStandardPalette();
		KImageEffect::dither( image, standardPalette, STANDARD_PAL_SIZE );
	}
	pixmap->convertFromImage( image );
}

//-----------------------------------------------------------------------------

KColorCells::KColorCells( QWidget *parent, int rows, int cols )
	: QTableView( parent )
{
	shade = true;
	setNumRows( rows );
	setNumCols( cols );
	colors = new QColor [ rows * cols ];

	for ( int i = 0; i < rows * cols; i++ )
		colors[i] = QColor();

	selected = 0;
        inMouse = false;

	// Drag'n'Drop
	setAcceptDrops( true);
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
        int w = 1;

	if (shade)
        {
		qDrawShadePanel( painter, 1, 1, cellWidth()-2,
		    cellHeight()-2, colorGroup(), TRUE, 1, &brush );
		w = 2;
        }
        QColor color = colors[ row * numCols() + col ];
        if (!color.isValid())
	{
		if (!shade) return;
		color = backgroundColor();
	}

	painter->setPen( color );
	painter->setBrush( QBrush( color ) );
	painter->drawRect( w, w, cellWidth()-w*2, cellHeight()-w*2 );

	if ( row * numCols() + col == selected )
		painter->drawWinFocusRect( w, w, cellWidth()-w*2, cellHeight()-w*2 );
}

void KColorCells::resizeEvent( QResizeEvent * )
{
	setCellWidth( width() / numCols() );
	setCellHeight( height() / numRows() );
}

void KColorCells::mousePressEvent( QMouseEvent *e )
{
    inMouse = true;
    mPos = e->pos();
}

int KColorCells::posToCell(const QPoint &pos, bool ignoreBorders)
{
   int row = pos.y() / cellHeight();
   int col = pos.x() / cellWidth();
   int cell = row * numCols() + col;

   if (!ignoreBorders)
   {
      int border = 2;
      int x = pos.x() - col * cellWidth();
      int y = pos.y() - row * cellHeight();
      if ( (x < border) || (x > cellWidth()-border) ||
           (y < border) || (y > cellHeight()-border))
         return -1;
   }
   return cell;
}

void KColorCells::mouseMoveEvent( QMouseEvent *e )
{
    if( !(e->state() && LeftButton)) return;

    if(inMouse) {
        int delay = KGlobalSettings::dndEventDelay();
        if(e->x() > mPos.x()+delay || e->x() < mPos.x()-delay ||
           e->y() > mPos.y()+delay || e->y() < mPos.y()-delay){
            // Drag color object
            int cell = posToCell(mPos);
            if ((cell != -1) && colors[cell].isValid())
            {
               KColorDrag *d = KColorDrag::makeDrag( colors[cell], this);
               d->dragCopy();
            }
        }
    }
}

void KColorCells::dragEnterEvent( QDragEnterEvent *event)
{
     event->accept( acceptDrags && KColorDrag::canDecode( event));
}

void KColorCells::dropEvent( QDropEvent *event)
{
     QColor c;
     if( KColorDrag::decode( event, c)) {
          int cell = posToCell(event->pos(), true);
	  setColor(cell,c);
     }
}

void KColorCells::mouseReleaseEvent( QMouseEvent *e )
{
	int cell = posToCell(mPos);
        int currentCell = posToCell(e->pos());

        // If we release the mouse in another cell and we don't have
        // a drag we should ignore this event.
        if (currentCell != cell)
           cell = -1;

	if ( (cell != -1) && (selected != cell) )
	{
		int prevSel = selected;
		selected = cell;
		updateCell( prevSel/numCols(), prevSel%numCols(), FALSE );
		updateCell( cell/numCols(), cell%numCols(), FALSE );
        }

        inMouse = false;
        if (cell != -1)
	    emit colorSelected( cell );
}

//-----------------------------------------------------------------------------

KColorPatch::KColorPatch( QWidget *parent ) : QFrame( parent )
{
	setFrameStyle( QFrame::Panel | QFrame::Sunken );
	colContext = 0;
	setAcceptDrops( true);
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

void KColorPatch::mouseMoveEvent( QMouseEvent *e )
{
        // Drag color object
        if( !(e->state() && LeftButton)) return;
	KColorDrag *d = KColorDrag::makeDrag( color, this);
	d->dragCopy();
}

void KColorPatch::dragEnterEvent( QDragEnterEvent *event)
{
     event->accept( KColorDrag::canDecode( event));
}

void KColorPatch::dropEvent( QDropEvent *event)
{
     QColor c;
     if( KColorDrag::decode( event, c)) {
	  setColor( c);
	  emit colorChanged( c);
     }
}


KPaletteTable::KPaletteTable( QWidget *parent, int minWidth, int cols)
	: QWidget( parent ), mMinWidth(minWidth), mCols(cols)
{
  cells = 0;
  mPalette = 0;
  i18n_customColors = i18n("* Custom Colors *");
  i18n_recentColors = i18n("* Recent Colors *");
  i18n_namedColors  = i18n("Named Colors");

  QStringList paletteList = KPalette::getPaletteList();
  paletteList.remove(customColors);
  paletteList.remove(recentColors);
  paletteList.prepend(i18n_customColors);
  paletteList.prepend(i18n_recentColors);
  paletteList.append( i18n_namedColors );

  QVBoxLayout *layout = new QVBoxLayout( this );

  combo = new QComboBox( false, this );
  combo->insertStringList( paletteList );
  layout->addWidget(combo);

  sv = new QScrollView( this );
  QSize cellSize = QSize( mMinWidth, 120);
  sv->setHScrollBarMode( QScrollView::AlwaysOff);
  sv->setVScrollBarMode( QScrollView::AlwaysOn);
  QSize minSize = QSize(sv->verticalScrollBar()->width(), 0);
  minSize += QSize(sv->frameWidth(), 0);
  minSize += QSize(cellSize);
  sv->setFixedSize(minSize);
  layout->addWidget(sv);

  mNamedColorList = new KListBox( this, "namedColorList", 0 );
  mNamedColorList->setFixedSize(minSize);
  mNamedColorList->hide();
  layout->addWidget(mNamedColorList);
  connect( mNamedColorList, SIGNAL(highlighted( const QString & )),
	   this, SLOT( slotColorTextSelected( const QString & )) );

  setFixedSize( sizeHint());
  connect( combo, SIGNAL(activated(const QString &)),
	this, SLOT(slotSetPalette( const QString &)));
}

KPaletteTable::~KPaletteTable()
{
   delete mPalette;
}

QString
KPaletteTable::palette()
{
  return combo->currentText();
}


static const char **namedColorFilePath( void )
{
  //
  // 2000-02-05 Espen Sand.
  // Add missing filepaths here. Make sure the last entry is 0!
  //
  static const char *path[] =
  {
#ifdef X11_RGBFILE
    X11_RGBFILE,
#endif
    "/usr/X11R6/lib/X11/rgb.txt",
    "/usr/openwin/lib/X11/rgb.txt", // for Solaris.
    0
  };
  return( path );
}




void
KPaletteTable::readNamedColor( void )
{
  if( mNamedColorList->count() != 0 )
  {
    return; // Strings already present
  }

  //
  // Code somewhat inspired by KPalette.
  //

  const char **path = namedColorFilePath();
  for( int i=0; path[i] != 0; i++ )
  {
    QFile paletteFile( path[i] );
    if( paletteFile.open( IO_ReadOnly ) == false )
    {
      continue;
    }

    QString line;
    QStringList list;
    while( paletteFile.readLine( line, 100 ) != -1 )
    {
      int red, green, blue;
      int pos = 0;

      if( sscanf(line.ascii(), "%d %d %d%n", &red, &green, &blue, &pos ) == 3 )
      {
	//
	// Remove duplicates. Every name with a space and every name
	// that start with "gray".
	//
	QString name = line.mid(pos).stripWhiteSpace();
	if( name.isNull() == true || name.find(' ') != -1 ||
	    name.find( "gray" ) != -1 )
	{
	  continue;
	}
	list.append( name );
      }
    }

    list.sort();
    mNamedColorList->insertStringList( list );
    break;
  }

  if( mNamedColorList->count() == 0 )
  {
    //
    // Give the error dialog box a chance to center above the
    // widget (or dialog). If we had displayed it now we could get a
    // situation where the (modal) error dialog box pops up first
    // preventing the real dialog to become visible until the
    // error dialog box is removed (== bad UI).
    //
    QTimer::singleShot( 10, this, SLOT(slotShowNamedColorReadError()) );
  }
}


void
KPaletteTable::slotShowNamedColorReadError( void )
{
  if( mNamedColorList->count() == 0 )
  {
    QString msg = i18n(""
      "Unable to read X11 rgb color strings. The following\n"
      "file location(s) were examined:\n\n");

    const char **path = namedColorFilePath();
    for( int i=0; path[i] != 0; i++ )
    {
      msg += path[i];
      msg += "\n";
    }
    KMessageBox::sorry( this, msg );
  }
}


//
// 2000-02-12 Espen Sand
// Set the color in two steps. The setPalette() slot will not emit a signal
// with the current color setting. The reason is that setPalette() is used
// by the color selector dialog on startup. In the color selector dialog
// we normally want to display a startup color which we specify
// when the dialog is started. The slotSetPalette() slot below will
// set the palette and then use the information to emit a signal with the
// new color setting. It is only used by the combobox widget.
//
void
KPaletteTable::slotSetPalette( const QString &_paletteName )
{
  setPalette( _paletteName );
  if( mNamedColorList->isVisible() == true )
  {
    int item = mNamedColorList->currentItem();
    mNamedColorList->setCurrentItem( item < 0 ? 0 : item );
    slotColorTextSelected( mNamedColorList->currentText() );
  }
  else
  {
    slotColorCellSelected(0); // FIXME: We need to save the current value!!
  }
}


void
KPaletteTable::setPalette( const QString &_paletteName )
{
  QString paletteName( _paletteName);
  if (paletteName.isEmpty())
     paletteName = i18n_recentColors;

  if (combo->currentText() != paletteName)
  {
     bool found = false;
     for(int i = 0; i < combo->count(); i++)
     {
        if (combo->text(i) == paletteName)
        {
           combo->setCurrentItem(i);
           found = true;
           break;
        }
     }
     if (!found)
     {
        combo->insertItem(paletteName);
        combo->setCurrentItem(combo->count()-1);
     }
  }

  if (paletteName == i18n_customColors)
     paletteName = customColors;
  else if (paletteName == i18n_recentColors)
     paletteName = recentColors;


  //
  // 2000-02-12 Espen Sand
  // The palette mode "i18n_namedColors" does not use the KPalette class.
  // In fact, 'mPalette' and 'cells' are 0 when in this mode. The reason
  // for this is maninly that KPalette reads from and writes to files using
  // "locate()". The colors used in "i18n_namedColors" mode comes from the
  // X11 diretory and is not writable. I dont think this fit in KPalette.
  //
  if( mPalette == 0 || mPalette->name() != paletteName )
  {
    if( paletteName == i18n_namedColors )
    {
      sv->hide();
      mNamedColorList->show();
      readNamedColor();

      delete cells; cells = 0;
      delete mPalette; mPalette = 0;
    }
    else
    {
      mNamedColorList->hide();
      sv->show();

      delete cells;
      delete mPalette;
      mPalette = new KPalette(paletteName);
      int rows = (mPalette->nrColors()+mCols-1) / mCols;
      if (rows < 1) rows = 1;
      cells = new KColorCells( sv->viewport(), rows, mCols);
      cells->setShading(false);
      cells->setAcceptDrags(false);
      QSize cellSize = QSize( mMinWidth, mMinWidth * rows / mCols);
      cells->setFixedSize( cellSize );
      for( int i = 0; i < mPalette->nrColors(); i++)
      {
        cells->setColor( i, mPalette->color(i) );
      }
      connect( cells, SIGNAL( colorSelected( int ) ),
	       SLOT( slotColorCellSelected( int ) ) );
      sv->addChild( cells );
      cells->show();
      sv->updateScrollBars();
    }
  }
}



void
KPaletteTable::slotColorCellSelected( int col )
{
  if (!mPalette || (col >= mPalette->nrColors()))
     return;
  emit colorSelected( mPalette->color(col), mPalette->colorName(col) );
}

void
KPaletteTable::slotColorTextSelected( const QString &colorText )
{
  emit colorSelected( QColor (colorText), colorText );
}


void
KPaletteTable::addToCustomColors( const QColor &color)
{
  setPalette(i18n_customColors);
  mPalette->addColor( color );
  mPalette->save();
  delete mPalette;
  mPalette = 0;
  setPalette(i18n_customColors);
}

void
KPaletteTable::addToRecentColors( const QColor &color)
{
  //
  // 2000-02-12 Espen Sand.
  // The 'mPalette' is always 0 when current mode is i18n_namedColors
  //
  bool recentIsSelected = false;
  if ( mPalette != 0 && mPalette->name() == recentColors)
  {
     delete mPalette;
     mPalette = 0;
     recentIsSelected = true;
  }
  KPalette *recentPal = new KPalette(recentColors);
  if (recentPal->findColor(color) == -1)
  {
     recentPal->addColor( color );
     recentPal->save();
  }
  delete recentPal;
  if (recentIsSelected)
     setPalette(i18n_recentColors);
}

class KColorDialog::KColorDialogPrivate {
public:
    KPaletteTable *table;
    bool bRecursion;
    bool bEditRgb;
    bool bEditHsv;
    bool bEditHtml;
    bool bColorPicking;
    QLabel *colorName;
    QLineEdit *htmlName;
    QLineEdit *hedit;
    QLineEdit *sedit;
    QLineEdit *vedit;
    QLineEdit *redit;
    QLineEdit *gedit;
    QLineEdit *bedit;
    KColorPatch *patch;
    KHSSelector *hsSelector;
    KPalette *palette;
    KValueSelector *valuePal;
    KColor selColor;
    QX11EventFilter oldfilter;
};


KColorDialog::KColorDialog( QWidget *parent, const char *name, bool modal )
  :KDialogBase( parent, name, modal, i18n("Select Color"),
		modal ? Help|Ok|Cancel : Help|Close,
		Ok, true )
{
  d = new KColorDialogPrivate;
  d->bRecursion = true;
  d->bColorPicking = false;
  d->oldfilter = 0;
  setHelp( QString::fromLatin1("kcolordialog.html"), QString::null );
  connect( this, SIGNAL(okClicked(void)),this,SLOT(slotWriteSettings(void)));
  connect( this, SIGNAL(closeClicked(void)),this,SLOT(slotWriteSettings(void)));

  QLabel *label;

  //
  // Create the top level page and its layout
  //
  QWidget *page = new QWidget( this );
  setMainWidget( page );

  QGridLayout *tl_layout = new QGridLayout( page, 3, 3, 0, spacingHint() );
  tl_layout->addColSpacing( 1, spacingHint() * 2 );

  //
  // the more complicated part: the right side
  // add a V-box
  //
  QVBoxLayout *l_left = new QVBoxLayout();
  tl_layout->addLayout(l_left, 0, 0);

  //
  // add a H-Box for the XY-Selector and a grid for the
  // entry fields
  //
  QHBoxLayout *l_ltop = new QHBoxLayout();
  l_left->addLayout(l_ltop);

  // a little space between
  l_left->addSpacing(10);

  QGridLayout *l_lbot = new QGridLayout(3, 6);
  l_left->addLayout(l_lbot);

  //
  // the palette and value selector go into the H-box
  //
  d->hsSelector = new KHSSelector( page );
  d->hsSelector->setMinimumSize(140, 70);
  l_ltop->addWidget(d->hsSelector, 8);
  connect( d->hsSelector, SIGNAL( valueChanged( int, int ) ),
	   SLOT( slotHSChanged( int, int ) ) );

  d->valuePal = new KValueSelector( page );
  d->valuePal->setMinimumSize(26, 70);
  l_ltop->addWidget(d->valuePal, 1);
  connect( d->valuePal, SIGNAL( valueChanged( int ) ),
	   SLOT( slotVChanged( int ) ) );


  //
  // add the HSV fields
  //
  label = new QLabel( QString::fromLatin1("H:"), page );
  label->setAlignment(AlignRight | AlignVCenter);
  l_lbot->addWidget(label, 0, 2);
  d->hedit = new QLineEdit( page );
  d->hedit->setValidator( new QIntValidator( d->hedit ) );
  l_lbot->addWidget(d->hedit, 0, 3);
  connect( d->hedit, SIGNAL( textChanged(const QString &) ),SLOT( slotHSVChanged() ) );

  label = new QLabel( QString::fromLatin1("S:"), page );
  label->setAlignment(AlignRight | AlignVCenter);
  l_lbot->addWidget(label, 1, 2);
  d->sedit = new QLineEdit( page );
  d->sedit->setValidator( new QIntValidator( d->sedit ) );
  l_lbot->addWidget(d->sedit, 1, 3);
  connect( d->sedit, SIGNAL( textChanged(const QString &) ),SLOT( slotHSVChanged() ) );

  label = new QLabel( QString::fromLatin1("V:"), page );
  label->setAlignment(AlignRight | AlignVCenter);
  l_lbot->addWidget(label, 2, 2);
  d->vedit = new QLineEdit( page );
  d->vedit->setValidator( new QIntValidator( d->vedit ) );
  l_lbot->addWidget(d->vedit, 2, 3);
  connect( d->vedit, SIGNAL( textChanged(const QString &) ),SLOT( slotHSVChanged() ) );

  //
  // add the RGB fields
  //
  label = new QLabel( QString::fromLatin1("R:"), page );
  label->setAlignment(AlignRight | AlignVCenter);
  l_lbot->addWidget(label, 0, 4);
  d->redit = new QLineEdit( page );
  d->redit->setValidator( new QIntValidator( d->redit ) );
  l_lbot->addWidget(d->redit, 0, 5);
  connect( d->redit, SIGNAL( textChanged(const QString &) ), SLOT( slotRGBChanged() ) );

  label = new QLabel( QString::fromLatin1("G:"), page );
  label->setAlignment(AlignRight | AlignVCenter);
  l_lbot->addWidget( label, 1, 4);
  d->gedit = new QLineEdit( page );
  d->gedit->setValidator( new QIntValidator( d->gedit ) );
  l_lbot->addWidget(d->gedit, 1, 5);
  connect( d->gedit, SIGNAL( textChanged(const QString &) ), SLOT( slotRGBChanged() ) );

  label = new QLabel( QString::fromLatin1("B:"), page );
  label->setAlignment(AlignRight | AlignVCenter);
  l_lbot->addWidget(label, 2, 4);
  d->bedit = new QLineEdit( page );
  d->bedit->setValidator( new QIntValidator( d->bedit ) );
  l_lbot->addWidget(d->bedit, 2, 5);
  connect( d->bedit, SIGNAL( textChanged(const QString &) ), SLOT( slotRGBChanged() ) );

  //
  // the entry fields should be wide enought to hold 88888
  //
  int w = d->hedit->fontMetrics().width(QString::fromLatin1("888888"));
  d->hedit->setFixedWidth(w);
  d->sedit->setFixedWidth(w);
  d->vedit->setFixedWidth(w);

  d->redit->setFixedWidth(w);
  d->gedit->setFixedWidth(w);
  d->bedit->setFixedWidth(w);

  //
  // add a layout for left-side (colors)
  //
  QVBoxLayout *l_right = new QVBoxLayout;
  tl_layout->addLayout(l_right, 0, 2);

  //
  // Add the palette table
  //
  d->table = new KPaletteTable( page );
  l_right->addWidget(d->table, 10);

  connect( d->table, SIGNAL( colorSelected( const QColor &, const QString & ) ),
	   SLOT( slotColorSelected( const QColor &, const QString & ) ) );

  //
  // a little space between
  //
  l_right->addSpacing(10);

  QHBoxLayout *l_hbox = new QHBoxLayout( l_right );

  //
  // The add to custom colors button
  //
  QPushButton *button = new QPushButton( page );
  button->setText(i18n("&Add to Custom Colors"));
  l_hbox->addWidget(button, 0, AlignLeft);
  connect( button, SIGNAL( clicked()),
           SLOT( slotAddToCustomColors()));

  //
  // The color picker button
  //
  button = new QPushButton( page );
  button->setPixmap( BarIcon("colorpicker"));
  l_hbox->addWidget(button, 0, AlignHCenter );
  connect( button, SIGNAL( clicked()),
           SLOT( slotColorPicker()));

  //
  // a little space between
  //
  l_right->addSpacing(10);

  //
  // and now the entry fields and the patch
  //
  QGridLayout *l_grid = new QGridLayout( l_right, 2, 3);

  l_grid->setColStretch(2, 1);

  label = new QLabel( page );
  label->setText(i18n("Name:"));
  l_grid->addWidget(label, 0, 1, AlignLeft);

  d->colorName = new QLabel( page );
  w = d->colorName->fontMetrics().width(QString::fromLatin1("Very Very long name")); // 7xF!
  d->colorName->setFixedWidth(w);
  l_grid->addWidget(d->colorName, 0, 2, AlignLeft);

  label = new QLabel( page );
  label->setText(i18n("HTML:"));
  l_grid->addWidget(label, 1, 1, AlignLeft);

  d->htmlName = new QLineEdit( page );
  d->htmlName->setMaxLength( 7 );
  d->htmlName->setText("#FFFFFF");
  w = d->htmlName->fontMetrics().width(QString::fromLatin1("#FFFFFFFF")); // 8xF!
  d->htmlName->setFixedWidth(w);
  l_grid->addWidget(d->htmlName, 1, 2, AlignLeft);
  connect( d->htmlName, SIGNAL( textChanged(const QString &) ), SLOT( slotHtmlChanged() ) );

  d->patch = new KColorPatch( page );
  d->patch->setFixedSize(48, 48);
  l_grid->addMultiCellWidget(d->patch, 0, 1, 0, 0, AlignHCenter | AlignVCenter);
  connect( d->patch, SIGNAL( colorChanged( const QColor&)),
	   SLOT( setColor( const QColor&)));

  tl_layout->activate();
  page->setMinimumSize( page->sizeHint() );

  readSettings();
  d->bRecursion = false;
  d->bEditHsv = false;
  d->bEditRgb = false;
  d->bEditHtml = false;

  disableResize();
}

KColorDialog::~KColorDialog()
{
    if (d->bColorPicking)
        qt_set_x11_event_filter(d->oldfilter);
    delete d;
}

void
KColorDialog::readSettings()
{
  KConfig* config = KGlobal::config();

  QString oldgroup = config->group();

  config->setGroup("Colors");
  QString palette = config->readEntry("CurrentPalette");
  d->table->setPalette(palette);

  config->setGroup( oldgroup );
}

void
KColorDialog::slotWriteSettings()
{
  KConfig* config = KGlobal::config();

  QString oldgroup = config->group();

  config->setGroup("Colors");
  config->writeEntry("CurrentPalette", d->table->palette() );

  config->setGroup( oldgroup );
}

QColor
KColorDialog::color()
{
  d->table->addToRecentColors( d->selColor );
  return d->selColor;
}

void KColorDialog::setColor( const QColor &col )
{
  _setColor( col );
}

//
// static function to display dialog and return color
//
int KColorDialog::getColor( QColor &theColor, QWidget *parent )
{
  KColorDialog dlg( parent, "Color Selector", TRUE );
  if ( theColor.isValid() )
    dlg.setColor( theColor );
  int result = dlg.exec();

  if ( result == Accepted )
  {
    theColor = dlg.color();
  }

  return result;
}


void KColorDialog::slotRGBChanged( void )
{
  if (d->bRecursion) return;
  int red = d->redit->text().toInt();
  int grn = d->gedit->text().toInt();
  int blu = d->bedit->text().toInt();

  if ( red > 255 || red < 0 ) return;
  if ( grn > 255 || grn < 0 ) return;
  if ( blu > 255 || blu < 0 ) return;

  KColor col;
  col.setRgb( red, grn, blu );
  d->bEditRgb = true;
  _setColor( col );
  d->bEditRgb = false;
}

void KColorDialog::slotHtmlChanged( void )
{
  if (d->bRecursion || d->htmlName->text().isEmpty()) return;

  unsigned int red = 256;
  unsigned int grn = 256;
  unsigned int blu = 256;

  if (sscanf(d->htmlName->text().latin1(), "#%02x%02x%02x", &red, &grn, &blu)!=3)
      return;

  if ( red > 255 || grn > 255 || blu > 255) return;

  KColor col;
  col.setRgb( red, grn, blu );
  d->bEditHtml = true;
  _setColor( col );
  d->bEditHtml = false;
}

void KColorDialog::slotHSVChanged( void )
{
  if (d->bRecursion) return;
  int hue = d->hedit->text().toInt();
  int sat = d->sedit->text().toInt();
  int val = d->vedit->text().toInt();

  if ( hue > 359 || hue < 0 ) return;
  if ( sat > 255 || sat < 0 ) return;
  if ( val > 255 || val < 0 ) return;

  KColor col;
  col.setHsv( hue, sat, val );
  d->bEditHsv = true;
  _setColor( col );
  d->bEditHsv = false;
}

void KColorDialog::slotHSChanged( int h, int s )
{
  int _h, _s, v;
  d->selColor.hsv(&_h, &_s, &v);
  if (v < 1)
     v = 1;
  KColor col;
  col.setHsv( h, s, v );
  _setColor( col );
}

void KColorDialog::slotVChanged( int v )
{
  int h, s, _v;
  d->selColor.hsv(&h, &s, &_v);
  KColor col;
  col.setHsv( h, s, v );
  _setColor( col );
}

void KColorDialog::slotColorSelected( const QColor &color )
{
  _setColor( color );
}

void KColorDialog::slotAddToCustomColors( )
{
  d->table->addToCustomColors( d->selColor );
}

void KColorDialog::slotColorSelected( const QColor &color, const QString &name )
{
  _setColor( color, name);
}

void KColorDialog::_setColor(const KColor &color, const QString &name)
{
  if (color == d->selColor) return;

  d->bRecursion = true;
  d->selColor = color;

  if (name.isEmpty())
     d->colorName->setText( i18n("-unnamed-"));
  else
     d->colorName->setText( name );

  d->patch->setColor( d->selColor );

  setRgbEdit();
  setHsvEdit();
  setHtmlEdit();

  int h, s, v;
  d->selColor.hsv( &h, &s, &v );
  d->hsSelector->setValues( h, s );
  d->valuePal->setHue( h );
  d->valuePal->setSaturation( s );
  d->valuePal->updateContents();
  d->valuePal->repaint( FALSE );
  d->valuePal->setValue( v );
  d->bRecursion = false;

  emit colorSelected( d->selColor );
}

static QWidget *kde_color_dlg_widget = 0;

int count = 0;
int kde_color_dlg_handler(XEvent *event)
{
    if (event->type == ButtonRelease)
    {
        QMouseEvent e( QEvent::MouseButtonRelease, QPoint(),
                       QPoint(event->xmotion.x_root, event->xmotion.y_root) , 0, 0 );
        QApplication::sendEvent( kde_color_dlg_widget, &e );
        return TRUE;
    }
    return FALSE;
}

void
KColorDialog::slotColorPicker()
{
    d->bColorPicking = true;
    d->oldfilter = qt_set_x11_event_filter(kde_color_dlg_handler);
    kde_color_dlg_widget = this;
    grabMouse( crossCursor );
    grabKeyboard();
}

void
KColorDialog::mouseReleaseEvent( QMouseEvent *e )
{
  if (d->bColorPicking)
  {
     d->bColorPicking = false;
     qt_set_x11_event_filter(d->oldfilter);
     d->oldfilter = 0;
     releaseMouse();
     releaseKeyboard();
     _setColor( grabColor( e->globalPos() ) );
     return;
  }
  KDialogBase::mouseReleaseEvent( e );
}

QColor
KColorDialog::grabColor(const QPoint &p)
{
    QWidget *desktop = QApplication::desktop();
    QPixmap pm = QPixmap::grabWindow( desktop->winId(), p.x(), p.y(), 1, 1);
    QImage i = pm.convertToImage();
    return i.pixel(0,0);
}

void
KColorDialog::keyPressEvent( QKeyEvent *e )
{
  if (d->bColorPicking)
  {
     if (e->key() == Key_Escape)
     {
        d->bColorPicking = false;
        qt_set_x11_event_filter(d->oldfilter);
        d->oldfilter = 0;
        releaseMouse();
        releaseKeyboard();
     }
     e->accept();
     return;
  }
  KDialogBase::keyPressEvent( e );
}

void KColorDialog::setRgbEdit( void )
{
  if (d->bEditRgb) return;
  int r, g, b;
  d->selColor.rgb( &r, &g, &b );
  QString num;

  num.setNum( r );
  d->redit->setText( num );
  num.setNum( g );
  d->gedit->setText( num );
  num.setNum( b );
  d->bedit->setText( num );
}

void KColorDialog::setHtmlEdit( void )
{
  if (d->bEditHtml) return;
  int r, g, b;
  d->selColor.rgb( &r, &g, &b );
  QString num;

  num.sprintf("#%02X%02X%02X", r,g,b);
  d->htmlName->setText( num );
}


void KColorDialog::setHsvEdit( void )
{
  if (d->bEditHsv) return;
  int h, s, v;
  d->selColor.hsv( &h, &s, &v );
  QString num;

  num.setNum( h );
  d->hedit->setText( num );
  num.setNum( s );
  d->sedit->setText( num );
  num.setNum( v );
  d->vedit->setText( num );
}

#include "kcolordialog.moc"

