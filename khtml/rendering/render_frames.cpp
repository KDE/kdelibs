
#include "render_frames.h"
#include "html_baseimpl.h"
#include "khtml.h"

#include <kapp.h>

#include <qlabel.h>

#include <assert.h>

using namespace khtml;

RenderFrameSet::RenderFrameSet( RenderStyle *style, HTMLFrameSetElementImpl *frameSet, KHTMLWidget *view,
			        QList<khtml::Length> *rows, QList<khtml::Length> *cols )
: RenderBox( style )
{
  m_frameset = frameSet;

  m_rows = rows;
  m_cols = cols;
  m_rowHeight = 0;
  m_colWidth = 0;

  m_resizing = false;

  m_hSplit = -1;
  m_vSplit = -1;

  m_hSplitVar = 0;
  m_vSplitVar = 0;

  m_view = view;
}

RenderFrameSet::~RenderFrameSet()
{
  if ( m_rowHeight ) delete [] m_rowHeight;
  if ( m_colWidth ) delete [] m_colWidth;

  if ( m_hSplitVar ) delete m_hSplitVar;
  if ( m_vSplitVar ) delete m_vSplitVar;
}

void RenderFrameSet::layout( bool deep )
{

#ifdef DEBUG_LAYOUT
    printf("%s(FrameSet)::layout(%d) width=%d, layouted=%d\n", nodeName().string().ascii(), deep, width, layouted());
#endif

    if ( strcmp( m_parent->renderName(), "RenderFrameSet" ) != 0 )
    {
      m_width = m_view->clipper()->width();
      m_height = m_view->clipper()->height();
    }

    int remainingWidth = m_width - (m_frameset->totalCols()-1)*m_frameset->border();
    if(remainingWidth<0) remainingWidth=0;
    int remainingHeight = m_height - (m_frameset->totalRows()-1)*m_frameset->border();
    if(remainingHeight<0) remainingHeight=0;
    int widthAvailable = remainingWidth;
    int heightAvailable = remainingHeight;

    if(m_rowHeight) delete [] m_rowHeight;
    if(m_colWidth) delete [] m_colWidth;
    m_rowHeight = new int[m_frameset->totalRows()];
    m_colWidth = new int[m_frameset->totalCols()];

    int i;
    int totalRelative = 0;
    int colsRelative = 0;
    int rowsRelative = 0;
    int remainingRelativeWidth = 0;
    // fixed rows first, then percent and then relative

    if(m_rows)
    {
	for(i = 0; i< m_frameset->totalRows(); i++)
	{
	    printf("setting row %d\n", i);
	    if(m_rows->at(i)->type == Fixed || m_rows->at(i)->type == Percent)
	    {
		m_rowHeight[i] = m_rows->at(i)->width(heightAvailable);
		printf("setting row height to %d\n", m_rowHeight[i]);
		remainingHeight -= m_rowHeight[i];
	    }
	    else if(m_rows->at(i)->type == Relative)
	    {
		totalRelative += m_rows->at(i)->value;
		rowsRelative++;
	    }
	}
	// ###
	if(remainingHeight < 0) remainingHeight = 0;

	if ( !totalRelative && rowsRelative )
	  remainingRelativeWidth = remainingHeight/rowsRelative;
	
	for(i = 0; i< m_frameset->totalRows(); i++)
	 {
	    if(m_rows->at(i)->type == Relative)
	    {
		if ( totalRelative )
		  m_rowHeight[i] = m_rows->at(i)->value*remainingHeight/totalRelative;
		else
		  m_rowHeight[i] = remainingRelativeWidth;
	  	remainingHeight -= m_rowHeight[i];
		totalRelative--;
	    }
	}
	
	// support for totally broken frame declarations
	if(remainingHeight)
	{
	    // just distribute it over all columns...
	    int rows = m_frameset->totalRows();
	    for(i = 0; i< m_frameset->totalRows(); i++)
	    {
		int toAdd = remainingHeight/rows;
		rows--;
		m_rowHeight[i] += toAdd;
		remainingHeight -= toAdd;
	    }
	}	
    }
    else
	m_rowHeight[0] = m_height;

    if(m_cols)
    {
	totalRelative = 0;
	remainingRelativeWidth = 0;

	for(i = 0; i< m_frameset->totalCols(); i++)
	{
	    if(m_cols->at(i)->type == Fixed || m_cols->at(i)->type == Percent)
	    {
		m_colWidth[i] = m_cols->at(i)->width(widthAvailable);
		remainingWidth -= m_colWidth[i];
	    }
	    else if(m_cols->at(i)->type == Relative)
	    {
		totalRelative += m_cols->at(i)->value;
		colsRelative++;
	    }
	}
	// ###
	if(remainingWidth < 0) remainingWidth = 0;
	
	if ( !totalRelative && colsRelative )
	  remainingRelativeWidth = remainingWidth/colsRelative;

	for(i = 0; i < m_frameset->totalCols(); i++)
	{
	    if(m_cols->at(i)->type == Relative)
	    {
		if ( totalRelative )
		  m_colWidth[i] = m_cols->at(i)->value*remainingWidth/totalRelative;
		else
		  m_colWidth[i] = remainingRelativeWidth;
		remainingWidth -= m_colWidth[i];
		totalRelative--;
	    }
	}

	// support for totally broken frame declarations
	if(remainingWidth)
	{
	    // just distribute it over all columns...
	    int cols = m_frameset->totalCols();
	    for(i = 0; i< m_frameset->totalCols(); i++)
	    {
		int toAdd = remainingHeight/cols;
		cols--;
		m_rowHeight[i] += toAdd;
		remainingHeight -= toAdd;
	    }
	}

    }
    else
	m_colWidth[0] = m_width;

    positionFrames(deep);

    RenderObject *child = firstChild();
    if ( !child )
      return;

    if(!m_hSplitVar && !m_vSplitVar)
    {
	printf("calculationg fixed Splitters\n");
	if(!m_vSplitVar && m_frameset->totalCols() > 1)
	{
	    m_vSplitVar = new bool[m_frameset->totalCols()];
	    for(int i = 0; i < m_frameset->totalCols(); i++) m_vSplitVar[i] = true;
	}
	if(!m_hSplitVar && m_frameset->totalRows() > 1)
	{
	    m_hSplitVar = new bool[m_frameset->totalRows()];
	    for(int i = 0; i < m_frameset->totalRows(); i++) m_hSplitVar[i] = true;
	}

	for(int r = 0; r < m_frameset->totalRows(); r++)
	{
	    for(int c = 0; c < m_frameset->totalCols(); c++)
	    {
		bool fixed = false;
		
		if ( strcmp( child->renderName(), "RenderFrameSet" ) == 0 )
		  fixed = static_cast<RenderFrameSet *>(child)->frameSetImpl()->noResize();
		else
		  fixed = static_cast<RenderFrame *>(child)->frameImpl()->noResize();

		/*
		if(child->id() == ID_FRAMESET)
		    fixed = (static_cast<HTMLFrameSetElementImpl *>(child))->noResize();
		else if(child->id() == ID_FRAME)
		    fixed = (static_cast<HTMLFrameElementImpl *>(child))->noResize();
		*/

		if(fixed)
		{
		    printf("found fixed cell %d/%d!\n", r, c);
		    if( m_frameset->totalCols() > 1)
		    {
			if(c>0) m_vSplitVar[c-1] = false;
			m_vSplitVar[c] = false;
		    }
		    if( m_frameset->totalRows() > 1)
		    {
			if(r>0) m_hSplitVar[r-1] = false;
			m_hSplitVar[r] = false;
		    }
		    child = child->nextSibling();
		    if(!child) goto end2;
		}		
		else
		    printf("not fixed: %d/%d!\n", r, c);
	    }
	}

    }
 end2:
    setLayouted();
}

