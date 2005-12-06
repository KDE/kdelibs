/* This file is part of the KDE libraries
   Copyright (C) 2000 Max Judin <novaprint@mtu-net.ru>
   Copyright (C) 2002,2003 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2005 Dominik Haumann <dhdev@gmx.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "k3dockwidget.h"
#include "k3dockwidget_p.h"
#include "k3dockwidget_private.h"

#include <qpainter.h>
#include <qcursor.h>
#include <kdebug.h>
#include <qtimer.h>
#include <qapplication.h>
#include <QResizeEvent>

#include <math.h> // need ceil

K3DockSplitter::K3DockSplitter(QWidget *parent, const char *name, Qt::Orientation orient, int pos)
: QWidget(parent, name)
{
  m_dontRecalc=false;
  divider = 0L;
  child0 = 0L;
  child1 = 0L;
  fixedWidth0=-1;
  fixedWidth1=-1;
  fixedHeight0=-1;
  fixedHeight1=-1;

  m_orientation = orient;
  mOpaqueResize = false;
  mKeepSize = false;
  setSeparatorPosInPercent( pos );
  initialised = false;
}

void K3DockSplitter::activate(QWidget *c0, QWidget *c1)
{
  if ( c0 ) child0 = c0;
  if ( c1 ) child1 = c1;

  setupMinMaxSize();

  if (divider) delete divider;
  divider = new QFrame(this, "pannerdivider");
  divider->setFrameStyle(QFrame::Panel | QFrame::Raised);
  divider->setLineWidth(1);
  divider->raise();

  if (m_orientation == Qt::Horizontal)
    divider->setCursor(QCursor(Qt::SizeVerCursor));
  else
    divider->setCursor(QCursor(Qt::SizeHorCursor));
  divider->installEventFilter(this);

  initialised= true;

  updateName();
  divider->show();

  // without this resize event, things will not work. why exactly? :(
  resizeEvent(0);


  K3DockWidget* dw0 = (K3DockWidget*) child0;
  K3DockWidget* dw1 = (K3DockWidget*) child1;

  // if fixed size is set, restore first, to restore xpos correctly
  if( fixedWidth0 != -1 || fixedHeight0 != -1 ) restoreFromForcedFixedSize( dw0 );
  if( fixedWidth1 != -1 || fixedHeight1 != -1 ) restoreFromForcedFixedSize( dw1 );


  // now force fixed sizes, if they are set.
  if( dw0->forcedFixedWidth() != -1 ) {
    setForcedFixedWidth( dw0, dw0->forcedFixedWidth() );
  }
  else if( dw1->forcedFixedWidth() != -1 ) {
    setForcedFixedWidth( dw1, dw1->forcedFixedWidth() );
  }

  if( dw0->forcedFixedHeight() != -1 ) {
    setForcedFixedHeight (dw0, dw0->forcedFixedHeight() );
  }
  else if( dw1->forcedFixedHeight() != -1 ) {
    setForcedFixedHeight( dw1, dw1->forcedFixedHeight() );
  }
}

/*
void K3DockSplitter::delayedResize()
{
	kdDebug(282)<<"*********************** DELAYED RESIZE !!!!!!!!!!!!!!!"<<endl;
	resizeEvent(0);
}*/

void K3DockSplitter::setForcedFixedWidth(K3DockWidget *dw,int w)
{
	if (dw==child0)
	{
		if (fixedWidth0==-1) savedXPos=xpos;
		if (w==fixedWidth0) return;
                fixedWidth0=w;
		setSeparatorPos(w*factor/width(),true);
//		kdDebug(282)<<"Set forced fixed width for widget 0 :"<<w<<endl;
	}
        else
	{
		if (fixedWidth1==-1) savedXPos=xpos;
		if (w==fixedWidth1) return;
                fixedWidth1=w;
		setSeparatorPos((width()-w)*factor/width(),true);
//		kdDebug(282)<<"Set forced fixed width for widget 1 :"<<w<<endl;
	}
	setupMinMaxSize();
    if (divider) divider->hide();
}

