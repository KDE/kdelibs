/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
              (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
              (C) 1997, 1998 Sven Radej (radej@kde.org)
              (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)
			  (C) 1999 Chris Schlaeger (cs@kde.org)

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

// $Id$
// $Log$
// Revision 1.131  1999/10/08 23:11:12  bero
// Fix compilation
//
// Revision 1.130  1999/09/21 11:03:54  waba
// WABA: Clean up interface
//
// Revision 1.129  1999/08/04 13:02:48  radej
// sven: Proposed change from Carsten Pfeiffer for buttons with delayed popups:
// When popup is visible, click on button will hide the popup and emit click.
//
// Revision 1.128  1999/08/03 23:31:17  ettrich
// make flat only with the handle, not the entire toolbar
//
// Revision 1.127  1999/07/26 19:42:44  pbrown
// fixed for qcombobox.
//
// Revision 1.126  1999/07/25 11:53:48  kulow
// taking out some headers from ktmainwindow.h, they just don't belong there
//
// Revision 1.125  1999/07/07 19:12:12  cschlaeg
// removed horizontal/vertical flipping for floating bars during resize
//
// Revision 1.124  1999/06/20 10:49:35  mario
// Mario: the menu bar was not correctly drawn. This hack fixes that
//
// Revision 1.123  1999/06/18 20:28:19  kulow
// getConfig -> config
//
// Revision 1.122  1999/06/15 20:36:33  cschlaeg
// some more cleanup in ktmlayout; fixed random toolbar handle highlighting
//
// Revision 1.121  1999/06/13 21:43:54  cschlaeg
// fixed-size main widgets are now working; support for fixed-width widget or heightForWidth-widget needs a different concept; will think about it; floating toolbars are still broken
//
// Revision 1.120  1999/06/12 21:43:58  knoll
// kapp->xxxFont() -> KGlobal::xxxFont()
//
// Revision 1.119  1999/06/11 04:40:14  glenebob
// printf -> debug (twice)
//
// Revision 1.118  1999/06/10 21:47:50  cschlaeg
// setFullWidth(false) ignore feature re-implemented; floating resize bug fixed; layout manager documented; resizing floating bars still does not work properly
//


#include <qpainter.h>
#include <qtooltip.h>
#include <qdrawutil.h>
#include <qpalette.h>
#include <qbitmap.h>
#include <qstring.h>
#include <qframe.h>
#include <qbutton.h>
#include <qrect.h>
//#include <qimage.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ktoolbar.h"
#include "klined.h"
#include "kseparator.h"
#include <ktmainwindow.h>
#include <klocale.h>
#include <kapp.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kwm.h>
#include <ktoolboxmgr.h>
#include <kstyle.h>


#include "ktoolbarbutton.h"
#include "ktoolbaritem.h"

// Use enums instead of defines. We are C++ and NOT C !
enum {
    CONTEXT_LEFT = 0,
    CONTEXT_RIGHT = 1,
    CONTEXT_TOP = 2,
    CONTEXT_BOTTOM = 3,
    CONTEXT_FLOAT = 4,
    CONTEXT_FLAT = 5
};

// this should be adjustable (in faar future... )
#define MIN_AUTOSIZE 150

/****************************** Tolbar **************************************/

KToolBar::KToolBar(QWidget *parent, const char *name, int _item_size)
  : QFrame( parent, name )
{
  items = new KToolBarItemList();
  item_size = _item_size;
  fixed_size =  (item_size > 0);
  if (!fixed_size)
  item_size = 26;
  maxHorWidth = maxVerHeight = -1;
  init();
  Parent = parent;        // our father
  mouseEntered=false;
  localResize=false;
  buttonDownOnHandle = FALSE;
}

void KToolBar::ContextCallback( int )
{
  int i = context->exec();
  switch ( i )
    {
    case CONTEXT_LEFT:
      setBarPos( Left );
      break;
    case CONTEXT_RIGHT:
      setBarPos( Right );
      break;
    case CONTEXT_TOP:
      setBarPos( Top );
      break;
    case CONTEXT_BOTTOM:
      setBarPos( Bottom );
      break;
    case CONTEXT_FLOAT:
      if (position == Floating)
	setBarPos (lastPosition);
      else
	{
	  setBarPos( Floating );
	  move(QCursor::pos());
	  show();
	}
      break;
    case CONTEXT_FLAT:
        setFlat (position != Flat);
	break;
    }

  mouseEntered=false;
  repaint(false);
}

void KToolBar::init()
{
  context = new QPopupMenu( 0, "context" );
  context->insertItem( i18n("Left"), CONTEXT_LEFT );
  context->insertItem( i18n("Top"),  CONTEXT_TOP );
  context->insertItem( i18n("Right"), CONTEXT_RIGHT );
  context->insertItem( i18n("Bottom"), CONTEXT_BOTTOM );
  context->insertItem( i18n("Floating"), CONTEXT_FLOAT );
  context->insertItem( i18n("Flat"), CONTEXT_FLAT );
//   connect( context, SIGNAL( activated( int ) ), this,
// 	   SLOT( ContextCallback( int ) ) );

  //MD (17-9-97) Toolbar full width by default
  fullSizeMode=true;

  position = Top;
  moving = true;
  icon_text = 0;
  highlight = 0;
  setFrameStyle(NoFrame);
  setLineWidth( 1 );
  transparent = false;
  min_width = min_height = -1;
  updateGeometry();

  items->setAutoDelete(true);
  enableFloating (true);
  // To make touch-sensitive handle - sven 040198
  setMouseTracking(true);
  haveAutoSized=false;      // do we have autosized item - sven 220198
  connect (kapp, SIGNAL(appearanceChanged()), this, SLOT(slotReadConfig()));
  slotReadConfig();

  mgr =0;
}

void KToolBar::slotReadConfig()
{
  KConfig *config = KGlobal::config();
  QString group = config->group();
  config->setGroup("Toolbar style");
  int icontext=config->readNumEntry("IconText", 0);
  int tsize=config->readNumEntry("Size", 26);
  int _highlight =config->readNumEntry("Highlighting", 1);
  int _transparent = config->readBoolEntry("TransparentMoving", true);
  config->setGroup(group);

  bool doUpdate=false;

  if (!fixed_size && tsize != item_size && tsize>20)
  {
    item_size = tsize;
    doUpdate=true;
  }

  if (icontext != icon_text)
  {
    if (icontext==3)
      item_size = (item_size<40)?40:item_size;
    icon_text=icontext;
    doUpdate=true;
  }

  if (_highlight != highlight)
  {
    highlight = _highlight;
    doUpdate=true;
  }

  if (_transparent != transparent)
  {
    transparent= _transparent;
    doUpdate=false;
  }

  if (doUpdate)
    emit modechange(); // tell buttons what happened
  if (isVisible ())
	  updateRects(true);
}

void KToolBar::drawContents ( QPainter *)
{
}

KToolBar::~KToolBar()
{

// what is that?! we do not need to recreate before
// destroying.... (Matthias)

  // OK (sven)

//   if (position == Floating)
//   {
//     debug ("KToolBar destructor: about to recreate");
//     recreate (Parent, oldWFlags, QPoint (oldX, oldY), false);
//     debug ("KToolBar destructor: recreated");
//   }

  // what is that?! toolbaritems are children of the toolbar, which
  // means, qt will delete them for us (Matthias)
  //for ( KToolBarItem *b = items->first(); b!=0L; b=items->next() )
  // items.remove();
  //Uhh... I'm embaresd... (sven)
  delete items;

  // I would never guess that (sven)
  if (!QApplication::closingDown())
	   delete context;

  //debug ("KToolBar destructor");
}

void KToolBar::setMaxHeight (int h)
{
	maxVerHeight = h;
	updateRects(true);
}

void KToolBar::setMaxWidth (int w)
{
	maxHorWidth = w;
	updateRects(true);
}

void
KToolBar::layoutHorizontal(int w)
{
	int xOffset = 4 + 9 + 3;
	int yOffset = 1;
	int widest = 0;
	int tallest = 0;

	horizontal = true;

	/* For the horizontal layout we have to iterate twice through the toolbar
	 * items. During the first iteration we position the left aligned items,
	 * find the auto-size item and accumulate the total with for the left
	 * aligned widgets. */
	KToolBarItem* autoSizeItem = 0;
	/* This variable is used to accumulate the horizontal space the
	 * left aligned items need. This includes the 3 pixel space
	 * between the items. */
	int totalRightItemWidth = 0;

	/* First iteration */
	QListIterator<KToolBarItem> qli(*items);
	for (; *qli; ++qli)
		if (!(*qli)->isRight())
		{
			int itemWidth = (*qli)->width();
			if ((*qli)->isAuto())
			{
				itemWidth = MIN_AUTOSIZE;
				autoSizeItem = *qli;
			}

			if (xOffset + 3 + itemWidth > w)
			{
				/* The current line is full. We need to wrap-around and start
				 * a new line. */
				xOffset = 4 + 9 + 3;
				yOffset += tallest + 3;
				tallest = 0;
			}

			(*qli)->move(xOffset, yOffset);
			xOffset += 3 + itemWidth;

			/* We need to save the tallest height and the widest width. */
			if (itemWidth > widest)
				widest = itemWidth;
			if ((*qli)->height() > tallest)
				tallest = (*qli)->height();
		}
		else
		{
			totalRightItemWidth += (*qli)->width() + 3;
			if ((*qli)->isAuto())
				debug("Right aligned toolbar item cannot be auto-sized!");
		}

	int newXOffset = w - (3 + (totalRightItemWidth + 3) % w);
	if (newXOffset < xOffset)
	{
		/* right aligned items do not fit in the current line, so we start
		 * a new line */
		if (autoSizeItem)
		{
			/* The auto-sized widget extends from the last normal left-alined
			 * item to the right edge of the widget */
			autoSizeItem->resize(w - xOffset - 3 + MIN_AUTOSIZE,
								 autoSizeItem->height());
		}
		yOffset += tallest + 3;
		tallest = 0;
	}
	else
	{
		/* Right aligned items do fit in the current line. The auto-space
		 * item may fill the space between left and right aligned items. */
		if (autoSizeItem)
			autoSizeItem->resize(newXOffset - xOffset - 3 + MIN_AUTOSIZE,
								 autoSizeItem->height());
	}
	xOffset = newXOffset;

	/* During the second iteration we position the left aligned items. */
	for (qli.toFirst(); *qli; ++qli)
		if ((*qli)->isRight())
		{
			if (xOffset + 3 + (*qli)->width() > w)
			{
				xOffset = 4 + 9 + 3;
				yOffset += tallest + 3;
				tallest = 0;
			}

			(*qli)->move(xOffset, yOffset);
			xOffset += 3 + (*qli)->width();

			/* We need to save the tallest height and the widest width. */
			if ((*qli)->width() > widest)
				widest = (*qli)->width();
			if ((*qli)->height() > tallest)
				tallest = (*qli)->height();
		}

	toolbarWidth = w;
	toolbarHeight = yOffset + tallest + 1;
	min_width = 4 + 9 + 3 + widest + 3;
	min_height = toolbarHeight;
	updateGeometry();
}

int
KToolBar::heightForWidth(int w) const
{
	/* This function only works for Top, Bottom or Floating tool
	 * bars. For other positions it should never be called. To be save
	 * on the save side the current minimum height is returned. */
	if (position != Top && position != Bottom && position != Floating)
		return (min_height);

	int xOffset = 4 + 9 + 3;
	int yOffset = 1;
	int tallest = 0;

	/* This variable is used to accumulate the horizontal space the
	 * left aligned items need. This includes the 3 pixel space
	 * between the items. */
	int totalRightItemWidth = 0;
	QListIterator<KToolBarItem> qli(*items);
	for (; *qli; ++qli)
	{
		if (!(*qli)->isRight())
		{
			if (xOffset + 3 + (*qli)->width() > w)
			{
				xOffset = 4 + 9 + 3;
				yOffset += tallest + 3;
				tallest = 0;
			}

			xOffset += 3 + (*qli)->width();

			/* We need to save the tallest height. */
			if ((*qli)->height() > tallest)
				tallest = (*qli)->height();
		}
		else
		   totalRightItemWidth += (*qli)->width() + 3;
	}

	int newXOffset = w - (3 + (totalRightItemWidth + 3) % w);
	if (newXOffset < xOffset)
	{
		xOffset = 4 + 9 + 3;
		yOffset += tallest + 3;
		tallest = 0;
	}
	else
		xOffset = newXOffset;

	/* During the second iteration we position the left aligned items. */
	for (qli.toFirst(); (*qli); ++qli)
	{
		if ((*qli)->isRight())
		{
			if (xOffset + 3 + (*qli)->width() > w)
			{
				xOffset = 4 + 9 + 3;
				yOffset += tallest + 3;
				tallest = 0;
			}

			xOffset += 3 + (*qli)->width();

			/* We need to save the tallest height. */
			if ((*qli)->height() > tallest)
				tallest = (*qli)->height();
		}
	}

	return (yOffset + tallest + 1);
}

void
KToolBar::layoutVertical(int h)
{
	int xOffset = 3;
	int yOffset = 3 + 9 + 4;
	int widest = 0;
	int tallest = 0;

	horizontal = false;

	QListIterator<KToolBarItem> qli(*items);
	for (; *qli; ++qli)
	{
		if (yOffset + (*qli)->height() + 3 > h)
		{
			/* A column has been filled. We need to start a new column */
			yOffset = 4 + 9 + 3;
			xOffset += widest + 3;
			widest = 0;
		}

		/* arrange the toolbar item */
		(*qli)->move(xOffset, yOffset);
		/* auto-size items are set to the minimum auto-size or the width of
		 * the widest widget so far. Wider widgets that follow have no
		 * impact on the auto-size widgets that are above in the column. We
		 * might want to improve this later. */
		if ((*qli)->isAuto())
			(*qli)->resize((widest > MIN_AUTOSIZE) ?
					  widest : MIN_AUTOSIZE, (*qli)->height());

		/* adjust yOffset */
		yOffset += (*qli)->height() + 3;
		/* keep track of the maximum with of the column */
		if ((*qli)->width() > widest)
			widest = (*qli)->width();
		/* keep track of the tallest overall widget */
		if ((*qli)->height() > tallest)
			tallest = (*qli)->height();
	}

	toolbarHeight = h;
	toolbarWidth = min_width = xOffset + widest + 3;
	min_height = 4 + 9 + 3 + tallest + 3;
	updateGeometry();
}

int
KToolBar::widthForHeight(int h) const
{
	/* This function only works for Top, Bottom or Floating tool
	 * bars. For other positions it should never be called. To be on
	 * the save side the current minimum height is returned. */
	if (position != Left && position != Right && position != Floating)
		return (min_height);

	int xOffset = 3;
	int yOffset = 3 + 9 + 4;
	int widest = 0;
	int tallest = 0;

	QListIterator<KToolBarItem> qli(*items);
	for (; *qli; ++qli)
	{
		if (yOffset + (*qli)->height() + 3 > h)
		{
			/* A column has been filled. We need to start a new column */
			yOffset = 4 + 9 + 3;
			xOffset += widest + 3;
			widest = 0;
		}

		int itemWidth = (*qli)->width();
		/* auto-size items are set to the minimum auto-size or the width of
		 * the widest widget so far. Wider widgets that follow have no
		 * impact on the auto-size widgets that are above in the column. We
		 * might want to improve this later. */
		if ((*qli)->isAuto())
			itemWidth = (widest > MIN_AUTOSIZE) ? widest : MIN_AUTOSIZE;

		/* adjust yOffset */
		yOffset += (*qli)->height() + 3;
		/* keep track of the maximum with of the column */
		if (itemWidth > widest)
			widest = itemWidth;
		/* keep track of the tallest overall widget */
		if ((*qli)->height() > tallest)
			tallest = (*qli)->height();
	}

	return(xOffset + widest + 3);
}

void
KToolBar::updateRects(bool res)
{
	switch (position)
	{
	case Flat:
		min_width = 30;
		min_height = 10;
		updateGeometry();
		break;

	case Top:
	case Bottom:
	{
		int mw = width();
		if (!fullSizeMode)
		{
			/* If we are not in full size mode and the user has requested a
			 * certain width, this will be used. If no size has been requested
			 * and the parent width is larger than the maximum width, we use
			 * the maximum width. */
			if (maxHorWidth != -1)
				mw = maxHorWidth;
			else if (width() > maximumSizeHint().width())
				mw = maximumSizeHint().width();
		}	
		layoutHorizontal(mw);
		break;
	}

	case Left:
	case Right:
	{
		int mh = height();
		if (!fullSizeMode)
		{
			/* If we are not in fullSize mode and the user has requested a
			 * certain height, this will be used. If no size has been requested
			 * and the parent height is larger than the maximum height, we use
			 * the maximum height. */
			if (maxVerHeight != -1)
				mh = maxVerHeight;
			else if (height() > maximumSizeHint().height())
				mh = maximumSizeHint().height();
		}
		layoutVertical(mh);
		break;
	}
	default:
		return;
	}

	if (res == true)
	{
		localResize = true;
		resize(toolbarWidth, toolbarHeight);
		localResize = false;
	}
}

QSize
KToolBar::sizeHint() const
{
	switch (position)
	{
	case Floating:
		/* Floating bars are under direct control of the WM. sizeHint() is
		 * ignored. */
		break;

	case Top:
	case Bottom:
		if (!fullSizeMode && (maxHorWidth != -1))
		{
			/* If fullSize mode is disabled and the user has requested a
			 * specific width, then we use this value. */
			return (QSize(maxHorWidth, min_height));
		}
		break;
	case Right:
	case Left:
		if (!fullSizeMode && (maxVerHeight != -1))
		{
			/* If fullSize mode is disabled and the user has requested a
			 * specific height, then we use this value. */
			return (QSize(min_width, maxVerHeight));
		}
		break;
	default:
		break;
	}
	
	return (QSize(min_width, min_height));
}

QSize
KToolBar::maximumSizeHint() const
{
	/* This function returns the maximum size the bar can have. All toolbar
	 * items are placed in a single line. */
	int prefWidth = -1;
	int prefHeight = -1;

	QListIterator<KToolBarItem> qli(*items);

	switch (position)
	{
	case Flat:
		prefWidth = 30;
		prefHeight = 10;
		break;

	case Floating:
	case Top:
	case Bottom:
		prefWidth = 4 + 9 + 3;
		prefHeight = 0;

		for (; *qli; ++qli)
		{
			int itemWidth = (*qli)->width();
			if ((*qli)->isAuto())
				itemWidth = MIN_AUTOSIZE;

			prefWidth += 3 + itemWidth;
			if ((*qli)->height() > prefHeight)
				prefHeight = (*qli)->height();
		}
		prefWidth += 3;		/* 3 more pixels to the right */
		prefHeight += 2;	/* one more pixels above and below */
		break;

	case Left:
	case Right:	
		prefWidth = 0;
		prefHeight = 4 + 9 + 3;

		for (; *qli; ++qli)
		{
			prefHeight += (*qli)->height() + 3;
			/* keep track of the maximum with of the column */
			if ((*qli)->isAuto())
			{
				if (MIN_AUTOSIZE > prefWidth)
					prefWidth = MIN_AUTOSIZE;
			}
			else
			{
				if ((*qli)->width() > prefWidth)
					prefWidth = (*qli)->width();
			}
		}
		prefWidth += 2;		/* one more pixels to the left and right */
		prefHeight += 3;	/* 3 more pixels below */
		break;
	}
	return (QSize(prefWidth, prefHeight));
}

QSize
KToolBar::minimumSizeHint() const
{
	return (sizeHint());
}

QSizePolicy
KToolBar::sizePolicy() const
{
	switch (position)
	{
	case Floating:
		/* Floating bars are under direct control of the WM. sizePolicy() is
		 * ignored. */
		return QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	case Top:
	case Bottom:
		return QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

	case Left:
	case Right:
		return QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);	

	default:
		return QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	}
}

