/* -*- C++ -*-
 * A dialog class that serves as a common base class for dialogs.
 * 
 * the KDE addressbook.
 * copyright:  (C) Mirko Sucker, 1998, 1999
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko@kde.org>
 * requires:   Qt 1.4x or later 
 * $Revision$
 */
#include "dialogbase.h"
#include <qpixmap.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qpainter.h>
#include <qrect.h>

// disable debugging system:
// #include "debug.h"
#define CHECK(x)
#define REQUIRE(x)
#define ENSURE(x)

#include <kquickhelp.h>
#include <kurllabel.h>
#include <kapp.h>
#include <klocale.h>

extern "C" {
#include <stdlib.h>
}

QPixmap* DialogBase::tile=0;
const int DialogBase::Grid=3;
DialogBase* DialogBase::dummy=0;

DialogBase::DialogBase(QWidget* parent, const char* name, bool modal)
  : QDialog(parent, name, modal),
    main(0),
    mainTile(false),
    baseTile(true),
    frameBase(new QFrame(this)),
    frameMain(new QFrame(this)),
    buttonOK(new QPushButton(this)),
    buttonApply(new QPushButton(this)),
    buttonCancel(new QPushButton(this)),
    kurlHelp(new KURLLabel(frameBase)),
    showHelp(false),
    init(false)
{
  // ############################################################################
  static int InProcess;
  // -----
  connect(buttonOK, SIGNAL(clicked()), SLOT(accept()));
  connect(buttonCancel, SIGNAL(clicked()), SLOT(reject()));
  connect(buttonApply, SIGNAL(clicked()), SLOT(applyPressed()));
  connect(kurlHelp, SIGNAL(leftClickedURL(const char*)), 
	  SLOT(helpClickedSlot(const char*)));
  frameBase->setFrameStyle(34);
  frameMain->setFrameStyle(50);
  kurlHelp->hide(); // only shown if the path to a help topic is set
  kurlHelp->setAlignment(AlignRight | AlignVCenter);
  // -----
  modal=modal; // shut up
  CHECK(modal==true && "Modeless dialogs are not supported");
  if(dummy==0 && InProcess==0)
    {
      InProcess=1; 
      dummy=new DialogBase; // will never be deleted until program ends
      atexit(cleanup); // see cleanup()
      InProcess=0;
    }
  connect(this, SIGNAL(backgroundChanged()), SLOT(updateBackground()));
  connect(kapp, SIGNAL(appearanceChanged()), SLOT(initializeGeometry()));
  // -----
  updateBackground(); // set it once
  initializeGeometry();
  resize(minimumSize());
  // -----
  setButtonOKText();
  setButtonApplyText();
  setButtonCancelText();
  // -----
  init=true;
  // ############################################################################
}


DialogBase::~DialogBase()
{
  // ############################################################################
  // ############################################################################
}

void
DialogBase::cleanup()
{
  // ############################################################################
  delete dummy;
  dummy=0;
  // ############################################################################
}

void
DialogBase::setButtonOKText(const char* text, const char* tooltip, 
			    const char* quickhelp)
{
  // ############################################################################
  const char* WhatsOK=i18n
    ("If you press the <b>OK</b> button, all changes\n"
     "you made will be used to proceed.");
  // -----
  buttonOK->setText(text==0 ? i18n("&OK") : (QString)text);
  QToolTip::add(buttonOK, tooltip==0 
		? i18n("Accept settings.") 
		: (QString)tooltip);
  if(init) // after first initialization is done
    {
      KQuickHelp::remove(buttonOK);
    }
  KQuickHelp::add(buttonOK, quickhelp==0 ? WhatsOK : quickhelp);
  // ############################################################################
}

