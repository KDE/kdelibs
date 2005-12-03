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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
//-----------------------------------------------------------------------------
// KDE color selection dialog.
//
// 1999-09-27 Espen Sand <espensa@online.no>
// KColorDialog is now subclassed from KDialogBase. I have also extended
// KColorDialog::getColor() so that it contains a parent argument. This
// improves centering capability.
//
// layout management added Oct 1997 by Mario Weilguni
// <mweilguni@sime.com>
//

#include <stdio.h>
#include <stdlib.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdesktopwidget.h>
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
#include <qspinbox.h>
#include <qtimer.h>

#include <kapplication.h>
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

#include "kcolordialog.h"
#include "kcolormimedata.h"
#include "kstaticdeleter.h"
#include <config.h>
#include <kdebug.h>

#include "config.h"
#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

struct ColorPaletteNameType
{
    const char* m_fileName;
    const char* m_displayName;
};



const ColorPaletteNameType colorPaletteName[]=
{
    { "Recent_Colors", I18N_NOOP2( "palette name", "* Recent Colors *" ) },
    { "Custom_Colors", I18N_NOOP2( "palette name", "* Custom Colors *" ) },
    { "40.colors",     I18N_NOOP2( "palette name", "Forty Colors" ) },
    { "Rainbow.colors",I18N_NOOP2( "palette name", "Rainbow Colors" ) },
    { "Royal.colors",  I18N_NOOP2( "palette name", "Royal Colors" ) },
    { "Web.colors",    I18N_NOOP2( "palette name", "Web Colors" ) },
    { 0, 0 } // end of data
};

static const int recentColorIndex = 0;
static const int customColorIndex = 1;
static const int fortyColorIndex = 2;

class KColorSpinBox : public QSpinBox
{
public:
  KColorSpinBox(int minValue, int maxValue, int step, QWidget* parent)
   : QSpinBox(parent)
  { setRange(minValue,maxValue); setSingleStep(step);}


  // Override Qt's braindead auto-selection.
  //XXX KDE4 : check this is no more necessary , was disabled to port to Qt4 //mikmak
  /*
  virtual void valueChange()
  {
      updateDisplay();
      emit valueChanged( value() );
      emit valueChanged( currentValueText() );
  }*/

};


#define STANDARD_PAL_SIZE 17

KColor::KColor()
: QColor()
{
  r = 0; g = 0; b = 0; h = 0; s = 0; v = 0;
}

KColor::KColor( const KColor &col)
: QColor( col )
{
  h = col.h; s = col.s; v = col.v;
  r = col.r; g = col.g; b = col.b;
}

KColor::KColor( const QColor &col)
: QColor( col )
{
  QColor::getRgb(&r, &g, &b);
  QColor::getHsv(&h, &s, &v);
}

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
  QColor::getRgb(&r, &g, &b);
}

void
KColor::setRgb(int _r, int _g, int _b)
{
  r = _r; g = _g; b = _b;
  QColor::setRgb(r, g, b);
  QColor::getHsv(&h, &s, &v);
}

void
KColor::rgb(int *_r, int *_g, int *_b) const
{
  *_r = r; *_g = g; *_b = b;
}

void
KColor::hsv(int *_h, int *_s, int *_v) const
{
  *_h = h; *_s = s; *_v = v;
}


static QColor *standardPalette = 0;
static KStaticDeleter<QColor> spd;

static void createStandardPalette()
{
    if ( standardPalette )
	return;

    spd.setObject(standardPalette, new QColor [STANDARD_PAL_SIZE], true/*array*/);

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


KHSSelector::KHSSelector( QWidget *parent )
	: KXYSelector( parent )
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
	QImage image( QSize(xSize, ySize), QImage::Format_RGB32 );
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

	if ( pixmap->depth() <= 8 )
	{
		createStandardPalette();
		KImageEffect::dither( image, standardPalette, STANDARD_PAL_SIZE );
	}
	*pixmap=QPixmap::fromImage( image );
}


//-----------------------------------------------------------------------------

