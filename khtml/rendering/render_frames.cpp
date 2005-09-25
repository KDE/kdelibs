/**
 * This file is part of the KDE project.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Simon Hausmann <hausmann@kde.org>
 *           (C) 2000 Stefan Schimanski (1Stein@gmx.de)
 *           (C) 2003 Apple Computer, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
//#define DEBUG_LAYOUT

#include "rendering/render_frames.h"
#include "rendering/render_canvas.h"
#include "html/html_baseimpl.h"
#include "html/html_objectimpl.h"
#include "html/htmltokenizer.h"
#include "misc/htmlattrs.h"
#include "xml/dom2_eventsimpl.h"
#include "xml/dom_docimpl.h"
#include "misc/htmltags.h"
#include "khtmlview.h"
#include "khtml_part.h"

#include <kapplication.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <klocale.h>
#include <kdebug.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qcursor.h>

#include <assert.h>

using namespace khtml;
using namespace DOM;

RenderFrameSet::RenderFrameSet( HTMLFrameSetElementImpl *frameSet)
    : RenderBox(frameSet)
{
  // init RenderObject attributes
    setInline(false);

  for (int k = 0; k < 2; ++k) {
      m_gridLen[k] = -1;
      m_gridDelta[k] = 0;
      m_gridLayout[k] = 0;
  }

  m_resizing = m_clientresizing= false;

  m_cursor = Qt::ArrowCursor;

  m_hSplit = -1;
  m_vSplit = -1;

  m_hSplitVar = 0;
  m_vSplitVar = 0;
}

RenderFrameSet::~RenderFrameSet()
{
    for (int k = 0; k < 2; ++k) {
        delete [] m_gridLayout[k];
        delete [] m_gridDelta[k];
    }
    delete [] m_hSplitVar;
    delete [] m_vSplitVar;
}

bool RenderFrameSet::nodeAtPoint(NodeInfo& info, int _x, int _y, int _tx, int _ty, HitTestAction hitTestAction,  bool inBox)
{
    RenderBox::nodeAtPoint(info, _x, _y, _tx, _ty, hitTestAction, inBox);

    bool inside = m_resizing || canResize(_x, _y);

    if ( inside && element() && !element()->noResize() && !info.readonly()) {
        info.setInnerNode(element());
        info.setInnerNonSharedNode(element());
    }

    return inside || m_clientresizing;
}

void RenderFrameSet::layout( )
{
    KHTMLAssert( needsLayout() );
    KHTMLAssert( minMaxKnown() );

    if ( !parent()->isFrameSet() ) {
        KHTMLView* view = canvas()->view();
        m_width = view ? view->visibleWidth() : 0;
        m_height = view ? view->visibleHeight() : 0;
    }

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(FrameSet)::layout( ) width=" << width() << ", height=" << height() << endl;
#endif

    int remainingLen[2];
    remainingLen[1] = m_width - (element()->totalCols()-1)*element()->border();
    if(remainingLen[1]<0) remainingLen[1]=0;
    remainingLen[0] = m_height - (element()->totalRows()-1)*element()->border();
    if(remainingLen[0]<0) remainingLen[0]=0;

    int availableLen[2];
    availableLen[0] = remainingLen[0];
    availableLen[1] = remainingLen[1];

    if (m_gridLen[0] != element()->totalRows() || m_gridLen[1] != element()->totalCols()) {
        // number of rows or cols changed
        // need to zero out the deltas
        m_gridLen[0] = element()->totalRows();
        m_gridLen[1] = element()->totalCols();
        for (int k = 0; k < 2; ++k) {
            delete [] m_gridDelta[k];
            m_gridDelta[k] = new int[m_gridLen[k]];
            delete [] m_gridLayout[k];
            m_gridLayout[k] = new int[m_gridLen[k]];
            for (int i = 0; i < m_gridLen[k]; ++i)
                m_gridDelta[k][i] = 0;
        }
    }

    for (int k = 0; k < 2; ++k) {
        int totalRelative = 0;
        int totalFixed = 0;
        int totalPercent = 0;
        int countRelative = 0;
        int countPercent = 0;
        int gridLen = m_gridLen[k];
        int* gridDelta = m_gridDelta[k];
        khtml::Length* grid =  k ? element()->m_cols : element()->m_rows;
        int* gridLayout = m_gridLayout[k];

        if (grid) {
            // first distribute the available width for fixed rows, then handle the
            // percentage ones and distribute remaining over relative
            for(int i = 0; i< gridLen; ++i)
                if (grid[i].isFixed()) {
                    gridLayout[i] = kMin(grid[i].value() > 0 ? grid[i].value() : 0, remainingLen[k]);
                    remainingLen[k] -= gridLayout[i];
                    totalFixed += gridLayout[i];
                }
                else if(grid[i].isRelative()) {
                    totalRelative += grid[i].value() > 1 ? grid[i].value() : 1;
                    countRelative++;
                }

            for(int i = 0; i < gridLen; i++)
                if(grid[i].isPercent()) {
                    gridLayout[i] = kMin(kMax(grid[i].width(availableLen[k]), 0), remainingLen[k]);
                    remainingLen[k] -= gridLayout[i];
                    totalPercent += grid[i].value();
                    countPercent++;
                }

            assert(remainingLen[k] >= 0);

            if (countRelative) {
                int remaining = remainingLen[k];
                for (int i = 0; i < gridLen; ++i)
                    if (grid[i].isRelative()) {
                        gridLayout[i] = ((grid[i].value() > 1 ? grid[i].value() : 1) * remaining) / totalRelative;
                        remainingLen[k] -= gridLayout[i];
                    }
            }

            // distribute the rest
            if (remainingLen[k]) {
                LengthType distributeType = countPercent ? Percent : Fixed;
                int total = countPercent ? totalPercent : totalFixed;
                if (!total) total = 1;
                for (int i = 0; i < gridLen; ++i)
                    if (grid[i].type() == distributeType) {
                        int toAdd = (remainingLen[k] * grid[i].value()) / total;
                        gridLayout[i] += toAdd;
                    }
            }

            // now we have the final layout, distribute the delta over it
            bool worked = true;
            for (int i = 0; i < gridLen; ++i) {
                if (gridLayout[i] && gridLayout[i] + gridDelta[i] <= 0)
                    worked = false;
                gridLayout[i] += gridDelta[i];
            }
            // now the delta's broke something, undo it and reset deltas
            if (!worked)
                for (int i = 0; i < gridLen; ++i) {
                    gridLayout[i] -= gridDelta[i];
                    gridDelta[i] = 0;
                }
        }
        else
            gridLayout[0] = remainingLen[k];
    }

    positionFrames();

    RenderObject *child = firstChild();
    if ( !child )
      return;

    if(!m_hSplitVar && !m_vSplitVar)
    {
#ifdef DEBUG_LAYOUT
        kdDebug( 6031 ) << "calculationg fixed Splitters" << endl;
#endif
        if(!m_vSplitVar && element()->totalCols() > 1)
        {
            m_vSplitVar = new bool[element()->totalCols()];
            for(int i = 0; i < element()->totalCols(); i++) m_vSplitVar[i] = true;
        }
        if(!m_hSplitVar && element()->totalRows() > 1)
        {
            m_hSplitVar = new bool[element()->totalRows()];
            for(int i = 0; i < element()->totalRows(); i++) m_hSplitVar[i] = true;
        }

        for(int r = 0; r < element()->totalRows(); r++)
        {
            for(int c = 0; c < element()->totalCols(); c++)
            {
                bool fixed = false;

                if ( child->isFrameSet() )
                  fixed = static_cast<RenderFrameSet *>(child)->element()->noResize();
                else
                  fixed = static_cast<RenderFrame *>(child)->element()->noResize();

                if(fixed)
                {
#ifdef DEBUG_LAYOUT
                    kdDebug( 6031 ) << "found fixed cell " << r << "/" << c << "!" << endl;
#endif
                    if( element()->totalCols() > 1)
                    {
                        if(c>0) m_vSplitVar[c-1] = false;
                        m_vSplitVar[c] = false;
                    }
                    if( element()->totalRows() > 1)
                    {
                        if(r>0) m_hSplitVar[r-1] = false;
                        m_hSplitVar[r] = false;
                    }
                    child = child->nextSibling();
                    if(!child) goto end2;
                }
#ifdef DEBUG_LAYOUT
                else
                    kdDebug( 6031 ) << "not fixed: " << r << "/" << c << "!" << endl;
#endif
            }
        }

    }
    RenderContainer::layout();
 end2:
    setNeedsLayout(false);
}

void RenderFrameSet::positionFrames()
{
  int r;
  int c;

  RenderObject *child = firstChild();
  if ( !child )
    return;

  //  NodeImpl *child = _first;
  //  if(!child) return;

  int yPos = 0;

  for(r = 0; r < element()->totalRows(); r++)
  {
    int xPos = 0;
    for(c = 0; c < element()->totalCols(); c++)
    {
      child->setPos( xPos, yPos );
#ifdef DEBUG_LAYOUT
      kdDebug(6040) << "child frame at (" << xPos << "/" << yPos << ") size (" << m_gridLayout[1][c] << "/" << m_gridLayout[0][r] << ")" << endl;
#endif
      // has to be resized and itself resize its contents
      if ((m_gridLayout[1][c] != child->width()) || (m_gridLayout[0][r] != child->height())) {
          child->setWidth( m_gridLayout[1][c] );
          child->setHeight( m_gridLayout[0][r] );
          child->setNeedsLayout(true);
          child->layout();
      }

      xPos += m_gridLayout[1][c] + element()->border();
      child = child->nextSibling();

      if ( !child )
        return;

    }

    yPos += m_gridLayout[0][r] + element()->border();
  }

  // all the remaining frames are hidden to avoid ugly
  // spurious unflowed frames
  while ( child ) {
      child->setWidth( 0 );
      child->setHeight( 0 );
      child->setNeedsLayout(false);

      child = child->nextSibling();
  }
}

bool RenderFrameSet::userResize( MouseEventImpl *evt )
{
    if (needsLayout()) return false;

  bool res = false;
  int _x = evt->clientX();
  int _y = evt->clientY();

  if ( !m_resizing && evt->id() == EventImpl::MOUSEMOVE_EVENT || evt->id() == EventImpl::MOUSEDOWN_EVENT )
  {
#ifdef DEBUG_LAYOUT
    kdDebug( 6031 ) << "mouseEvent:check" << endl;
#endif

    m_hSplit = -1;
    m_vSplit = -1;
    //bool resizePossible = true;

    // check if we're over a horizontal or vertical boundary
    int pos = m_gridLayout[1][0] + xPos();
    for(int c = 1; c < element()->totalCols(); c++)
    {
      if(_x >= pos && _x <= pos+element()->border())
      {
        if(m_vSplitVar && m_vSplitVar[c-1] == true) m_vSplit = c-1;
#ifdef DEBUG_LAYOUT
        kdDebug( 6031 ) << "vsplit!" << endl;
#endif
        res = true;
        break;
      }
      pos += m_gridLayout[1][c] + element()->border();
    }

    pos = m_gridLayout[0][0] + yPos();
    for(int r = 1; r < element()->totalRows(); r++)
    {
      if( _y >= pos && _y <= pos+element()->border())
      {
        if(m_hSplitVar && m_hSplitVar[r-1] == true) m_hSplit = r-1;
#ifdef DEBUG_LAYOUT
        kdDebug( 6031 ) << "hsplitvar = " << m_hSplitVar << endl;
        kdDebug( 6031 ) << "hsplit!" << endl;
#endif
        res = true;
        break;
      }
      pos += m_gridLayout[0][r] + element()->border();
    }
#ifdef DEBUG_LAYOUT
    kdDebug( 6031 ) << m_hSplit << "/" << m_vSplit << endl;
#endif
  }


  m_cursor = Qt::ArrowCursor;
  if(m_hSplit != -1 && m_vSplit != -1)
      m_cursor = Qt::SizeAllCursor;
  else if( m_vSplit != -1 )
      m_cursor = Qt::SizeHorCursor;
  else if( m_hSplit != -1 )
      m_cursor = Qt::SizeVerCursor;

  if(!m_resizing && evt->id() == EventImpl::MOUSEDOWN_EVENT)
  {
      setResizing(true);
      KApplication::setOverrideCursor(QCursor(m_cursor));
      m_vSplitPos = _x;
      m_hSplitPos = _y;
      m_oldpos = -1;
  }

  // ### check the resize is not going out of bounds.
  if(m_resizing && evt->id() == EventImpl::MOUSEUP_EVENT)
  {
    setResizing(false);
    KApplication::restoreOverrideCursor();

    if(m_vSplit != -1 )
    {
#ifdef DEBUG_LAYOUT
      kdDebug( 6031 ) << "split xpos=" << _x << endl;
#endif
      int delta = m_vSplitPos - _x;
      m_gridDelta[1][m_vSplit] -= delta;
      m_gridDelta[1][m_vSplit+1] += delta;
    }
    if(m_hSplit != -1 )
    {
#ifdef DEBUG_LAYOUT
      kdDebug( 6031 ) << "split ypos=" << _y << endl;
#endif
      int delta = m_hSplitPos - _y;
      m_gridDelta[0][m_hSplit] -= delta;
      m_gridDelta[0][m_hSplit+1] += delta;
    }

    // this just schedules the relayout
    // important, otherwise the moving indicator is not correctly erased
    setNeedsLayout(true);
  }

  KHTMLView *view = canvas()->view();
  if ((m_resizing || evt->id() == EventImpl::MOUSEUP_EVENT) && view) {
      QPainter paint( view );
      paint.setPen( Qt::gray );
      paint.setBrush( Qt::gray );
      paint.setCompositionMode(QPainter::CompositionMode_Xor);
      QRect r(xPos(), yPos(), width(), height());
      const int rBord = 3;
      int sw = element()->border();
      int p = m_resizing ? (m_vSplit > -1 ? _x : _y) : -1;
      if (m_vSplit > -1) {
          if ( m_oldpos >= 0 )
              paint.drawRect( m_oldpos + sw/2 - rBord , r.y(),
                              2*rBord, r.height() );
          if ( p >= 0 )
              paint.drawRect( p  + sw/2 - rBord, r.y(), 2*rBord, r.height() );
      } else {
          if ( m_oldpos >= 0 )
              paint.drawRect( r.x(), m_oldpos + sw/2 - rBord,
                              r.width(), 2*rBord );
          if ( p >= 0 )
              paint.drawRect( r.x(), p + sw/2 - rBord, r.width(), 2*rBord );
      }
      m_oldpos = p;
  }

  return res;
}

void RenderFrameSet::setResizing(bool e)
{
      m_resizing = e;
      for (RenderObject* p = parent(); p; p = p->parent())
          if (p->isFrameSet()) static_cast<RenderFrameSet*>(p)->m_clientresizing = m_resizing;
}

bool RenderFrameSet::canResize( int _x, int _y )
{
    // if we haven't received a layout, then the gridLayout doesn't contain useful data yet
    if (needsLayout() || !m_gridLayout[0] || !m_gridLayout[1] ) return false;

    // check if we're over a horizontal or vertical boundary
    int pos = m_gridLayout[1][0];
    for(int c = 1; c < element()->totalCols(); c++)
        if(_x >= pos && _x <= pos+element()->border())
            return true;

    pos = m_gridLayout[0][0];
    for(int r = 1; r < element()->totalRows(); r++)
        if( _y >= pos && _y <= pos+element()->border())
            return true;

    return false;
}

#ifdef ENABLE_DUMP
void RenderFrameSet::dump(QTextStream &stream, const QString &ind) const
{
    RenderBox::dump(stream,ind);
    stream << " totalrows=" << element()->totalRows();
    stream << " totalcols=" << element()->totalCols();

    if ( m_hSplitVar )
        for (uint i = 0; i < (uint)element()->totalRows(); i++) {
            stream << " hSplitvar(" << i << ")=" << m_hSplitVar[i];
        }

    if ( m_vSplitVar )
        for (uint i = 0; i < (uint)element()->totalCols(); i++)
            stream << " vSplitvar(" << i << ")=" << m_vSplitVar[i];
}
#endif

/**************************************************************************************/

