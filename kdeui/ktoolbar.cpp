#include <qpainter.h>
#include <qtooltip.h> 
#include <qdrawutl.h>
#include <qpalette.h>
#include <qbitmap.h>
#include <qcombo.h>
#include <qpalette.h>
#include "ktoolbar.h"
#include "kbutton.h"
#include "klined.h"
#include "kcombo.h"
#include "ktoolbar.moc"
#include "ktopwidget.h"
#define TOOLBARHEIGHT 30

#define CONTEXT_LEFT 0
#define CONTEXT_RIGHT 1
#define CONTEXT_TOP 2
#define CONTEXT_BOTTOM 3
#define CONTEXT_FLOAT 4

#define ITEM_LINED 0
#define ITEM_BUTTON 1
#define ITEM_COMBO 2

#define KToolBarItem QWidget

#define DEBUG

KToolBarCombo::KToolBarCombo(bool rw, int ID,
                             QWidget *parent, char *name) : KCombo (rw, parent, name)
{ 
  id = ID;
  // setBackGroundColor(...);
}
KToolBarLined::KToolBarLined(const char *text, int ID,
                             QWidget *parent, char *name) : KLined (parent, name)
{
  id = ID;
  setText (text);
  autoSized = false;
}

KToolBarCombo::~KToolBarCombo ()
{
  autoSized=false;
}

void KToolBarLined::enable (bool enable)
{
  QWidget::setEnabled (enable);
}

void KToolBarCombo::enable (bool enable)
{
  QWidget::setEnabled (enable);
}


KToolBarButton::KToolBarButton( QPixmap& pixmap, int ID,
								QWidget *parent, char *name) : KButton( parent, name )
{
  resize(24,24);
  id = ID;
  if ( ! pixmap.isNull() ) 
    enabledPixmap = pixmap;
  else 
    {
      warning("KToolBarButton: pixmap is empty, perhaps some missing file");
      enabledPixmap.resize( 22, 22);
    };
  makeDisabledPixmap();
  KButton::setPixmap( enabledPixmap );
  connect( this, SIGNAL( clicked() ), 
		   this, SLOT( ButtonClicked() ) );
  connect(this, SIGNAL( pressed() ), this, SLOT( ButtonPressed() ) );
  connect(this, SIGNAL( released() ), this, SLOT( ButtonReleased() ) );
}

void KToolBarButton::setPixmap( QPixmap &pixmap )
{
  if ( ! pixmap.isNull() )
    enabledPixmap = pixmap;
  else
    {
      warning("KToolBarButton: pixmap is empty, perhaps some missing file");
      enabledPixmap.resize( 22, 22);
    }
  // makeDisabledPixmap();
  KButton::setPixmap( enabledPixmap );
}            

void KToolBarButton::makeDisabledPixmap()
{
  QPalette pal = palette();
  QColorGroup g = pal.disabled();
  
  // Find the outline of the colored portion of the normal pixmap
  
  QBitmap *pmm = (QBitmap*) enabledPixmap.mask();
  QPixmap pm;
  if (pmm != NULL) 
    {
      pmm->setMask( *pmm );
      pm = *pmm;
    } 
  else 
    {
      pm.resize(22 , 22);
      enabledPixmap.fill(this, 0, 0);
      warning("KToolBarButton::makeDisabledPixmap: mask is null.");
    };
  
  // Prepare the disabledPixmap for drawing
  
  disabledPixmap.resize(22,22);
  disabledPixmap.fill( g.background() );
  
  // Draw the outline found above in white and then overlay a grey version
  // for a cheesy 3D effect ! BTW. this is the way that Qt 1.2+ does it for
  // Windows style
  
  QPainter p;
  p.begin( &disabledPixmap );
  p.setPen( white );
  p.drawPixmap(1, 1, pm);
  p.setPen( g.text() );
  p.drawPixmap(0, 0, pm);
  p.end();
}
  icontext=config->readNumEntry("IconText", 0);
void KToolBarButton::paletteChange(const QPalette &)
{
  if( ID() != -1 ) 
    {
      makeDisabledPixmap();
      if ( !isEnabled() )
		KButton::setPixmap( disabledPixmap );
      repaint( TRUE );
    }
}
  bool doUpdate=false;
KToolBarButton::KToolBarButton( QWidget *parentWidget, char *name )
  : KButton( parentWidget , name)
{
  resize(6,6);
  hide();
  id = -1;
}
    else