void KToolBar::mouseMoveEvent ( QMouseEvent *mev)
{
	/* The toolbar handles are hightlighted when the mouse moves over
     * the handle. */
	if ((horizontal && (mev->x() < 0 || mev->x() > 9)) ||
		(!horizontal && (mev->y() < 0 || mev->y() > 9)))
	{
		/* Mouse is outside of the handle. If it's still hightlighed we have
		 * to de-highlight it. */
		if (mouseEntered)
		{
			mouseEntered = false;
			repaint();
		}
		return;
	}
	else
	{
		/* Mouse is over the handle. If the handle is not yet hightlighted we
		 * have to to it now. */
		if (!mouseEntered)
		{
			mouseEntered = true;
			repaint();
		}
	}
		
	if (!buttonDownOnHandle)
		return;
	buttonDownOnHandle = FALSE;

	if (position != Flat)
	{
		int ox, oy, ow, oh;

		QRect rr(Parent->geometry());
		ox = rr.x();
		oy = rr.y();
		ow = rr.width();
		oh = rr.height();
		if (Parent->inherits("KTMainWindow"))
		{
			QRect mainView = ((KTMainWindow*) Parent)->mainViewGeometry();

			ox += mainView.left();
			oy += mainView.top();
			ow = mainView.width();
			oh = mainView.height();
		}

		int fat = 25; //ness

		mgr = new KToolBoxManager(this, transparent);

		//Firt of all discover _your_ position

		if (position == Top )
			mgr->addHotSpot(geometry(), true);             // I'm on top
		else
			mgr->addHotSpot(rr.x(), oy, rr.width(), fat); // top

		if (position == Bottom)
			mgr->addHotSpot(geometry(), true);           // I'm on bottom
		else
			mgr->addHotSpot(rr.x(), oy+oh-fat, rr.width(), fat); // bottom

		if (position == Left)
			mgr->addHotSpot(geometry(), true);           // I'm on left
		else
			mgr->addHotSpot(ox, oy, fat, oh); // left

		if (position == Right)
			mgr->addHotSpot(geometry(), true);           // I'm on right
		else
			mgr->addHotSpot(ox+ow-fat, oy, fat, oh); //right

		movePos = position;
		connect (mgr, SIGNAL(onHotSpot(int)), SLOT(slotHotSpot(int)));
		if (transparent)
			mgr->doMove(true, false, true);
		else
		{
			/*
			  QList<KToolBarItem> ons;
			  for (KToolBarItem *b = items->first(); b; b=items->next())
			  {
			  if (b->isEnabled())
			  ons.append(b);
			  b->setEnabled(false);
			  }
			*/
			mgr->doMove(true, false, false);
			/*
			  for (KToolBarItem *b = ons.first(); b; b=ons.next())
			  b->setEnabled(true);
			*/
		}
		if (transparent)
		{
			setBarPos (movePos);

			if (movePos == Floating)
				move (mgr->x(), mgr->y());
			if (!isVisible())
				show();
		}
		mouseEntered = false;
		delete mgr;
		mgr=0;
		repaint (false);
	}
}