void
DialogBase::setButtonApplyText(const char* text, const char* tooltip, 
			       const char* quickhelp)
{
  // ############################################################################
  const char* WhatsApply=i18n
    ("When clicking <b>Apply</b>, the settings will be\n"
     "handed over to the program, but the dialog\n"
     "will not be closed."
     "Use this to try different settings.");
  // -----
  buttonApply->setText(text==0 ? i18n("&Apply") : (QString)text);
  QToolTip::add(buttonApply, tooltip==0 
		? i18n("Apply settings.") 
		: (QString)tooltip);
  if(init) // after first initialization is done
    {
      KQuickHelp::remove(buttonApply);
    }
  KQuickHelp::add(buttonApply, quickhelp==0 ? WhatsApply : quickhelp);
  // ############################################################################
}

void
DialogBase::setButtonCancelText(const char* text, const char* tooltip, 
				const char* quickhelp)
{
  // ############################################################################
  const char* WhatsCancel=i18n
    ("If you press the <b>Cancel</b> button, all changes\n"
     "you made will be abandoned and the dialog\n"
     "will be closed.\n"
     "The program will be in the state before\n"
     "opening the dialog.");
  // -----
  buttonCancel->setText(text==0 ? i18n("&Cancel") : (QString)text);
  QToolTip::add(buttonCancel, tooltip==0 
		? i18n("Cancel settings.") 
		: (QString)tooltip);
  if(init) // after first initialization is done
    {
      KQuickHelp::remove(buttonCancel);
    }  
  KQuickHelp::add(buttonCancel, quickhelp==0 ? WhatsCancel : quickhelp);
  // ############################################################################
}

bool DialogBase::haveBackgroundTile()
{
  // ############################################################################
  return (tile==0);
  // ############################################################################
}

const QPixmap* DialogBase::getBackgroundTile()
{
  // ############################################################################
  return tile;
  // ############################################################################
}

void DialogBase::setBackgroundTile(const QPixmap* p)
{
  // ############################################################################
  if(p!=0)
    { // ----- enable background tile:
      if(tile==0)
	{
	  tile=new QPixmap(*p); 
	} else {
	  *tile=*p;
	}
      // CHECK(tile!=0); // hangs, possibly Qt problem? Smart pointers?
    } else { // ----- disable it:
      delete tile;
      tile=0;
    }
  if(dummy!=0)
    {    
      dummy->emitBackgroundChanged();
    }
  // ############################################################################
}

void DialogBase::updateBackground()
{
  // ############################################################################
  if(tile!=0)
    {
      frameBase->setBackgroundPixmap(*tile);
      showBaseFrameTile(baseTile);
      frameMain->setBackgroundPixmap(*tile);
      showMainFrameTile(mainTile);
    } else {
      QPixmap temp; // a Null pixmap
      frameBase->setBackgroundPixmap(temp);
      frameMain->setBackgroundPixmap(temp);      
      frameBase->setBackgroundMode(PaletteBackground);
      frameMain->setBackgroundMode(PaletteBackground);
    }
  // ############################################################################
}

