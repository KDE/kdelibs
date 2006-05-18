#include <QMessageBox>
#include <QTextEdit>
#include <QDir>

#include <stdlib.h>

#include <kstatusbar.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <khelpmenu.h>
#include <kcmdlineargs.h>
#include <kmenu.h>
#include <kactionclasses.h>
#include "kwindowtest.h"
#include <klineedit.h>
#include <kstdaction.h>
#include <kstandarddirs.h>
#include <kxmlguifactory.h>

/*
 Ok this is a constructor of our top widget. It inherits KMainWindow.
 In constructor wi will create all of our interface elements:
 menubar, toolbar(s), statusbar, and main widget. Non of those
 interface is obligatory, i.e. you don't have to use menubar,
 toolbars or statusbar if you don't want to. Theoreticly, you
 don't need even main widget (but in that case, you'll get blank
 KMainWindow).
 */

testWindow::testWindow (QWidget *parent)
    : KMainWindow (parent)
{
    ena=false;
    setCaption("test window");
	
#if 0
    /******************************/
    /* First, we setup setup Menus */
    /******************************/
    menuBar = new KMenuBar (this);
    setMenuBar(menuBar);

    // First popup...
    fileMenu = new QMenu;
    // We insert this popup in menubar with caption "File".
    // Prefix "&" means that "F" will be underlined
    QAction* fileAction = menuBar->addMenu(fileMenu);
    fileAction->setText("&File");
    // We insert item "Quit" with accelerator ALT-Q, and connect
    // it to application's exit-slot.
    fileMenu->addAction("&Quit", KApplication::kApplication(),
                          SLOT( quit() ), Qt::ALT + Qt::Key_Q );

    // Another popup...
    toolBarMenu = new QMenu;
    QAction* toolBarsAction = menuBar->addAction("&Toolbars");
    toolBarsAction->setMenu(toolBarMenu);
    toolBarMenu->addAction ("(Un)Hide toolbar 1", this, SLOT(slotHide1()));
    toolBarMenu->addAction ("(Un)Hide toolbar 2", this, SLOT(slotHide2()));

    itemsMenu = new QMenu;
    QAction* itemsAction = menuBar->addAction ("&Items");
    itemsAction->setMenu(itemsMenu);

    exitB = true;   // exit button is shown
    lineL = true;   // LineEdit is enabled
    greenF = false;  // Frame not inserted

    itemsMenu->addAction ("delete/insert exit button", this, SLOT(slotExit()));
    //itemsMenu->addAction ("insert/delete green frame!", this, SLOT(slotFrame()));
    itemsMenu->addAction ("enable/disable lineedit", this, SLOT(slotLined()));
    itemsMenu->addAction ("Toggle fileNew", this, SLOT(slotNew()));
    itemsMenu->addAction ("Combo: clear", this, SLOT(slotClearCombo()));
    itemsMenu->addAction ("Combo: insert list", this, SLOT(slotInsertListInCombo()));
    itemsMenu->addAction ("Combo: make item 3 current", this, SLOT(slotMakeItem3Current()));
    //itemsMenu->addAction ("Insert clock!", this, SLOT(slotInsertClock()));
    itemsMenu->addAction ("Important msg in statusbar", this, SLOT(slotImportant()));

    menuBar->addSeparator();
    helpMenu = new KHelpMenu(this, "KWindowTest was programmed by Sven Radej");
    QAction* helpAction = menuBar->addAction( "&Help" );
    helpAction->setMenu(helpMenu->menu());

#endif
    KStdAction::configureToolbars( this, SLOT( configureToolbars() ),  actionCollection() );
	KStdAction::keyBindings( guiFactory(), SLOT( configureShortcuts() ), actionCollection() );


    /**************************************************/
    /*Now, we setup statusbar; order is not important. */
    /**************************************************/
    statusBar = new KStatusBar (this);
    statusBar->insertItem("Hi there!                         ", 0);
    statusBar->insertItem("Look for tooltips to see functions", 1);
    setStatusBar(statusBar);

    //DigitalClock *clk = new DigitalClock (statusBar);
    //clk->setFrameStyle(QFrame::NoFrame);
    //statusBar->insertWidget(clk, 70, 2);

    /***********************/
    /* And now the toolbar */
    /***********************/

    // Create main toolbar...
    
	// and set it to full width
    //tb->setFullSize(true);
    //addToolBarBreak();


    // First four  buttons
    fileNewAction = new KAction(KIcon("filenew"), "Create.. (toggles upper button)", actionCollection(), "filenew");
    fileNewAction->setCheckable(true);
    connect(fileNewAction, SIGNAL(triggered(bool)), SLOT(slotNew()));

    KAction* fileOpenAction = new KAction(KIcon("fileopen"), "Open", actionCollection(), "fileopen");
    connect(fileOpenAction, SIGNAL(triggered(bool)), SLOT(slotOpen()));

    KActionMenu* fileFloppyAction = new KActionMenu(KIcon("filefloppy"), "Save (beep or delayed popup)", actionCollection(), "filefloppy");
    fileFloppyAction->setDelayed(true);
    connect(fileFloppyAction, SIGNAL(triggered(bool)), SLOT(slotSave()));

    KAction* filePrintAction = new KAction(KIcon("fileprint"), "Print (enables/disables open)", actionCollection(), "fileprint");
    connect(fileFloppyAction, SIGNAL(triggered(bool)), SLOT(slotPrint()));

    // And a combobox
    // arguments: text (or strList), ID, writable, signal, object, slot, enabled,
    //            tooltiptext, size
	testComboBox = new KComboBox(toolBar());
    //K3WidgetAction* comboAction = new K3WidgetAction(testComboBox, QString(), 0, 0, 0, actionCollection(), "combobox");
    //connect(testComboBox, SIGNAL(activated(const QString&)), this, SLOT(slotList(const QString&)));


    // Then one line editor
    // arguments: text, id, signal, object (this), slot, enabled, tooltiptext, size
	testLineEdit = new KLineEdit(toolBar());
    testLineEdit->setText("ftp://ftp.kde.org/pub/kde");
//    K3WidgetAction* lineEditAction = new K3WidgetAction(testLineEdit, QString(), 0, 0, 0, actionCollection(), "location");
//    connect(testLineEdit, SIGNAL(returnPressed()), this, SLOT(slotReturn()));

    // Now add another button and align it right
    exitAction = new KAction("exit", "Exit", actionCollection(), "exit");
    connect (exitAction, SIGNAL(triggered(bool)), KApplication::kApplication(), SLOT( quit() ));

    // Another toolbar

    KAction* fileNewAction2 = new KAction("filenew", "Create new file2 (Toggle)", actionCollection(), "filenew2");
    connect(fileNewAction2, SIGNAL(toggled(bool)), this, SLOT(slotToggle(bool)));

    KAction* fileOpenAction2 = new KAction("fileopen", "Open (starts progres in sb)", actionCollection(), "fileopen2");
    connect(fileOpenAction2, SIGNAL(triggered(bool)), SLOT(slotOpen()));

    
    KAction* fileFloppyAction2 = new KAction("filefloppy", "Save file2 (autorepeat)", actionCollection(), "filefloppy2");
    connect(fileFloppyAction2, SIGNAL(triggered(bool)), this, SLOT(slotSave()));

    KAction* filePrintAction2 = new KAction("fileprint", "Print (pops menu)", actionCollection(), "fileprint2");
    filePrintAction2->setMenu(itemsMenu);

	
    // *** RADIO BUTTONS
    QActionGroup* radioGroup = new QActionGroup(this);
    radioGroup->setExclusive(true);
    new KToggleAction("filenew", "Radiobutton1", actionCollection(), "radioButton1", radioGroup);
    new KToggleAction("fileopen", "Radiobutton2", actionCollection(), "radioButton2", radioGroup);
    new KToggleAction("filefloppy", "Radiobutton3", actionCollection(), "radioButton3", radioGroup);
    new KToggleAction("fileprint", "Radiobutton4", actionCollection(), "radioButton4", radioGroup);

    connect (radioGroup, SIGNAL(triggered(QAction*)), this, SLOT(slotToggled(QAction*)));


    // Set main widget. In this example it is Qt's multiline editor.
    widget = new QTextEdit (this);
    setCentralWidget (widget);

    // Setup is now complete

    setAutoSaveSettings();

    // This is not strictly related to toolbars, menubars or KMainWindow.
    // Setup popup for completions
    completions = new QMenu;

    completions->addAction("/");
    completions->addAction("/usr/");
    completions->addAction("/lib/");
    completions->addAction("/var/");
    completions->addAction("/bin/");
    completions->addAction("/kde/");
    completions->addAction("/home/");
    completions->addAction("/vmlinuz :-)");

    connect (completions, SIGNAL(triggered(QAction*)), this, SLOT(slotCompletionsMenu(QAction*)));
    pr = 0;
    
    setStandardToolBarMenuEnabled(true);
	
    // KXMLGUIClient looks in the "data" resource for the .rc files
    // Let's add $PWD (ideally $srcdir instead...) to it
	KGlobal::dirs()->addResourceDir( "data", QDir::currentPath() );
	createGUI( "./kwindowtest.rc" );
	
	tb=toolBar();
	tb1=toolBar("AnotherToolBar");
//	qobject_cast<QToolButton*>(tb1->widgetForAction(fileFloppyAction2))->setAutoRepeat(true);


}
/***********************************/
/*  Now slots for toolbar actions  */
/***********************************/
void testWindow::slotToggled(QAction*)
{
  statusBar->showMessage ("Button toggled", 1500);
}

