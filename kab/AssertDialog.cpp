/* A dialog to handle assertions raised in the code.
 * Implementation.
 *
 * the KDE addressbook
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class, Qt > 1.40
 *             NANA (for debugging)
 * $Revision$
 */

extern "C" {
#include <signal.h>
#include <string.h>
#include "bug_3d.xpm"
	   }
#include "AssertDialog.h"
#include <kapp.h>

AssertDialog::AssertDialog(QWidget* parent, const char* name)
  : AssertDialogData(parent, name),
    action(0),
    file(i18n("(unknown file)")),
    line(i18n("(unknown line number)")),
    condition(i18n("(unknown condition)"))
{
  // ###########################################################################
  string temp;
  QPixmap pixmap((const char**)bug_3d);
  // -----
  connect(kapp, SIGNAL(appearanceChanged()), SLOT(initializeGeometry()));
  // -----
  temp=(string)i18n("You have found a bug in ")+(string)kapp->appName()+".";
  labelHeadline->setText(temp.c_str());
  // pixmap.load("bug_3d.xpm");
  labelImage->setPixmap(pixmap);
  labelImage->adjustSize();
  buttonIgnore=new QRadioButton(bgActions);
  buttonIgnore->setText
    (i18n("Ignore the error, possibly causing much more damage"));
  buttonKill=new QRadioButton(bgActions);
  buttonKill->setText(i18n("Kill the application, loosing all data"));
  buttonMail=new QRadioButton(bgActions);
  buttonMail->setText(i18n("Send an email to the application maintainer"));
  buttonMail->setEnabled(false);
  buttonOK->setText(i18n("OK"));
  bgActions->setButton(0);
  // -----
  setErrorText();
  initializeGeometry();
  // ###########################################################################
}


AssertDialog::~AssertDialog()
{
  // ###########################################################################
  // ###########################################################################
}

void AssertDialog::initializeGeometry()
{
  // ###########################################################################
  const int Grid=3;
  int cx, cy, x1, y1, x2, y2;
  // ----- 1. measure button group:
  x2=QMAX(buttonIgnore->sizeHint().width(), 
	  QMAX(buttonKill->sizeHint().width(), 
	       buttonMail->sizeHint().width()))+2*bgActions->frameWidth();  
  // ----- 2. pixmap, headline and error text:
  x1=frameBase->frameWidth()+Grid;
  y1=x1;
  labelHeadline->move(x1, y1);
  labelError->move (x1, y1+2*Grid+labelHeadline->height());
  cx=labelImage->x()+labelImage->width()+3*Grid+2*frameBase->frameWidth();
  if(cx<x2+frameBase->frameWidth()+2*Grid)
    {
      cx=x2+frameBase->frameWidth()+2*Grid;
    }
  labelImage->move(cx-labelImage->width()-x1, y1);
  cy=QMAX(labelImage->height(),
	  2*Grid+labelHeadline->height()+labelError->height())
    +2*Grid+2*frameBase->frameWidth();
  // ----- 2. button group:
  bgActions->setGeometry(x1, cy, cx-2*x1, 5*buttonIgnore->sizeHint().height());
  x2=bgActions->frameWidth()+Grid; 
  x1=bgActions->frameRect().width()-2*Grid;
  y1=buttonIgnore->sizeHint().height();
  y2=x2+y1;
  buttonIgnore->setGeometry(x2, y2, x1, y1);
  y2+=y1;
  buttonKill->setGeometry(x2, y2, x1, y1);
  y2+=y1;
  buttonMail->setGeometry(x2, y2, x1, y1);
  cy+=Grid+bgActions->height();
  // ----- 3. OK button:
  buttonOK->move(frameBase->frameWidth()+Grid, cy);
  cy+=buttonOK->height()+Grid;
  setFixedSize(cx, cy);
  frameBase->setGeometry(0, 0, cx, cy);
  // ###########################################################################
}

void AssertDialog::setFile(string f)
{
  // ###########################################################################
  file=f;
  setErrorText();
  // ###########################################################################
}

void AssertDialog::setLine(string l)
{
  // ###########################################################################
  line=l;
  setErrorText();
  // ###########################################################################
}

void AssertDialog::setCondition(string c)
{
  // ###########################################################################
  condition=c;
  setErrorText();
  // ###########################################################################
}

void AssertDialog::setErrorText()
{
  // ###########################################################################
  string temp;
  // -----
  temp=(string)i18n("The following condition failed:\n")
    +(string)i18n("In file ")+file+(string)i18n(", line ")+line+(string)":\n"
    +condition;
  labelError->setText(temp.c_str());
  labelError->adjustSize();
  // initializeGeometry();
  // ###########################################################################
}

void AssertDialog::setAction(int i)
{
  // ###########################################################################
  action=i;
  bgActions->setButton(i);
  // ###########################################################################
}

int AssertDialog::getAction()
{
  // ###########################################################################
  return action;
  // ###########################################################################
}

void evaluate_assertion(bool condition, const char* file, int line, 
			const char* cond_text)
{
  // ###########################################################################
  char buffer[64];
  // -----
  sprintf(buffer, "%i", line);
  if(!condition)
    {
      AssertDialog dialog;
      dialog.setFile(file);
      dialog.setLine(buffer);
      dialog.setCondition(cond_text);
      dialog.exec();
      switch(dialog.getAction())
	{
	case 0: { // ignore the failure
	  break;
	}
	default: // kill the app
	  raise(SIGABRT);
	  return; // never reached, I suppose
	}
    }
  // ###########################################################################
}


#include "AssertDialog.moc"
#include "AssertDialogData.moc"