void K3DockSplitter::setForcedFixedHeight(K3DockWidget *dw,int h)
{
	if (dw==child0)
	{
		if (fixedHeight0==-1) savedXPos=xpos;
		if (h==fixedHeight0) return;
                fixedHeight0=h;
		setSeparatorPos(h*factor/height(),true);
// //		kdDebug(282)<<"Set forced fixed width for widget 0 :"<<h<<endl;
	}
        else
	{
		if (fixedHeight1==-1) savedXPos=xpos;
		if (h==fixedHeight1) return;
                fixedHeight1=h;
		setSeparatorPos((height()-h)*factor/height(),true);
//		kdDebug(282)<<"Set forced fixed height for widget 1 :"<<h<<endl;
	}
	setupMinMaxSize();
    if (divider) divider->hide();
}

void K3DockSplitter::restoreFromForcedFixedSize(K3DockWidget *dw)
{
    if (divider) divider->show();
	if (dw==child0)
	{
		fixedWidth0=-1;
		fixedHeight0=-1;
		setSeparatorPos(savedXPos,true);
	}
	else
	{
		fixedWidth1=-1;
		fixedHeight1=-1;
		setSeparatorPos(savedXPos,true);
	}
}


void K3DockSplitter::setupMinMaxSize()
{
  // Set the minimum and maximum sizes for the K3DockSplitter (this)
  int minx, maxx, miny, maxy;
  if (m_orientation == Qt::Horizontal) {
    miny = child0->minimumHeight() + child1->minimumHeight() + 4;
    maxy = child0->maximumHeight() + child1->maximumHeight() + 4;
    minx = (child0->minimumWidth() > child1->minimumWidth()) ? child0->minimumWidth() : child1->minimumWidth();
    maxx = (child0->maximumWidth() > child1->maximumWidth()) ? child0->maximumWidth() : child1->maximumWidth();

    if (miny < 4)     miny = 4;
    if (maxy > 32000) maxy = 32000;
    if (minx < 2)     minx = 2;
    if (maxx > 32000) maxx = 32000;
  }
  else
  {
    minx = child0->minimumWidth() + child1->minimumWidth() + 4;
    maxx = child0->maximumWidth() + child1->maximumWidth() + 4;
    miny = (child0->minimumHeight() > child1->minimumHeight()) ? child0->minimumHeight() : child1->minimumHeight();
    maxy = (child0->maximumHeight() > child1->maximumHeight()) ? child0->maximumHeight() : child1->maximumHeight();

    if (miny < 2)     miny = 2;
    if (maxy > 32000) maxy = 32000;
    if (minx < 4)     minx = 4;
    if (maxx > 32000) maxx = 32000;
  }

  setMinimumSize(minx, miny);
  setMaximumSize(maxx, maxy);
}

void K3DockSplitter::deactivate()
{
  if (divider) delete divider;
  divider = 0L;
  initialised= false;
}

int K3DockSplitter::separatorPosInPercent()
{
    return xpos / (factor/100);
}

void K3DockSplitter::setSeparatorPosInPercent(int percent)
{
    xpos = percent * (factor/100);
}

void K3DockSplitter::setSeparatorPos(int pos, bool do_resize)
{
  xpos = pos;
  if (do_resize)
    resizeEvent(0);
}

void K3DockSplitter::setSeparatorPosX(int pos, bool do_resize)
{
  savedXPos = pos;
  setSeparatorPos( pos, do_resize );
}

int K3DockSplitter::separatorPos() const
{
  return xpos;
}