KValueSelector::KValueSelector( QWidget *parent )
	: KSelector( Qt::Vertical, parent ), _hue(0), _sat(0)
{
	setRange( 0, 255 );
}

KValueSelector::KValueSelector(Qt::Orientation o, QWidget *parent )
	: KSelector( o, parent ), _hue(0), _sat(0)
{
	setRange( 0, 255 );
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

void KValueSelector::drawPalette( QPixmap *pixmap )
{
	int xSize = contentsRect().width(), ySize = contentsRect().height();
	QImage image( QSize(xSize, ySize), QImage::Format_RGB32 );
	QColor col;
	uint *p;
	QRgb rgb;

	if ( orientation() == Qt::Horizontal )
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

	if( orientation() == Qt::Vertical )
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

	if ( pixmap->depth() <= 8 )
	{
		createStandardPalette();
		KImageEffect::dither( image, standardPalette, STANDARD_PAL_SIZE );
	}
	*pixmap=QPixmap::fromImage( image );
}

//-----------------------------------------------------------------------------

KColorCells::KColorCells( QWidget *parent, int rows, int cols )
	: Q3GridView( parent )
{
	setFrameShape(QFrame::NoFrame);
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

	setHScrollBarMode( AlwaysOff );
	setVScrollBarMode( AlwaysOff );
	viewport()->setBackgroundMode( Qt::PaletteBackground );
	setBackgroundMode( Qt::PaletteBackground );
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
		    cellHeight()-2, colorGroup(), true, 1, &brush );
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

	if ( row * numCols() + col == selected ) {
		painter->setCompositionMode( QPainter::CompositionMode_Xor );
		painter->drawRect( w, w, cellWidth()-w*2, cellHeight()-w*2 );
	//	painter->drawWinFocusRect( w, w, cellWidth()-w*2, cellHeight()-w*2 );
	}
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
    if( !(e->buttons() & Qt::LeftButton)) return;

    if(inMouse) {
        int delay = KGlobalSettings::dndEventDelay();
        if(e->x() > mPos.x()+delay || e->x() < mPos.x()-delay ||
           e->y() > mPos.y()+delay || e->y() < mPos.y()-delay){
            // Drag color object
            int cell = posToCell(mPos);
            if ((cell != -1) && colors[cell].isValid())
            {
               KColorMimeData::createDrag( colors[cell], this)->start();
            }
        }
    }
}

void KColorCells::dragEnterEvent( QDragEnterEvent *event)
{
     event->setAccepted( acceptDrags && KColorMimeData::canDecode( event->mimeData()));
}

void KColorCells::dropEvent( QDropEvent *event)
{
     QColor c=KColorMimeData::fromMimeData(event->mimeData());
     if( c.isValid()) {
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
		updateCell( prevSel/numCols(), prevSel%numCols() );
		updateCell( cell/numCols(), cell%numCols() );
        }

        inMouse = false;
        if (cell != -1)
	    emit colorSelected( cell );
}

void KColorCells::mouseDoubleClickEvent( QMouseEvent * /*e*/ )
{
  int cell = posToCell(mPos);

  if (cell != -1)
    emit colorDoubleClicked( cell );
}


//-----------------------------------------------------------------------------

KColorPatch::KColorPatch( QWidget *parent ) : QFrame( parent )
{
	setFrameStyle( QFrame::Panel | QFrame::Sunken );
	setAcceptDrops( true);
}

KColorPatch::~KColorPatch()
{
}

void KColorPatch::setColor( const QColor &col )
{
	color.setRgb( col.rgb() );

	update();
}

void KColorPatch::paintEvent ( QPaintEvent* pe )
{
	QFrame::paintEvent( pe );
	QPainter painter( this );
	painter.setPen( color );
	painter.setBrush( QBrush( color ) );
	painter.drawRect( contentsRect() );
}

void KColorPatch::mouseMoveEvent( QMouseEvent *e )
{
        // Drag color object
        if( !(e->buttons() & Qt::LeftButton)) return;
	KColorMimeData::createDrag( color, this)->start();
}