RenderPart::RenderPart(DOM::HTMLElementImpl* node)
    : RenderWidget(node)
{
    // init RenderObject attributes
    setInline(false);
}

void RenderPart::setWidget( QWidget *widget )
{
#ifdef DEBUG_LAYOUT
    kdDebug(6031) << "RenderPart::setWidget()" << endl;
#endif

    setQWidget( widget );
    widget->setFocusPolicy(Qt::WheelFocus);
    if(widget->inherits("KHTMLView"))
        connect( widget, SIGNAL( cleared() ), this, SLOT( slotViewCleared() ) );

    setNeedsLayoutAndMinMaxRecalc();

    // make sure the scrollbars are set correctly for restore
    // ### find better fix
    slotViewCleared();
}

bool RenderPart::partLoadingErrorNotify(khtml::ChildFrame *, const KURL& , const QString& )
{
    return false;
}

short RenderPart::intrinsicWidth() const
{
    return 300;
}

int RenderPart::intrinsicHeight() const
{
    return 200;
}

void RenderPart::slotViewCleared()
{
}

/***************************************************************************************/

RenderFrame::RenderFrame( DOM::HTMLFrameElementImpl *frame )
    : RenderPart(frame)
{
    setInline( false );
}

void RenderFrame::slotViewCleared()
{
#warning "Adjust this once we're a QAbstractScrollWidget (and in RenderPartObject)!"
    if(m_widget->inherits("Q3ScrollView")) {
#ifdef DEBUG_LAYOUT
        kdDebug(6031) << "frame is a scrollview!" << endl;
#endif
        Q3ScrollView *view = static_cast<Q3ScrollView *>(m_widget);
        if(!element()->frameBorder || !((static_cast<HTMLFrameSetElementImpl *>(element()->parentNode()))->frameBorder()))
            view->setFrameStyle(Q3Frame::NoFrame);
	    view->setVScrollBarMode(element()->scrolling );
	    view->setHScrollBarMode(element()->scrolling );
        if(view->inherits("KHTMLView")) {
#ifdef DEBUG_LAYOUT
            kdDebug(6031) << "frame is a KHTMLview!" << endl;
#endif
            KHTMLView *htmlView = static_cast<KHTMLView *>(view);
            if(element()->marginWidth != -1) htmlView->setMarginWidth(element()->marginWidth);
            if(element()->marginHeight != -1) htmlView->setMarginHeight(element()->marginHeight);
        }
    }
}

