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


#include "kwizard.h"


struct KWizProtected
{
  QPopupMenu       *menu;
  QWidget          *currentwidget;
  KSeparator       *sep1;
  KSeparator       *sep2;
  KDirectionButton *left;
  KDirectionButton *right;
  QPushButton      *ok;
  QPushButton      *cancel;
  QPushButton      *def;
  QPushButton      *help;
  QPushButton      *next;
  QPushButton      *previous;
  QLabel           *title;
  QLabel           *pagina;
  bool             directionsreflectspage;
  bool             enablepopupmenu;
  bool             enablearrowbuttons;
  int              numpages;
  int              current;
  int              minheight;
  int              minwidth;
  
};

KDialog::KDialog(QWidget *parent, const char *name, bool modal, WFlags f)
  : QDialog(parent, name, modal, f)
{
  //debug("KDialog creation");

  initMetaObject();
  setFocusPolicy(QWidget::StrongFocus);

}

// Grab QDialogs keypresses if non-modal.
void KDialog::keyPressEvent(QKeyEvent *e)
{
  if ( e->state() == 0 )
  {
    switch ( e->key() )
    {
      case Key_Escape:
      case Key_Enter:
      case Key_Return:
      {
        if(testWFlags(WType_Modal))
          QDialog::keyPressEvent(e);
        else
        {
          //debug("KDialog - Eating keyevent");
  	  e->ignore();
        }
      }
      break;
      default:
	e->ignore();
	return;
    }
  }
  else
  {
    e->ignore();
  }
}

KWizard::KWizard(QWidget *parent, const char *name, bool modal, WFlags f)
  : KDialog(parent, name, modal, f)
//  : QWidget(parent, name, modal ? (f | WType_Modal) : f)
{
  //debug("KWizard creation");

  initMetaObject();

  pwiz = new KWizProtected;
  pages = new QList<KWizardPage>;
  pages->setAutoDelete(true);

  pwiz->menu = new QPopupMenu();
  connect( pwiz->menu, SIGNAL(activated(int)), SLOT(gotoPage(int)));

  pwiz->numpages = 0;
  pwiz->current = -1;
  pwiz->minwidth = 300;
  pwiz->minheight = 300;
  pwiz->currentwidget = 0L;
  pwiz->ok = pwiz->cancel = pwiz->def = pwiz->help = pwiz->next = pwiz->previous = 0L;
  pwiz->directionsreflectspage = pwiz->enablepopupmenu = pwiz->enablearrowbuttons = false;

  pwiz->title = new QLabel(this);
  pwiz->title->installEventFilter(this);
  pwiz->title->setAlignment(AlignLeft|AlignVCenter);

  pwiz->pagina = new QLabel("Page 1 of 1", this);
  pwiz->pagina->setAlignment(AlignLeft|AlignVCenter);

  pwiz->left = new KDirectionButton(LeftArrow, this);
  connect( pwiz->left, SIGNAL(clicked()), SLOT(previousPage()));
  pwiz->left->hide();
  pwiz->right = new KDirectionButton(RightArrow, this);
  connect( pwiz->right, SIGNAL(clicked()), SLOT(nextPage()));
  pwiz->right->hide();

  pwiz->sep1 = new KSeparator(this);
  //sep1->setFixedHeight(sep1->sizeHint().height());
  pwiz->sep2 = new KSeparator(this);
  //sep2->setFixedHeight(sep2->sizeHint().height());

  pwiz->previous = new QPushButton("<< " + i18n("&Previous"), this);
  pwiz->previous->hide();
  connect( pwiz->previous, SIGNAL(clicked()), SLOT(previousPage()));

  pwiz->next = new QPushButton(i18n("&Next") + " >>", this);
  pwiz->next->hide();
  connect( pwiz->next, SIGNAL(clicked()), SLOT(nextPage()));

  installEventFilter(this);

  //debug("KWizard created");
}

// Destructor
KWizard::~KWizard()
{
  //debug("KWizard - destructor");
  //if (menu) delete menu;
  //menu = 0L;
  delete pwiz;
  delete pages;
  //debug("KWizard - destructor done");
}

