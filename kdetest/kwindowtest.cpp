#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qpixmap.h>
#include <qkeycode.h>
#include <qwidget.h>
#include <qstring.h>

#include <stdlib.h>

#include <kstatusbar.h>
#include <kapp.h>
#include <ktmainwindow.h>
#include <kmenubar.h>
#include <kiconloader.h>
#include <qmultilinedit.h>
#include "kwindowtest.h"

#include <kglobal.h>


//#include <dclock.h>

/*
 Ok this is a constructor of our top widget. It inherits KTW.
 In constructor wi will create all of our interface elements:
 menubar, toolbar(s), statusbar, and main widget. Non of those
 interface is obligatory, i.e. you don't have to use menubar,
 toolbars or statusbar if you don't want to. Theoreticly, you
 don't need even main widget (but in that case, you'll get blank
 KTW).
 */

testWindow::testWindow (QWidget *, const char *name)
    : KTMainWindow (name)
{
    ena=false;
    setCaption ("Test KTW");
    /******************************/
    /* First, we setup setup Menus */
    /******************************/
    // Create menubar. Delete it in destructor!
    menuBar = new KMenuBar (this);

    // First popup... 
    fileMenu = new QPopupMenu;
    // We insert this popup in menubar with caption "File".
    // Prefix "&" means that "F" will be underlined
    menuBar->insertItem ("&File", fileMenu);
    // We insert item "Exit" with accelerator ALT-Q, and connect
    // it to application's exit-slot.
    fileMenu->insertItem ("&Exit", KApplication::getKApplication(),
                          SLOT( quit() ), ALT + Key_Q );

    // Another popup...
    toolBarMenu = new QPopupMenu;
    menuBar->insertItem ("&Toolbars", toolBarMenu);
    toolBarMenu->insertItem ("(Un)Hide tollbar 1", this, SLOT(slotHide1()));
    toolBarMenu->insertItem ("(Un)Hide tollbar 2", this, SLOT(slotHide2()));

    itemsMenu = new QPopupMenu;
    menuBar->insertItem ("&Items", itemsMenu);

    exitB = TRUE;   // exit button is shown
    lineL = TRUE;   // Lined is enabled
    greenF = FALSE;  // Frame not inserted
    
    itemsMenu->insertItem ("delete/insert exit button", this, SLOT(slotExit()));
    itemsMenu->insertItem ("insert/delete green frame!", this, SLOT(slotFrame()));
    itemsMenu->insertItem ("enable/disable Lined", this, SLOT(slotLined()));
    itemsMenu->insertItem ("Toggle fileNew", this, SLOT(slotNew()));
    itemsMenu->insertItem ("Clear comboBox", this, SLOT(slotClearCombo()));
    itemsMenu->insertItem ("Insert List in Combo", this, SLOT(slotInsertListInCombo()));
    itemsMenu->insertItem ("Make item 3 curent", this, SLOT(slotMakeItem3Current()));
    //itemsMenu->insertItem ("Insert clock!", this, SLOT(slotInsertClock()));
    itemsMenu->insertItem ("Important!", this, SLOT(slotImportant()));

	menuBar->insertSeparator();
	helpMenu = kapp->getHelpMenu( true, "KWindowTest was programmed by Sven Radej");
	menuBar->insertItem( "&Help", helpMenu );

    /**************************************************/
    /*Now, we setup statusbar order is not important. */
    /**************************************************/
    statusBar = new KStatusBar (this);
    statusBar->insertItem("Hi there!                         ", 0);
    statusBar->insertItem("Look for tooltips to see functions", 1);

    //DigitalClock *clk = new DigitalClock (statusBar);
    //clk->setFrameStyle(QFrame::NoFrame);
    //statusBar->insertWidget(clk, 70, 2);

    /***********************/
    /* And now the toolbar */
    /***********************/

    // pixmap which we will use
    QPixmap pix;

    // Create toolbar...
    toolBar = new KToolBar(this);

    // and set it to full width
    toolBar->setFullWidth(TRUE);


    
    // First four  buttons
    pix = ICON("filenew.xpm");
    toolBar->insertButton(pix, 0, SIGNAL(clicked()), this, SLOT(slotNew()),
                         TRUE, "Create.. (toggles upper button)");
    pix = ICON("fileopen.xpm");
    toolBar->insertButton(pix, 1, SIGNAL(clicked()), this, SLOT(slotOpen()),
                         false, "Open");
    pix = ICON("filefloppy.xpm");
    toolBar->insertButton(pix, 2, SIGNAL(clicked()), this, SLOT(slotSave()),
                          TRUE, "Save (beep or delayed popup)");
    toolBar->setDelayedPopup(2, itemsMenu);
    pix = ICON("fileprint.xpm");
    toolBar->insertButton(pix, 3, SIGNAL(clicked()), this, SLOT(slotPrint()),
                         TRUE, "Print (enables/disables open)");

    // And a combobox
    // arguments: text (or strList), ID, writable, signal, object, slot, enabled,
    //            tooltiptext, size
    toolBar->insertCombo (QString("one"), 4, TRUE, SIGNAL(activated(const QString&)), this,
                          SLOT(slotList(const QString&)), TRUE, "ComboBox", 150);


    // Then one line editor
    // arguments: text, id, signal, object (this), slot, enabled, tooltiptext, size
    toolBar->insertLined ("ftp://ftp.kde.org/pub/kde", 5, SIGNAL(returnPressed()), this,
                          SLOT(slotReturn()), TRUE, "Location", 200);

    // Set this Lined to auto size itself. Note that only one item (Lined or Combo)
    // Can be set to autosize; If you specify more of them only last (according to
    /// index) will be set to autosize itself. Only Lined or Combo can be
    // set to autosize. All items after autoSized one must  be aligned right.
    // Auto size is valid only for fullWidth toolbars.

    toolBar->setItemAutoSized (5);

    // Now add connection for Lined (ID 4) signal "completion" (ctrl-d pressed)
    // arguments: id, signal, object, slot
    toolBar->addConnection(5, SIGNAL(completion()), this, SLOT(slotCompletion()));
    // Add signal rotation also (ctrl-s)

    toolBar->addConnection(5, SIGNAL(rotation()), this, SLOT(slotCompletion()));

    // Now add another button and align it right
    pix = ICON("exit.xpm");
    toolBar->insertButton(pix, 6, SIGNAL(clicked()), KApplication::getKApplication(),
                          SLOT( quit() ), TRUE, "Exit");
    toolBar->alignItemRight (6);

    // Another toolbar
    tb1 = new KToolBar (this); // this one is normal and has separators


    pix = ICON("filenew.xpm");
    tb1->insertButton(pix, 0, TRUE, "Create new file2 (Toggle)");
    tb1->setToggle(0);
    tb1->addConnection (0, SIGNAL(toggled(bool)), this, SLOT(slotToggle(bool)));

    pix = ICON("fileopen.xpm");
    tb1->insertButton(pix, 1, SIGNAL(clicked()), this, SLOT(slotOpen()),
                          TRUE, "Open (starts progres in sb)");

    tb1->insertSeparator ();
    
    pix = ICON("filefloppy.xpm");
    tb1->insertButton(pix, 2, SIGNAL(clicked()), this, SLOT(slotSave()),
                      TRUE, "Save file2 (autorepeat)");
    tb1->setAutoRepeat(2);
    
    pix = ICON("fileprint.xpm");
    tb1->insertButton(pix, 3, itemsMenu, true, "Print (pops menu)");
    
    tb1->insertSeparator ();
    /**** RADIO BUTTONS */
    pix = ICON("filenew.xpm");
    tb1->insertButton(pix, 4, true, "Radiobutton1");
    tb1->setToggle(4);

    pix = ICON("fileopen.xpm");
    tb1->insertButton(pix, 5, true, "Radiobutton2");
    tb1->setToggle(5);
    
    pix = ICON("filefloppy.xpm");
    tb1->insertButton(pix, 6, true, "Radiobutton3");
    tb1->setToggle(6);
    
    pix = ICON("fileprint.xpm");
    tb1->insertButton(pix, 7, true, "Radiobutton4");
    tb1->setToggle(7);

    //Create
    rg = new KRadioGroup (tb1);

    rg->addButton(4);
    rg->addButton(5);
    rg->addButton(6);
    rg->addButton(7);

    connect (tb1, SIGNAL(toggled(int)), this, SLOT(slotToggled(int)));
    
    // Set caption for floating toolbars
    toolBar->setTitle ("Toolbar 1");
    tb1->setTitle ("Toolbar 2");

    // Set main widget. In this example it is Qt's multiline editor.
    widget = new QMultiLineEdit (this);

    // Setup is now complete

    // install menuBar
    setMenu (menuBar);

    // add two toolbars
    toolbar2 = addToolBar (tb1);
    toolbar1 = addToolBar (toolBar);

    connect (toolBar, SIGNAL(highlighted(int,bool)), this, SLOT(slotMessage(int, bool)));
    connect (tb1, SIGNAL(highlighted(int, bool)), this, SLOT(slotMessage(int, bool)));

    // Floating is enabled by default, so you don't need this.
    // toolBar->enableFloating(TRUE);
    // tb1->enableFloating(TRUE);

    // Show toolbars
    enableToolBar(KToolBar::Toggle, toolbar1);
    enableToolBar(KToolBar::Toggle, toolbar2);

    // install statusbar
    setStatusBar (statusBar);

    //... and main widget
    setView (widget, FALSE);

    // This is not strictly related to tollbars, menubars or KTW.
    // Setup popup for completions
    completions = new QPopupMenu;
  
    completions->insertItem("/");
    completions->insertItem("/usr/");
    completions->insertItem("/lib/");
    completions->insertItem("/var/");
    completions->insertItem("/bin/");
    completions->insertItem("/kde/");
    completions->insertItem("/home/");
    completions->insertItem("/vmlinuz :-)");

    connect (completions, SIGNAL(activated(int)), this, SLOT(slotCompletionsMenu(int)));
    pr = 0;
}
/***********************************/
/*  Now slots for toolbar actions  */
/***********************************/
void testWindow::slotToggled(int)
{
  statusBar->message ("Buton toggled", 1500);
}

