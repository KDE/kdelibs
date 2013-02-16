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

#include <QApplication>
#include <kxmlguiwindow.h>
#include <klineedit.h>
#include <ktoolbarlabelaction.h>
#include <ksqueezedtextlabel.h>
#include <kactioncollection.h>

#include <assert.h>

#include <QVBoxLayout>
#include <QtTest/QtTest>

class MainWindow : public KXmlGuiWindow
{
  public:

    MainWindow()
    {
      QWidget* main = new QWidget( this );
      setCentralWidget( main );

      QVBoxLayout* boxLayout = new QVBoxLayout( main );
      boxLayout->setMargin(0);
      boxLayout->setSpacing(0);

      KSqueezedTextLabel* accel = new KSqueezedTextLabel( "&Really long, long, long and boring text goes here", main );

      mainLabel = new KSqueezedTextLabel( "Click me to change Label 1 text", main );
      mainLabel->installEventFilter(this);

      boxLayout->addWidget(accel);
      boxLayout->addWidget(mainLabel);

      // first constructor
      label1 = new KToolBarLabelAction( "&Label 1", this );
      actionCollection()->addAction("label1", label1);

      KLineEdit* lineEdit = new KLineEdit( this );
      QWidgetAction* lineEditAction = new QWidgetAction(this);
      lineEditAction->setText("Line Edit");
      actionCollection()->addAction( "lineEdit", lineEditAction );
      lineEditAction->setDefaultWidget(lineEdit);

      // second constructor
      KToolBarLabelAction *label2 = new KToolBarLabelAction( lineEditAction, "This is the &second label", this );
      actionCollection()->addAction( "label2", label2 );


      // set buddy for label1
      label1->setBuddy( lineEditAction );

      // set buddy for accel
      accel->setBuddy( lineEdit );

      // another widget so lineEdit can loose focus and check budyness works
      boxLayout->addWidget(new KLineEdit( main ));

      setupGUI( Default, QFINDTESTDATA("ktoolbarlabelactiontestui.rc") );
    }

    bool eventFilter(QObject * watched, QEvent * event )
    {
      if (watched == mainLabel && event->type() == QEvent::MouseButtonPress)
      {
        label1->setText("&You clicked to make me change!");
      }
      return KXmlGuiWindow::eventFilter(watched, event);
    }

    KToolBarLabelAction* label1;
    KSqueezedTextLabel *mainLabel;
};

int main( int argc, char **argv )
{
  QApplication::setApplicationName("test");
  QApplication app(argc, argv);

  MainWindow* mw = new MainWindow;
  mw->show();

  return app.exec();
}