void KToolBarButton::enable( bool enabled )
{
  KButton::setPixmap( (enabled ? enabledPixmap : disabledPixmap) );
  setEnabled( enabled );
}
  if (fullWidth == true)
void KToolBarButton::ButtonClicked()
{
  emit clicked( id );
}
         }
void KToolBarButton::ButtonPressed()
{
  emit pressed( id );
}
  //debug ("Ho, ho, hooo... Up-Date!!! (vertical)");
void KToolBarButton::ButtonReleased()
{
  emit released( id );
}
   QApplication::sendEvent() which, as far as I understand, notifies
KToolBar::KToolBar(QWidget *parent, char *name)
  : QFrame( parent, name )
{
  init();
  Parent = parent;        // our father

  noPaint = FALSE;
  noSize = FALSE;
  noUpdate = FALSE;
  
  title = 0;
}
{
void KToolBar::ContextCallback( int index )
{
  switch ( index ) {
  case CONTEXT_LEFT:
	setPos( Left );
	break;
  case CONTEXT_RIGHT:
	setPos( Right );
	break;
  case CONTEXT_TOP:
	setPos( Top );
	break;
  case CONTEXT_BOTTOM:
	setPos( Bottom );
	break;
  case CONTEXT_FLOAT:
	if (position == Floating)
	  setPos (lastPosition);
	else
	  setPos( Floating );
    break;
  }
}
{
void KToolBar::init()
{
  context = new QPopupMenu( 0, "context" );
  context->insertItem( "Left", CONTEXT_LEFT );
  context->insertItem( "Top",  CONTEXT_TOP );
  context->insertItem( "Right", CONTEXT_RIGHT );
  context->insertItem( "Bottom", CONTEXT_BOTTOM );
  context->insertItem( "Floating", CONTEXT_FLOAT );
  connect( context, SIGNAL( activated( int ) ), this,
		   SLOT( ContextCallback( int ) ) );
  
  position = Top;
  moving = TRUE;
  setFrameStyle( QFrame::Panel | QFrame::Raised );
  setLineWidth( 0 );
  resize( width(), TOOLBARHEIGHT );
  
  enableFloating (TRUE);       // For now ... mmrggghftr

  
}
    emit (moved(position));
void KToolBar::drawContents ( QPainter *)
{
}

KToolBar::~KToolBar()
{
  // Get a seg. fault in this destructor with the following line included.
  // Don't know why.
  
  // delete context;
int KToolBar::insertLineSeparator( int index )
  if (position == Floating)
	setPos (lastPosition);
  
  for ( KToolBarItem *b = items.first(); b!=NULL; b=items.next() )
    delete b;
}
  if (position == Floating)
void KToolBar::setMaxHeight (int h)
{
  max_height = h;
}
    emit (moved(position));
void KToolBar::layoutHorizontal ()
{
  int offset=3+9+4;
  int rightOffset;
  int yOffset=3;
  KToolBarItem *autoSize = 0;

  if (fullWidth == TRUE)
    toolbarWidth = Parent->width();
  else
    toolbarWidth = offset;
  item->show();
  rightOffset=Parent->width();
  toolbarHeight= TOOLBARHEIGHT;
    updateRects( true );
  for ( KToolBarItem *b = items.first(); b!=NULL; b=items.next() )
	{
	  if (fullWidth == TRUE)
		{
		  if (isItemRight(b) == TRUE)
			{
			  rightOffset=rightOffset-1-b->width();

			  if (rightOffset <= (offset+1))
				{
				  yOffset += TOOLBARHEIGHT;
				  toolbarHeight += TOOLBARHEIGHT;
				}
			  b->move (rightOffset, yOffset);
			}
		  else // Not right
			{
			  int myWidth;
			  if (isItemAutoSized(b) == TRUE)
				{
				  autoSize = b;
				  myWidth = 70; // Min width for autosized
				}
			  else
				myWidth = b->width();
			  if (offset > (rightOffset-myWidth+1))
				{
				  offset =1;
				  yOffset += TOOLBARHEIGHT;
				  toolbarHeight += TOOLBARHEIGHT;
				}
			  b->move( offset, yOffset );
			  offset+=myWidth+1;
			}
		}
	  else // Not fullwidth, autosize
		{
		  if (offset > (Parent->width()-b->width()+1))
			{
			  offset = 1;
			  yOffset += TOOLBARHEIGHT;
			  toolbarHeight += TOOLBARHEIGHT;
			  b->move( offset, yOffset );
			  offset += b->width()+1;
			}
		  else
			{
			  b->move( offset, yOffset );
			  offset += b->width()+1;
			  if (offset > toolbarWidth)
				toolbarWidth = offset;
			}
		}
	}
  if (autoSize != 0)
    autoSize->resize(rightOffset - autoSize->x()-1, autoSize->height() );
  if (fullWidth != TRUE)
    toolbarWidth += 2;
}

void KToolBar::layoutVertical ()
{
  int offset=3+9+4;
  int yOffset=3;
  int widest;

  toolbarHeight = offset;

  toolbarWidth= TOOLBARHEIGHT;
  widest =  TOOLBARHEIGHT;
  /*
	I have (had) ten thousand problems here. When toplevel shrinked (vert.) it's ok.
	But when we enlarge toplevel (vert.), it first sets up toolbars and then view.
	do we don't know that toplevel is larger than before.
	Solution: setMaxHeight (in h) is public member here. Call it from your toplevel
	AFTER you have managed Top and Bottom toolbars, with max free height
  */

  for ( KToolBarItem *b = items.first(); b!=NULL; b=items.next() )
	{
	  if (offset > ((max_height)-(b->height()+3)))
		{
		  offset = 1;
		  yOffset += widest;
		  toolbarWidth += TOOLBARHEIGHT;
		  b->move( yOffset, offset );
		  if ((yOffset + b->width()) > toolbarWidth) // is item wider than we are
			{
			  if (isItemAutoSized(b) == TRUE)
				b->resize (100, b->height());
			  toolbarWidth = b->width() +yOffset+2;
			}
		  if ((b->width() +2) > widest)
			widest =(b->width() +2);
		  offset += b->height()+1;
		}
	  else
		{
		  b->move( yOffset, offset );
		  if (b->width() > toolbarWidth) // is item wider than we are
			{
			  if (isItemAutoSized(b) == TRUE)
				b->resize (100, b->height());
			  toolbarWidth = b->width() +yOffset+2;
			}
		  if ((b->width() +2) > widest)
			widest =(b->width() +2);
		  offset += b->height()+1;
		  if (offset > toolbarHeight)
			toolbarHeight = offset;
		}
	}
  toolbarHeight +=2;
}

void KToolBar::updateRects( bool res )
{

  if (noUpdate == TRUE)
	return;
  
  noUpdate = TRUE;
  
  Parent = parentWidget();
  
  switch ( position )
	{
    case Top:
    case Bottom:
    case Floating:
      layoutHorizontal ();
      break;

    case Left:
    case Right:
      layoutVertical ();
      break;
	}

  if (res == TRUE)
	{
	  if (position == Floating)
		Float->resize (toolbarWidth, toolbarHeight);
	  resize (toolbarWidth, toolbarHeight);

	}
  noUpdate = FALSE;
}

void KToolBar::mousePressEvent ( QMouseEvent *m )
{
  if ( moving )
    context->popup( mapToGlobal( m->pos() ), 0 );
}


void KToolBar::resizeEvent( QResizeEvent * )
{
  if (noUpdate == TRUE)
    return;

  if ( position == Floating )
    updateRects(TRUE);

}

void KToolBar::paintEvent( QPaintEvent * )
{
  if (noUpdate == TRUE)
    return;

  updateRects (TRUE);

  int offset=3;
  QColorGroup g = QWidget::colorGroup();

  QPainter *paint = new QPainter();
  paint->begin( this );

  // Handle point
  
  switch ( position )
	{
    case Top:
    case Bottom:
    case Floating:
      qDrawShadePanel( paint, offset, 3, 4, TOOLBARHEIGHT-8,
                       g , FALSE, 2);
      offset+=4;
      qDrawShadePanel( paint, offset, 3, 4, TOOLBARHEIGHT-8,
					   g , FALSE, 2);
      offset+=9;
      break;

    case Left:
    case Right:
      qDrawShadePanel( paint, 3, offset, TOOLBARHEIGHT-8, 4,
                       g , FALSE, 2);
      offset+=4;
      qDrawShadePanel( paint, 3, offset,  TOOLBARHEIGHT-8, 4,
                       g , FALSE, 2);
      offset+=9;
      break;
	}

  for ( KToolBarItem *b = items.first(); b!=NULL; b=items.next() )
	{
	  switch ( position )
		{
		case Top:
		case Bottom:
		case Floating:
		  if ( getID(b) == -1 )   // is a separator
			qDrawShadePanel(paint, b->x()+2, b->y(), 2, TOOLBARHEIGHT-4,
							g , TRUE, 1);
		  break;
		case Left:
		case Right:
		  if ( getID(b) == -1 )  // is a separator
			qDrawShadePanel(paint, b->x(), b->y()+2, TOOLBARHEIGHT-4, 2,
							g , TRUE, 1);
		  break;
		}
	}

  qDrawShadePanel(paint, 0, 0, width(), height(),
                  g , FALSE, 2);

  paint->end();
  delete paint;
}

int KToolBar::getID(QWidget *w)
{
  switch (getType (w))
	{
    case ITEM_LINED:
      return ((KToolBarLined *) w)->ID();
      break;
    case ITEM_BUTTON:
      return ((KToolBarButton *) w)->ID();
      break;
    case ITEM_COMBO:
      return ((KToolBarCombo *) w)->ID();
      break;
	}
  return 0;
}

bool KToolBar::isItemAutoSized(QWidget *w)
{
  switch (getType (w))
	{
    case ITEM_LINED:
      return ((KToolBarLined *) w)->isAuto();
      break;
    case ITEM_BUTTON:
      return FALSE;;
      break;
    case ITEM_COMBO:
      return ((KToolBarCombo *) w)->isAuto();
      break;
	}
  return FALSE;
}

int KToolBar::getType(QWidget *w)
{
  if (w->isA("KToolBarLined"))
    return ITEM_LINED;
  if (w->isA("KToolBarButton"))
    return ITEM_BUTTON;
  if (w->isA("KToolBarCombo"))
    return ITEM_COMBO;
  return 0;
}

bool KToolBar::isItemRight (QWidget *w)
{
  switch (getType (w))
	{
    case ITEM_LINED:
      return ((KToolBarLined *) w)->isRight();
      break;
    case ITEM_BUTTON:
      return ((KToolBarButton *) w)->isRight();
      break;
    case ITEM_COMBO:
      return ((KToolBarCombo *) w)->isRight();
      break;
	}
  return FALSE;
}

void KToolBar::floatKilled ()
{
  /*
	This slot is called from KFloaters destructor
	He did return your widget to you, and will be
	deleted as soon as you exit this slot. DO NOT
	DELETE IT!
   */
  
  position = lastPosition;
  context->changeItem ("Float", CONTEXT_FLOAT);
  emit moved (position);
}


void KToolBar::ButtonClicked( int id )
{
  emit clicked( id );
}

void KToolBar::ButtonPressed( int id )
{
  emit pressed( id );
}
      items.remove();
void KToolBar::ButtonReleased( int id )
{
  emit released( id );
}

/********************\
 *                    *
 * I N T E R F A C E  *
 *                    *
 \********************/

/***** BUTTONS *****/

/// Inserts a button.
int KToolBar::insertButton( QPixmap& pixmap, int id, bool enabled,
							char *tooltiptext, int index )
{
  KToolBarButton *button = new KToolBarButton( pixmap, id, this );
  if ( index == -1 )
    items.append( button );
  else
    items.insert( index, button );
  
  connect(button, SIGNAL(clicked(int)), this, SLOT(ButtonClicked(int)));
  connect(button, SIGNAL(released(int)), this, SLOT(ButtonReleased(int)));
  connect(button, SIGNAL(pressed(int)), this, SLOT(ButtonPressed(int)));
  button->enable( enabled );

  if ( tooltiptext )
    QToolTip::add( button, tooltiptext );
		
  updateRects( TRUE );
  return items.at();
}

/// Inserts a button with connection.

int KToolBar::insertButton( QPixmap& pixmap, int id, const char *signal,
							const QObject *receiver, const char *slot, bool enabled,
							char *tooltiptext, int index )
{
  KToolBarButton *button = new KToolBarButton( pixmap, id, this );
  if ( index == -1 ) 
    items.append( button );
  else
    items.insert( index, button );
		
  if ( tooltiptext )
    QToolTip::add( button, tooltiptext );
		
  connect( button, signal, receiver, slot );
  button->enable( enabled );
  updateRects( TRUE );
  /*	printf("insertButton %d\n",items.at()); */
  return items.at();
}

/********* SEPARATOR *********/
/// Inserts separator

int KToolBar::insertSeparator( int index )
{
  KToolBarButton *separ = new KToolBarButton( this );
  if ( index == -1 )
    items.append( separ );
  else
    items.insert( index, separ );
	
  updateRects( TRUE );
  return items.at();
}

/************** LINE EDITOR **************/
/// Inserts a KLined. KLined is derived from QLineEdit and has another signal, tabPressed,
//  for completions.

int KToolBar::insertLined(const char *text, int id, const char *signal,
						  const QObject *receiver, const char *slot,
						  bool enabled, char *tooltiptext, int size, int index)
{
  KToolBarLined *lined = new KToolBarLined (text, id, this);
  if (index == -1)
    items.append (lined);
  else
    items.insert(index, lined);
  if (tooltiptext)
    QToolTip::add( lined, tooltiptext );
  connect( lined, signal, receiver, slot );
  lined->resize(size, 24);
  lined->enable(enabled);
  updateRects(TRUE);
  return items.at();
}

/************** COMBO BOX **************/
/// Inserts comboBox with QStrList

int KToolBar::insertCombo (QStrList *list, int id, bool writable,
                           const char *signal, QObject *receiver,
                           const char *slot, bool enabled,
                           char *tooltiptext,
                           int size, int index,
                           KCombo::Policy policy)
{
  KToolBarCombo *combo = new KToolBarCombo (writable, id, this);
  if (index == -1)
    items.append (combo);
  else
    items.insert (index, combo);
  combo->insertStrList (list);
  combo->setInsertionPolicy(policy);
  if (tooltiptext)
    QToolTip::add( combo, tooltiptext );
  connect ( combo, signal, receiver, slot );
  combo->resize(size, 24);
  combo->enable(enabled);
  updateRects(TRUE);
  return items.at();
}


/// Inserts combo with text

int KToolBar::insertCombo (const char *text, int id, bool writable,
                           const char *signal, QObject *receiver,
                           const char *slot, bool enabled,
                           char *tooltiptext, int size, int index,
                           KCombo::Policy policy)
{
  KToolBarCombo *combo = new KToolBarCombo (writable, id, this);
  if (index == -1)
    items.append (combo);
  else
    items.insert (index, combo);
  combo->insertItem (text);
  combo->setInsertionPolicy(policy);
  if (tooltiptext)
    QToolTip::add( combo, tooltiptext );
  connect (combo, signal, receiver, slot);
  combo->resize(size, 24);
  combo->enable(enabled);
  updateRects(TRUE);
  return items.at();
}

/// ******** Tools

/// misc
void KToolBar::addConnection (int id, const char *signal,
                              const QObject *receiver, const char *slot)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
	  {
		switch(getType(b))
		  {
		  case ITEM_BUTTON:
			connect (((KToolBarButton *) b), signal, receiver, slot);
			break;
		  case ITEM_LINED:
			connect (((KToolBarLined *) b), signal, receiver, slot);
			break;
		  case ITEM_COMBO:
			connect (((KToolBarCombo *) b), signal, receiver, slot);
			break;
		  }
	  }
}

/// Common
void KToolBar::setItemEnabled( int id, bool enabled )
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
	  {
		switch(getType(b))
		  {
		  case ITEM_BUTTON:
			((KToolBarButton *) b)->enable( enabled );
			break;

		  case ITEM_LINED:
			((KToolBarLined *) b)->enable( enabled );
			break;

		  case ITEM_COMBO:
			((KToolBarCombo *) b)->enable( enabled );
			break;
		  }
	  }
}