void KToolBar::mouseReleaseEvent ( QMouseEvent *m)
{
    buttonDownOnHandle = FALSE;
    if (mgr)
	mgr->stop();
    if ( position != Floating &&
	 ((horizontal && m->x()<9) || (!horizontal && m->y()<9)) ) {
	setFlat (position != Flat);
    }
}

void KToolBar::mousePressEvent ( QMouseEvent *m )
{
    buttonDownOnHandle |=   ((horizontal && m->x()<9) || (!horizontal && m->y()<9));

  if (moving)
      if (m->button() == RightButton)
	{
	    context->popup( mapToGlobal( m->pos() ), 0 );
	    buttonDownOnHandle = FALSE;
	    ContextCallback(0);
        }
      else if (m->button() == MidButton && position != Floating)
	  setFlat (position != Flat);
}

void KToolBar::slotHotSpot(int hs)
{
  if (mgr == 0)
    return;
  if (!transparent) // opaque
  {
    switch (hs)
    {
      case 0: //top
        setBarPos(Top);
        break;

      case 1: //bottom
        setBarPos(Bottom);
        break;

      case 2: //left
        setBarPos(Left);
        break;

      case 3: //right
        setBarPos(Right);
        break;

      case -1: // left all
        setBarPos(Floating);
        break;
    }
    if (position != Floating)
    {
      QPoint p(Parent->mapToGlobal(pos())); // OH GOOOOODDDD!!!!!
      mgr->setGeometry(p.x(), p.y(), width(), height());
    }
    if (!isVisible())
      show();
  }
  else // transparent
  {
    switch (hs)
    {
      case 0: //top
        mgr->setGeometry(0);
        movePos=Top;
        break;

      case 1: //bottom
        mgr->setGeometry(1);
        movePos=Bottom;
        break;

      case 2: //left
        mgr->setGeometry(2);
        movePos=Left;
        break;

      case 3: //right
        mgr->setGeometry(3);
        movePos=Right;
        break;

      case -1: // left all
        mgr->setGeometry(mgr->mouseX(), mgr->mouseY(), width(), height());
        movePos=Floating;
        break;
    }
  }
}

