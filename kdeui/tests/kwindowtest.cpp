#include <QtGui/QMessageBox>
#include <QtGui/QTextEdit>
#include <QtCore/QDir>

#include <stdlib.h>

#include <kstatusbar.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <khelpmenu.h>
#include <kcmdlineargs.h>
#include <kmenu.h>
#include "kwindowtest.h"
#include <klineedit.h>
#include <kicon.h>
#include <kstandardaction.h>
#include <kstandarddirs.h>
#include <kxmlguifactory.h>
#include <kactionmenu.h>
#include <kactioncollection.h>
#include <ktoggleaction.h>
#include <kdebug.h>

TestWindow::TestWindow (QWidget *parent)
    : KXmlGuiWindow (parent)
{
    ena=false;
    exitB = true;   // exit button is shown
    lineL = true;   // LineEdit is enabled
    greenF = false;  // Frame not inserted
    timer = 0;

    setCaption("test window");

    // The xmlgui file defines the layout of the menus and toolbars.
    // We only need to create actions with the right name here.

    // First four  buttons
    fileNewAction = new KAction(KIcon("document-new"), "Create.. (toggles upper button)", this);
    actionCollection()->addAction("filenew", fileNewAction);
    fileNewAction->setCheckable(true);
    connect(fileNewAction, SIGNAL(triggered(bool)), SLOT(slotNew()));

    KAction* fileOpenAction = new KAction(KIcon("document-open"), "Open", this);
    actionCollection()->addAction("fileopen", fileOpenAction);
    connect(fileOpenAction, SIGNAL(triggered(bool)), SLOT(slotOpen()));

    KActionMenu* fileFloppyAction = new KActionMenu(KIcon("filefloppy"), "Save (beep or delayed popup)", this);
    actionCollection()->addAction("filefloppy", fileFloppyAction);
    fileFloppyAction->setDelayed(true);
    connect(fileFloppyAction, SIGNAL(triggered(bool)), SLOT(slotSave()));

    KAction* filePrintAction = new KAction(KIcon("document-print"), "Print (enables/disables open)", this);
    actionCollection()->addAction("fileprint", filePrintAction);
    filePrintAction->setToolTip("This tooltip does not work for menu items");
    filePrintAction->setWhatsThis("This is the longer explanation of the action");
    filePrintAction->setStatusTip("This action is supposed to print, but in fact enables/disables open");
    connect(filePrintAction, SIGNAL(triggered(bool)), SLOT(slotPrint()));

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
    exitAction = new KAction(KIcon( "application-exit" ), "Exit", this);
    actionCollection()->addAction("exit", exitAction);
    connect (exitAction, SIGNAL(triggered(bool)), KApplication::kApplication(), SLOT( quit() ));

    // Another toolbar

    KAction* fileNewAction2 = new KAction(KIcon( "document-new" ), "Create new file2 (Toggle)", this);
    actionCollection()->addAction("filenew2", fileNewAction2);
    connect(fileNewAction2, SIGNAL(toggled(bool)), this, SLOT(slotToggle(bool)));
    fileNewAction2->setToolTip("Tooltip");
    fileNewAction2->setStatusTip("Statustip");
    fileNewAction2->setWhatsThis("WhatsThis");

    KAction* fileOpenAction2 = new KAction(KIcon( "document-open" ), "Open (starts progress in sb)", this);
    actionCollection()->addAction("fileopen2", fileOpenAction2);
    connect(fileOpenAction2, SIGNAL(triggered(bool)), SLOT(slotOpen()));
    fileOpenAction2->setHelpText("This action starts a progressbar inside the statusbar");

    KAction* fileFloppyAction2 = new KAction(KIcon( "filefloppy" ), "Save file2 (autorepeat)", this);
    actionCollection()->addAction("filefloppy2", fileFloppyAction2);
    connect(fileFloppyAction2, SIGNAL(triggered(bool)), this, SLOT(slotSave()));

    itemsMenu = new QMenu;
    itemsMenu->addAction ("delete/insert exit button", this, SLOT(slotExit()));
    itemsMenu->addAction ("enable/disable lineedit", this, SLOT(slotLined()));
    itemsMenu->addAction ("Toggle fileNew", this, SLOT(slotNew()));
    itemsMenu->addAction ("Combo: clear", this, SLOT(slotClearCombo()));
    itemsMenu->addAction ("Combo: insert list", this, SLOT(slotInsertListInCombo()));
    itemsMenu->addAction ("Combo: make item 3 current", this, SLOT(slotMakeItem3Current()));
    itemsMenu->addAction ("Important msg in statusbar", this, SLOT(slotImportant()));


    KAction* filePrintAction2 = new KAction(KIcon( "document-print" ), "Print (pops menu)", this);
    actionCollection()->addAction("fileprint2", filePrintAction2);
    filePrintAction2->setMenu(itemsMenu);


    // *** RADIO BUTTONS
    QActionGroup* radioGroup = new QActionGroup(this);
    radioGroup->setExclusive(true);

    KToggleAction *radioButton1 = new KToggleAction(KIcon( "document-new" ), "Radiobutton1", this);
    actionCollection()->addAction("radioButton1", radioButton1);
    radioButton1->setActionGroup(radioGroup);

    KToggleAction *radioButton2 = new KToggleAction(KIcon( "document-open" ), "Radiobutton2", this);
    actionCollection()->addAction("radioButton2", radioButton2);
    radioButton2->setActionGroup(radioGroup);

    KToggleAction *radioButton3 = new KToggleAction(KIcon( "filefloppy" ), "Radiobutton3", this);
    actionCollection()->addAction("radioButton3", radioButton3);
    radioButton3->setActionGroup(radioGroup);

    KToggleAction *radioButton4 = new KToggleAction(KIcon( "document-print" ), "Radiobutton4", this);
    actionCollection()->addAction("radioButton4", radioButton4);
    radioButton4->setActionGroup(radioGroup);

    connect (radioGroup, SIGNAL(triggered(QAction*)), this, SLOT(slotToggled(QAction*)));



    /**************************************************/
    /*Now, we setup statusbar; order is not important. */
    /**************************************************/
    statusBar = new KStatusBar (this);
    //statusBar->insertItem("Hi there!                         ", 0);
    //statusBar->insertItem("Look for tooltips to see functions", 1);
    setStatusBar(statusBar);

    //DigitalClock *clk = new DigitalClock (statusBar);
    //clk->setFrameStyle(QFrame::NoFrame);
    //statusBar->insertWidget(clk, 70, 2);


    // Set main widget. In this example it is Qt's multiline text editor.
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

    // KXMLGUIClient looks in the "data" resource for the .rc files
    // This line is for test programs only!
    KGlobal::dirs()->addResourceDir( "data", KDESRCDIR );
    setupGUI( QSize(400, 500), Default, "kwindowtest.rc" );

    tb=toolBar();
    tb1=toolBar("AnotherToolBar");

}
/***********************************/
/*  Now slots for toolbar actions  */
/***********************************/
void TestWindow::slotToggled(QAction*)
{
  statusBar->showMessage ("Button toggled", 1500);
}