void KToolBar::setItemAutoSized ( int id, bool enabled )
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
	  {
		switch(getType(b))
		  {
		  case ITEM_BUTTON:
			return;
			break;

		  case ITEM_LINED:
			((KToolBarLined *) b)->autoSize( enabled );
			break;

		  case ITEM_COMBO:
			((KToolBarCombo *) b)->autoSize( enabled );
			break;
		  }
	  }
}

void KToolBar::alignItemRight(int id, bool yes)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
	  {
		switch(getType(b))
		  {
		  case ITEM_BUTTON:
			((KToolBarButton *) b)->alignRight (yes);
			break;

		  case ITEM_LINED:
			((KToolBarLined *) b)->alignRight (yes);
			break;

		  case ITEM_COMBO:
			((KToolBarCombo *) b)->alignRight (yes);
			break;
		  }
	  }
}

/// Butoons
void KToolBar::setButtonPixmap( int id, QPixmap& _pixmap )
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_BUTTON)
		((KToolBarButton *) b)->setPixmap( _pixmap ); 
}

/// Lined
void KToolBar::setLinedText (int id, const char *text)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_LINED)
		{
		  ((KToolBarLined *) b)->setText(text);
		  ((KToolBarLined *) b)->cursorAtEnd();
		}
}

const char *KToolBar::getLinedText (int id )
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_LINED)
		return ((KToolBarLined *) b)->text();
  return 0;
}