void KWizard::setCancelButton(const QString& name)
{
    if (name.isNull())
	setCancelButton(i18n("&Cancel"));

  if(!pwiz->cancel)
  {
    pwiz->cancel = new QPushButton(name, this);
    pwiz->cancel->show();
    connect( pwiz->cancel, SIGNAL(clicked()), SLOT(cancelClicked()));
  }
  else
    pwiz->cancel->setText(name);
  setSizes();
}

QButton *KWizard::getCancelButton()
{
  return pwiz->cancel;
}

void KWizard::setDefaultButton(const QString& name)
{
    if (name.isNull())
	setDefaultButton(i18n("&Default"));

  if(!pwiz->def)
  {
    pwiz->def = new QPushButton(name, this);
    pwiz->def->show();
    connect( pwiz->def, SIGNAL(clicked()), SLOT(defaultClicked()));
  }
  else
    pwiz->def->setText(name);
  setSizes();
}

QButton *KWizard::getDefaultButton()
{
  return pwiz->def;
}

void KWizard::setHelpButton(const QString& name)
{
    if (name.isNull())
	setHelpButton(i18n("&Help"));

  if(!pwiz->help)
  {
    pwiz->help = new QPushButton(name, this);
    pwiz->help->show();
    connect( pwiz->help, SIGNAL(clicked()), SLOT(helpClicked()));
  }
  else
    pwiz->help->setText(name);
  setSizes();
}

QButton *KWizard::getHelpButton()
{
  return pwiz->help;
}

void KWizard::setOkButton(const QString& name)
{
    if (name.isNull())
	setOkButton(i18n("&OK"));

  if(!pwiz->ok)
  {
    pwiz->ok = new QPushButton(name, this);
    pwiz->ok->show();
    connect( pwiz->ok, SIGNAL(clicked()), SLOT(okClicked()));
  }
  else
    pwiz->ok->setText(name);
  setSizes();
}

QButton *KWizard::getOkButton()
{
  return pwiz->ok;
}

QButton *KWizard::getNextButton()
{
  return pwiz->next;
}

QButton *KWizard::getPreviousButton()
{
  return pwiz->previous;
}

KDirectionButton *KWizard::getLeftArrow()
{
  return pwiz->left;
}

KDirectionButton *KWizard::getRightArrow()
{
  return pwiz->right;
}

QSize KWizard::pageSize()
{
  //debug("Calculating sizes");
  QSize size(0,0);
  //int x = 0, y = 0;
  for(int i = 0; i < pwiz->numpages; i++)
  {
    QSize csize = pages->at(i)->w->minimumSize();
    if(size.height() < csize.height())
      size.setHeight(csize.height());
    if(size.width() < csize.width())
      size.setWidth(csize.width());
    //debug("Page size: %d x %d", size.width(), size.height());
  }

  return size;
}

void KWizard::setSizes()
{
  //debug("setSizes");
  QFont titlefont;
  //titlefont.setPointSize(titlefont.pointSize()*2);
  titlefont.setWeight(QFont::Bold);
  pwiz->title->setFont(titlefont);
  pwiz->title->adjustSize();

  //debug("Title points: %d", titlefont.pointSize());
  //debug("Title height: %d", title->height());

  pwiz->pagina->adjustSize();

  QSize pagesize = pageSize();

  int subtr = 5;
  bool f = false;
  if(pwiz->ok)
  {
    pwiz->ok->adjustSize();
    f = true;
    subtr = pwiz->ok->height() + 10;
  }
  if(pwiz->cancel)
  {
    pwiz->cancel->adjustSize();
    if(!f)
      subtr = pwiz->cancel->height() + 10;
  }
  if(pwiz->def)
  {
    pwiz->def->adjustSize();
    if(!f)
      subtr = pwiz->def->height() + 10;
  }
  if(pwiz->help)
  {
    pwiz->help->adjustSize();
    if(!f)
      subtr = pwiz->help->height() + 10;
  }

  int y = pwiz->title->height() + pagesize.height() +  pwiz->next->height() + 35 + subtr;
  int x = 20 + pagesize.width();
  setMinimumSize(x, y);
  //resize(x, y);
}

