/* This file is part of the KDE libraries
    Copyright (C) 2004 Felix Berger <felixberger@beldesign.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qguardedptr.h>

#include <kapplication.h>
#include <kmainwindow.h>
#include <klineedit.h>
#include <klistview.h>
#include <kstandarddirs.h>
#include <ktoolbarlabelaction.h>
#include <ksqueezedtextlabel.h> 
#include <kdebug.h>
#include <qvbox.h>

#include <assert.h>

class MainWindow : public KMainWindow
{
public:
  MainWindow()
  {
    QVBox* main = new QVBox(this);
    setCentralWidget(main);

    KSqueezedTextLabel* accel = new KSqueezedTextLabel
      ("&Really long, long, long and boring text goes here", main, 
       "kde toolbar widget");
    new KSqueezedTextLabel
      ("Really long, long, long and boring text goes here", main, 
       "kde toolbar widget");


    // first constructor
    KToolBarLabelAction* label1 = new KToolBarLabelAction("&Label 1", 0,
							  0, 0,
							  actionCollection(),
							  "label1");
    // second constructor
    KLineEdit* lineEdit = new KLineEdit(this);
    new KWidgetAction(lineEdit, "Line Edit", 0, this, 0,
		      actionCollection(), "lineEdit");
    KToolBarLabelAction* label2 = 
      new KToolBarLabelAction(lineEdit, "L&abel 2", 0, 0, 0,
			      actionCollection(),
			      "label2");

    // set buddy for label1
    label1->setBuddy(lineEdit);
    accel->setBuddy(lineEdit);

     // third constructor
    QLabel* customLabel =  new KSqueezedTextLabel
      ("&Really long, long, long and boring text goes here", this, 
        "kde toolbar widget");

    KToolBarLabelAction* label3 = new KToolBarLabelAction(customLabel, 0, 0, 0,
 							  actionCollection(),
							  "label3");
  
    // set buddy for label3
    label3->setBuddy(lineEdit);

    // customLabel->setText("&test me again some time soon");
    
    createGUI("ktoolbarlabelactiontestui.rc");
  }
};

int main( int argc, char **argv )
{
  KApplication app( argc, argv, "ktoolbarlabelactiontest" );

  KGlobal::instance()->dirs()->addResourceDir("data", ".");

  MainWindow* mw = new MainWindow;
  app.setMainWidget(mw);
  mw->show();

  return app.exec();
}