/// Combos
void KToolBar::insertComboItem (int id, const char *text, int index)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_COMBO)
		((KToolBarCombo *) b)->insertItem(text, index);
}

void KToolBar::insertComboList (int id, QStrList *list, int index)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_COMBO)
		((KToolBarCombo *) b)->insertStrList(list, index);
}

void KToolBar::removeComboItem (int id, int index)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_COMBO)
		((KToolBarCombo *) b)->removeItem(index);
}

void KToolBar::changeComboItem  (int id, const char *text, int index)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_COMBO)
		((KToolBarCombo *) b)->changeItem(text, index);
}

void KToolBar::clearCombo (int id)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_COMBO)
		((KToolBarCombo *) b)->clear();
}

const char *KToolBar::getComboItem (int id, int index)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_COMBO)
		return ((KToolBarCombo *) b)->text(index);
  return 0;
}

/// Toolbar itself

void KToolBar::setFullWidth(bool flag)
{
  fullWidth = flag;
}

void KToolBar::enableMoving(bool flag)
{
  moving = flag; 
}

void KToolBar::setPos(Position pos)
{
  if (position != pos)
	{
	  if (pos == Floating)
		{
		  lastPosition = position;
		  position = pos;
		  Float = new KFloater (title);
		  connect (Float, SIGNAL(killed()), this, SLOT(floatKilled()));
		  Float->move(QCursor::pos());
		  Float->grabWidget (this);
		  context->changeItem ("UnFloat", CONTEXT_FLOAT);
		  emit moved (pos);
		  return;
		}
	  else if (position == Floating) // was floating
		{
		  position = pos;
		  Float->releaseWidget (this);  // won't emit killed
		  delete Float;                 // won't emit killed
		  // if you just delete it without release it will emit killed
		  context->changeItem ("Float", CONTEXT_FLOAT);
		  emit moved (pos);
		  return;
		}
	  else
		{
		  position = pos;
		  // updateRects( TRUE );
		  emit moved ( pos );
		  return;
		}
	}
}