void KWizard::resizeEvent(QResizeEvent *)
{
  //QSize pagesize = pageSize();
  //debug("KWizard, resizeEvent()");

  int subtr = 4;
  if(pwiz->ok && pwiz->ok->isVisible())
    subtr = pwiz->ok->height() + 10;
  else if(pwiz->cancel && pwiz->cancel->isVisible())
    subtr = pwiz->cancel->height() + 10;
  else if(pwiz->def && pwiz->def->isVisible())
    subtr = pwiz->cancel->height() + 10;
  else if(pwiz->help && pwiz->help->isVisible())
    subtr = pwiz->help->height() + 10;

  //main->setGeometry(2, 2, width()-4, height()-(subtr));
  //debug("main");
  //pagina->adjustSize();

  pwiz->title->setGeometry(7, 7,
                           width()-14,
                           pwiz->title->sizeHint().height());

  pwiz->right->setGeometry(width()-(pwiz->title->sizeHint().height()+10),
                           5, // had to make a little hack :-(
                           pwiz->title->sizeHint().height(),
                           pwiz->title->sizeHint().height()+2);

  pwiz->left->setGeometry(width()-((pwiz->title->sizeHint().height()*2)+10),
                          5,
                          pwiz->title->sizeHint().height(),
                          pwiz->title->sizeHint().height()+2);

  pwiz->pagina->setGeometry(
        width()-(pwiz->pagina->sizeHint().width()+(pwiz->title->sizeHint().height()*2)+20),
                          7,
                          pwiz->pagina->sizeHint().width(),
                          pwiz->pagina->sizeHint().height());
  if(pwiz->numpages > 1)
    pwiz->pagina->show();
  else
    pwiz->pagina->hide();

  //debug("header - pagina: %d", pagina->width());
  pwiz->sep1->setGeometry(7,
                          pwiz->title->height()+10,
                          width()-14,
                          2);
  //debug("sep1");

  if(pwiz->currentwidget)
  {
    int hack = 15;
    if(pwiz->ok || pwiz->cancel || pwiz->help) hack = 0;
    int offs = pwiz->title->height() + 12;
    pwiz->currentwidget->setGeometry( 7, offs, width()-14,
           height()-( pwiz->next->height() + 40 + subtr + hack));
  }
  else debug("No currentwidget!");
  //debug("currentwidget");
  if(pwiz->directionsreflectspage)
  {
    QString str;
    if(pwiz->current < pwiz->numpages-1)
    {
      str = pages->at(pwiz->current+1)->title.data();
      str += " >>";
      pwiz->next->setText(str.data());
    }
    if(pwiz->current > 0)
    {
      str = "<< ";
      str += pages->at(pwiz->current-1)->title.data();
      pwiz->previous->setText(str.data());
    }
  }
  else
  {
    pwiz->next->setText(i18n("&Next") + " >>");
    pwiz->previous->setText("<< " + i18n("&Previous"));
  }

  pwiz->sep2->setGeometry(7, height()-(pwiz->next->height()+14+subtr),
                    width()-14, 2);
  //debug("sep2");
  pwiz->next->adjustSize();
  pwiz->previous->adjustSize();
  pwiz->next->move(width()-(pwiz->next->width()+7),
             height()-(pwiz->next->height()+7+subtr));
  pwiz->previous->move(width()-(pwiz->previous->width()+14+pwiz->next->width()),
             height()-(pwiz->previous->height()+7+subtr));

  int offs = 1;
  if(pwiz->ok && pwiz->ok->isVisible())
  {
    pwiz->ok->setGeometry(offs, height()-(pwiz->ok->height()+2),
                          pwiz->ok->width(), pwiz->ok->height());
    offs = 6 + pwiz->ok->width();
  }
  if(pwiz->cancel && pwiz->cancel->isVisible())
  {
    pwiz->cancel->setGeometry(offs, height()-(pwiz->cancel->height()+2),
                              pwiz->cancel->width(), pwiz->cancel->height());
    offs += 6 + pwiz->cancel->width();
  }
  if(pwiz->def && pwiz->def->isVisible())
    pwiz->def->setGeometry(offs, height()-(pwiz->def->height()+2),
                           pwiz->def->width(), pwiz->def->height());
  if(pwiz->help && pwiz->help->isVisible())
    pwiz->help->setGeometry(width()-(pwiz->help->width()+1),
                            height()-(pwiz->help->height()+2),
                            pwiz->help->width(), pwiz->cancel->height());

  //debug("KWizard, resizeEvent done");
}