void KToolBar::resizeEvent(QResizeEvent*)
{
	/*
	 * The resize can affect the arrangement of the toolbar items so
	 * we have to call updateRects(). But we need not trigger another
	 * resizeEvent!  */
	updateRects();

	if (position == Floating)
	{
		/* It's flicker time again. If the size is under direct control of
		 * the WM we have to force the height to make the heightForWidth
		 * feature work. */
		if (horizontal)
		{
			/* horizontal bar */
			if (height() != heightForWidth(width()))
				resize(width(), heightForWidth(width()));
		}
		else
		{
			/* vertical bar */
			if (width() != widthForHeight(height()))
				resize(height(), widthForHeight(height()));
		}
	}
}

void KToolBar::paintEvent(QPaintEvent *)
{
  if (mgr)
    return;
  //MD Lots of rewrite

  // This code should be shared with the aequivalent in kmenubar!
  // (Marcin Dalecki).

  toolbarHeight = height ();
  if (position == Flat)
	  toolbarWidth = min_width;
  else
	  toolbarWidth = width ();

  int stipple_height;

  // Moved around a little to make variables available for KStyle (mosfet).

  QColorGroup g = QWidget::colorGroup();
  // Took higlighting handle from kmenubar - sven 040198
  QBrush b;
  if (mouseEntered && highlight)
      b = colorGroup().highlight(); // this is much more logical then
  // the hardwired value used before!!
  else
      b = QWidget::backgroundColor();

  QPainter *paint = new QPainter();
  paint->begin( this );

  if(kapp->kstyle()){
      kapp->kstyle()->drawKToolBar(paint, 0, 0, toolbarWidth, toolbarHeight,
                                   colorGroup(), position == Floating);
      if(moving){
          if(horizontal)
              kapp->kstyle()->drawKBarHandle(paint, 0, 0, 9, toolbarHeight,
                                             colorGroup(), true,  &b);
          else
              kapp->kstyle()->drawKBarHandle(paint, 0, 0, toolbarWidth, 9,
                                             colorGroup(), false, &b);
      }
      paint->end();
      delete paint;
      return;
  }

  if (moving)
  {
    // Handle point
    if (horizontal)
    {
      if (style() == MotifStyle)
      {
        qDrawShadePanel( paint, 0, 0, 9, toolbarHeight,
                         g , false, 1, &b);
        paint->setPen( g.light() );
	paint->drawLine( 9, 0, 9, toolbarHeight);
        stipple_height = 3;
        while ( stipple_height < toolbarHeight-4 ) {
          paint->drawPoint( 1, stipple_height+1);
          paint->drawPoint( 4, stipple_height);
          stipple_height+=3;
        }
        paint->setPen( g.dark() );
        stipple_height = 4;
        while ( stipple_height < toolbarHeight-4 ) {
          paint->drawPoint( 2, stipple_height+1);
          paint->drawPoint( 5, stipple_height);
          stipple_height+=3;
        }
      }
      else // Windows style handle
      {
        int w = 6;
        int h = toolbarHeight;
        paint->setClipRect(0, 2, w, h-4);

        qDrawPlainRect ( paint, 0, 0, 9, toolbarHeight,
                         g.mid(), 0, &b);

        paint->setPen( g.light() );
        int a=0-w;
        while (a <= h+5)
        {
          paint->drawLine(0, h-a, h, 0-a);
          paint->drawLine(0, h-a+1, h, 0-a+1);
          a +=6;
        }
        a=0-w;
        paint->setPen( g.dark() );
        while (a <= h+5)
        {
          paint->drawLine(0, h-a+2, h, 0-a+2);
          paint->drawLine(0, h-a+3, h, 0-a+3);
          a +=6;
        }
      }
    }
    else // vertical
    {
      if (style() == MotifStyle)
      {
        qDrawShadePanel( paint, 0, 0, toolbarWidth, 9,
                         g , false, 1, &b);

        paint->setPen( g.light() );
	paint->drawLine( 0, 9, toolbarWidth, 9);
        stipple_height = 3;
        while ( stipple_height < toolbarWidth-4 ) {
          paint->drawPoint( stipple_height+1, 1);
          paint->drawPoint( stipple_height, 4 );
          stipple_height+=3;
        }
        paint->setPen( g.dark() );
        stipple_height = 4;
        while ( stipple_height < toolbarWidth-4 ) {
          paint->drawPoint( stipple_height+1, 2 );
          paint->drawPoint( stipple_height, 5);
          stipple_height+=3;
        }
      }
      else
      {
        qDrawPlainRect( paint, 0, 0, toolbarWidth, 9,
                        g.mid(), 0, &b);

        int w = toolbarWidth;
        int h = 15;

        paint->setClipRect(2, 0, w-4, 6);

        //qDrawPlainRect ( paint, 0, 0, 9, toolbarHeight,
        //                 g.mid(), 0, &b);

        paint->setPen( g.light() );
        int a = 0-h;
        while (a <= w+h)
        {
          paint->drawLine(w-a, h, w-a+h, 0);
          paint->drawLine(w-a+1, h, w-a+1+h, 0);
          a +=6;
        }
        a = 0-h;
        paint->setPen( g.dark() );
        while (a <= w+h)
        {
          paint->drawLine(w-a+2, h, w-a+2+h, 0);
          paint->drawLine(w-a+3, h, w-a+3+h, 0);
          a +=6;
        }
      }

    }
  } //endif moving

  if (position != Floating)
    if ( style() == MotifStyle )
      qDrawShadePanel(paint, 0, 0, width(), height(), g , false, 1);
    //else
      //qDrawShadeRect(paint, 0, 0, width(), height(), g , true, 1);

  paint->end();
  delete paint;
}