/****************************************************************************************/

RenderPartObject::RenderPartObject( DOM::HTMLElementImpl* element )
    : RenderPart( element )
{
    // init RenderObject attributes
    setInline(true);
}

void RenderPartObject::updateWidget()
{
  QString url;
  KHTMLPart *part = m_view->part();

  setNeedsLayoutAndMinMaxRecalc();

  if (element()->id() == ID_IFRAME) {

      HTMLIFrameElementImpl *o = static_cast<HTMLIFrameElementImpl *>(element());
      url = o->url.string();
      if (!o->getDocument()->isURLAllowed(url)) return;
      part->requestFrame( this, url, o->name.string(), QStringList(), true );
  // ### this should be constant true - move iframe to somewhere else
  } else {

      QStringList params;
      HTMLObjectBaseElementImpl * objbase = static_cast<HTMLObjectBaseElementImpl *>(element());
      url = objbase->url;

      for (NodeImpl* child = element()->firstChild(); child; child=child->nextSibling()) {
          if ( child->id() == ID_PARAM ) {
              HTMLParamElementImpl *p = static_cast<HTMLParamElementImpl *>( child );

              QString aStr = p->name();
              aStr += QLatin1String("=\"");
              aStr += p->value();
              aStr += QLatin1String("\"");
              QString name_lower = p->name().toLower();
              if (name_lower == QLatin1String("type") && objbase->id() != ID_APPLET) {
                  objbase->setServiceType(p->value());
              } else if (url.isEmpty() &&
                         (name_lower == QLatin1String("src") ||
                          name_lower == QLatin1String("movie") ||
                          name_lower == QLatin1String("code"))) {
                  url = p->value();
              }
              params.append(aStr);
          }
      }
      if (element()->id() != ID_OBJECT) {
          // add all attributes set on the embed object
          NamedAttrMapImpl* a = objbase->attributes();
          if (a) {
              for (unsigned long i = 0; i < a->length(); ++i) {
                  NodeImpl::Id id = a->idAt(i);
                  DOMString value = a->valueAt(i);
              params.append(objbase->getDocument()->getName(NodeImpl::AttributeId, id).string() + "=\"" + value.string() + "\"");
              }
          }
      }
      params.append( QLatin1String("__KHTML__PLUGINEMBED=\"YES\"") );
      params.append( QString::fromLatin1("__KHTML__PLUGINBASEURL=\"%1\"").arg(element()->getDocument()->baseURL().url()));

      HTMLEmbedElementImpl *embed = 0;
      QString serviceType = objbase->serviceType;
      if ( element()->id() == ID_EMBED ) {

          embed = static_cast<HTMLEmbedElementImpl *>( objbase );

      }
      else { // if(element()->id() == ID_OBJECT || element()->id() == ID_APPLET)

          // check for embed child object
          for (NodeImpl *child = objbase->firstChild(); child; child = child->nextSibling())
              if ( child->id() == ID_EMBED ) {
                  embed = static_cast<HTMLEmbedElementImpl *>( child );
                  break;
              }

          params.append( QString::fromLatin1("__KHTML__CLASSID=\"%1\"").arg( objbase->classId ) );
          params.append( QString::fromLatin1("__KHTML__CODEBASE=\"%1\"").arg( objbase->getAttribute(ATTR_CODEBASE).string() ) );
          if (!objbase->getAttribute(ATTR_WIDTH).isEmpty())
              params.append( QString::fromLatin1("WIDTH=\"%1\"").arg( objbase->getAttribute(ATTR_WIDTH).string() ) );
          else if (embed && !embed->getAttribute(ATTR_WIDTH).isEmpty()) {
              params.append( QString::fromLatin1("WIDTH=\"%1\"").arg( embed->getAttribute(ATTR_WIDTH).string() ) );
              objbase->setAttribute(ATTR_WIDTH, embed->getAttribute(ATTR_WIDTH));
          }
          if (!objbase->getAttribute(ATTR_HEIGHT).isEmpty())
              params.append( QString::fromLatin1("HEIGHT=\"%1\"").arg( objbase->getAttribute(ATTR_HEIGHT).string() ) );
          else if (embed && !embed->getAttribute(ATTR_HEIGHT).isEmpty()) {
              params.append( QString::fromLatin1("HEIGHT=\"%1\"").arg( embed->getAttribute(ATTR_HEIGHT).string() ) );
              objbase->setAttribute(ATTR_HEIGHT, embed->getAttribute(ATTR_HEIGHT));
          }

          if ( embed ) {
              // render embed object
              url = embed->url;
              if (!embed->serviceType.isEmpty())
                  serviceType = embed->serviceType;
          } else if (url.isEmpty() && objbase->classId.startsWith("java:")) {
              serviceType = "application/x-java-applet";
              url = objbase->classId.mid(5);
          }
          if(serviceType.isEmpty() && !objbase->classId.isEmpty()) {

#if 0
              // We have a clsid, means this is activex (Niko)
              serviceType = "application/x-activex-handler";
#endif

              if(objbase->classId.find(QLatin1String("D27CDB6E-AE6D-11cf-96B8-444553540000")) >= 0) {
                  // It is ActiveX, but the nsplugin system handling
                  // should also work, that's why we don't override the
                  // serviceType with application/x-activex-handler
                  // but let the KTrader in khtmlpart::createPart() detect
                  // the user's preference: launch with activex viewer or
                  // with nspluginviewer (Niko)
                  serviceType = "application/x-shockwave-flash";
              }
              else if(objbase->classId.find(QLatin1String("CFCDAA03-8BE4-11cf-B84B-0020AFBBCCFA")) >= 0)
                  serviceType = "audio/x-pn-realaudio-plugin";
              else if(objbase->classId.find(QLatin1String("8AD9C840-044E-11D1-B3E9-00805F499D93")) >= 0 ||
                      objbase->classId.find(QLatin1String("CAFEEFAC-0014-0000-0000-ABCDEFFEDCBA")) >= 0)
                  serviceType = "application/x-java-applet";
              // http://www.apple.com/quicktime/tools_tips/tutorials/activex.html
              else if(objbase->classId.find(QLatin1String("02BF25D5-8C17-4B23-BC80-D3488ABDDC6B")) >= 0)
                  serviceType = "video/quicktime";
              // http://msdn.microsoft.com/library/en-us/dnwmt/html/adding_windows_media_to_web_pages__etse.asp?frame=true
              else if(objbase->classId.find(QLatin1String("6BF52A52-394A-11d3-B153-00C04F79FAA6")) >= 0 ||
                      objbase->classId.find(QLatin1String("22D6f312-B0F6-11D0-94AB-0080C74C7E95")) >= 0)
                  serviceType = "video/x-msvideo";

              else
                  kdDebug(6031) << "ActiveX classId " << objbase->classId << endl;

              // TODO: add more plugins here
          }
      }
      if ((url.isEmpty() && !embed) || !document()->isURLAllowed(url) || !part->requestObject( this, url, serviceType, params ))
          objbase->renderAlternative();
  }
}

