/* -*- C++ -*-
 * This file declares a class for creating "About ..." dialogs
 * in a general way. It provides geometry management and some
 * options to connect for, like emailing the author or
 * maintainer.
 *
 * copyright:  (C) Mirko Sucker, 1999
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko@kde.org>
 * requires:   at least Qt 1.4
 * $Revision$
 */

#include "kaboutdialog.h"
#include <qrect.h>
#include <qlabel.h>
#include <kurllabel.h>
#include <qpainter.h>
#include <kapp.h>
#include <klocale.h>

#define WORKTEXT_IDENTATION 16
#define Grid 3

// #############################################################################
// MOC OUTPUT FILES:
#include "kaboutdialog.moc"
// #############################################################################

KAboutContributor::KAboutContributor(QWidget* parent, const char* n)
  : QFrame(parent, n),
    name(new QLabel(this)),
    email(new KURLLabel(this)),
    url(new KURLLabel(this))
{
  // ############################################################################
  if(name==0 || email==0)
    { // this will nearly never happen (out of memory in about box?)
      debug("KAboutContributor::KAboutContributor: Out of memory.");
      kapp->quit();
    }
  setFrameStyle(QFrame::Panel | QFrame::Raised);
  // -----
  connect(email, SIGNAL(leftClickedURL(const QString&)),
	  SLOT(emailClickedSlot(const QString&)));
  connect(url, SIGNAL(leftClickedURL(const QString&)),
	  SLOT(urlClickedSlot(const QString&)));
  // ############################################################################
}

void
KAboutContributor::setName(const QString& n)
{
  // ############################################################################
  name->setText(n);
  // ############################################################################
}

QString
KAboutContributor::getName()
{
  // ############################################################################
  return name->text();
  // ############################################################################
}
void
KAboutContributor::setURL(const QString& u)
{
  // ############################################################################
  url->setText(u);
  // ############################################################################
}

QString
KAboutContributor::getURL()
{
  // ############################################################################
  return url->text();
  // ############################################################################
}

void
KAboutContributor::setEmail(const QString& e)
{
  // ############################################################################
  email->setText(e);
  // ############################################################################
}

QString
KAboutContributor::getEmail()
{
  // ############################################################################
  return email->text();
  // ############################################################################
}

void
KAboutContributor::emailClickedSlot(const QString& e)
{
  // ############################################################################
  debug("KAboutContributor::emailClickedSlot: called.");
  emit(sendEmail(name->text(), e));
  // ############################################################################
}

void
KAboutContributor::urlClickedSlot(const QString& u)
{
  // ############################################################################
  debug("KAboutContributor::urlClickedSlot: called.");
  emit(openURL(u));
  // ############################################################################
}

void
KAboutContributor::setWork(const QString& w)
{
  // ############################################################################
  work=w;
  // ############################################################################
}

QSize
KAboutContributor::sizeHint()
{ 
  // ############################################################################
  const int FrameWidth=frameWidth();
  const int WorkTextWidth=200;
  int maxx, maxy;
  QRect rect;
  // ----- first calculate name and email width: 
  maxx=name->sizeHint().width();
  maxx=QMAX(maxx, email->sizeHint().width()+WORKTEXT_IDENTATION);
  // ----- now determine "work" text rectangle:
  if(!work.isEmpty()) // save time
    {
      rect=fontMetrics().boundingRect
	(0, 0, WorkTextWidth, 32000, WordBreak | AlignLeft, work);
    }
  if(maxx<rect.width())
  {
    maxx=WorkTextWidth+WORKTEXT_IDENTATION;
  }
  maxx=QMAX(maxx, url->sizeHint().width()+WORKTEXT_IDENTATION);
  // -----
  maxy=2*(name->sizeHint().height()+Grid); // need a space above the KURLLabels
  maxy+=/* email */ name->sizeHint().height();
  maxy+=rect.height();
  // -----
  maxx+=2*FrameWidth;
  maxy+=2*FrameWidth;
  return QSize(maxx, maxy);
  // ############################################################################
}

void
KAboutContributor::resizeEvent(QResizeEvent*)
{ // the widgets are simply aligned from top to bottom, since the parent is
  // expected to respect the size hint
  // ############################################################################
  int framewidth=frameWidth(), childwidth=width()-2*framewidth;
  int cy=framewidth;
  // -----
  name->setGeometry
    (framewidth, framewidth, childwidth, name->sizeHint().height());
  cy=name->height()+Grid;
  email->setGeometry
    (framewidth+WORKTEXT_IDENTATION, cy,
     childwidth-WORKTEXT_IDENTATION, /* email */ name->sizeHint().height());
  cy+=name->height()+Grid;
  url->setGeometry
    (framewidth+WORKTEXT_IDENTATION, cy,
     childwidth-WORKTEXT_IDENTATION, /* url */ name->sizeHint().height());
  // the work text is drawn in the paint event
  // ############################################################################
}

