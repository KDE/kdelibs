#include <qpopmenu.h>
#include <qpixmap.h>
#include <qkeycode.h>
#include <qwidget.h>
#include <qstring.h>

#include <stdlib.h>

#include <kstatusbar.h>
#include <kapp.h>
#include <ktopwidget.h>
#include <kmenubar.h>

#include "ktoolbartest.moc"

#include "ktoolbar.h"

testWindow::testWindow (QWidget *, const char *name)
    : KTopLevelWidget (name)
{
    menuBar = new KMenuBar (this);
    fileMenu = new QPopupMenu;
    menuBar->insertItem ("&File", fileMenu);
    fileMenu->insertItem ("&Exit", KApplication::getKApplication(),
                          SLOT( quit() ), ALT + Key_Q );

    toolBarMenu = new QPopupMenu;
    menuBar->insertItem ("&Toolbars", toolBarMenu);
    
    toolBarMenu->insertItem ("(Un)Hide tollbar 1", this, SLOT(slotHide1()));
    toolBarMenu->insertItem ("(Un)Hide tollbar 2", this, SLOT(slotHide2()));

    

    statusBar = new KStatusBar (this);
    statusBar->insertItem("(ctrl-d for completions in LineEdit) ", 0);

    QPixmap pix;
    QString dir= KApplication::getKApplication()->kdedir()
        + QString("/lib/pics/toolbar/");

    // Create toolbar
    toolBar = new KToolBar(this);

    // set full width

    toolBar->setFullWidth(TRUE);


    
    // First three buttons
    pix.load(dir+"filenew.xpm");
    toolBar->insertButton(pix, 0, SIGNAL(clicked()), this, SLOT(slotNew()),
                         TRUE, "Create new file");
    pix.load(dir+"fileopen.xpm");
    toolBar->insertButton(pix, 1, SIGNAL(clicked()), this, SLOT(slotOpen()),
                         TRUE, "Open existing file");
    pix.load(dir+"filefloppy.xpm");
    toolBar->insertButton(pix, 2, SIGNAL(clicked()), this, SLOT(slotSave()),
                         TRUE, "Save file");
    pix.load(dir+"fileprint.xpm");
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


    // Now add another button and align it right
    pix.load(dir+"exit.xpm");
    toolBar->insertButton(pix, 6, SIGNAL(clicked()), KApplication::getKApplication(),
                          SLOT( quit() ), TRUE, "Exit");
    toolBar->alignItemRight (6);

    // Another toolbar
    tb1 = new KToolBar (this); // this one is normal and has separators


    pix.load(dir+"filenew.xpm");
    tb1->insertButton(pix, 0, SIGNAL(clicked()), this, SLOT(slotNew()),
                         TRUE, "Create new file2");

    pix.load(dir+"fileopen.xpm");
    tb1->insertButton(pix, 1, SIGNAL(clicked()), this, SLOT(slotOpen()),
                          TRUE, "Open existing file2");

    tb1->insertSeparator ();
    
    pix.load(dir+"filefloppy.xpm");
    tb1->insertButton(pix, 2, SIGNAL(clicked()), this, SLOT(slotSave()),
                         TRUE, "Save file2");
    pix.load(dir+"fileprint.xpm");
    tb1->insertButton(pix, 3, SIGNAL(clicked()), this, SLOT(slotPrint()),
                         TRUE, "Print file2");
    
    tb1->insertSeparator ();

    pix.load(dir+"filenew.xpm");
    tb1->insertButton(pix, 4, SIGNAL(clicked()), this, SLOT(slotNew()),
                         TRUE, "Create new file2");

    pix.load(dir+"fileopen.xpm");
    tb1->insertButton(pix, 5, SIGNAL(clicked()), this, SLOT(slotOpen()),
                          TRUE, "Open existing file2");

    tb1->insertSeparator ();
    
    pix.load(dir+"filefloppy.xpm");
    tb1->insertButton(pix, 6, SIGNAL(clicked()), this, SLOT(slotSave()),
                         TRUE, "Save file2");
    pix.load(dir+"fileprint.xpm");
    tb1->insertButton(pix, 7, SIGNAL(clicked()), this, SLOT(slotPrint()),
                         TRUE, "Print file2");

    // Set caption for floating toolbars
    toolBar->setTitle ("Toolbar 1");
    tb1->setTitle ("Toolbar 2");
    
    widget = new QMultiLineEdit (this);
    setMenu (menuBar);

    toolbar2 = addToolBar (tb1);
    toolbar1 = addToolBar (toolBar);

    // Enable floating. Floating is disabled by default
    toolBar->enableFloating(TRUE);
    tb1->enableFloating(TRUE);
    
    enableToolBar(KToolBar::Toggle, toolbar1);
    enableToolBar(KToolBar::Toggle, toolbar2);

    setStatusBar (statusBar);
    setView (widget);

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

void testWindow::slotNew()
{
    statusBar->changeItem("New file pressed", 0);
}
void testWindow::slotOpen()
{
    statusBar->changeItem("Open file pressed", 0);

}
void testWindow::slotSave()
{
    statusBar->changeItem("Save file pressed", 0);

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
   when ctrl-d is pressed. KToolBar will also put cursor at end of text in Lined
   after inserting text with setLinedText (...).
  */

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




int main( int argc, char *argv[] )
{
        KApplication *myApp = new KApplication( argc, argv );
        testWindow *test = new testWindow;

        myApp->setMainWidget(test);
        test->show();
        test->resize(test->width(), test->height()); // I really really really dunno why it doesn't show
        int ret = myApp->exec();
/*
        delete test;      // Don't do this before you have restored all toolbars from floating
        delete myApp;     // I cannot get KFloater to notify it's destruction properly
*/
        return ret;
} 






