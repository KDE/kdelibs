/*  This file is part of the KDE Libraries
    Copyright (C) 1998 Thomas Tanghus (tanghus@earthling.net)

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

#include "knotebook.h"
#include <klocale.h>

struct KNoteBookProtected
{
  KWizard     *currentwiz;
  QPushButton *cancel;
  QPushButton *ok;
  QPushButton *def;
  QPushButton *help;
  KTabBar     *tabbar;
  QWidget     *main;
  QPopupMenu  *menu;
  int         current;
  int         numtabs;
  int         currentmenu;
  bool        enablepopupmenu;
  bool        enablearrowbuttons;
  bool        directionsreflectspage;
};


KNoteBook::KNoteBook(QWidget *parent, const char *name, 
		     bool modal, WFlags f)
    : KDialog(parent, name, modal, f)
{
  initMetaObject();
  init();
}

KNoteBook::~KNoteBook()
{
  //debug("KNoteBook - destructor");
  delete pnote;
  delete sections;
  //debug("KNoteBook - destructor done");
}

void KNoteBook::init()
{
  //debug("KNoteBook::init");
  sections = new QList<KWizard>;
  sections->setAutoDelete(true);
  pnote = new KNoteBookProtected;
  pnote->directionsreflectspage = pnote->enablepopupmenu = pnote->enablearrowbuttons = false;
  pnote->current = -1;
  pnote->numtabs = 0;
  pnote->currentwiz = 0L;
  pnote->ok = pnote->cancel = pnote->def = pnote->help = 0L;
  pnote->tabbar = new KTabBar(this);
  connect( pnote->tabbar, SIGNAL(selected(int)), SLOT( showSection(int)) );
  connect( pnote->tabbar, SIGNAL(scrolled(ArrowType)),
                   SLOT( tabScroll(ArrowType)) );
  //debug("tabbar");
  pnote->menu = new QPopupMenu();
  connect( pnote->menu, SIGNAL(highlighted(int)), SLOT( menuChoice(int)) );
  connect( pnote->menu, SIGNAL(activatedRedirect(int)), SLOT( menuChoiceRedirect(int)) );

  //debug("init - done");
}

void KNoteBook::menuChoice(int c)
{
  //debug("Activated: %d", c);
  pnote->currentmenu = c;
}

void KNoteBook::tabScroll( ArrowType )
{
  //debug("KNoteBook::tabScroll");
  // fake a resize event to trigger child widget moves
  //QResizeEvent r( size(), size() );
  //resizeEvent( &r );
  //repaint(true);
}

void KNoteBook::menuChoiceRedirect(int c)
{
  //debug("ActivatedRedirect: %d", c);
  if(pnote->tabbar->isTabEnabled(pnote->currentmenu) && 
      sections->at(pnote->currentmenu)->isPageEnabled(c))
  {
    gotoTab(pnote->currentmenu);
    pnote->currentwiz->gotoPage(c);  
  }
}

int KNoteBook::addTab(QTab *tab, KWizardPage *p)
{
  //debug("addTab");
  int id = 0;
  KWizard *wiz = new KWizard(this, 0, false); // non-modal wizard
  wiz->setDirectionsReflectsPage(pnote->directionsreflectspage);
  wiz->setEnableArrowButtons(pnote->enablearrowbuttons);
  wiz->hide();
  //debug("KWizard created");
  sections->append(wiz);
  if(!pnote->numtabs) // the first tab
  {
    pnote->current = 0;
    pnote->currentwiz = wiz;
  }
  pnote->numtabs++;
  connect( wiz, SIGNAL(popup(QPoint)), SLOT(popupMenu(QPoint)) );
  connect( wiz, SIGNAL(nomorepages(bool, bool)), SLOT(directionButton(bool, bool)) );

  //tab->id = pnote->numtabs;
  //debug("Before adding to tabbar");
  id = pnote->tabbar->addTab(tab);
  pnote->menu->insertItem(tab->label, wiz->getMenu(), id);
  pnote->menu->setItemEnabled(id, tab->enabled);
  //debug("After adding to tabbar");

  if(p)
    wiz->addPage(p);

  setSizes();

  //debug("addTab - done");

  return id;
}

int KNoteBook::addPage(KWizardPage *p)
{
  if(!pnote->numtabs)
  {
    debug("Trying to add page when no KWizards are added!");
    return -1;
  }
  KWizard *wiz = sections->at(pnote->numtabs-1);
  CHECK_PTR(wiz);

  return (wiz->addPage(p));
}

void KNoteBook::gotoTab(int t)
{
  int i = 0;
  if(t < 0 || t >= pnote->numtabs || t == pnote->current)
    return;
  else if(t > pnote->current)
    for(i = t; i < pnote->numtabs; i++)
    {
      if(pnote->tabbar->isTabEnabled(i))
        break;
    }
  else
    for(i = t; i >= 0; i--)
    {
      if(pnote->tabbar->isTabEnabled(i))
        break;
    }
  //debug("gototab: %d", i);
  if(pnote->tabbar->isTabEnabled(i))
    pnote->tabbar->setCurrentTab(i);
}

void KNoteBook::setCancelButton()
{
  setCancelButton(i18n("&Cancel"));
}

void KNoteBook::setCancelButton(const QString& name)
{
  if(!pnote->cancel)
  {
    pnote->cancel = new QPushButton(name, this);
    pnote->cancel->show();
    connect( pnote->cancel, SIGNAL(clicked()), SLOT(cancelClicked()));
  }
  else
    pnote->cancel->setText(name);
  setSizes();
}

QButton *KNoteBook::getCancelButton()
{
  return pnote->cancel;
}

void KNoteBook::setDefaultButton()
{
  setDefaultButton(i18n("&Default"));
}

void KNoteBook::setDefaultButton(const QString& name)
{
  if(!pnote->def)
  {
    pnote->def = new QPushButton(name, this);
    pnote->def->show();
    connect( pnote->def, SIGNAL(clicked()), SLOT(defaultClicked()));
  }
  else
    pnote->def->setText(name);
  setSizes();
}

QButton *KNoteBook::getDefaultButton()
{
  return pnote->def;
}

void KNoteBook::setHelpButton()
{
  setHelpButton(i18n("&Help"));
}

void KNoteBook::setHelpButton(const QString& name)
{
  if(!pnote->help)
  {
    pnote->help = new QPushButton(name, this);
    pnote->help->show();
    connect( pnote->help, SIGNAL(clicked()), SLOT(helpClicked()));
  }
  else
    pnote->help->setText(name);
  setSizes();
}

QButton *KNoteBook::getHelpButton()
{
  return pnote->help;
}

void KNoteBook::setOkButton()
{
  setOkButton(i18n("&OK"));
}

void KNoteBook::setOkButton(const QString& name)
{
  if(!pnote->ok)
  {
    pnote->ok = new QPushButton(name, this);
    pnote->ok->show();
    connect( pnote->ok, SIGNAL(clicked()), SLOT(okClicked()));
  }
  else
    pnote->ok->setText(name);
  setSizes();
}

QButton *KNoteBook::getOkButton()
{
  return pnote->ok;
}

void KNoteBook::okClicked()
{
  emit okclicked();
}

void KNoteBook::cancelClicked()
{
  emit cancelclicked();
}

void KNoteBook::defaultClicked()
{
  emit defaultclicked(pnote->current);
}

void KNoteBook::helpClicked()
{
  emit helpclicked(pnote->current);
}

void KNoteBook::showSection(int s)
{
  //debug("showSection: %d", s);
  pnote->current = s;
  pnote->currentwiz->hide();
  pnote->currentwiz = sections->at(s);
  pnote->currentwiz->gotoPage(0);
  pnote->currentwiz->adjustSize();
  // fake a resize event to trigger child widget moves
  QResizeEvent r( size(), size() );
  resizeEvent( &r );
  pnote->currentwiz->show();
}

void KNoteBook::popupMenu(QPoint pos)
{
  if(pnote->enablepopupmenu)
  {
    pnote->menu->popup(pos);
  }
}

QSize KNoteBook::childSize()
{
  //debug("Calculating sizes");
  QSize size(0,0);
  //int x = 0, y = 0;
  for(int i = 0; i < pnote->numtabs; i++)
  {
    QSize csize = sections->at(i)->sizeHint();
    if(csize.isNull())
      csize = sections->at(i)->size();
    if(size.height() < csize.height())
      size.setHeight(csize.height());
    if(size.width() < csize.width())
      size.setWidth(csize.width());
    //debug("Child size: %d x %d", size.width(), size.height());
  }

  return size;
}

void KNoteBook::setEnableArrowButtons(bool state)
{
  pnote->enablearrowbuttons = state;
  for(int i = 0; i < pnote->numtabs; i++)
    sections->at(i)->setEnableArrowButtons(state);
}

bool KNoteBook::enableArrowButtons()
{
  return pnote->enablearrowbuttons;
}

void KNoteBook::setDirectionsReflectsPage(bool state)
{
  pnote->directionsreflectspage = state;
  for(int i = 0; i < pnote->numtabs; i++)
    sections->at(i)->setDirectionsReflectsPage(state);
  setSizes();
}

bool KNoteBook::directionsReflectsPage()
{
  return pnote->directionsreflectspage;
}

void KNoteBook::setEnablePopupMenu(bool state)
{
  pnote->enablepopupmenu = state;
}

bool KNoteBook::enablePopupMenu()
{
  return pnote->enablepopupmenu;
}

QPopupMenu *KNoteBook::getMenu()
{
  return pnote->menu;
}

KTabBar *KNoteBook::getTabBar()
{
  return pnote->tabbar;
}

void KNoteBook::setTabEnabled(int tab, bool state)
{
  pnote->tabbar->setTabEnabled( tab, state );
  pnote->menu->setItemEnabled( tab, state );
}

void KNoteBook::setPageEnabled(int tab, int page, bool state)
{
  sections->at(tab)->setPageEnabled( page, state );
}

bool KNoteBook::isTabEnabled(int tab)
{
  return pnote->tabbar->isTabEnabled(tab);
}

bool KNoteBook::isPageEnabled(int tab, int page)
{
  return sections->at(tab)->isPageEnabled(page);
}

int KNoteBook::numTabs()
{
  return pnote->numtabs;
}

void KNoteBook::directionButton(bool changetab, bool forward)
{
  //debug("directionButton");
  QButton *button, *arrow;
  QString str;

  if(changetab)
  {
    if(forward)
    {
      //debug("changing to tab: %d", current+1);
      gotoTab(pnote->current+1);
      button = pnote->currentwiz->getPreviousButton();
      arrow = pnote->currentwiz->getLeftArrow();
      if(pnote->directionsreflectspage)
      {
        str = "<< ";
        str += sections->at(pnote->current-1)->getTitle(sections->at(pnote->current-1)->numPages()-1);
        button->setText(str.data());
      }
      else
        button->setText("<< " + i18n("&Previous"));
      //debug("Setting previous to: %s", str.data());
      button->show();
      if(pnote->enablearrowbuttons)
        arrow->show();
    }
    else
    {
      //debug("changing to tab: %d", pnote->current-1);
      gotoTab(pnote->current-1);
      pnote->currentwiz->gotoPage(pnote->currentwiz->numPages()-1);
      button = pnote->currentwiz->getNextButton();
      arrow = pnote->currentwiz->getRightArrow();
      if(pnote->directionsreflectspage)
      {
        str = sections->at(pnote->current+1)->getTitle(0);
        str += " >>";
        button->setText(str.data());
      }
      else
        button->setText(i18n("&Next") + " >>");
      //debug("Setting next to: %s", str.data());
      button->show();
      if(pnote->enablearrowbuttons)
        arrow->show();
    }
  }
  else
  {
    //debug("dont change tab");
    if(forward)
    {
      button = pnote->currentwiz->getNextButton();
      arrow = pnote->currentwiz->getRightArrow();
      if((pnote->current+1) >= pnote->numtabs)
      {
        button->hide();
        if(pnote->enablearrowbuttons)
          arrow->hide();
      }
      else
      {
        str = sections->at(pnote->current+1)->getTitle(0);
        str += " >>";
        //debug("Setting next to: %s", str.data());
        button->setText(str.data());
        button->show();
        if(pnote->enablearrowbuttons)
          arrow->show();
      }
    }
    else
    {
      button = pnote->currentwiz->getPreviousButton();
      arrow = pnote->currentwiz->getLeftArrow();
      if((pnote->current) == 0)
      {
        button->hide();
        if(pnote->enablearrowbuttons)
          arrow->hide();
      }
      else
      {
        str = "<< ";
        str += sections->at(pnote->current-1)->getTitle(sections->at(pnote->current-1)->numPages()-1);
        //debug("Setting previous to: %s", str.data());
        button->setText(str.data());
        button->show();
        if(pnote->enablearrowbuttons)
          arrow->show();
      }
    }
  }
}

void KNoteBook::setSizes()
{
  //debug("setSizes");
  QSize childsize = childSize();

  int extra = 16;
  bool f = false;
  if(pnote->ok)
  {
    pnote->ok->adjustSize();
    f = true;
    extra += pnote->ok->height();
  }
  if(pnote->cancel)
  {
    pnote->cancel->adjustSize();
    if(!f)
    {
      f = true;
      extra += pnote->cancel->height();
    }
  }
  if(pnote->def)
  {
    pnote->def->adjustSize();
    if(!f)
    {
      f = true;
      extra += pnote->def->height();
    }
  }
  if(pnote->help)
  {
    pnote->help->adjustSize();
    if(!f)
      extra += pnote->help->height();
  }

  pnote->tabbar->adjustSize();
  int y = pnote->tabbar->height() + childsize.height() + extra;
  int x = 16 + childsize.width();
  setMinimumSize(x, y);
  resize(x, y);
  //debug("setSizes - done");
}

void KNoteBook::resizeEvent(QResizeEvent *)
{
  //debug("KNote, resizing");

  pnote->tabbar->setGeometry( 2, 2, width()-3, pnote->tabbar->sizeHint().height());

  // Check to see if there are any buttons.
  QPushButton *tmp = 0L;
  int x = 8, y = pnote->tabbar->height() + 7;
  int cx = width() - 14, cy = height()-(pnote->tabbar->height()+14);
  if( pnote->ok && pnote->ok->isVisible() )
    tmp = pnote->ok;
  else if(pnote->cancel && pnote->cancel->isVisible())
    tmp = pnote->cancel;
  else if(pnote->def && pnote->def->isVisible())
    tmp = pnote->def;
  else if(pnote->help && pnote->help->isVisible())
    tmp = pnote->help;
  if(tmp)
    cy -= (tmp->height());

  if(pnote->currentwiz)
    pnote->currentwiz->setGeometry( x, y, cx, cy );

  int offs = 2;
  if(pnote->ok && pnote->ok->isVisible())
  {
    pnote->ok->setGeometry(offs, height()-(pnote->ok->height()+2),
                           pnote->ok->width(), pnote->ok->height());
    offs += 5 + pnote->ok->width();
  }
  if(pnote->cancel && pnote->cancel->isVisible())
  {
    pnote->cancel->setGeometry(offs, height()-(pnote->cancel->height()+2),
                           pnote->cancel->width(), pnote->cancel->height());
    offs += 5 + pnote->cancel->width();
  }
  if(pnote->def && pnote->def->isVisible())
    pnote->def->setGeometry(offs, height()-(pnote->def->height()+2),
                           pnote->def->width(), pnote->def->height());
  if(pnote->help && pnote->help->isVisible())
    pnote->help->setGeometry(width()-(pnote->help->width()+1),
                             height()-(pnote->help->height()+2),
                             pnote->help->width(), pnote->help->height());

  //debug("KNote, resize - done");
}

void KNoteBook::paintEvent(QPaintEvent *)
{
  //debug("KNoteBook::paintEvent");
  // Check to see if there are any buttons.
  QPushButton *tmp = 0L;
  int w = width(), h = height(), s = 2;
  int tw = pnote->tabbar->width();
  int boffs = 4, toffs = pnote->tabbar->height() + 1;
  if( pnote->ok && pnote->ok->isVisible() )
    tmp = pnote->ok;
  else if(pnote->cancel && pnote->cancel->isVisible())
    tmp = pnote->cancel;
  else if(pnote->def && pnote->def->isVisible())
    tmp = pnote->def;
  else if(pnote->help && pnote->help->isVisible())
    tmp = pnote->help;
  if(tmp)
    boffs = tmp->height() + 4;

  // start painting widget
  QPainter paint;
  paint.begin( this );
  QPen pen( white, 1 );
  paint.setPen( pen );

  // left outer
  paint.drawLine( s, h-(boffs), s, toffs-1);

  // top outer
  paint.drawLine( tw+s , toffs, w-s, toffs);
  
  // right inner
  paint.drawLine( w-(s*3) , toffs+(s*2)+1, w-(s*3), h-(boffs+(s+1)));

  // bottom inner
  paint.drawLine( (s*3)+1 , h-(boffs+(s*2)-1), w-(s*3), h-(boffs+(s*2)-1));

  pen.setColor( black );
  paint.setPen( pen );

  // right outer
  paint.drawLine( w-s , toffs+1, w-s, h-(boffs));

  // bottom outer
  paint.drawLine( w-s , h-(boffs), s+1, h-(boffs));

  // left inner
  paint.drawLine( s*3 , h-(boffs+(s+1)), s*3, toffs+((s*2)+1) );

  // top inner
  paint.drawLine( (s*3)+1, toffs+(s*2), w-(s*3), toffs+(s*2));

  paint.end();
}
/*
// Grab QDialogs keypresses if non-modal
bool KNoteBook::eventFilter( QObject *obj, QEvent *e )
{
  if ( e->type() == Event_KeyPress && obj == this && !testWFlags(WType_Modal))
  {
    QKeyEvent *k = (QKeyEvent*)e;
    if(k->key() == Key_Escape || k->key() == Key_Return || k->key() == Key_Enter)
      return true;
  }

  return false;
}
*/

#include "knotebook.moc"
