#include <qpopupmenu.h>
#include <qwidget.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qmultilinedit.h>
#include <qkeycode.h>
#include <qpixmap.h>

#include <stdlib.h>

#include "kstatusbar.h"
#include <kapp.h>
#include <ktmainwindow.h>
#include <kmenubar.h>
#include "kstatusbartest.h"

testWindow::testWindow (QWidget *, const char *name)
    : KTMainWindow (name)
 {
    // Setup Menus
    menuBar = new KMenuBar (this);
    fileMenu = new QPopupMenu;
    menuBar->insertItem ("&File", fileMenu);
    fileMenu->insertItem ("&Exit", KApplication::kApplication(),
                          SLOT( quit() ), ALT + Key_Q );
    statusbar = new KStatusBar (this);
    statusbar->insertItem("Zoom: XXXX", 0);
    statusbar->insertItem("XXX", 1);
    statusbar->insertItem("Line: XXXXX", 2);

    statusbar->changeItem("Zoom: 100%", 0);
    statusbar->changeItem("INS", 1);
    insert = TRUE;
    statusbar->changeItem("Line: 13567", 2);

    connect (statusbar, SIGNAL(pressed(int)), this, SLOT(slotPress(int)));
    connect (statusbar, SIGNAL(released(int)), this, SLOT(slotClick(int)));

    widget = new QMultiLineEdit (this);

    setMenu (menuBar);
    setStatusBar (statusbar);
    setView (widget);

	setCaption( KApplication::kApplication()->caption() );

    smenu = new QPopupMenu;
  
    smenu->insertItem("50%");
    smenu->insertItem("75%");
    smenu->insertItem("100%");
    smenu->insertItem("150%");
    smenu->insertItem("200%");
    smenu->insertItem("400%");
    smenu->insertItem("oo%");

    connect (smenu, SIGNAL(activated(int)), this, SLOT(slotMenu(int)));
}

void testWindow::slotClick(int id)
{
  switch (id)
   {
    case 0:
      break;

    case 1:
      if (insert == TRUE)
       {
         insert = FALSE;
         statusbar->changeItem("OVR", 1);
       }
      else
       {
         insert = TRUE;
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

void testWindow::slotMenu(int id)
{
  QString s = "Zoom: ";
  s.append (smenu->text(id));
  statusbar->changeItem(s.data(),0);
}

testWindow::~testWindow ()
{
  // I would delete toolbars here, but there are none
  delete statusbar;
}

int main( int argc, char *argv[] )
{
        KApplication *myApp = new KApplication( argc, argv, "KStatusBarTest" );
        testWindow *test = new testWindow;

        myApp->setMainWidget(test);
        test->show();
        test->resize(test->width(), test->height()); // I really really really dunno why it doesn't show
        int ret = myApp->exec();

        delete test;

        return ret;
} 

#include "kstatusbartest.moc"