void KWizard::paintEvent(QPaintEvent *)
{
  int subtr = 4;
  if(pwiz->ok)
    subtr = pwiz->ok->height() + 10;
  else if(pwiz->cancel)
    subtr = pwiz->cancel->height() + 10;
  else if(pwiz->help)
    subtr = pwiz->help->height() + 10;

  // start painting widget
  QPainter paint;
  paint.begin( this );
  QPen pen( white, 1 );
  paint.setPen( pen );

  // left
  paint.drawLine( 2, height()-subtr, 2, 3);

  // top
  paint.drawLine( 3 , 2, width()-4, 2);

  pen.setColor( black );
  paint.setPen( pen );

  // right
  paint.drawLine( width()-4 , 3, width()-4, height()-subtr);

  // bottom
  paint.drawLine( width()-4 , height()-subtr, 3, height()-subtr);

  paint.end();
}

int KWizard::addPage(KWizardPage *p)
{
  CHECK_PTR(p->w);

  p->w->recreate(this, 0, QPoint(0, 0));
  //debug("recreated");
  p->w->hide();
  //debug("hidden");
  if(pwiz->numpages == 0)
  {
    pwiz->current = 0;
    pwiz->currentwidget = p->w;
    pwiz->title->setText(p->title.data());
    pwiz->title->setEnabled(p->enabled);
    pwiz->currentwidget->setEnabled(p->enabled);
    pwiz->pagina->setEnabled(p->enabled);
    pwiz->currentwidget->show();
  }

  p->id = pwiz->numpages;
  pwiz->numpages++;
  pwiz->menu->insertItem(p->title.data(), p->id);
  pwiz->menu->setItemEnabled(p->id, p->enabled);
  //debug("inserted menuitem");
  pages->append(p);
  //titles.append(strtitle);

  if(pwiz->numpages > 1)
  {
    if(pwiz->current < (pwiz->numpages-1))
    {
      pwiz->next->show();
      if(pwiz->enablearrowbuttons)
        pwiz->right->show();
    }
    if(pwiz->current > 0)
    {
      pwiz->previous->show();
      if(pwiz->enablearrowbuttons)
        pwiz->left->show();
    }
  }

  //debug("enabled buttons");
  QString strpagina = i18n("Page %1 of %2").arg(pwiz->current+1).arg(pwiz->numpages);
  if(pwiz->numpages > 1)
    pwiz->pagina->setText(strpagina.data());
  else
    pwiz->pagina->setText("");
  //debug("pagina");
  setSizes();
  //debug("Add page, done");

  return (p->id);
}

void KWizard::setPage(int id, QWidget *w)
{
  if(!w || pages->count() <= (uint)id)
    return;
  pages->at(id)->w = w;
}

void KWizard::setPage(int id, QString title)
{
  if(pages->count() <= (uint)id)
    return;
  pages->at(id)->title = title;
}

void KWizard::setPageEnabled(int id, bool state)
{
  if(id >= 0 && id < pwiz->numpages)
  {
    pages->at(id)->enabled = state;
    pwiz->menu->setItemEnabled(id, state);
    if(id == pwiz->current)
    {
      pwiz->title->setEnabled(state);
      pwiz->pagina->setEnabled(state);
      pwiz->currentwidget->setEnabled(state);
    }
  }
}

void KWizard::okClicked()
{
  emit okclicked();
}

void KWizard::cancelClicked()
{
  emit cancelclicked();
}

void KWizard::defaultClicked()
{
  emit defaultclicked(pwiz->current);
}

void KWizard::helpClicked()
{
  emit helpclicked(pwiz->current);
}

void KWizard::setDirectionsReflectsPage(bool state)
{
  pwiz->directionsreflectspage = state;
  setSizes();
}

bool KWizard::directionsReflectsPage()
{
  return pwiz->directionsreflectspage;
}

void KWizard::setEnablePopupMenu(bool state)
{
  pwiz->enablepopupmenu = state;
}

bool KWizard::enablePopupMenu()
{
  return pwiz->enablepopupmenu;
}

QPopupMenu *KWizard::getMenu()
{
  return pwiz->menu;
}

void KWizard::setEnableArrowButtons(bool state)
{
  pwiz->enablearrowbuttons = state;
}

bool KWizard::enableArrowButtons()
{
  return pwiz->enablearrowbuttons;
}