// ugly..
void RenderPartObject::close()
{
    RenderPart::close();

    if ( element()->id() != ID_IFRAME )
        updateWidget();
    // deleted here
}


bool RenderPartObject::partLoadingErrorNotify( khtml::ChildFrame *childFrame, const KURL& url, const QString& serviceType )
{
    KHTMLPart *part = static_cast<KHTMLView *>(m_view)->part();
    kdDebug(6031) << "RenderPartObject::partLoadingErrorNotify serviceType=" << serviceType << endl;
    // Check if we just tried with e.g. nsplugin
    // and fallback to the activexhandler if there is a classid
    // and a codebase, where we may download the ocx if it's missing
    if( serviceType != "application/x-activex-handler" && element()->id()==ID_OBJECT ) {

        // check for embed child object
        HTMLObjectElementImpl *o = static_cast<HTMLObjectElementImpl *>(element());
        HTMLEmbedElementImpl *embed = 0;
        NodeImpl *child = o->firstChild();
        while ( child ) {
            if ( child->id() == ID_EMBED )
                embed = static_cast<HTMLEmbedElementImpl *>( child );

            child = child->nextSibling();
        }
        if( embed && !o->classId.isEmpty() &&
            !( static_cast<ElementImpl *>(o)->getAttribute(ATTR_CODEBASE).string() ).isEmpty() )
        {
            KParts::URLArgs args;
            args.serviceType = "application/x-activex-handler";
            kdDebug(6031) << "set to activex" << endl;
            if (part->requestObject( childFrame, url, args ))
                return true; // success

            return false;
        }
    }
    // Dissociate ourselves from the current event loop (to prevent crashes
    // due to the message box staying up)
    QTimer::singleShot( 0, this, SLOT( slotPartLoadingErrorNotify() ) );
#if 0
    Tokenizer *tokenizer = static_cast<DOM::DocumentImpl *>(part->document().handle())->tokenizer();
    if (tokenizer) tokenizer->setOnHold( true );
    slotPartLoadingErrorNotify();
    if (tokenizer) tokenizer->setOnHold( false );
#endif
    return false;
}