void KColorPatch::dragEnterEvent( QDragEnterEvent *event)
{
     event->setAccepted( KColorMimeData::canDecode( event->mimeData()));
}

void KColorPatch::dropEvent( QDropEvent *event)
{
     QColor c=KColorMimeData::fromMimeData(event->mimeData());
     if(c.isValid()) {
	  setColor( c);
	  emit colorChanged( c);
     }
}

class KPaletteTable::KPaletteTablePrivate
{
public:
    QMap<QString,QColor> m_namedColorMap;
};

KPaletteTable::KPaletteTable( QWidget *parent, int minWidth, int cols)
    : QWidget( parent ), cells(0), mPalette(0), mMinWidth(minWidth), mCols(cols)
{
  d = new KPaletteTablePrivate;

  i18n_namedColors  = i18n("Named Colors");

  QStringList diskPaletteList = KPalette::getPaletteList();
  QStringList paletteList;

  // We must replace the untranslated file names by translate names (of course only for KDE's standard palettes)
  for ( int i = 0; colorPaletteName[i].m_fileName; ++i )
  {
      diskPaletteList.removeAll( colorPaletteName[i].m_fileName );
      paletteList.append( i18n( "palette name", colorPaletteName[i].m_displayName ) );
  }
  paletteList += diskPaletteList;
  paletteList.append( i18n_namedColors );

  QVBoxLayout *layout = new QVBoxLayout( this );

  combo = new QComboBox( this );
  combo->setEditable(false);
  combo->addItems( paletteList );
  layout->addWidget(combo);

  sv = new Q3ScrollView( this );
  QSize cellSize = QSize( mMinWidth, 120);
  sv->setHScrollBarMode( Q3ScrollView::AlwaysOff);
  sv->setVScrollBarMode( Q3ScrollView::AlwaysOn);
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
   delete d;
}

QString
KPaletteTable::palette() const
{
  return combo->currentText();
}


static const char * const *namedColorFilePath( void )
{
  //
  // 2000-02-05 Espen Sand.
  // Add missing filepaths here. Make sure the last entry is 0!
  //
  static const char * const path[] =
  {
#ifdef X11_RGBFILE
    X11_RGBFILE,
#endif
    "/usr/X11R6/lib/X11/rgb.txt",
    "/usr/openwin/lib/X11/rgb.txt", // for Solaris.
    0
  };
  return path;
}