int KWizard::numPages()
{
  return pwiz->numpages;
}

void KWizard::gotoPage(int p)
{
  //debug("gotopage: %d", p);

  if(p >= pwiz->numpages) // || p == pwiz->current)
    return;

  //debug("Changing to page %d", p);

  if(p < (pwiz->numpages-1))
  {
    pwiz->next->show();
    if(pwiz->enablearrowbuttons)
      pwiz->right->show();
  }
  if(p > 0 && p < pwiz->numpages)
  {
    pwiz->previous->show();
    if(pwiz->enablearrowbuttons)
      pwiz->left->show();
  }
  if(p == (pwiz->numpages-1))
  {
    pwiz->next->hide();
    if(pwiz->enablearrowbuttons)
      pwiz->right->hide();
    emit nomorepages(false, true);
  }
  if(p == 0)
  {
    if(pwiz->numpages > 1)
    {
      pwiz->next->show();
      if(pwiz->enablearrowbuttons)
        pwiz->right->show();
    }
    pwiz->previous->hide();
    if(pwiz->enablearrowbuttons)
      pwiz->left->hide();
    emit nomorepages(false, false);
  }

  pwiz->current = p;

  QString strpagina = i18n("Page %d of %d").arg(pwiz->current+1).arg(pwiz->numpages);
  if(pwiz->numpages > 1)
    pwiz->pagina->setText(strpagina.data());
  else
    pwiz->pagina->setText("");

  pwiz->title->setText(pages->at(pwiz->current)->title.data());
  pwiz->currentwidget->hide();
  pwiz->currentwidget = pages->at(pwiz->current)->w;
  pwiz->title->setEnabled(pages->at(pwiz->current)->enabled);
  pwiz->currentwidget->setEnabled(pages->at(pwiz->current)->enabled);
  pwiz->pagina->setEnabled(pages->at(pwiz->current)->enabled);
  pwiz->currentwidget->show();
  //setSizes();
  // fake a resize event to trigger child widget moves
  QResizeEvent r( size(), size() );
  resizeEvent( &r );
  emit selected(pwiz->current);
}

void KWizard::nextPage()
{
  if((pwiz->current+1) == pwiz->numpages)
    emit nomorepages(true, true);
  else
    gotoPage(pwiz->current + 1);
}

void KWizard::previousPage()
{
  if((pwiz->current-1) < 0)
    emit nomorepages(true, false);
  else
    gotoPage(pwiz->current - 1);
}

// Grab QDialogs keypresses if non-modal and grab mouse events on title
// to popup menu.
bool KWizard::eventFilter( QObject *obj, QEvent *e )
{
  if ( e->type() == QEvent::MouseButtonPress && obj == pwiz->title)
  {
    QMouseEvent *m = (QMouseEvent*)e;
    if(pwiz->title->rect().contains( m->pos()) && m->button() == RightButton)
    {
      //debug("KWizard::eventFilter() - inside");
      emit popup(mapToGlobal(m->pos()));
      if(!pwiz->enablepopupmenu)
        return false;
      pwiz->menu->popup(mapToGlobal(m->pos()));
      return true;
    }
  }

  if ( e->type() == QEvent::KeyPress && obj == this)
  {
    QKeyEvent *k = (QKeyEvent*)e;
    if(k->key() == Key_PageUp)
    {
      //debug("Received keyevent PageUp");
      previousPage();
      return true;
    }
    else if(k->key() == Key_PageDown)
    {
      //debug("Received keyevent PageDown");
      nextPage();
      return true;
    }
    else
      return false;
  }

  return false;
}

void KWizard::closeEvent(QCloseEvent *e)
{
  e->accept();
  emit closed();
}

QSize KWizard::sizeHint()
{
  QSize s = minimumSize();
  int h = 0;
  if(pwiz->title)
    h += pwiz->title->height();
  if(pwiz->previous)
    h += pwiz->previous->height();
  if(pwiz->ok)
    h += pwiz->ok->height();
  else if(pwiz->cancel)
    h += pwiz->cancel->height();
  else if(pwiz->def)
    h += pwiz->def->height();
  else if(pwiz->help)
    h += pwiz->help->height();

  h += 20;

  s.setHeight(s.height() + h);
  return s;
}


#include "kwizard.moc"