void KToolBar::closeEvent (QCloseEvent *e)
{
  if (position == Floating)
   {
     setBarPos(lastPosition);
     e->ignore();
     return;
   }
  e->accept();
}


void KToolBar::ButtonClicked( int id )
{
  emit clicked( id );
}

void KToolBar::ButtonDblClicked( int id )
{
  emit doubleClicked( id );
}

void KToolBar::ButtonPressed( int id )
{
  emit pressed( id );
}

void KToolBar::ButtonReleased( int id )
{
  emit released( id );
}

void KToolBar::ButtonToggled( int id )
{
  emit toggled( id );
}

void KToolBar::ButtonHighlighted(int id, bool on )
{
  emit highlighted(id, on);
}


 /********************\
 *                    *
 * I N T E R F A C E  *
 *                    *
 \********************/

/***** BUTTONS *****/

/// Inserts a button.
int KToolBar::insertButton( const QPixmap& pixmap, int id, bool enabled,
			    const QString&_text, int index )
{
  KToolBarButton *button = new KToolBarButton( pixmap, id, this, 0L, item_size,
                                               _text);
  KToolBarItem *item = new KToolBarItem(button, ITEM_BUTTON, id,
                                        true);
  if ( index == -1 )
    items->append( item );
  else
    items->insert( index, item );

  connect(button, SIGNAL(clicked(int)), this, SLOT(ButtonClicked(int)));
  connect(button, SIGNAL(doubleClicked(int)), this, SLOT(ButtonDblClicked(int)));
  connect(button, SIGNAL(released(int)), this, SLOT(ButtonReleased(int)));
  connect(button, SIGNAL(pressed(int)), this, SLOT(ButtonPressed(int)));
  connect(button, SIGNAL(highlighted(int, bool)), this,
          SLOT(ButtonHighlighted(int, bool)));

  item->setEnabled( enabled );
  item->show();
  updateRects(true);
  return items->at();
}