void testWindow::slotInsertClock()
{
  //DigitalClock *clock = new DigitalClock(tb1);;
  //clock->setFrameStyle(QFrame::NoFrame);
  //tb1->insertWidget(8, 70, clock);
}

void testWindow::slotNew()
{
 tb1->toggleButton(0);
}
void testWindow::slotOpen()
{
  if (pr == 0)
    pr = new QProgressBar (statusBar);
  statusBar->message(pr);
  timer = new QTimer (pr);

  connect (timer, SIGNAL(timeout()), this, SLOT(slotGoGoGoo()));
  timer->start(100);
}

void testWindow::slotGoGoGoo()
{
  pr->setProgress(pr->progress()+1);
  if (pr->progress()==100)
  {
    timer->stop();
    statusBar->clear();
    delete pr;
    pr = 0;
  }
}

void testWindow::slotSave()
{
  kapp->beep();
  statusBar->changeItem("Saving properties...", 0);
}

void testWindow::slotPrint()
{
    statusBar->changeItem("Print file pressed", 0);
    ena=!ena;
    toolBar->setItemEnabled(1,ena );
}
void testWindow::slotReturn()
{
    QString s = "You entered ";
    s = s + toolBar->getLinedText(5);
    statusBar->changeItem(s.data(), 0);

}
void testWindow::slotList(const char *str)
{
    QString s = "You chose ";
    s = s + str;
    statusBar->changeItem(s.data(), 0);
}