void RenderFrameSet::positionFrames( bool deep )
{
  int r;
  int c;

  RenderObject *child = firstChild();
  if ( !child )
    return;

  //  NodeImpl *child = _first;
  //  if(!child) return;

  int yPos = 0;

  for(r = 0; r < m_frameset->totalRows(); r++)
  {
    int xPos = 0;
    for(c = 0; c < m_frameset->totalCols(); c++)
    {
    //	    HTMLElementImpl *e = static_cast<HTMLElementImpl *>(child);
      child->setPos( xPos, yPos );
      child->setSize( m_colWidth[c], m_rowHeight[r] );

      //if ( deep )
      child->layout( deep );
	
      xPos += m_colWidth[c] + m_frameset->border();
      child = child->nextSibling();

      if ( !child )
        return;
    /*	
	    e->renderer()->setXPos(xPos);
	    e->renderer()->setYPos(yPos);
	    e->setWidth(colWidth[c]);
	    e->setAvailableWidth(colWidth[c]);
	    e->setDescent(rowHeight[r]);
	    if(deep)
		e->layout(deep);
	    xPos += colWidth[c] + border;
	    child = child->nextSibling();
	    if(!child) return;
    */
	}
	yPos += m_rowHeight[r] + m_frameset->border();
    }
}

bool RenderFrameSet::userResize( int _x, int _y, DOM::NodeImpl::MouseEventType type )
{
  bool res = false;

  if ( !m_resizing && type == DOM::NodeImpl::MouseMove || type == DOM::NodeImpl::MousePress )
  {
    printf("mouseEvent:check\n");

    m_hSplit = -1;
    m_vSplit = -1;
    //bool resizePossible = true;

    // check if we're over a horizontal or vertical boundary
    int pos = m_colWidth[0];
    for(int c = 1; c < m_frameset->totalCols(); c++)
    {
      if(_x >= pos && _x <= pos+m_frameset->border())
      {
        if(m_vSplitVar && m_vSplitVar[c-1] == true) m_vSplit = c-1;
        printf("vsplit!\n");
	res = true;
        break;
      }
      pos += m_colWidth[c] + m_frameset->border();
    }
	
    pos = m_rowHeight[0];
    for(int r = 1; r < m_frameset->totalRows(); r++)
    {
      if( _y >= pos && _y <= pos+m_frameset->border())
      {
        if(m_hSplitVar && m_hSplitVar[r-1] == true) m_hSplit = r-1;
        printf("hsplitvar = %p\n", m_hSplitVar);
        printf("hsplit!\n");
	res = true;
        break;
      }
      pos += m_rowHeight[r] + m_frameset->border();
    }
    printf("%d/%d\n", m_hSplit, m_vSplit);

    QCursor cursor;
    if(m_hSplit != -1 && m_vSplit != -1)
    {
      cursor = Qt::sizeAllCursor;
    }
    else if( m_vSplit != -1 )
    {
      cursor = Qt::splitHCursor;
    }	
    else if( m_hSplit != -1 )
    {
      cursor = Qt::splitVCursor;
    }

    if(type == DOM::NodeImpl::MousePress)
    {
      m_resizing = true;
      KApplication::setOverrideCursor(cursor);
      m_vSplitPos = _x;
      m_hSplitPos = _y;
    }
    else
      static_cast<KHTMLWidget *>(m_view)->setCursor(cursor);

  }

  // ### need to draw a nice movin indicator for the resize.
  // ### check the resize is not going out of bounds.
  if(m_resizing && type == DOM::NodeImpl::MouseRelease)
  {
    m_resizing = false;
    KApplication::restoreOverrideCursor();

    if(m_vSplit != -1 )
    {
      printf("split xpos=%d\n", _x);
      int delta = m_vSplitPos - _x;
      m_colWidth[m_vSplit] -= delta;
      m_colWidth[m_vSplit+1] += delta;
    }	
    if(m_hSplit != -1 )
    {
      printf("split ypos=%d\n", _y);
      int delta = m_hSplitPos - _y;
      m_rowHeight[m_hSplit] -= delta;
      m_rowHeight[m_hSplit+1] += delta;
    }

    positionFrames( true );
  }

  return res;
}

RenderFrame::RenderFrame( RenderStyle *style, QScrollView *view, DOM::HTMLFrameElementImpl *frame )
: RenderWidget( style, view )
{
  m_view = view;
  m_frame = frame;
}

RenderFrame::~RenderFrame()
{
}

void RenderFrame::setWidget( QWidget *widget )
{
  m_widget = widget;
  layout();
  repaint();
}

void RenderFrame::layout( bool deep  )
{
  if ( m_widget )
    m_widget->resize( m_width, m_height );
}