/// Inserts a button with popup.
int KToolBar::insertButton( const QPixmap& pixmap, int id, QPopupMenu *_popup,
                            bool enabled, const QString&_text, int index)
{
  KToolBarButton *button = new KToolBarButton( pixmap, id, this,
                                               0L, item_size, _text);
  KToolBarItem *item = new KToolBarItem(button, ITEM_BUTTON, id,
                                        true);
  button->setPopup(_popup);

  if ( index == -1 )
    items->append( item );
  else
    items->insert( index, item );

  item->setEnabled( enabled );
  item->show();

  connect(button, SIGNAL(clicked(int)), this, SLOT(ButtonClicked(int)));
  connect(button, SIGNAL(doubleClicked(int)), this, SLOT(ButtonDblClicked(int)));
  connect(button, SIGNAL(released(int)), this, SLOT(ButtonReleased(int)));
  connect(button, SIGNAL(pressed(int)), this, SLOT(ButtonPressed(int)));
  connect(button, SIGNAL(highlighted(int, bool)), this,
          SLOT(ButtonHighlighted(int, bool)));

  updateRects(true);
  return items->at();
}


/// Inserts a button with connection.

int KToolBar::insertButton( const QPixmap& pixmap, int id, const char *signal,
			    const QObject *receiver, const char *slot, bool enabled,
			    const QString&_text, int index )
{
  KToolBarButton *button = new KToolBarButton( pixmap, id, this,
                                               0L, item_size, _text);
  KToolBarItem *item = new KToolBarItem(button, ITEM_BUTTON, id,
                                        true);

  if ( index == -1 )
    items->append( item );
  else
    items->insert( index, item );

  connect(button, SIGNAL(clicked(int)), this, SLOT(ButtonClicked(int)));
  connect(button, SIGNAL(doubleClicked(int)), this, SLOT(ButtonDblClicked(int)));
  connect(button, SIGNAL(released(int)), this, SLOT(ButtonReleased(int)));
  connect(button, SIGNAL(pressed(int)), this, SLOT(ButtonPressed(int)));
  connect(button, SIGNAL(highlighted(int, bool)), this,
          SLOT(ButtonHighlighted(int, bool)));

  connect( button, signal, receiver, slot );
  item->setEnabled( enabled );
  item->show();
  updateRects(true);
  return items->at();
}

/********* SEPARATOR *********/
/// Inserts separator

int KToolBar::insertSeparator( int index )
{
  KToolBarButton *separ = new KToolBarButton( this );
  KToolBarItem *item = new KToolBarItem(separ, ITEM_BUTTON, -1,
                                        true);

  if ( index == -1 )
    items->append( item );
  else
    items->insert( index, item );
	
  updateRects(true);
  return items->at();
}


/********* LINESEPARATOR *********/
/// Inserts line separator

int KToolBar::insertLineSeparator( int index )
{

  KSeparator *separ = new KSeparator(QFrame::VLine, this);

  KToolBarItem *item = new KToolBarItem(separ, ITEM_FRAME, -1, true);
  item->resize( 5, item_size - 2 );

  if ( index == -1 )
    items->append( item );
  else
    items->insert( index, item );
	
  updateRects(true);
  return items->at();
}


/********* Frame **********/
/// inserts QFrame

int KToolBar::insertFrame (int _id, int _size, int _index)
{
  debug ("insertFrame is deprecated. use insertWidget");

  QFrame *frame;
  bool mine = false;

  // ok I'll do it for you;
  frame = new QFrame (this);
  mine = true;

  KToolBarItem *item = new KToolBarItem(frame, ITEM_FRAME, _id, mine);

  if (_index == -1)
    items->append (item);
  else
    items->insert(_index, item);
  item-> resize (_size, item_size-2);
  item->show();
  updateRects(true);
  return items->at();
}
/* A poem all in G-s! No, any widget */

int KToolBar::insertWidget(int _id, int _size, QWidget *_widget,
		int _index )
{
  KToolBarItem *item = new KToolBarItem(_widget, ITEM_FRAME, _id, false);

  if (_index == -1)
    items->append (item);
  else
    items->insert(_index, item);
  item-> resize (_size, item_size-2);
  item->show();
  updateRects(true);
  return items->at();
}

/************** LINE EDITOR **************/
// Inserts a KLineEdit. KLineEdit is derived from QLineEdit and has
//  another signal, tabPressed, for completions.

