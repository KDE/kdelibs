#include <qpopmenu.h>
#include <qpixmap.h>
#include <qkeycode.h>
#include <qwidget.h>
#include <qstring.h>

#include <stdlib.h>

#include <kstatusbar.h>
#include <kapp.h>
#include <kdocktoolbar.h>
#include <ktopwidget.h>
#include <kmenubar.h>
#include <kiconloader.h>
#include <qmlined.h>
#include "kwindowtest.moc"

#include <kmsgbox.h>

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
    : KTopLevelWidget (name)
{

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
    itemsMenu->insertItem ("decrease MaxWidth", this, SLOT(slotWidth()));
    itemsMenu->insertItem ("Toggle fileNew", this, SLOT(slotNew()));
    itemsMenu->insertItem ("Clear comboBox", this, SLOT(slotClearCombo()));
    itemsMenu->insertItem ("Insert List in Combo", this, SLOT(slotInsertListInCombo()));
    itemsMenu->insertItem ("Make item 3 curent", this, SLOT(slotMakeItem3Current()));

    /**************************************************/
    /*Now, we setup statusbar order is not important. */
    /**************************************************/
    statusBar = new KStatusBar (this);
    statusBar->insertItem("(ctrl-d for completions in LineEdit or Combo) ", 0);

    /***********************/
    /* And now the toolbar */
    /***********************/

    // pixmap which we will use
    QPixmap pix;

    // We get iconloader from KApplication
    KIconLoader *loader = KApplication::getKApplication()->getIconLoader();

    // Create toolbar...
    toolBar = new KToolBar(this);

    // and set it to full width
    toolBar->setFullWidth(TRUE);


    
    // First four  buttons
    pix = loader->loadIcon("filenew.xpm");
    toolBar->insertButton(pix, 0, SIGNAL(clicked()), this, SLOT(slotNew()),
                         TRUE, "Create new file");
    pix = loader->loadIcon("fileopen.xpm");
    toolBar->insertButton(pix, 1, SIGNAL(clicked()), this, SLOT(slotOpen()),
                         TRUE, "Read window properties");
    pix = loader->loadIcon("filefloppy.xpm");
    toolBar->insertButton(pix, 2, SIGNAL(clicked()), this, SLOT(slotSave()),
                         TRUE, "Save window properties");
    pix = loader->loadIcon("fileprint.xpm");
    toolBar->insertButton(pix, 3, SIGNAL(clicked()), this, SLOT(slotPrint()),
                         TRUE, "Print file");

    // And a combobox
    // arguments: text (or strList), ID, writable, signal, object, slot, enabled,
    //            tooltiptext, size
    toolBar->insertCombo ("one", 4, TRUE, SIGNAL(activated(const char *)), this,
                          SLOT(slotList(const char *)), TRUE, "ComboBox", 150);


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

    // Add signals for combo, too

    toolBar->addConnection(4, SIGNAL(completion()), this, SLOT(slotListCompletion()));
    toolBar->addConnection(4, SIGNAL(rotation()), this, SLOT(slotListCompletion()));

    // Now add another button and align it right
    pix = loader->loadIcon("exit.xpm");
    toolBar->insertButton(pix, 6, SIGNAL(clicked()), KApplication::getKApplication(),
                          SLOT( quit() ), TRUE, "Exit");
    toolBar->alignItemRight (6);

    // Another toolbar
    tb1 = new KToolBar (this); // this one is normal and has separators


    pix = loader->loadIcon("filenew.xpm");
    tb1->insertButton(pix, 0, TRUE, "Create new file2 (Toggle)");
    tb1->setToggle(0);
    tb1->addConnection (0, SIGNAL(toggled(bool)), this, SLOT(slotToggle(bool)));

    pix = loader->loadIcon("fileopen.xpm");
    tb1->insertButton(pix, 1, SIGNAL(clicked()), this, SLOT(slotOpen()),
                          TRUE, "Open existing file2");

    tb1->insertSeparator ();
    
    pix = loader->loadIcon("filefloppy.xpm");
    tb1->insertButton(pix, 2, SIGNAL(clicked()), this, SLOT(slotSave()),
                         TRUE, "Save file2");
    pix = loader->loadIcon("fileprint.xpm");
    tb1->insertButton(pix, 3, SIGNAL(clicked()), this, SLOT(slotPrint()),
                         TRUE, "Print file2");
    
    tb1->insertSeparator ();

    pix = loader->loadIcon("filenew.xpm");
    tb1->insertButton(pix, 4, SIGNAL(clicked()), this, SLOT(slotNew()),
                         TRUE, "Create new file2");

    pix = loader->loadIcon("fileopen.xpm");
    tb1->insertButton(pix, 5, SIGNAL(clicked()), this, SLOT(slotOpen()),
                          TRUE, "Open existing file2");

    tb1->insertSeparator ();
    
    pix = loader->loadIcon("filefloppy.xpm");
    tb1->insertButton(pix, 6, SIGNAL(clicked()), this, SLOT(slotSave()),
                         TRUE, "Save file2");
    pix = loader->loadIcon("fileprint.xpm");
    tb1->insertButton(pix, 7, SIGNAL(clicked()), this, SLOT(slotPrint()),
                         TRUE, "Print file2");

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
}
/***********************************/
/*  Now slots for toolbar actions  */
/***********************************/

