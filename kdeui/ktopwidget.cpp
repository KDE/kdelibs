
#include <ktopwidget.h>

#include <ktopwidget.moc>


KTopLevelWidget::KTopLevelWidget( const char *name = NULL )
  : QWidget( 0L, name )
{
  kmenubar = NULL;
  kmainwidget = NULL;
  kstatusbar = NULL;
  borderwidth = 0;

  kmainwidgetframe = new QFrame( this );
  CHECK_PTR( kmainwidgetframe );
  kmainwidgetframe ->setFrameStyle( QFrame::Panel | QFrame::Sunken);
  kmainwidgetframe ->setLineWidth(2);
  kmainwidgetframe ->hide();
}

KTopLevelWidget::~KTopLevelWidget()
{
}

int KTopLevelWidget::addToolBar( KToolBar *toolbar, int index )
{
  if ( index == -1 )
	toolbars.append( toolbar );
  else
	toolbars.insert( index, toolbar );
  index = toolbars.at();
  connect ( toolbar, SIGNAL( moved (Position) ),
			this, SLOT( updateRects() ) );  
  updateRects();
  return index;
}

void KTopLevelWidget::setView( QWidget *view, bool show_frame )
{
  kmainwidget = view;
  if( show_frame ){

	// Set a default frame borderwidth, for a toplevelwidget with 
	// frame. 

	if(borderwidth == 0 )
	  setFrameBorderWidth(1);
	  
	kmainwidgetframe->show();
  }

  // In the case setView(...,TRUE),
  // we leave the default frame borderwith at 0 so that we don't get
  // an unwanted border -- after all we didn't request a frame. If you
  // still want a border ( though no frame, call setFrameBorderWidth()
  // before setView(...,FALSE).
	  
}

void KTopLevelWidget::setMenu( KMenuBar *menu )
{
  kmenubar = menu;
}

void KTopLevelWidget::setStatusBar( KStatusBar *statusbar )
{
  kstatusbar = statusbar;
}

void KTopLevelWidget::focusInEvent( QFocusEvent *)
{
  repaint( FALSE );
}

void KTopLevelWidget::focusOutEvent( QFocusEvent *)
{
  repaint( FALSE );
}