int KToolBar::insertLined(const QString& text, int id, const char *signal,
			  const QObject *receiver, const char *slot,
			  bool enabled, const QString& tooltiptext, int size, int index)
{
  KLineEdit *lined = new KLineEdit (this, 0);
  KToolBarItem *item = new KToolBarItem(lined, ITEM_LINED, id,
                                        true);


  if (index == -1)
    items->append (item);
  else
    items->insert(index, item);
  if (!tooltiptext.isNull())
    QToolTip::add( lined, tooltiptext );
  connect( lined, signal, receiver, slot );
  lined->setText(text);
  item->resize(size, item_size-2);
  item->setEnabled(enabled);
  item->show();
  updateRects(true);
  return items->at();
}

/************** COMBO BOX **************/
/// Inserts comboBox with QStrList

int KToolBar::insertCombo (QStrList *list, int id, bool writable,
                           const char *signal, QObject *receiver,
                           const char *slot, bool enabled,
                           const QString& tooltiptext,
                           int size, int index,
                           QComboBox::Policy policy)
{
  QComboBox *combo = new QComboBox (writable, this);
  KToolBarItem *item = new KToolBarItem(combo, ITEM_COMBO, id,
                                        true);

  if (index == -1)
    items->append (item);
  else
    items->insert (index, item);
  combo->insertStrList (list);
  combo->setInsertionPolicy(policy);
  if (!tooltiptext.isNull())
    QToolTip::add( combo, tooltiptext );
  connect ( combo, signal, receiver, slot );
  combo->setAutoResize(false);
  item->resize(size, item_size-2);
  item->setEnabled(enabled);
  item->show();
  updateRects(true);
  return items->at();
}

/// Inserts comboBox with QStringList

int KToolBar::insertCombo (const QStringList &list, int id, bool writable,
                           const char *signal, QObject *receiver,
                           const char *slot, bool enabled,
                           const QString& tooltiptext,
                           int size, int index,
                           QComboBox::Policy policy)
{
  QComboBox *combo = new QComboBox (writable, this);
  KToolBarItem *item = new KToolBarItem(combo, ITEM_COMBO, id,
                                        true);

  if (index == -1)
    items->append (item);
  else
    items->insert (index, item);
  combo->insertStringList (list);
  combo->setInsertionPolicy(policy);
  if (!tooltiptext.isNull())
    QToolTip::add( combo, tooltiptext );
  connect ( combo, signal, receiver, slot );
  combo->setAutoResize(false);
  item->resize(size, item_size-2);
  item->setEnabled(enabled);
  item->show();
  updateRects(true);
  return items->at();
}


/// Inserts combo with text

int KToolBar::insertCombo (const QString& text, int id, bool writable,
                           const char *signal, QObject *receiver,
                           const char *slot, bool enabled,
                           const QString& tooltiptext, int size, int index,
                           QComboBox::Policy policy)
{
  QComboBox *combo = new QComboBox (writable, this);
  KToolBarItem *item = new KToolBarItem(combo, ITEM_COMBO, id,
                                        true);

  if (index == -1)
    items->append (item);
  else
    items->insert (index, item);
  combo->insertItem (text);
  combo->setInsertionPolicy(policy);
  if (!tooltiptext.isNull())
    QToolTip::add( combo, tooltiptext );
  connect (combo, signal, receiver, slot);
  combo->setAutoResize(false);
  item->resize(size, item_size-2);
  item->setEnabled(enabled);
  item->show();
  updateRects(true);
  return items->at();
}

/// Removes item by ID

void KToolBar::removeItem (int id)
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
    {
      if(b->isAuto())
        haveAutoSized=false;
      items->remove();
    }
  updateRects(true);
}

void KToolBar::showItem (int id)
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
    {
      if(b->isAuto())
        haveAutoSized=false;
      b->show();
    }
  updateRects(true);
}

void KToolBar::hideItem (int id)
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
    {
      if(b->isAuto())
        haveAutoSized=false;
      b->hide();
    }
  updateRects(true);
}
/// ******** Tools

/// misc
void KToolBar::addConnection (int id, const char *signal,
                              const QObject *receiver, const char *slot)
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
           connect (b->getItem(), signal, receiver, slot);
}

/// Common
void KToolBar::setItemEnabled( int id, bool enabled )
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
      b->setEnabled(enabled);
}

void KToolBar::setItemAutoSized ( int id, bool enabled )
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
    {
      b->autoSize(enabled);
      haveAutoSized = true;
	  updateRects(true);
    }
}

void KToolBar::alignItemRight(int id, bool yes)
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
    {
      b->alignRight (yes);
	  updateRects();
    }
}

/// Butoons
void KToolBar::setButtonPixmap( int id, const QPixmap& _pixmap )
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
      ((KToolBarButton *) b->getItem())->setPixmap( _pixmap );
}


void KToolBar::setDelayedPopup (int id , QPopupMenu *_popup)
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
      ((KToolBarButton *) b->getItem())->setDelayedPopup(_popup);
}


/// Toggle buttons
void KToolBar::setToggle ( int id, bool yes )
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
    {
      ((KToolBarButton *) b->getItem())->beToggle(yes);
      connect (b->getItem(), SIGNAL(toggled(int)),
               this, SLOT(ButtonToggled(int)));
    }
}

void KToolBar::toggleButton (int id)
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
    {
      if (((KToolBarButton *) b->getItem())->isToggleButton() == true)
        ((KToolBarButton *) b->getItem())->toggle();
    }
}

void KToolBar::setButton (int id, bool on)
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
      ((KToolBarButton *) b->getItem())->on(on);
}

//Autorepeat buttons
void KToolBar::setAutoRepeat (int id, bool flag /*, int delay, int repeat */)
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
      ((KToolBarButton *) b->getItem())->setAutoRepeat(flag);
}


bool KToolBar::isButtonOn (int id)
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
    {
      if (((KToolBarButton *) b->getItem())->isToggleButton() == true)
        return ((KToolBarButton *) b->getItem())->isOn();
    }
  return false;
}

/// Lined
void KToolBar::setLinedText (int id, const QString& text)
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
    {
      ((KLineEdit *) b->getItem())->setText(text);
    }
}

QString KToolBar::getLinedText (int id )
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
      return ((KLineEdit *) b->getItem())->text();
  return QString::null;
}

/// Combos
void KToolBar::insertComboItem (int id, const QString& text, int index)
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
    {
      ((QComboBox *) b->getItem())->insertItem(text, index);
    }
}