void testWindow::slotNew()
{
 tb1->toggleButton(0);
}
void testWindow::slotOpen()
{
    statusBar->changeItem("Reading properties...", 0);
    if (readProperties () == TRUE) // specific then global
        statusBar->changeItem("Reading properties... Done.", 0);
    else
        statusBar->changeItem("Error reading properties.", 0);

}
void testWindow::slotSave()
{
    statusBar->changeItem("Saving properties...", 0);
    if (saveProperties (FALSE) == TRUE) // global
        statusBar->changeItem("Saving properties... Done.", 0);
    else
        statusBar->changeItem("Error saving properties.", 0);

}
void testWindow::slotPrint()
{
    statusBar->changeItem("Print file pressed", 0);

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
  toolBar->changeComboItem(4, s.data());   // setTextIncombo

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
    /*                THIS IS IMPORTANT!!                  */
    /*                                                      */
    /********************************************************/
    
  delete toolBar;
  delete tb1;
  delete menuBar;
}

void testWindow::beFixed()
{
    widget->setFixedSize (400, 300);
}

void testWindow::beYFixed()
{
    widget->setMinimumSize(400, 300);
    widget->setMaximumSize(9999, 300);
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
     KIconLoader *loader = KApplication::getKApplication()->getIconLoader();
     QPixmap pix;
     pix = loader->loadIcon("exit.xpm");
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

void testWindow::slotWidth()
{
  // this sets maximal width for horizontal toolbars
  // argument is width of free space between end of horizontal toolbar and
  // toplewel end of toplevelwidget
  // That is we, want toolbar to be max KTopLevelWidget::width() - 40 wide
  
  toolBar->setMaxWidth(40);
  tb1->setMaxWidth(40);
}

// Now few Combo slots, for Torben

void testWindow::slotClearCombo()
{
  toolBar->clearCombo(4);
}

void testWindow::slotInsertListInCombo()
{
  QStrList *list = new QStrList;
  list->append("ListOne");
  list->append("ListTwo");
  list->append("ListThree");
  list->append("ListFour");
  list->append("ListFive");
  list->append("ListSix");
  list->append("ListSeven");
  list->append("ListEight");
  list->append("ListNine");
  list->append("ListTen");
  list->append("ListEleven");
  list->append("ListAndSoOn");
  toolBar->insertComboList (4, list, 0);
}

void testWindow::slotMakeItem3Current()
{
  toolBar->setCurrentComboItem(4, 3);
}

int main( int argc, char *argv[] )
{
    int i;
    KApplication *myApp = new KApplication( argc, argv );
    testWindow *test = new testWindow;

    myApp->setMainWidget(test);
    i = KMsgBox::yesNoCancel(0, "Select", "Select type of mainwidget",
                             0, "Fixed", "Y-fixed", "Resizable");
    if (i == 1)
        test->beFixed();
    else if (i == 2)
        test->beYFixed();

    test->show();
    int ret = myApp->exec();

    delete test;
    return ret;
}