void K3DockSplitter::resizeEvent(QResizeEvent *ev)
{
  //
  // As already stated in the .h file we always have to differentiate
  // between 6 modes.
  // If we can cast child0->getWidget() or child1.getWidget() to
  // K3DockContainer* we *do* have a dockwidget around. For dockwidgets
  // we have to take special care in the resizing routines, for example
  // if mKeepSize is true and the dockcontainer is on the bottom or right,
  // we always have to move the xpos splitter position. If there are no
  // dockcontainers around, resizing is handeled like if child0 would
  // be a dockcontainer.
  //

//  kdDebug(282)<<"ResizeEvent :"<< ((initialised) ? "initialised":"not initialised")<<", "<< ((ev) ? "real event":"")<<", "<<(isVisible() ?"visible":"")<<endl;

  if (initialised) {
    K3DockContainer *dc = 0L;
    K3DockWidget *c0 = (K3DockWidget*)child0;
    K3DockWidget *c1 = (K3DockWidget*)child1;
    bool stdHandling=false; // true: if closed or nonoverlap mode. false: overlap mode

    //
    // Check whether this is a real resize event or a pseudo resize event
    // Real resize events occure if the width() or height() changes. ev != 0L.
    // Pseudo resize events occure if the dockwidget mode changes (overlaped,
    // sticky or closed). ev == 0L.
    //
    if (ev && isVisible() && divider->isVisible()) {
      // real resize event.
//       kdDebug(282)<<"mKeepSize : "<< ((m_orientation == Horizontal) ? "Horizontal":"Vertical") <<endl;

      if (mKeepSize) {
        // keep the splitter on a fixed position. This may be a bit inaccurate, because
        // xpos saves a proportional value, which means there might occur rounding errors.
        // However, this works surprising well!
        if (m_orientation == Qt::Horizontal) {
          if (ev->oldSize().height() != ev->size().height()) {
            if( (c1->getWidget()) && (dc=dynamic_cast<K3DockContainer*>(c1->getWidget()))) {
              // dockwidget is on the bottom. move xpos so that the size from child1 stays
              xpos = (int)ceil(((double)factor) * checkValue(height() - child1->height() - 4) / height());
            } else {
              // xpos should not change, the docking is on the top
              // checkValue is *fuzzy* here, it leads to ugly rounding bugs
              // In truth, it is not needed, because it is called when calculating the "position".
              xpos = qRound(((double)xpos) * ev->oldSize().height() / height());
            }
          }
        } else {
          if (ev->oldSize().width() != width()) {
            if( (c1->getWidget()) && (dc=dynamic_cast<K3DockContainer*>(c1->getWidget()))) {
              xpos = (int)ceil(((double)factor) * checkValue(width() - child1->width() - 4) / width());
            } else {
              // xpos should not change
              // checkValue is *fuzzy* here, it leads to ugly rounding bugs
              xpos = qRound(((double)xpos) * ev->oldSize().width() / width());
            }
          }
        }
      } else {
        // dockwidget size proportional!
        // Which means, xpos is always right (ratio value). Do nothing! :)
      }
    }
    else
    {
      //
      // Maybe a multitabbartab was clicked, so force an update of the fixed
      // values.
      //
      if ( isVisible()) {
        if (m_orientation == Qt::Horizontal) {
          if (fixedHeight0!=-1)
            xpos = checkValue(fixedHeight0) * factor / height();
          else if (fixedHeight1!=-1)
            xpos = checkValue(height()-fixedHeight1) * factor / height();
        }
        else
        {
          if (fixedWidth0!=-1)
            xpos = checkValue(fixedWidth0) * factor / width();
          else if (fixedWidth1!=-1)
            xpos = checkValue(width()-fixedWidth1) * factor / width();
        }
      }
//      else kdDebug(282)<<"Something else happened"<<endl;
    }

/*
    // --- debugging information ---
    kdDebug(282) << "isVisible() is         : " << isVisible() << endl;
    kdDebug(282) << "Orientation            : " << (m_orientation==Horizontal?"Horizontal":"Vertical")
                                                << endl;
    kdDebug(282) << "Splitter visibility    : " << divider->isVisible() << endl;;
    kdDebug(282) << "Splitter procentual pos: " << xpos << endl;
    if (c0->getWidget()) {
        dc=dynamic_cast<K3DockContainer*>(c0->getWidget());
        kdDebug(282) << "Child 0 K3DockContainer?: " << dc << endl;
    }
    if (c1->getWidget()) {
        dc=dynamic_cast<K3DockContainer*>(c1->getWidget());
        kdDebug(282) << "Child 1 K3DockContainer?: " << dc << endl;
    }
    kdDebug(282) << "Child0                 : " << child0 << endl;
    kdDebug(282) << "child1                 : " << child1 << endl;
*/

    //
    // handle overlapped widgets only.
    //
    if( ( (m_orientation==Qt::Vertical) &&((fixedWidth0==-1) && (fixedWidth1==-1)) ) ||
        ( (m_orientation==Qt::Horizontal)  &&((fixedHeight0==-1) && (fixedHeight1==-1)) ) ) {
      if ((c0->getWidget()) && (dc=dynamic_cast<K3DockContainer*>(c0->getWidget()))
           && (dc->isOverlapMode())) {
        // child0 ist a K3DockContainer
        int position;
        child0->show();
        child0->raise();
        divider->raise();
        if (m_orientation == Qt::Horizontal) {
          position = checkValueOverlapped( height() * xpos / factor, child0 );
          child0->setGeometry(0, 0, width(), position);
          child1->setGeometry(0, dc->m_nonOverlapSize, width(), height()-dc->m_nonOverlapSize);
          divider->setGeometry(0, position, width(), 4);
        } else {
          position = checkValueOverlapped( width() * xpos / factor, child0 );
          child0->setGeometry(0, 0, position, height());
          child1->setGeometry(dc->m_nonOverlapSize, 0, width()-dc->m_nonOverlapSize, height());
          divider->setGeometry(position, 0, 4, height());
        }
      } else {
        if ((c1->getWidget()) && (dc=dynamic_cast<K3DockContainer*>(c1->getWidget()))
             && (dc->isOverlapMode())) {
          // child1 ist a K3DockContainer
          int position;
          child1->show();
          child1->raise();
          divider->raise();
          if (m_orientation == Qt::Horizontal) {
            position = checkValueOverlapped( height() * xpos / factor, child1 );
            child0->setGeometry(0, 0, width(), height()-dc->m_nonOverlapSize);
            child1->setGeometry(0, position+4, width(), height()-position-4);
            divider->setGeometry(0, position, width(), 4);
          } else {
            position = checkValueOverlapped( width() * xpos / factor, child1 );
            child0->setGeometry(0, 0, width()-dc->m_nonOverlapSize, height());
            child1->setGeometry(position+4, 0, width()-position-4, height());
            divider->setGeometry(position, 0, 4, height());
          }
        }
        else // no K3DockContainer available, this means the mode cannot be overlapped
          stdHandling=true;
      }
    }
    else // no K3DockContainer available
      stdHandling=true;

    //
    // stdHandling == true means either sticky mode (=nonoverlap mode) or
    // closed mode. In both modes the widgets do *not* overlap, so we know
    // the child0 and child1 adjoin.
    //
    if (stdHandling) {
      int position = checkValue( (m_orientation == Qt::Vertical ? width() : height()) * xpos / factor );
      int diff = 0;

      if (m_orientation == Qt::Horizontal) {
        if ((c1->getWidget()) && (dc=dynamic_cast<K3DockContainer*>(c1->getWidget()))) {
          // bottom is dockcontainer
          if( divider->isVisible() ) {
            child0->setGeometry(0, 0, width(), position);
            child1->setGeometry(0, position+4, width(), height()-position-4);
          } else {
            child0->setGeometry(0, 0, width(), height()-dc->m_nonOverlapSize);
            child1->setGeometry(0, height()-dc->m_nonOverlapSize, width(), height());
          }
        } else {
          if( divider->isVisible() ) diff = 4;
          child0->setGeometry(0, 0, width(), position);
          child1->setGeometry(0, position+diff, width(), height()-position-diff);
        }
      divider->setGeometry(0, position, width(), 4);
      } else {
        if ((c1->getWidget()) && (dc=dynamic_cast<K3DockContainer*>(c1->getWidget()))) {
          // right is dockcontainer
          if( divider->isVisible() ) {
            child0->setGeometry(0, 0, position, height());
            child1->setGeometry(position+4, 0, width()-position-4, height());
          } else {
            child0->setGeometry(0, 0, width()-dc->m_nonOverlapSize, height());
            child1->setGeometry(width()-dc->m_nonOverlapSize, 0, width(), height());
          }
        } else {
          if( divider->isVisible() ) diff = 4;
          child0->setGeometry(0, 0, position, height());
          child1->setGeometry(position+diff, 0, width()-position-diff, height());
        }
        divider->setGeometry(position, 0, 4, height());
      }
    }
  }
}