void testWindow::slotCompletion()
{
  // Now do a completion
  // Call your completing function and set that text in klined
  // QString s = toolBar->getLinedText(/* ID */ 4)
  // QString completed = complete (s);
  // toolBar->setLinedText(/* ID */ 4, completed.data())

  // for now this:

  completions->popup(QCursor::pos()); // This popup should understunf keys up and down

  /* This is just an example. KLined automaticly sets cursor at end of string
   when ctrl-d or ctrl-s is pressed. KToolBar will also put cursor at end of text in Lined
   after inserting text with setLinedText (...).
  */

}

void testWindow::slotListCompletion()
{
    /*
     Combo is not behaving good and it is ugly. I will see how it behaves in Qt-1.3,
     and then decide should I make a new combobox.
     */
  QString s(toolBar->getComboItem(4));  // get text in combo
  s+= "(completing)";
  //toolBar->getCombo(4)->changeItem(s.data());   // setTextIncombo

}

void testWindow::slotCompletionsMenu(int id)
{
  // Now set text in lined
  QString s =completions->text(id);
  toolBar->setLinedText(5, s.data());  // Cursor is automaticly at the end of string after this
}

void testWindow::slotHide2 ()
{
  enableToolBar(KToolBar::Toggle, toolbar2);
}

void testWindow::slotHide1 ()
{
  enableToolBar(KToolBar::Toggle, toolbar1);
}