void RenderPartObject::slotPartLoadingErrorNotify()
{
    // First we need to find out the servicetype - again - this code is too duplicated !
    HTMLEmbedElementImpl *embed = 0;
    QString serviceType;
    if( element()->id()==ID_OBJECT ) {

        // check for embed child object
        HTMLObjectElementImpl *o = static_cast<HTMLObjectElementImpl *>(element());
        serviceType = o->serviceType;
        NodeImpl *child = o->firstChild();
        while ( child ) {
            if ( child->id() == ID_EMBED )
                embed = static_cast<HTMLEmbedElementImpl *>( child );

            child = child->nextSibling();
        }

    } else if( element()->id()==ID_EMBED ) {
        embed = static_cast<HTMLEmbedElementImpl *>(element());
    }
    if ( embed )
	serviceType = embed->serviceType;

    // prepare for the local eventloop in KMessageBox
    ref();

    KHTMLPart *part = static_cast<KHTMLView *>(m_view)->part();
    KParts::BrowserExtension *ext = part->browserExtension();
    if( embed && !embed->pluginPage.isEmpty() && ext ) {
        // Prepare the mimetype to show in the question (comment if available, name as fallback)
        QString mimeName = serviceType;
        KMimeType::Ptr mime = KMimeType::mimeType(serviceType);
        if ( mime->name() != KMimeType::defaultMimeType() )
            mimeName = mime->comment();

        // Check if we already asked the user, for this page
        if (!mimeName.isEmpty() && part->docImpl() && !part->pluginPageQuestionAsked( serviceType ) )
        {
            part->setPluginPageQuestionAsked( serviceType );
            // Prepare the URL to show in the question (host only if http, to make it short)
            KURL pluginPageURL( embed->pluginPage );
            QString shortURL = pluginPageURL.protocol() == "http" ? pluginPageURL.host() : pluginPageURL.prettyURL();
            int res = KMessageBox::questionYesNo( m_view,
                                                  i18n("No plugin found for '%1'.\nDo you want to download one from %2?").arg(mimeName).arg(shortURL),
                                                  i18n("Missing Plugin"), i18n("Download"), i18n("Do Not Download"), QString("plugin-")+serviceType);
            if ( res == KMessageBox::Yes )
            {
                // Display vendor download page
                ext->createNewWindow( pluginPageURL );
                return;
            }
        }
    }

    // didn't work, render alternative content.
    if ( element() && (
         element()->id() == ID_OBJECT || element()->id() == ID_EMBED || element()->id() == ID_APPLET))
        static_cast<HTMLObjectBaseElementImpl*>( element() )->renderAlternative();

    deref();
}

