/**
 * This file is part of the KDE project.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Simon Hausmann <hausmann@kde.org>
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */
#ifndef __render_frames_h__
#define __render_frames_h__

#include "render_replaced.h"
#include "xml/dom_nodeimpl.h"

class KHTMLWidget;

namespace DOM
{
  class HTMLFrameSetElementImpl;
  class HTMLFrameElementImpl;
};

namespace khtml
{

class RenderFrameSet : public RenderBox
{
public:
  RenderFrameSet( RenderStyle *style, DOM::HTMLFrameSetElementImpl *frameSet, KHTMLWidget *view,
		  QList<khtml::Length> *rows, QList<khtml::Length> *cols );

  virtual ~RenderFrameSet();

  virtual const char *renderName() const { return "RenderFrameSet"; }

  virtual void layout( bool deep = true );

  void positionFrames( bool deep = true );

  virtual bool isInline() const { return false; }

  bool resizing() const { return m_resizing; }

  bool userResize( int _x, int _y, DOM::NodeImpl::MouseEventType type );

  DOM::HTMLFrameSetElementImpl *frameSetImpl() const { return m_frameset; }

private:
  DOM::HTMLFrameSetElementImpl *m_frameset;

  QList<khtml::Length> *m_rows;
  QList<khtml::Length> *m_cols;
  int *m_rowHeight;
  int *m_colWidth;

  int m_hSplit;     // the split currently resized
  int m_vSplit;
  int m_hSplitPos;
  int m_vSplitPos;
  bool *m_hSplitVar; // is this split variable?
  bool *m_vSplitVar;

  bool m_resizing;

  KHTMLWidget *m_view;
};

// ### rename to generic RenderPart
class RenderFrame : public RenderWidget
{
public:
  RenderFrame( RenderStyle *style, QScrollView *view, DOM::HTMLFrameElementImpl *frame );
  virtual ~RenderFrame();

  virtual const char *renderName() const { return "RenderFrame"; }

  virtual bool isInline() const { return false; }

  virtual void layout( bool deep = true );

  void setWidget( QWidget *widget );

  DOM::HTMLFrameElementImpl *frameImpl() const { return m_frame; }

private:
  DOM::HTMLFrameElementImpl *m_frame;
};

};

#endif