void
KAboutContributor::paintEvent(QPaintEvent* e)
{ // the widgets are simply aligned from top to bottom, since the parent is
  // expected to respect the size hint (the widget is only used locally by now)
  // ############################################################################
  int cy=frameWidth()+name->height()+email->height()+Grid+url->height()+Grid;
  int h=height()-cy-frameWidth();
  int w=width()-WORKTEXT_IDENTATION-2*frameWidth();
  // -----
  QFrame::paintEvent(e);
  if(work.isEmpty()) return;
  QPainter paint(this); // construct painter only if there is something to draw
  // -----
  paint.drawText(WORKTEXT_IDENTATION, cy, w, h, AlignLeft | WordBreak, work);
  // ############################################################################
}

KAboutWidget::KAboutWidget(QWidget* parent, const char* name)
  : QWidget(parent, name),
    version(new QLabel(this)),
    cont(new QLabel(this)),
    logo(new QLabel(this)),
    author(new KAboutContributor(this)),
    maintainer(new KAboutContributor(this)),
    showMaintainer(false)
{
  // ############################################################################
  if(version==0 || cont==0 || logo==0 ||
     author==0 || maintainer==0)
    { // this will nearly never happen (out of memory in about box?)
      debug("KAboutWidget::KAboutWidget: Out of memory.");
      kapp->quit();
    }
  // -----
  cont->setText(i18n("Other Contributors:"));
  logo->setText(i18n("(No logo available)"));
  logo->setFrameStyle(QFrame::Panel | QFrame::Raised);
  version->setAlignment(AlignCenter);
  // -----
  connect(author, SIGNAL(sendEmail(const QString&, const QString&)),
	  SLOT(sendEmailSlot(const QString&, const QString&)));
  connect(author, SIGNAL(openURL(const QString&)),
	  SLOT(openURLSlot(const QString&)));
  connect(maintainer, SIGNAL(sendEmail(const QString&, const QString&)),
	  SLOT(sendEmailSlot(const QString&, const QString&)));
  connect(maintainer, SIGNAL(openURL(const QString&)),
	  SLOT(openURLSlot(const QString&)));
  // ############################################################################
}

void
KAboutWidget::adjust()
{
  // ############################################################################
  int cx, cy, tempx;
  list<KAboutContributor*>::iterator pos;
  int maintWidth, maintHeight;
  QSize size;
  // -----
  if(showMaintainer)
    {
      size=maintainer->sizeHint();
      maintWidth=size.width();
      maintHeight=size.height();
    } else {
      maintWidth=0;
      maintHeight=0;
    }
  size=author->sizeHint();
  logo->adjustSize();
  cy=version->sizeHint().height()+Grid;
  cx=logo->width();
  tempx=QMAX(size.width(), maintWidth);
  cx+=Grid+tempx;
  cx=QMAX(cx, version->sizeHint().width());
  cy+=QMAX(logo->height(),
	   size.height()+(showMaintainer ? Grid+maintHeight : 0));
  // -----
  if(!contributors.empty())
    {
      cx=QMAX(cx, cont->sizeHint().width());
      cy+=cont->sizeHint().height()+Grid;
      for(pos=contributors.begin(); pos!=contributors.end(); ++pos)
	{
	  cy+=(*pos)->sizeHint().height();
	}
    }
  // -----
  setMinimumSize(cx, cy);
  // ############################################################################
}

void 
KAboutWidget::setLogo(const QPixmap& i)
{
  // ############################################################################
  logo->setPixmap(i);
  // ############################################################################
}

void 
KAboutWidget::sendEmailSlot(const QString& name, const QString& email)
{
  // ############################################################################
  debug("KAboutWidget::sendEmailSlot: called.");
  emit(sendEmail(name, email));
  // ############################################################################
}

void 
KAboutWidget::openURLSlot(const QString& url)
{
  // ############################################################################
  debug("KAboutWidget::openURLSlot: called.");
  emit(openURL(url));
  // ############################################################################
}

void
KAboutWidget::setAuthor(const QString& name, const QString& email,
			const QString& url, const QString& w)
{
  // ############################################################################
  author->setName(name);
  author->setEmail(email);
  author->setURL(url);
  author->setWork(w);
  // ############################################################################
}

void
KAboutWidget::setMaintainer(const QString& name, const QString& email,
			    const QString& url, const QString& w)
{
  // ############################################################################
  maintainer->setName(name);
  maintainer->setEmail(email);
  maintainer->setWork(w);
  maintainer->setURL(url);
  showMaintainer=true;
  // ############################################################################
}