void KToolBar::insertComboList (int id, QStrList *list, int index)
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
	((QComboBox *) b->getItem())->insertStrList(list, index);
}

void KToolBar::insertComboList (int id, const QStringList &list, int index)
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
	((QComboBox *) b->getItem())->insertStringList(list, index);
}

void KToolBar::setCurrentComboItem (int id, int index)
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
    {
      ((QComboBox *) b->getItem())->setCurrentItem(index);
    }
}

void KToolBar::removeComboItem (int id, int index)
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
      ((QComboBox *) b->getItem())->removeItem(index);
}

void KToolBar::changeComboItem  (int id, const QString& text, int index)
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
    {
      if (index == -1)
      {
        index = ((QComboBox *) b->getItem())->currentItem();
        ((QComboBox *) b->getItem())->changeItem(text, index);
      }
      else
      {
        ((QComboBox *) b->getItem())->changeItem(text, index);
      }
    }
}

void KToolBar::clearCombo (int id)
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
	((QComboBox *) b->getItem())->clear();
}

QString KToolBar::getComboItem (int id, int index)
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
    {
      if (index == -1)
        index = ((QComboBox *) b->getItem())->currentItem();
      return ((QComboBox *) b->getItem())->text(index);
    }
  return QString::null;
}

QComboBox *KToolBar::getCombo (int id)
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
      return ((QComboBox *) b->getItem());
  return 0;
}

KLineEdit *KToolBar::getLined (int id)
{
  for (KToolBarItem *b = items->first(); b!=NULL; b=items->next())
    if (b->ID() == id )
      return ((KLineEdit *) b->getItem());
  return 0;
}


KToolBarButton* KToolBar::getButton( int id )
{
  for( KToolBarItem* b = items->first(); b != NULL; b = items->next() )
    if(b->ID() == id )
      return ((KToolBarButton *) b->getItem());
  return 0;
}

QFrame *KToolBar::getFrame (int id)
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
      return ((QFrame *) b->getItem());
  return 0;
}

QWidget *KToolBar::getWidget (int id)
{
  for (KToolBarItem *b = items->first(); b; b=items->next())
    if (b->ID() == id )
      return (b->getItem());
  return 0;
}


/// Toolbar itself

void KToolBar::setFullWidth(bool flag)
{
  fullSizeMode = flag;
}

bool KToolBar::fullSize() const
{
	return (fullSizeMode);
}

void KToolBar::enableMoving(bool flag)
{
  moving = flag;
}

void KToolBar::setBarPos(BarPosition bpos)
{
	if (position != bpos)
	{
		if (bpos == Floating)
		{
			lastPosition = position;
			position = bpos;
			oldX = x();
			oldY = y();
			oldWFlags = getWFlags();
			QPoint p = mapToGlobal(QPoint(0,0));
			parentOffset = pos();
			hide();
			emit moved (bpos);  // this sets up KTW but not toolbar which floats
			updateRects(false); // we need this to set us up
			recreate(0, 0, p, false);
			XSetTransientForHint( qt_xdisplay(), winId(), Parent->topLevelWidget()->winId());
			KWM::setDecoration(winId(), 2);
			KWM::moveToDesktop(winId(), KWM::desktop(Parent->winId()));
			setCaption(""); // this triggers a qt bug
			if (!title.isNull()){
				setCaption(title);
			} else {
				QString s = Parent->caption();
				s.append(" [tools]");
				setCaption(s);
			}
			context->changeItem (i18n("UnFloat"), CONTEXT_FLOAT);
			context->setItemEnabled (CONTEXT_FLAT, FALSE);
			setMouseTracking(true);
			mouseEntered=false;
			return;
		}
		else if (position == Floating) // was floating
		{
			position = bpos;
			hide();
			recreate(Parent, oldWFlags, QPoint(oldX, oldY), true);
			emit moved (bpos); // another bar::updateRects (damn) No! It's ok.
			context->changeItem (i18n("Float"), CONTEXT_FLOAT);
			context->setItemEnabled (CONTEXT_FLAT, TRUE);
			setMouseTracking(true);
			mouseEntered = false;
			updateRects ();
			return;
		}
		else
		{
			if (bpos == Flat)
			{
				setFlat (true);
				return;
			}
			position = bpos;
			enableFloating (true);
			emit moved ( bpos );
			updateRects();
			return;
		}
	}
}

void KToolBar::enableFloating (bool arrrrrrgh)
{
    context->setItemEnabled (CONTEXT_FLOAT, arrrrrrgh);
}

void KToolBar::setIconText(int icontext)
{
    bool doUpdate=false;

    if (icontext != icon_text)
    {
        icon_text=icontext;
        doUpdate=true;
    }

    if (doUpdate)
        emit modechange(); // tell buttons what happened
    if (isVisible ())
		updateRects(true);
}

bool KToolBar::enable(BarStatus stat)
{
  bool mystat = isVisible();

  if ( (stat == Toggle && mystat) || stat == Hide )
   {
     //if (position == Floating)
       //iconify();
     //else
     hide();       //Sven 09.12.1998: hide in both cases
   }
  else
    show();

  emit moved (position); // force KTM::updateRects (David)
  return ( isVisible() == mystat );
}

/*************************************************************

Mouse move and drag routines

*************************************************************/


void KToolBar::leaveEvent (QEvent *)
{
  if (mgr)
    return;
  mouseEntered = false;
  repaint();
}


void KToolBar::setFlat (bool flag)
{

#define also

  if (position == Floating )
    return;
  if ( flag == (position == Flat))
    also return;


  if (flag) //flat
  {
    context->changeItem (i18n("UnFlat"), CONTEXT_FLAT);
    lastPosition = position; // test float. I did and it works by miracle!?
    //debug ("Flat");
    position = Flat;
    horizontal = false;
    for (KToolBarItem *b = items->first(); b; b=items->next()) // Nasty hack:
      b->move(100,100);       // move items out of sight
    enableFloating(false);
  }
  else //unflat
  {
    context->changeItem (i18n("Flat"), CONTEXT_FLAT);
    //debug ("Unflat");
    setBarPos(lastPosition);
    enableFloating(true);
  }
  emit moved(Flat); // KTM will block this->updateRects
  updateRects();
}

#include "ktoolbar.moc"