void RenderPartObject::layout( )
{
    KHTMLAssert( needsLayout() );
    KHTMLAssert( minMaxKnown() );

    calcWidth();
    calcHeight();

    RenderPart::layout();

    setNeedsLayout(false);
}

void RenderPartObject::slotViewCleared()
{
  if(m_widget->inherits("Q3ScrollView") ) {
#ifdef DEBUG_LAYOUT
      kdDebug(6031) << "iframe is a scrollview!" << endl;
#endif
      Q3ScrollView *view = static_cast<Q3ScrollView *>(m_widget);
      int frameStyle = Q3Frame::NoFrame;
      Q3ScrollView::ScrollBarMode scroll = Q3ScrollView::Auto;
      int marginw = -1;
      int marginh = -1;
      if ( element()->id() == ID_IFRAME) {
	  HTMLIFrameElementImpl *frame = static_cast<HTMLIFrameElementImpl *>(element());
	  if(frame->frameBorder)
	      frameStyle = Q3Frame::Box;
	  scroll = frame->scrolling;
	  marginw = frame->marginWidth;
	  marginh = frame->marginHeight;
      }
      view->setFrameStyle(frameStyle);
      view->setVScrollBarMode(scroll );
      view->setHScrollBarMode(scroll );
      if(view->inherits("KHTMLView")) {
#ifdef DEBUG_LAYOUT
          kdDebug(6031) << "frame is a KHTMLview!" << endl;
#endif
          KHTMLView *htmlView = static_cast<KHTMLView *>(view);
          htmlView->setIgnoreWheelEvents( element()->id() == ID_IFRAME );
          if(marginw != -1) htmlView->setMarginWidth(marginw);
          if(marginh != -1) htmlView->setMarginHeight(marginh);
        }
  }
}

#include "render_frames.moc"
