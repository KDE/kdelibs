#include <ktopwidget.h>
#include "ktopwidget.moc"

KTopLevelWidget::KTopLevelWidget(char *name=NULL) : QWidget(0L,name) {
  kmenubar = NULL;
  kmainwidget = NULL;
  kstatusbar = NULL;
};

KTopLevelWidget::~KTopLevelWidget() {
};

void KTopLevelWidget::addToolbar(KToolbar *toolbar, int index) {
  if (index==-1)
    toolbars.append(toolbar);
  else
    toolbars.insert(index, toolbar);
};

void KTopLevelWidget::setView(QWidget *view) {
  kmainwidget = view;
};

void KTopLevelWidget::setMenu(QMenuBar *menu) {
  kmenubar = menu;
};

void KTopLevelWidget::setStatusBar(KStatusBar *statusbar) {
  kstatusbar = statusbar;
};

void KTopLevelWidget::focusInEvent ( QFocusEvent *) {
  repaint(FALSE);
};

void KTopLevelWidget::focusOutEvent ( QFocusEvent *) {
  repaint(FALSE);
};

void KTopLevelWidget::updateRects() {
  int t=0, b=0, l=0, r=0;
  int to=-1, bo=-1, lo=-1, ro=-1;
  int h = height();
  
  if (kmenubar && kmenubar->isVisible()) {
    t += kmenubar->height(); // the menu is always on top
  };

  if (kstatusbar && kstatusbar->isVisible()) {
    kstatusbar->setGeometry(0, height() - kstatusbar->height(),
			    width(), kstatusbar->height());
    b += kstatusbar->height();
  };
  for (KToolbar *toolbar=toolbars.first();
       toolbar != NULL; toolbar=toolbars.next()) 
    if (toolbar->Pos() == KToolbar::Top && toolbar->isVisible()) {
      if (to<0) {
	to=0;
	t+=toolbar->height();
      };
      toolbar->move(to, t-toolbar->height());
      to += toolbar->width();
      if (to > width()) {
	to=0;
	t+=toolbar->height();
      };
      break;
    };
  for (KToolbar *toolbar=toolbars.first();
       toolbar != NULL; toolbar=toolbars.next()) 
    if (toolbar->Pos() == KToolbar::Bottom && toolbar->isVisible()) {
      if (bo<0) {
	bo=0;
	b+=toolbar->height();
      };
      toolbar->move(bo, height() - b );
      bo += toolbar->width();
      if (bo > width()) {
	bo=0;
	b+=toolbar->height();
      };
      break;
    };
  h = height() - t - b;
  for (KToolbar *toolbar=toolbars.first();
       toolbar != NULL; toolbar=toolbars.next()) 
    if (toolbar->Pos() == KToolbar::Left && toolbar->isVisible()) {
      if (lo<0) {
	lo=0;
	l+=toolbar->width();
      };
      toolbar->move(l-toolbar->width(), t + lo);
      lo += toolbar->height();
      if (lo > h) {
	lo=0;
	l+=toolbar->width();
      };
      break;
    };
  for (KToolbar *toolbar=toolbars.first();
       toolbar != NULL; toolbar=toolbars.next()) 
    if (toolbar->Pos() == KToolbar::Right && toolbar->isVisible()) {
      if (ro<0) {
	ro=0;
	r+=toolbar->width();
      };
      toolbar->move(width() - r, t + ro);
      ro += toolbar->height();
      if (ro > h) {
	ro=0;
	r+=toolbar->width();
      };
      break;
    };
  if (kmainwidget)
    kmainwidget->setGeometry(l,t,width()-l-r,height()-t-b);
};

void KTopLevelWidget::resizeEvent( QResizeEvent *) {
  //menu resizes themself
  updateRects();
};

KStatusBar *KTopLevelWidget::statusBar() {
  return kstatusbar;
};

KToolbar *KTopLevelWidget::toolBar(int ID) {
  return toolbars.at(ID);
};

void KTopLevelWidget::enableToolBar(BarStatus stat, int ID) {
  if ((stat == Toggle && toolbars.at(ID)->isVisible()) || stat == Hide)
    toolbars.at(ID)->hide();
  else
    toolbars.at(ID)->show();
  updateRects();
};

void KTopLevelWidget::enableStatusBar(BarStatus stat) {
  CHECK_PTR(kstatusbar);
  if ((stat == Toggle && kstatusbar->isVisible()) || stat == Hide)
    kstatusbar->hide();
  else
    kstatusbar->show();
  updateRects();
};