void
KPaletteTable::readNamedColor( void )
{
  if( mNamedColorList->count() != 0 )
  {
    return; // Strings already present
  }

  KGlobal::locale()->insertCatalog("kdelibs_colors");

  //
  // Code somewhat inspired by KPalette.
  //

  const char * const *path = namedColorFilePath();
  for( int i=0; path[i]; ++i )
  {
    QFile paletteFile( path[i] );
    if( !paletteFile.open( QIODevice::ReadOnly ) )
    {
      continue;
    }

    QByteArray line;
    QStringList list;
    while( !paletteFile.atEnd() )
    {
      line = paletteFile.readLine();

      int red, green, blue;
      int pos = 0;

      if( sscanf(line, "%d %d %d%n", &red, &green, &blue, &pos ) == 3 )
      {
	//
	// Remove duplicates. Every name with a space and every name
	// that start with "gray".
	//
	QString name = line.mid(pos).trimmed();
	QByteArray s1 = line.mid(pos);
	if( name.isNull() || name.indexOf(' ') != -1 ||
	    name.indexOf( "gray" ) != -1 ||  name.indexOf( "grey" ) != -1 )
	{
	  continue;
	}

        const QColor color ( red, green, blue );
        if ( color.isValid() )
        {
            const QString colorName( i18n("color", name.toLatin1().data()) );
            list.append( colorName );
            d->m_namedColorMap[ colorName ] = color;
        }
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
      "Unable to read X11 RGB color strings. The following "
      "file location(s) were examined:\n");

    const char * const *path = namedColorFilePath();
    for( int i=0; path[i]; ++i )
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
  if( mNamedColorList->isVisible() )
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

  if (combo->currentText() != paletteName)
  {
     bool found = false;
     for(int i = 0; i < combo->count(); i++)
     {
        if (combo->text(i) == paletteName)
        {
           combo->setCurrentIndex(i);
           found = true;
           break;
        }
     }
     if (!found)
     {
        combo->addItem(paletteName);
        combo->setCurrentIndex(combo->count()-1);
     }
  }

  // We must again find the file name of the palette from the eventual translation
  for ( int i = 0; colorPaletteName[i].m_fileName; ++i )
  {
      if ( paletteName == i18n( "palette name", colorPaletteName[i].m_displayName ) )
      {
          paletteName = colorPaletteName[i].m_fileName;
          break;
      }
  }


  //
  // 2000-02-12 Espen Sand
  // The palette mode "i18n_namedColors" does not use the KPalette class.
  // In fact, 'mPalette' and 'cells' are 0 when in this mode. The reason
  // for this is maninly that KPalette reads from and writes to files using
  // "locate()". The colors used in "i18n_namedColors" mode comes from the
  // X11 diretory and is not writable. I don't think this fit in KPalette.
  //
  if( !mPalette || mPalette->name() != paletteName )
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
      connect( cells, SIGNAL( colorDoubleClicked( int ) ),
	       SLOT( slotColorCellDoubleClicked( int ) ) );
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
KPaletteTable::slotColorCellDoubleClicked( int col )
{
  if (!mPalette || (col >= mPalette->nrColors()))
     return;
  emit colorDoubleClicked( mPalette->color(col), mPalette->colorName(col) );
}


void
KPaletteTable::slotColorTextSelected( const QString &colorText )
{
  emit colorSelected( d->m_namedColorMap[ colorText ], colorText );
}


void
KPaletteTable::addToCustomColors( const QColor &color)
{
  setPalette(i18n("palette name",colorPaletteName[customColorIndex].m_displayName));
  mPalette->addColor( color );
  mPalette->save();
  delete mPalette;
  mPalette = 0;
  setPalette(i18n("palette name",colorPaletteName[customColorIndex].m_displayName));
}

void
KPaletteTable::addToRecentColors( const QColor &color)
{
  //
  // 2000-02-12 Espen Sand.
  // The 'mPalette' is always 0 when current mode is i18n_namedColors
  //
  bool recentIsSelected = false;
  if ( mPalette && mPalette->name() == colorPaletteName[ recentColorIndex ].m_fileName )
  {
     delete mPalette;
     mPalette = 0;
     recentIsSelected = true;
  }
  KPalette *recentPal = new KPalette( colorPaletteName[ recentColorIndex ].m_fileName );
  if (recentPal->findColor(color) == -1)
  {
     recentPal->addColor( color );
     recentPal->save();
  }
  delete recentPal;
  if (recentIsSelected)
      setPalette( i18n( "palette name", colorPaletteName[ recentColorIndex ].m_displayName ) );
}

class KCDPickerFilter;

class KColorDialog::KColorDialogPrivate {
public:
    KPaletteTable *table;
    QString originalPalette;
    bool bRecursion;
    bool bEditRgb;
    bool bEditHsv;
    bool bEditHtml;
    bool bColorPicking;
    QLabel *colorName;
    QLineEdit *htmlName;
    KColorSpinBox *hedit;
    KColorSpinBox *sedit;
    KColorSpinBox *vedit;
    KColorSpinBox *redit;
    KColorSpinBox *gedit;
    KColorSpinBox *bedit;
    KColorPatch *patch;
    KHSSelector *hsSelector;
    KPalette *palette;
    KValueSelector *valuePal;
    QVBoxLayout* l_right;
    QGridLayout* tl_layout;
    QCheckBox *cbDefaultColor;
    KColor defaultColor;
    KColor selColor;
#ifdef Q_WS_X11
    KCDPickerFilter* filter;
#endif
};

#ifdef Q_WS_X11
class KCDPickerFilter: public QWidget
{
public:
  KCDPickerFilter(QWidget* parent): QWidget(parent)
  {}

  virtual bool x11Event (XEvent* event)
  {
    if (event->type == ButtonRelease)
    {
        QMouseEvent e( QEvent::MouseButtonRelease, QPoint(),
                       QPoint(event->xmotion.x_root, event->xmotion.y_root) , Qt::NoButton, Qt::NoButton,Qt::NoModifier );
        QApplication::sendEvent( parentWidget(), &e );
        return true;
    }
    else return false;
  }
};

#endif


KColorDialog::KColorDialog( QWidget *parent, const char *name, bool modal )
  :KDialogBase( parent, name, modal, i18n("Select Color"),
		modal ? Ok|Cancel : Close,
		Ok, true )
{
  d = new KColorDialogPrivate;
  d->bRecursion = true;
  d->bColorPicking = false;
#ifdef Q_WS_X11
  d->filter = 0;
#endif
  d->cbDefaultColor = 0L;
  connect( this, SIGNAL(okClicked(void)),this,SLOT(slotWriteSettings(void)));
  connect( this, SIGNAL(closeClicked(void)),this,SLOT(slotWriteSettings(void)));

  QLabel *label;

  //
  // Create the top level page and its layout
  //
  QWidget *page = new QWidget( this );
  setMainWidget( page );

  QGridLayout *tl_layout = new QGridLayout( page, 3, 3, 0, spacingHint() );
  d->tl_layout = tl_layout;
  tl_layout->addColSpacing( 1, spacingHint() * 2 );

  //
  // the more complicated part: the left side
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
  label = new QLabel( i18n("H:"), page );
  label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  l_lbot->addWidget(label, 0, 2);
  d->hedit = new KColorSpinBox( 0, 359, 1, page );
  l_lbot->addWidget(d->hedit, 0, 3);
  connect( d->hedit, SIGNAL( valueChanged(int) ),
  	SLOT( slotHSVChanged() ) );

  label = new QLabel( i18n("S:"), page );
  label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  l_lbot->addWidget(label, 1, 2);
  d->sedit = new KColorSpinBox( 0, 255, 1, page );
  l_lbot->addWidget(d->sedit, 1, 3);
  connect( d->sedit, SIGNAL( valueChanged(int) ),
  	SLOT( slotHSVChanged() ) );

  label = new QLabel( i18n("V:"), page );
  label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  l_lbot->addWidget(label, 2, 2);
  d->vedit = new KColorSpinBox( 0, 255, 1, page );
  l_lbot->addWidget(d->vedit, 2, 3);
  connect( d->vedit, SIGNAL( valueChanged(int) ),
  	SLOT( slotHSVChanged() ) );

  //
  // add the RGB fields
  //
  label = new QLabel( i18n("R:"), page );
  label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  l_lbot->addWidget(label, 0, 4);
  d->redit = new KColorSpinBox( 0, 255, 1, page );
  l_lbot->addWidget(d->redit, 0, 5);
  connect( d->redit, SIGNAL( valueChanged(int) ),
  	SLOT( slotRGBChanged() ) );

  label = new QLabel( i18n("G:"), page );
  label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  l_lbot->addWidget( label, 1, 4);
  d->gedit = new KColorSpinBox( 0, 255,1, page );
  l_lbot->addWidget(d->gedit, 1, 5);
  connect( d->gedit, SIGNAL( valueChanged(int) ),
  	SLOT( slotRGBChanged() ) );

  label = new QLabel( i18n("B:"), page );
  label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  l_lbot->addWidget(label, 2, 4);
  d->bedit = new KColorSpinBox( 0, 255, 1, page );
  l_lbot->addWidget(d->bedit, 2, 5);
  connect( d->bedit, SIGNAL( valueChanged(int) ),
  	SLOT( slotRGBChanged() ) );

  //
  // the entry fields should be wide enough to hold 8888888
  //
  int w = d->hedit->fontMetrics().width("8888888");
  d->hedit->setFixedWidth(w);
  d->sedit->setFixedWidth(w);
  d->vedit->setFixedWidth(w);

  d->redit->setFixedWidth(w);
  d->gedit->setFixedWidth(w);
  d->bedit->setFixedWidth(w);

  //
  // add a layout for the right side
  //
  d->l_right = new QVBoxLayout;
  tl_layout->addLayout(d->l_right, 0, 2);

  //
  // Add the palette table
  //
  d->table = new KPaletteTable( page );
  d->l_right->addWidget(d->table, 10);

  connect( d->table, SIGNAL( colorSelected( const QColor &, const QString & ) ),
	   SLOT( slotColorSelected( const QColor &, const QString & ) ) );

  connect(
    d->table,
    SIGNAL( colorDoubleClicked( const QColor &, const QString & ) ),
    SLOT( slotColorDoubleClicked( const QColor &, const QString & ) )
  );
  // Store the default value for saving time.
  d->originalPalette = d->table->palette();

  //
  // a little space between
  //
  d->l_right->addSpacing(10);

  QHBoxLayout *l_hbox = new QHBoxLayout();
  d->l_right->addItem(l_hbox);

  //
  // The add to custom colors button
  //
  QPushButton *button = new QPushButton( page );
  button->setText(i18n("&Add to Custom Colors"));
  l_hbox->addWidget(button, 0, Qt::AlignLeft);
  connect( button, SIGNAL( clicked()), SLOT( slotAddToCustomColors()));

  //
  // The color picker button
  //
  button = new QPushButton( page );
  button->setIcon( BarIconSet("colorpicker"));
  l_hbox->addWidget(button, 0, Qt::AlignHCenter );
  connect( button, SIGNAL( clicked()), SLOT( slotColorPicker()));

  //
  // a little space between
  //
  d->l_right->addSpacing(10);

  //
  // and now the entry fields and the patch (=colored box)
  //
  QGridLayout *l_grid = new QGridLayout( d->l_right, 2, 3);

  l_grid->setColStretch(2, 1);

  label = new QLabel( page );
  label->setText(i18n("Name:"));
  l_grid->addWidget(label, 0, 1, Qt::AlignLeft);

  d->colorName = new QLabel( page );
  l_grid->addWidget(d->colorName, 0, 2, Qt::AlignLeft);

  label = new QLabel( page );
  label->setText(i18n("HTML:"));
  l_grid->addWidget(label, 1, 1, Qt::AlignLeft);

  d->htmlName = new QLineEdit( page );
  d->htmlName->setMaxLength( 13 ); // Qt's QColor allows 12 hexa-digits
  d->htmlName->setText("#FFFFFF"); // But HTML uses only 6, so do not worry about the size
  w = d->htmlName->fontMetrics().width(QLatin1String("#DDDDDDD"));
  d->htmlName->setFixedWidth(w);
  l_grid->addWidget(d->htmlName, 1, 2, Qt::AlignLeft);

  connect( d->htmlName, SIGNAL( textChanged(const QString &) ),
      SLOT( slotHtmlChanged() ) );

  d->patch = new KColorPatch( page );
  d->patch->setFixedSize(48, 48);
  l_grid->addMultiCellWidget(d->patch, 0, 1, 0, 0, Qt::AlignHCenter | Qt::AlignVCenter);
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
  KColor col;
  col.setHsv( 0, 0, 255 );
  _setColor( col );

  d->htmlName->installEventFilter(this);
  d->hsSelector->installEventFilter(this);
  d->hsSelector->setAcceptDrops(true);
}

KColorDialog::~KColorDialog()
{
#ifdef Q_WS_X11
    if (d->bColorPicking && kapp)
        kapp->removeX11EventFilter(d->filter);
#endif
    delete d;
}

bool
KColorDialog::eventFilter( QObject *obj, QEvent *ev )
{
    if ((obj == d->htmlName) || (obj == d->hsSelector))
    switch(ev->type())
    {
      case QEvent::DragEnter:
      case QEvent::DragMove:
      case QEvent::DragLeave:
      case QEvent::Drop:
      case QEvent::DragResponse:
            qApp->sendEvent(d->patch, ev);
            return true;
      default:
            break;
    }
    return KDialogBase::eventFilter(obj, ev);
}

void
KColorDialog::setDefaultColor( const QColor& col )
{
    if ( !d->cbDefaultColor )
    {
        //
        // a little space between
        //
        d->l_right->addSpacing(10);

        //
        // and the "default color" checkbox, under all items on the right side
        //
        d->cbDefaultColor = new QCheckBox( i18n( "Default color" ), mainWidget() );
        d->cbDefaultColor->setChecked(true);

        d->l_right->addWidget( d->cbDefaultColor );

        mainWidget()->setMaximumSize( QWIDGETSIZE_MAX, QWIDGETSIZE_MAX ); // cancel setFixedSize()
        d->tl_layout->activate();
        mainWidget()->setMinimumSize( mainWidget()->sizeHint() );
        disableResize();

        connect( d->cbDefaultColor, SIGNAL( clicked() ), SLOT( slotDefaultColorClicked() ) );
    }

    d->defaultColor = col;

    slotDefaultColorClicked();
}

QColor KColorDialog::defaultColor() const
{
    return d->defaultColor;
}

void KColorDialog::slotDefaultColorClicked()
{
    if ( d->cbDefaultColor->isChecked() )
    {
        d->selColor = d->defaultColor;
        showColor( d->selColor, i18n( "-default-" ) );
    } else
    {
        showColor( d->selColor, QString::null );
    }
}

void
KColorDialog::readSettings()
{
  KConfig* config = KGlobal::config();

  QString oldgroup = config->group();

  config->setGroup("Colors");
  QString palette = config->readEntry("CurrentPalette");
  if (palette.isEmpty()) palette=i18n("palette name",colorPaletteName[fortyColorIndex].m_displayName);
  d->table->setPalette(palette);
  config->setGroup( oldgroup );
}

void
KColorDialog::slotWriteSettings()
{
  KConfig* config = KGlobal::config();
  config->setGroup("Colors");
  QString palette = d->table->palette();
  if (!config->hasDefault("CurrentPalette") &&
      (d->table->palette() == d->originalPalette))
  {
     config->revertToDefault("CurrentPalette");
  }
  else
  {
     config->writeEntry("CurrentPalette", d->table->palette()); //Shouldn't here the unstranslated name be saved ??
  }
}

QColor
KColorDialog::color() const
{
  if ( d->cbDefaultColor && d->cbDefaultColor->isChecked() )
     return QColor();
  if ( d->selColor.isValid() )
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
  KColorDialog dlg( parent, "Color Selector", true );
  if ( theColor.isValid() )
    dlg.setColor( theColor );
  int result = dlg.exec();

  if ( result == Accepted )
  {
    theColor = dlg.color();
  }

  return result;
}

//
// static function to display dialog and return color
//
int KColorDialog::getColor( QColor &theColor, const QColor& defaultCol, QWidget *parent )
{
  KColorDialog dlg( parent, "Color Selector", true );
  dlg.setDefaultColor( defaultCol );
  dlg.setColor( theColor );
  int result = dlg.exec();

  if ( result == Accepted )
    theColor = dlg.color();

  return result;
}

void KColorDialog::slotRGBChanged( void )
{
  if (d->bRecursion) return;
  int red = d->redit->value();
  int grn = d->gedit->value();
  int blu = d->bedit->value();

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

  QString strColor( d->htmlName->text() );

  // Assume that a user does not want to type the # all the time
  if ( strColor[0] != '#' )
  {
    bool signalsblocked = d->htmlName->signalsBlocked();
    d->htmlName->blockSignals(true);
    strColor.prepend("#");
    d->htmlName->setText(strColor);
    d->htmlName->blockSignals(signalsblocked);
  }

  const QColor color( strColor );

  if ( color.isValid() )
  {
    KColor col( color );
    d->bEditHtml = true;
    _setColor( col );
    d->bEditHtml = false;
  }
}

void KColorDialog::slotHSVChanged( void )
{
  if (d->bRecursion) return;
  int hue = d->hedit->value();
  int sat = d->sedit->value();
  int val = d->vedit->value();

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
  if (v < 0)
     v = 0;
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

void KColorDialog::slotColorDoubleClicked
(
  const QColor  & color,
  const QString & name
)
{
  _setColor(color, name);
  accept();
}

void KColorDialog::_setColor(const KColor &color, const QString &name)
{
  if (color.isValid())
  {
     if (d->cbDefaultColor && d->cbDefaultColor->isChecked())
        d->cbDefaultColor->setChecked(false);
     d->selColor = color;
  }
  else
  {
     if (d->cbDefaultColor && d->cbDefaultColor->isChecked())
        d->cbDefaultColor->setChecked(true);
     d->selColor = d->defaultColor;
  }

  showColor( d->selColor, name );

  emit colorSelected( d->selColor );
}

// show but don't set into selColor, nor emit colorSelected
void KColorDialog::showColor( const KColor &color, const QString &name )
{
  d->bRecursion = true;

  if (name.isEmpty())
     d->colorName->setText( i18n("-unnamed-"));
  else
     d->colorName->setText( name );

  d->patch->setColor( color );

  setRgbEdit( color );
  setHsvEdit( color );
  setHtmlEdit( color );

  int h, s, v;
  color.hsv( &h, &s, &v );
  d->hsSelector->setValues( h, s );
  d->valuePal->blockSignals(true);
  d->valuePal->setHue( h );
  d->valuePal->setSaturation( s );
  d->valuePal->setValue( v );
  d->valuePal->updateContents();
  d->valuePal->blockSignals(false);
  d->valuePal->repaint(); // false );
  d->bRecursion = false;
}



void
KColorDialog::slotColorPicker()
{
    d->bColorPicking = true;
#ifdef Q_WS_X11
    d->filter = new KCDPickerFilter(this);
    kapp->installX11EventFilter(d->filter);
#endif
    grabMouse( Qt::CrossCursor );
    grabKeyboard();
}

void
KColorDialog::mouseReleaseEvent( QMouseEvent *e )
{
  if (d->bColorPicking)
  {
     d->bColorPicking = false;
#ifdef Q_WS_X11
     kapp->removeX11EventFilter(d->filter);
     delete d->filter; d->filter = 0;
#endif
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
    QImage i = pm.toImage();
    return i.pixel(0,0);
}

void
KColorDialog::keyPressEvent( QKeyEvent *e )
{
  if (d->bColorPicking)
  {
     if (e->key() == Qt::Key_Escape)
     {
        d->bColorPicking = false;
#ifdef Q_WS_X11
        kapp->removeX11EventFilter(d->filter);
        delete d->filter; d->filter = 0;
#endif
        releaseMouse();
        releaseKeyboard();
     }
     e->accept();
     return;
  }
  KDialogBase::keyPressEvent( e );
}

void KColorDialog::setRgbEdit( const KColor &col )
{
  if (d->bEditRgb) return;
  int r, g, b;
  col.rgb( &r, &g, &b );

  d->redit->setValue( r );
  d->gedit->setValue( g );
  d->bedit->setValue( b );
}

void KColorDialog::setHtmlEdit( const KColor &col )
{
  if (d->bEditHtml) return;
  int r, g, b;
  col.rgb( &r, &g, &b );
  QString num;

  num.sprintf("#%02X%02X%02X", r,g,b);
  d->htmlName->setText( num );
}


void KColorDialog::setHsvEdit( const KColor &col )
{
  if (d->bEditHsv) return;
  int h, s, v;
  col.hsv( &h, &s, &v );

  d->hedit->setValue( h );
  d->sedit->setValue( s );
  d->vedit->setValue( v );
}

void KHSSelector::virtual_hook( int id, void* data )
{ KXYSelector::virtual_hook( id, data ); }

void KValueSelector::virtual_hook( int id, void* data )
{ KSelector::virtual_hook( id, data ); }

void KPaletteTable::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KColorCells::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KColorPatch::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KColorDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }


#include "kcolordialog.moc"
//#endif