int K3DockSplitter::checkValueOverlapped(int position, QWidget *overlappingWidget) const
{
  if (initialised) {
    if (m_orientation == Qt::Vertical) {
      if (child0==overlappingWidget) {
        if (position < child0->minimumWidth() || position > width())
          position = child0->minimumWidth();
      } else {
        if (position > (width()-child1->minimumWidth()-4) || position < 0)
          position = width()-child1->minimumWidth()-4;
      }
    } else {// orientation  == Horizontal
      if (child0==overlappingWidget) {
        if (position < (child0->minimumHeight()) || position > height())
          position = child0->minimumHeight();
      } else {
        if (position>(height()-child1->minimumHeight()-4) || position < 0)
          position = height()-child1->minimumHeight()-4;
      }
    }
  }
  return position;
}

int K3DockSplitter::checkValue( int position ) const
{
  if (initialised) {
    if (m_orientation == Qt::Vertical) {
      if (position < child0->minimumWidth())
        position = child0->minimumWidth();
      if ((width()-4-position) < (child1->minimumWidth()))
        position = width() - (child1->minimumWidth()) - 4;
    } else {
      if (position < (child0->minimumHeight()))
        position = child0->minimumHeight();
      if ((height()-4-position) < child1->minimumHeight())
        position = height() - (child1->minimumHeight()) - 4;
    }
  }

  if (position < 0) position = 0;

  if ((m_orientation == Qt::Vertical) && (position > width()))
    position = width();
  if ((m_orientation == Qt::Horizontal) && (position > height()))
    position = height();

  return position;
}

