#define QT3_SUPPORT
#define QT3_SUPPORT_WARNINGS

#include <QtGui/QWidget>
#include <QtCore/QString>
#include <QtGui/QMessageBox>
#include <QtGui/QTextEdit>
#include <QtCore/QCOORD>
#include <QtGui/QPixmap>
#include <QtGui/QCursor>

#include <stdlib.h>

#include "kstatusbar.h"
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kmainwindow.h>
#include <kmenubar.h>
#include "kstatusbartest.h"

testWindow::testWindow (QWidget *)
    : KXmlGuiWindow (0)
 {
    // Setup Menus
    menuBar = new KMenuBar (this);
    fileMenu = new QMenu;
    menuBar->addAction ( "&File" );
    QAction *action = fileMenu->addAction("&Exit");
    action->setShortcut( Qt::ALT + Qt::Key_Q );

    connect( action, SIGNAL( triggered() ), KApplication::kApplication(), SLOT( quit() ) );

    statusbar = new KStatusBar (this);
    statusbar->insertItem("Zoom: XXXX", 0);
    statusbar->insertItem("XXX", 1);
    statusbar->insertItem("Line: XXXXX", 2);

    statusbar->changeItem("Zoom: 100%", 0);
    statusbar->changeItem("INS", 1);
    insert = true;
    statusbar->changeItem("Line: 13567", 2);

    connect (statusbar, SIGNAL(pressed(int)), this, SLOT(slotPress(int)));
    connect (statusbar, SIGNAL(released(int)), this, SLOT(slotClick(int)));

    widget = new QTextEdit (this);

    setCentralWidget(widget);

    setCaption( KGlobal::caption() );

    smenu = new QMenu;

    smenu->addAction("50%");
    smenu->addAction("75%");
    smenu->addAction("100%");
    smenu->addAction("150%");
    smenu->addAction("200%");
    smenu->addAction("400%");
    smenu->addAction("oo%");

    connect (smenu, SIGNAL(triggered(QAction*)), this, SLOT(slotMenu(QAction*)));
}

void testWindow::slotClick(int id)
{
  switch (id)
   {
    case 0:
      break;

    case 1:
      if (insert == true)
       {
         insert = false;
         statusbar->changeItem("OVR", 1);
       }
      else
       {
         insert = true;
         statusbar->changeItem("INS", 1);
       }
      break;

    case 2:
      QMessageBox::information(0, "Go to line", "Enter line number:", "where?");
      statusbar->changeItem("16543", 2);
      break;
   }
}

void testWindow::slotPress(int id)
{
  if (id == 0)
    smenu->popup(QCursor::pos()); // This popup should understand keys up and down
}

void testWindow::slotMenu(QAction *action)
{
  QString s = "Zoom: ";
  s.append (action->text());
  statusbar->changeItem(s,0);
}

testWindow::~testWindow ()
{
  // I would delete toolbars here, but there are none
  delete statusbar;
}

int main( int argc, char *argv[] )
{
        KCmdLineArgs::init( argc, argv, "test", 0, ki18n("Test"), "1.0", ki18n("test app"));
        KApplication *myApp = new KApplication;
        testWindow *test = new testWindow;

        test->show();
        test->resize(test->width(), test->height()); // I really really really dunno why it doesn't show
        int ret = myApp->exec();

        delete test;

        return ret;
} 

#include "kstatusbartest.moc"