void TestWindow::slotInsertClock()
{
  //DigitalClock *clock = new DigitalClock(tb1);
  //clock->setFrameStyle(QFrame::NoFrame);
  //tb1->insertWidget(8, 70, clock);
}

void TestWindow::slotNew()
{
    //tb1->actions()[0]->toggle();
    //toolBar()->removeAction( fileNewAction );
}
void TestWindow::slotOpen()
{
  if (pr == 0) {
    pr = new QProgressBar (statusBar);
    pr->show();
  }
//  statusBar->message(pr);
  if (!timer) {
      timer = new QTimer (this);
      connect (timer, SIGNAL(timeout()), this, SLOT(slotGoGoGoo()));
  }
  timer->start(100);
}

void TestWindow::slotGoGoGoo()
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

void TestWindow::slotSave()
{
  kapp->beep();
  statusBar->changeItem("Saving properties...", 0);
}

void TestWindow::slotPrint()
{
    statusBar->changeItem("Print file pressed", 0);
    ena=!ena;
    qobject_cast<KAction*>(sender())->setEnabled(ena);
}
void TestWindow::slotReturn()
{
    QString s = "You entered ";
    s = s + testLineEdit->text();
    statusBar->changeItem(s, 0);

}
void TestWindow::slotList(const QString &str)
{
    QString s = "You chose ";
    s = s + str;
    statusBar->changeItem(s, 0);
}

void TestWindow::slotCompletion()
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

void TestWindow::slotListCompletion()
{
    /*
     Combo is not behaving good and it is ugly. I will see how it behaves in Qt-1.3,
     and then decide should I make a new combobox.
     */
  QString s(testComboBox->currentText());  // get text in combo
  s+= "(completing)";
  //tb->getCombo(4)->changeItem(s.data());   // setTextIncombo

}

void TestWindow::slotCompletionsMenu(QAction* action)
{
  // Now set text in lined
  QString s =action->text();
  testLineEdit->setText(s);  // Cursor is automatically at the end of string after this
}

TestWindow::~TestWindow ()
{
    kDebug() << "kwindowtest finished";
}

void TestWindow::beFixed()
{
    widget->setFixedSize (400, 200);
}

void TestWindow::beYFixed()
{
    widget->setMinimumSize(400, 200);
    widget->setMaximumSize(9999, 200);
}

void TestWindow::slotImportant ()
{
  statusBar->showMessage("This important message will go away in 15 seconds", 15000);
}

void TestWindow::slotExit ()
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

void TestWindow::slotLined()
{
  lineL = !lineL;
  testLineEdit->setEnabled(lineL); // enable/disable lined
}

void TestWindow::slotToggle (bool on)
{
  if (on == true)
    statusBar->changeItem("Toggle is on", 0);
  else
    statusBar->changeItem("Toggle is off", 0);
}

void TestWindow::slotFrame()
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

void TestWindow::slotMessage(int, bool boo)
{
  if (boo)
    statusBar->showMessage("This button does this and that", 1500);
  else
    statusBar->clearMessage();
}
// Now few Combo slots, for Torben

void TestWindow::slotClearCombo()
{
  testComboBox->clear();
}

void TestWindow::slotInsertListInCombo()
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

void TestWindow::slotMakeItem3Current()
{
  testComboBox->setCurrentIndex(3);
}

int main( int argc, char *argv[] )
{
    KCmdLineArgs::init(argc, argv, "kwindowtest", 0, ki18n("KWindowTest"), "version", ki18n("description"));

    KApplication myApp;
    TestWindow *test = new TestWindow;

    myApp.setQuitOnLastWindowClosed( false ); // don't quit after the messagebox!

#if 0
    int i = QMessageBox::information(0, "Select", "Select type of mainwidget",
                                 "Fixed", "Y-fixed", "Resizable");
    if (i == 0)
        test->beFixed();
    else if (i == 1)
        test->beYFixed();
#endif

    test->show();
    myApp.setQuitOnLastWindowClosed( true );
    int ret = myApp.exec();

    //delete test;
    return ret;
}

#include "kwindowtest.moc"