bool K3DockSplitter::eventFilter(QObject *o, QEvent *e)
{
  QMouseEvent *mev;
  bool handled = false;

  switch (e->type()) {
    case QEvent::MouseMove:
      mev= (QMouseEvent*)e;
      child0->setUpdatesEnabled(mOpaqueResize);
      child1->setUpdatesEnabled(mOpaqueResize);
      if (m_orientation == Qt::Horizontal) {
        if ((fixedHeight0!=-1) || (fixedHeight1!=-1))
        {
          handled=true; break;
        }

        if (!mOpaqueResize) {
          int position = checkValue( mapFromGlobal(mev->globalPos()).y() );
          divider->move( 0, position );
        } else {
          int tmp_xpos = factor * checkValue( mapFromGlobal(mev->globalPos()).y() ) / height();
          if (tmp_xpos != xpos) {
            xpos = tmp_xpos;
            resizeEvent(0);
            divider->repaint(true);
          }
        }
      } else {
        if ((fixedWidth0!=-1) || (fixedWidth1!=-1))
        {
          handled=true; break;
        }
        if (!mOpaqueResize) {
          int position = checkValue( mapFromGlobal(QCursor::pos()).x() );
          divider->move( position, 0 );
        } else {
          int tmp_xpos = factor * checkValue( mapFromGlobal( mev->globalPos()).x() ) / width();
          if (tmp_xpos != xpos) {
            xpos = tmp_xpos;
            resizeEvent(0);
            divider->repaint(true);
          }
        }
      }
      handled= true;
      break;
    case QEvent::MouseButtonRelease:
      child0->setUpdatesEnabled(true);
      child1->setUpdatesEnabled(true);
      mev= (QMouseEvent*)e;
      if (m_orientation == Qt::Horizontal){
        if ((fixedHeight0!=-1) || (fixedHeight1!=-1))
        {
          handled=true; break;
        }
        xpos = factor* checkValue( mapFromGlobal(mev->globalPos()).y() ) / height();
        resizeEvent(0);
        divider->repaint(true);
      } else {
        if ((fixedWidth0!=-1) || (fixedWidth1!=-1))
        {
          handled=true; break;
        }
        xpos = factor* checkValue( mapFromGlobal(mev->globalPos()).x() ) / width();
        resizeEvent(0);
        divider->repaint(true);
      }
      handled= true;
      break;
    default:
      break;
  }
  return (handled) ? true : QWidget::eventFilter( o, e );
}

bool K3DockSplitter::event( QEvent* e )
{
  if ( e->type() == QEvent::LayoutHint ){
    // change children min/max size. This is needed, otherwise
    // it is possible the divider get's out of bounds.
    setupMinMaxSize();
    resizeEvent(0);
  }
  return QWidget::event(e);
}

QWidget* K3DockSplitter::getAnother( QWidget* w ) const
{
  return ( w == child0 ) ? child1 : child0;
}