void DialogBase::resizeEvent(QResizeEvent*)
{
  /* Attention:
   * The resizeEvent  handler does not take care of widgets that are set to a too
   * small size. The widget will still be attempted to resize, but very likely it
   * will look ugly then. Be careful to use nothing much smaller than the widgets
   * minimum size.
   */
  // ############################################################################
  const int ButtonHeight=buttonOK->sizeHint().height(),
    ButtonWidth=QMAX(buttonOK->sizeHint().width(),
		     QMAX(buttonCancel->sizeHint().width(),
			  buttonApply->sizeHint().width())),
    FrameBaseFrameWidth=frameBase->frameWidth(),
    FrameMainWidth=width()-2*Grid-2*FrameBaseFrameWidth;
  // -----
  if(width()<sizeHint().width() || height()<sizeHint().height())
     {
       debug("DialogBase::resizeEvent: "
	     "warning - size is smaller than size hint, sloppy WM?");
     }
  int count, x, y, frameMainHeight, cx, cy;
  QPushButton *buttons[]= { buttonOK, buttonApply, 0, buttonCancel};
  const int Size=sizeof(buttons)/sizeof(buttons[0]);
  // ----- 1. set geometry of base frame:
  frameBase->setGeometry(0, 0, width(), height());
  // ----- 2. set geometry of buttons:
  x=FrameBaseFrameWidth+Grid;
  y=height()-FrameBaseFrameWidth-Grid-ButtonHeight;
  for(count=0; count<Size; count++)
    {
      if(buttons[count]!=0)
	{
	  buttons[count]->setGeometry(x, y, ButtonWidth, ButtonHeight);
	  x+=Grid+ButtonWidth;
	} else {
	  x+=16;
	}
    }
  // ----- 3. set geometry of inner "main" frame and the help label:
  x=FrameBaseFrameWidth+Grid;
  if(showHelp)
    {
      y=FrameBaseFrameWidth+2*Grid
	+ /* kurlHelp->sizeHint().height() */ ButtonHeight;
      kurlHelp->setGeometry
	(FrameBaseFrameWidth+Grid, FrameBaseFrameWidth+Grid, 
	 FrameMainWidth, ButtonHeight);
      frameMainHeight=height()-2*FrameBaseFrameWidth-5*Grid-2*ButtonHeight;
    } else {
      y=FrameBaseFrameWidth+Grid;
      frameMainHeight=height()-2*FrameBaseFrameWidth-3*Grid-ButtonHeight;
    }
  frameMain->setGeometry(x, y, FrameMainWidth, frameMainHeight);
  // ----- 4. set geometry of main widget:
  if(main!=0)
    {
      x=frameMain->x()+frameMain->frameWidth()+Grid;
      y=frameMain->y()+frameMain->frameWidth()+Grid;
      cx=frameMain->width()-2*frameMain->frameWidth()-2*Grid;
      cy=frameMain->height()-2*frameMain->frameWidth()-2*Grid;
      main->setGeometry(x, y, cx, cy);    
    }
  // ----- 5. set geometry of headline label and whats-this-button:
  x=FrameBaseFrameWidth+Grid;
  y=x;
  cx=width()-2*frameMain->frameWidth()-3*Grid /* -WhatsThisWidth */;
  x=width()-x /* -WhatsThisWidth */;
  // ############################################################################
}

void DialogBase::applyPressed()
{
  // ############################################################################
  emit(apply());
  // ############################################################################
}

void DialogBase::enableButtonOK(bool state)
{
  // ############################################################################
  buttonOK->setEnabled(state);
  // ############################################################################
}

void DialogBase::enableButtonApply(bool state)
{
  // ############################################################################
  buttonApply->setEnabled(state);
  // ############################################################################
}

void DialogBase::enableButtonCancel(bool state)
{
  // ############################################################################
  buttonCancel->setEnabled(state);
  // ############################################################################
}

void DialogBase::showButtonOK(bool state)
{
  // ############################################################################
  state ? buttonOK->show() : buttonOK->hide();
  // ############################################################################
}

void DialogBase::showButtonApply(bool state)
{
  // ############################################################################
  state ? buttonApply->show() : buttonApply->hide();
  // ############################################################################
}

void DialogBase::showButtonCancel(bool state)
{
  // ############################################################################
  state ? buttonCancel->show() : buttonCancel->hide();
  // ############################################################################
}

QRect DialogBase::getContentsRect()
{
  // ############################################################################
  QRect temp;
  // -----
  temp=frameMain->frameRect();
  temp.setX(temp.x()+frameMain->x());
  temp.setY(temp.y()+frameMain->y());
  return temp;
  // ############################################################################
}

