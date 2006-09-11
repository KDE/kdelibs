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

#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kmainwindow.h>
#include <klineedit.h>
#include <kstandarddirs.h>
#include <ktoolbarlabelaction.h>
#include <ksqueezedtextlabel.h>
#include <kvbox.h>

#include <assert.h>

class MainWindow : public KMainWindow
{
  public:

    MainWindow()
    {
      KVBox* main = new KVBox( this );
      setCentralWidget( main );

      KSqueezedTextLabel* accel = new KSqueezedTextLabel( "&Really long, long, long and boring text goes here", main );

      new KSqueezedTextLabel( "Really long, long, long and boring text goes here", main );


      // first constructor
      KToolBarLabelAction* label1 = new KToolBarLabelAction( "&Label 1", actionCollection(), "label1" );

      // second constructor
      KLineEdit* lineEdit = new KLineEdit( this );
      KAction* lineEditAction = new KAction( "Line Edit", actionCollection(), "lineEdit");
      lineEditAction->setDefaultWidget(lineEdit);

      new KToolBarLabelAction( lineEditAction, "L&abel 2", actionCollection(), "label2" );

      // set buddy for label1
      label1->setBuddy( lineEditAction );
      accel->setBuddy( lineEdit );

      // third constructor
      KToolBarLabelAction* label3 = new KToolBarLabelAction( "&Really long, long, long and boring text goes here",
                                                             actionCollection(), "label3" );

      // set buddy for label3
      label3->setBuddy( lineEditAction );

      createGUI( "ktoolbarlabelactiontestui.rc" );
    }
};

int main( int argc, char **argv )
{
  KCmdLineArgs::init( argc, argv, "test", "Test" ,"test app" ,"1.0" );
  KApplication app;

  KGlobal::instance()->dirs()->addResourceDir( "data", "." );

  MainWindow* mw = new MainWindow;
  mw->show();

  return app.exec();
}