void K3DockSplitter::updateName()
{
  if ( !initialised ) return;

  QString new_name = QString( child0->name() ) + "," + child1->name();
  parentWidget()->setName( new_name.latin1() );
  parentWidget()->setCaption( child0->caption() + "," + child1->caption() );
  parentWidget()->repaint( false );

  ((K3DockWidget*)parentWidget())->firstName = child0->name();
  ((K3DockWidget*)parentWidget())->lastName = child1->name();
  ((K3DockWidget*)parentWidget())->splitterOrientation = m_orientation;

  QWidget* p = parentWidget()->parentWidget();
  if ( p && p->inherits("K3DockSplitter" ) )
    ((K3DockSplitter*)p)->updateName();
}

void K3DockSplitter::setOpaqueResize(bool b)
{
  mOpaqueResize = b;
}

bool K3DockSplitter::opaqueResize() const
{
  return mOpaqueResize;
}

void K3DockSplitter::setKeepSize(bool b)
{
  mKeepSize = b;
}

bool K3DockSplitter::keepSize() const
{
  return mKeepSize;
}



/*************************************************************************/
K3DockButton_Private::K3DockButton_Private( QWidget *parent, const char * name )
:QPushButton( parent, name )
{
  moveMouse = false;
  setFocusPolicy( Qt::NoFocus );
}

K3DockButton_Private::~K3DockButton_Private()
{
}

void K3DockButton_Private::drawButton( QPainter* p )
{
  p->fillRect( 0,0, width(), height(), QBrush(colorGroup().brush(QColorGroup::Background)) );
  p->drawPixmap( (width() - pixmap()->width()) / 2, (height() - pixmap()->height()) / 2, *pixmap() );
  if ( moveMouse && !isDown() ){
    p->setPen( Qt::white );
    p->drawLine( 0, height() - 1, 0, 0 );
    p->drawLine( 0, 0, width() -1 , 0 );

    p->setPen( colorGroup().dark() );
    p->drawLine( width() -1, 0, width() - 1, height() - 1 );
    p->drawLine( width() - 1, height() - 1, 0, height() - 1 );
  }
  if ( isOn() || isDown() ){
    p->setPen( colorGroup().dark() );
    p->drawLine(0 , height() - 1, 0, 0);
    p->drawLine(0, 0, width() -1 , 0);

    p->setPen( Qt::white );
    p->drawLine(width() - 1, height() - 1, 0, height() - 1);
  }
}

void K3DockButton_Private::enterEvent( QEvent * )
{
  moveMouse = true;
  repaint();
}

void K3DockButton_Private::leaveEvent( QEvent * )
{
  moveMouse = false;
  repaint();
}

void K3DockButton_Private::paintEvent( QPaintEvent * )
{
  QPainter painter( this );
  //KDE4 enable this if you want to use the custom drawing function, though pixmap() returning null, it will crash :)
//  drawButton( &painter );
}

/*************************************************************************/
K3DockWidgetPrivate::K3DockWidgetPrivate()
  : QObject()
  ,index(-1)
  ,splitPosInPercent(50)
  ,pendingFocusInEvent(false)
  ,blockHasUndockedSignal(false)
  ,pendingDtor(false)
  ,forcedWidth(-1)
  ,forcedHeight(-1)
  ,isContainer(false)
  ,container(0)
  ,resizePos(0,0)
  ,resizing(false)
{
#ifndef NO_KDE2
  windowType = NET::Normal;
#endif

  _parent = 0L;
  transient = false;
}

K3DockWidgetPrivate::~K3DockWidgetPrivate()
{
}

void K3DockWidgetPrivate::slotFocusEmbeddedWidget(QWidget* w)
{
   if (w) {
      QWidget* embeddedWdg = ((K3DockWidget*)w)->getWidget();
      if (embeddedWdg && ((embeddedWdg->focusPolicy() == Qt::ClickFocus) || (embeddedWdg->focusPolicy() == Qt::StrongFocus))) {
         embeddedWdg->setFocus();
      }
   }
}

#ifndef NO_INCLUDE_MOCFILES // for Qt-only projects, because tmake doesn't take this name
#include "k3dockwidget_private.moc"
#endif