testWindow::~testWindow ()
{
  /********************************************************/
  /*                                                      */
  /*   THIS IS NOT ANY MORE IMPORTANT BUT ALLOWED!!!      */
  /*                                                      */
  /********************************************************/

  delete tb1->getWidget(8);
  //debug ("kwindowtest: deleted clock");
  
  if (toolBar)
    delete toolBar;
  if (tb1)
    delete tb1;
  if (menuBar)
  delete menuBar;

  debug ("kwindowtest finished");
}

void testWindow::beFixed()
{
    widget->setFixedSize (400, 200);
}

void testWindow::beYFixed()
{
    widget->setMinimumSize(400, 200);
    widget->setMaximumSize(9999, 200);
}

void testWindow::slotImportant ()
{
  statusBar->message("This important message will go away in 15 seconds", 15000);
}

void testWindow::slotExit ()
{
  if (exitB == TRUE)
   {
     toolBar->removeItem(6);
     exitB = FALSE;
   }
  else
   {
     QPixmap pix;
     pix = ICON("exit.xpm");
     toolBar->insertButton(pix, 6, SIGNAL(clicked()), KApplication::getKApplication(),
                           SLOT( quit() ), TRUE, "Exit");
     toolBar->alignItemRight (6);
     exitB = TRUE;
   }
}

void testWindow::slotLined()
{
  lineL = !lineL;
  toolBar->setItemEnabled(5, lineL); // enable/disable lined
}

void testWindow::slotToggle (bool on)
{
  if (on == TRUE)
    statusBar->changeItem("Toggle is on", 0);
  else
    statusBar->changeItem("Toggle is off", 0);
}

void testWindow::slotFrame()
{
  if (greenF == FALSE)
   {
     tb1->insertFrame(10, 100);
     tb1->alignItemRight (10); // this is pointless 'cause tb1 is not fullwidth

     QFrame *myFrame = tb1->getFrame(10); // get frame pointer

     if (myFrame == 0)
      {
        warning ("bad frame ID");
        return;
      }

     //paint it green
     // Or do whatever you want with it, just don't change its height (height = hardcoded = 24)
     // And don't move it
     // If you want to have something right from your toolbar you can reduce its
     // max_width with setMaxWidth()
     myFrame->setBackgroundColor (QColor("green"));

     greenF = TRUE;
   }
  else
   {
     tb1->removeItem (10);
     greenF = FALSE;
   }
}

void testWindow::slotMessage(int, bool boo)
{
  if (boo)
    statusBar->message("This button does this and that", 1500);
  else
    statusBar->clear();
}
// Now few Combo slots, for Torben

void testWindow::slotClearCombo()
{
  toolBar->getCombo(4)->clear();
}

void testWindow::slotInsertListInCombo()
{
  QStringList list;
  list.append("ListOne");
  list.append("ListTwo");
  list.append("ListThree");
  list.append("ListFour");
  list.append("ListFive");
  list.append("ListSix");
  list.append("ListSeven");
  list.append("ListEight");
  list.append("ListNine");
  list.append("ListTen");
  list.append("ListEleven");
  list.append("ListAndSoOn");
  toolBar->getCombo(4)->insertStringList (list,0);
}

void testWindow::slotMakeItem3Current()
{
  toolBar->getCombo(4)->setCurrentItem(3);
}

int main( int argc, char *argv[] )
{
    int i;
    KApplication *myApp = new KApplication( argc, argv );
//    testWindow *test = new testWindow;
    testWindow test;

    myApp->setMainWidget(&test);

    i = QMessageBox::information(0, "Select", "Select type of mainwidget",
                             "Fixed", "Y-fixed", "Resizable");
    if (i == 0)
        test.beFixed();
    else if (i == 1)
        test.beYFixed();

    test.show();
	test.resize(400, 500);
    int ret = myApp->exec();

    //delete test;
    return ret;
}

#include "kwindowtest.moc"