void testWindow::slotInsertClock()
{
  //DigitalClock *clock = new DigitalClock(tb1);
  //clock->setFrameStyle(QFrame::NoFrame);
  //tb1->insertWidget(8, 70, clock);
}

void testWindow::slotNew()
{
 tb1->actions()[0]->toggle();
 //toolBar()->removeAction( fileNewAction );
}
void testWindow::slotOpen()
{
  if (pr == 0)
    pr = new QProgressBar (statusBar);
//  statusBar->message(pr);
  timer = new QTimer (pr);

  connect (timer, SIGNAL(timeout()), this, SLOT(slotGoGoGoo()));
  timer->start(100);
}

void testWindow::slotGoGoGoo()
{
  pr->setValue(pr->value()+1);
  if (pr->value()==100)
  {
    timer->stop();
    statusBar->clearMessage();
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
    qobject_cast<KAction*>(sender())->setEnabled(ena);
}
void testWindow::slotReturn()
{
    QString s = "You entered ";
    s = s + testLineEdit->text();
    statusBar->changeItem(s, 0);

}
void testWindow::slotList(const QString &str)
{
    QString s = "You chose ";
    s = s + str;
    statusBar->changeItem(s, 0);
}

void testWindow::slotCompletion()
{
  // Now do a completion
  // Call your completing function and set that text in lineedit
  // QString s = tb->getLinedText(/* ID */ 4)
  // QString completed = complete (s);
  // tb->setLinedText(/* ID */ 4, completed.data())

  // for now this:

  completions->popup(QCursor::pos()); // This popup should understunf keys up and down

  /* This is just an example. KLineEdit automatically sets cursor at end of string
   when ctrl-d or ctrl-s is pressed. KToolBar will also put cursor at end of text in LineEdit
   after inserting text with setLinedText (...).
  */

}

void testWindow::slotListCompletion()
{
    /*
     Combo is not behaving good and it is ugly. I will see how it behaves in Qt-1.3,
     and then decide should I make a new combobox.
     */
  QString s(testComboBox->currentText());  // get text in combo
  s+= "(completing)";
  //tb->getCombo(4)->changeItem(s.data());   // setTextIncombo

}

void testWindow::slotCompletionsMenu(QAction* action)
{
  // Now set text in lined
  QString s =action->text();
  testLineEdit->setText(s);  // Cursor is automatically at the end of string after this
}

void testWindow::slotHide2 ()
{
  tb1->show();
}

void testWindow::slotHide1 ()
{
  tb->show();
}

testWindow::~testWindow ()
{
  //debug ("kwindowtest: deleted clock");

  delete tb;
  delete tb1;
  delete menuBar;

  qDebug ("kwindowtest finished");
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
  statusBar->showMessage("This important message will go away in 15 seconds", 15000);
}

void testWindow::slotExit ()
{
  if (exitB == true)
   {
     tb->removeAction(exitAction);
     exitB = false;
   }
  else
   {
     if (tb->actions().count() >= 7)
      tb->insertAction(tb->actions()[6], exitAction);
     else
      tb->addAction(exitAction);
     exitB = true;
   }
}

void testWindow::slotLined()
{
  lineL = !lineL;
  testLineEdit->setEnabled(lineL); // enable/disable lined
}

void testWindow::slotToggle (bool on)
{
  if (on == true)
    statusBar->changeItem("Toggle is on", 0);
  else
    statusBar->changeItem("Toggle is off", 0);
}

void testWindow::slotFrame()
{
#if 0
  if (greenF == false)
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

     greenF = true;
   }
  else
   {
     tb1->removeItem (10);
     greenF = false;
   }
#endif
}

void testWindow::slotMessage(int, bool boo)
{
  if (boo)
    statusBar->showMessage("This button does this and that", 1500);
  else
    statusBar->clearMessage();
}
// Now few Combo slots, for Torben

void testWindow::slotClearCombo()
{
  testComboBox->clear();
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
  testComboBox->addItems (list);
}

void testWindow::slotMakeItem3Current()
{
  testComboBox->setCurrentIndex(3);
}

int main( int argc, char *argv[] )
{
    int i;
    KCmdLineArgs::init(argc, argv, "kwindowtest", "KWindowTest", "description", "version");

    KApplication myApp;
    testWindow *test = new testWindow;

    myApp.setQuitOnLastWindowClosed( false ); // don't quit after the messagebox!

    i = QMessageBox::information(0, "Select", "Select type of mainwidget",
                                 "Fixed", "Y-fixed", "Resizable");
    if (i == 0)
        test->beFixed();
    else if (i == 1)
        test->beYFixed();

    test->show();
    test->resize(400, 500);
    myApp.setQuitOnLastWindowClosed( true );
    int ret = myApp.exec();

    //delete test;
    return ret;
}

#include "kwindowtest.moc"