KToolBar::Position KToolBar::pos()
{
  return position;	
}

void KToolBar::enableFloating (bool arrrrrrgh)
{
  context->setItemEnabled (CONTEXT_FLOAT, arrrrrrgh);
}


bool KToolBar::enable(BarStatus stat)
{
  bool mystat = isVisible();

  if ( (stat == Toggle && mystat) || stat == Hide )
	{
	  if (position == Floating)
        Float->iconify();
	  hide();
	}
  else
	{
	  if (position == Floating)
		Float->show();
	  show();
	}
  return ( isVisible() == mystat );
}

/*********************************************************/
/***********    O L D     I N T E R F A C E   ************/
/*********************************************************/

int KToolBar::insertItem(QPixmap& _pixmap, int _ID, bool enabled,
						 char *ToolTipText, int index)

{
  warning ("KToolBar: insertItem is obsolete. Use insertButton");
  return (this->insertButton (_pixmap, _ID, enabled, ToolTipText, index));
}

int KToolBar::insertItem(QPixmap& _pixmap, int _ID, const char *signal,
						 const QObject *receiver, const char *slot,
						 bool enabled,
						 char *tooltiptext, int index)
{
  warning ("KToolBar: insertItem is obsolete. Use InsertButton");
  return (this->insertButton (_pixmap, _ID, signal, receiver, slot, enabled, tooltiptext, index));
}

void KToolBar::setItemPixmap( int _id, QPixmap& _pixmap )

{
  warning ("KToolBar: setItemPixmap is obsolete. Use setButtonPixmap");
  this->setButtonPixmap (_id, _pixmap);
}


      ++it;
    }
  }
}

#include "ktoolbar.moc"