void
KAboutWidget::addContributor(const QString& n, const QString& e,
			     const QString& url, const QString& w)
{
  // ############################################################################
  KAboutContributor *c=new KAboutContributor(this);
  // -----
  c->setName(n);
  c->setEmail(e);
  c->setURL(url);
  c->setWork(w);
  contributors.push_back(c);
  connect(c, SIGNAL(sendEmail(const QString&, const QString&)),
	  SLOT(sendEmailSlot(const QString&, const QString&)));
  connect(c, SIGNAL(openURL(const QString&)), SLOT(openURLSlot(const QString&)));
  // ############################################################################
}

void
KAboutWidget::setVersion(const QString& name)
{
  // ############################################################################
  version->setText(name);
  // ############################################################################
}

void
KAboutWidget::resizeEvent(QResizeEvent*)
{
  // ############################################################################
  int x, y, cx, tempx, tempy;
  list<KAboutContributor*>::iterator pos;
  // -----
  x=0;
  // ----- set version label geometry:
  version->setGeometry(0, 0, width(), version->sizeHint().height());
  y=version->height()+Grid;
  // ----- move logo to correct position:
  logo->adjustSize();
  logo->move(0, y);
  // ----- move author and maintainer right to it:
  tempx=logo->width()+Grid;
  cx=width()-tempx;
  author->setGeometry
    (tempx, y, cx, author->sizeHint().height());
  maintainer->setGeometry
    (tempx, y+author->height()+Grid, cx, maintainer->sizeHint().height());
  y+=QMAX(logo->height(),
	  author->height()+(showMaintainer ? Grid+maintainer->height() : 0));
  // -----
  if(!contributors.empty())
    {
      tempy=cont->sizeHint().height();
      cont->setGeometry(0, y, width(), tempy);
      cont->show();
      y+=tempy+Grid;
    } else {
      cont->hide();
    }
  for(pos=contributors.begin(); pos!=contributors.end(); ++pos)
    {
      tempy=(*pos)->sizeHint().height();
      // y+=Grid;
      (*pos)->setGeometry(x, y, width(), tempy);
      y+=tempy;
    }
  if(showMaintainer)
    {
      maintainer->show();
    } else {
      maintainer->hide();
    }
  // ############################################################################
}

KAboutDialog::KAboutDialog(QWidget* parent, const char* name)
  : DialogBase(parent, name),
    about(new KAboutWidget(this))
{
  // ############################################################################
  if(about==0)
    { // this will nearly never happen (out of memory in about box?)
      debug("KAboutDialog::KAboutDialog: Out of memory.");
      kapp->quit();
    }
  setMainWidget(about);
  showButtonApply(false);
  showButtonCancel(false);
  connect(about, SIGNAL(sendEmail(const QString&, const QString&)),
	  SLOT(sendEmailSlot(const QString&, const QString&)));
  connect(about, SIGNAL(openURL(const QString&)), SLOT(openURLSlot(const QString&)));
  // ############################################################################
}

void
KAboutDialog::setLogo(const QPixmap& i)
{
  // ############################################################################
  about->setLogo(i);
  // ############################################################################
}

void
KAboutDialog::adjust()
{
  // ############################################################################
  about->adjust();
  initializeGeometry();
  resize(minimumSize());
  // ############################################################################
}

void
KAboutDialog::setMaintainer(const QString& name, const QString& email,
			    const QString& url, const QString& w)
{
  // ############################################################################
  about->setMaintainer(name, email, url, w);
  // ############################################################################
}

void
KAboutDialog::setAuthor(const QString& name, const QString& email,
			const QString& url, const QString& work)
{
  // ############################################################################
  about->setAuthor(name, email, url, work);
  // ############################################################################
}

void
KAboutDialog::addContributor(const QString& n, const QString& e,
			     const QString& u, const QString& w)
{
  // ############################################################################
  about->addContributor(n, e, u, w);
  // ############################################################################
}

void
KAboutDialog::setVersion(const QString& name)
{
  // ############################################################################
  about->setVersion(name);
  // ############################################################################
}

void
KAboutDialog::sendEmailSlot(const QString& name, const QString& email)
{
  // ############################################################################
  debug("KAboutDialog::sendEmailSlot: request to send an email to %s <%s>.",
	name.ascii(), email.ascii());
  emit(sendEmail(name, email));
  // ############################################################################
}

void
KAboutDialog::openURLSlot(const QString& url)
{
  // ############################################################################
  debug("KAboutDialog::openURLSlot: request to open URL <%s>.", url.ascii());
  emit(openURL(url));
  // ############################################################################
}