void KTopLevelWidget::updateRects()
{
  int t=0, b=0, l=0, r=0;
  int to=-1, bo=-1, lo=-1, ro=-1;
  int h = height();

  if ( kmenubar && kmenubar->isVisible() ) {
	t += kmenubar->height(); // the menu is always on top
  }

  if ( kstatusbar && kstatusbar->isVisible() ) {
	kstatusbar->setGeometry(0, height() - kstatusbar->height(),
							width(), kstatusbar->height());
	b += kstatusbar->height();
  }
	
  for ( KToolBar *toolbar = toolbars.first() ;
		toolbar != NULL ; toolbar = toolbars.next() ) 
	if ( toolbar->pos() == KToolBar::Top && toolbar->isVisible() ) {
	  toolbar->repaint ();     // Sven: You have to do this
	  if ( to < 0 ) {
		to = 0;
		t += toolbar->height();
	  }
	  if (to + toolbar->width() > width()) {
		to = 0;
		t += toolbar->height();
	  }
	  toolbar->move( to, t-toolbar->height() );
	  to += toolbar->width();
	}
		
  for ( KToolBar *toolbar = toolbars.first();
		toolbar != NULL; toolbar = toolbars.next() ) 
	if ( toolbar->pos() == KToolBar::Bottom && toolbar->isVisible() ) {
	  toolbar->repaint ();   // Sven: You have to this
	  if ( bo < 0 ) {
		bo = 0;
		b += toolbar->height();
	  }
	  if (bo + toolbar->width() > width()) {
		bo = 0;
		b += toolbar->height();
	  }
	  toolbar->move(bo, height()  - b );
	  bo += toolbar->width();
	}
		
  h = height() - t - b;
  for ( KToolBar *toolbar = toolbars.first();
		toolbar != NULL; toolbar = toolbars.next() ) 
	if ( toolbar->pos() == KToolBar::Left && toolbar->isVisible() ) {
	  toolbar->setMaxHeight(h);   // Sven: You have to do this here
	  toolbar->repaint ();        // Sven: You have to this
	  if ( lo < 0 ) {
		lo = 0;
		l += toolbar->width();
	  }
	  if ( lo + toolbar->height() > h) {
		lo = 0;
		l += toolbar->width();
	  }
	  toolbar->move( l-toolbar->width(), t  + lo );
	  lo += toolbar->height();
	}
	
  for ( KToolBar *toolbar = toolbars.first();
		toolbar != NULL; toolbar = toolbars.next() ) 
	if ( toolbar->pos() == KToolBar::Right && toolbar->isVisible() ) {
	  toolbar->setMaxHeight(h);   // Sven: You have to do this here
	  toolbar->repaint ();   // Sven: You have to this
	  if ( ro < 0 ) {
		ro = 0;
		r += toolbar->width();
	  }
	  if (ro + toolbar->height() > h) {
		ro = 0;
		r += toolbar->width();
	  }
	  toolbar->move(width() - r , t + ro);
	  ro += toolbar->height();
	}
	
  view_left=0;
  view_right=width();
  view_top=0;
  view_bottom=height();
	
  for (KToolBar *toolbar = toolbars.first();
	   toolbar != NULL; toolbar = toolbars.next()) {
		
	if ( toolbar->pos() == KToolBar::Top && toolbar->isVisible() )
	  view_top = toolbar->y() + toolbar->height() - 2;
	else if ( kmenubar && kmenubar->isVisible() ) {
	  if( view_top < kmenubar->height() - 2 )
		view_top = kmenubar->height() - 2;
	} else if ( view_top < 0 )
	  view_top = 0;
			
	if ( toolbar->pos() == KToolBar::Bottom && toolbar->isVisible() )
	  view_bottom = toolbar->y() + 2;
	else if ( kstatusbar && kstatusbar->isVisible() ) {
	  if ( view_bottom > kstatusbar->y() )
		view_bottom = kstatusbar->y();
	} else if ( view_bottom > height() )
	  view_bottom = height();

	if ( toolbar->pos() == KToolBar::Right && toolbar->isVisible() )
	  view_right = toolbar->x()+2;
			
	if ( toolbar->pos() == KToolBar::Right && !toolbar->isVisible() )
	  view_right = width();
			
	if ( toolbar->pos() == KToolBar::Left && toolbar->isVisible() )
	  view_left = toolbar->x()+toolbar->width() - 2;
		
	if ( toolbar->pos() == KToolBar::Left && !toolbar->isVisible() )
	  view_left = 0;
  }
	
  //printf("l = %d, r = %d, t = %d, b = %d\n", view_left, view_right, view_top,
  //view_bottom);
	
  if ( kmainwidget ) {

	 
	kmainwidgetframe->setGeometry( view_left, view_top,
								   view_right - view_left,
								   view_bottom - view_top );

					 
	kmainwidget->setGeometry( view_left + borderwidth, view_top + borderwidth,
							  view_right - view_left - 2 * borderwidth,
							  view_bottom - view_top - 2 * borderwidth );


  }
}

void KTopLevelWidget::setFrameBorderWidth(int size){

  borderwidth = size;

}

void KTopLevelWidget::resizeEvent( QResizeEvent * )
{
  //menu resizes themself

  updateRects();
}

KStatusBar *KTopLevelWidget::statusBar()
{
  return kstatusbar;
}

KToolBar *KTopLevelWidget::toolBar( int ID )
{
  return toolbars.at( ID );
}

void KTopLevelWidget::enableToolBar( KToolBar::BarStatus stat, int ID )
{
  KToolBar *t = toolbars.at( ID );
  if ( t )
	t->enable( stat );
  updateRects();
}

void KTopLevelWidget::enableStatusBar( KStatusBar::BarStatus stat )
{
  CHECK_PTR( kstatusbar );
  if ( ( stat == KStatusBar::Toggle && kstatusbar->isVisible() )
	   || stat == KStatusBar::Hide )
	kstatusbar->hide();
  else
	kstatusbar->show();
  updateRects();
}