void DialogBase::getBorderWidths(int& ulx, int& uly, int& lrx, int& lry) const
{
  // ############################################################################
  const int FrameBaseFrameWidth=frameBase->frameWidth(),
    FrameMainFrameWidth=frameMain->frameWidth(),
    ButtonHeight=buttonOK->sizeHint().height();
  // -----
  ulx=FrameBaseFrameWidth+Grid+FrameMainFrameWidth;
  if(showHelp)
    {
      uly=ulx+Grid+kurlHelp->height();
    } else {
      uly=ulx;
    }
  lrx=ulx;
  lry=FrameBaseFrameWidth+2*Grid+ButtonHeight+FrameMainFrameWidth;
  ENSURE(ulx>0 && uly>0 && lrx>0 && lry>0);
  // ############################################################################
}

void DialogBase::emitBackgroundChanged()
{
  // ############################################################################
  emit(backgroundChanged());
  // ############################################################################
}

QSize DialogBase::calculateSize(int w, int h)
{
  REQUIRE(w>0 && h>0);
  // ############################################################################
  QSize size;
  int ulx, uly, lrx, lry; // borders around inner frame
  // -----
  getBorderWidths(ulx, uly, lrx, lry);
  size.setWidth(ulx+w+lrx);
  size.setHeight(uly+h+lry);
  ENSURE(size.width()>0 && size.height()>0);
  return size;
  // ############################################################################
}

void DialogBase::showBaseFrameTile(bool state)
{
  // ############################################################################
  baseTile=state;
  if(state)
    {
      frameBase->setBackgroundPixmap(*tile);
    } else {
      frameBase->setBackgroundMode(PaletteBackground);
    }
  kurlHelp->setTransparentMode(state);
  // ############################################################################
}

void DialogBase::showMainFrameTile(bool state)
{
  // ############################################################################
  mainTile=state;
  if(state)
    {
      frameMain->setBackgroundPixmap(*tile);
    } else {
      frameMain->setBackgroundMode(PaletteBackground);
    }
  // ############################################################################
}

void DialogBase::setMainWidget(QWidget* widget)
{
  // ############################################################################
  main=widget;
  initializeGeometry();
  // ----- resize subwidgets:
  // resize(cx, cy);
  // ############################################################################
}

QWidget* DialogBase::getMainWidget()
{
  // ############################################################################
  return main;
  // ############################################################################
}

void DialogBase::initializeGeometry()
{ // this method only calculates and sets the minimum size
  // ############################################################################
  const int ButtonWidth=
    QMAX(buttonCancel->sizeHint().width(), 
	 QMAX(buttonOK->sizeHint().width(), buttonApply->sizeHint().width()));
  QSize size;
  int cx, cy; 
  int ulx, uly, lrx, lry;
  // -----
  getBorderWidths(ulx, uly, lrx, lry);
  cx=3*ButtonWidth+4*Grid+16+2*frameBase->frameWidth();
  cy=uly+lry+2*Grid;
  if(main!=0)
    {
      size=main->minimumSize();
    } else {
      cx+=100; 
      cy+=66;
    }
  // ----- set minimum size of whole dialog:
  cx=QMAX(ulx+lrx+2*Grid+(size.width()>0 ? size.width() : 0), cx);
  if(size.height()>0) 
    {
      cy+=size.height();
    }
  setMinimumSize(cx, cy);
  // ############################################################################
}

void DialogBase::setHelp(const char* newpath, const char* newtopic, 
			 const char* text)
{
  // ############################################################################
  path= (newpath==0) ? "" : newpath;
  topic= (newtopic==0) ? "" : newtopic;
  kurlHelp->setText(text==0 ? i18n("Get help...") : (QString)text);
  showHelp=(!path.isEmpty());
  if(showHelp)
    {
      kurlHelp->show();
    } else {
      kurlHelp->hide();
    }
  // ############################################################################
}

void DialogBase::helpClickedSlot(const char* url)
{
  // ############################################################################
  if(path.isEmpty())
    {
      debug("DialogBase::helpClickedSlot: no help topic.");
      kapp->beep();
    } else {
      kapp->invokeHTMLHelp(path, topic);
    }
  // ############################################################################
}

// ##############################################################################
// MOC OUTPUT FILES:
#include "dialogbase.moc"
// ##############################################################################


