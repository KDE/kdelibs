#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kdialogbase.h>

#include <qstring.h>
#include <q3textview.h>

int main(int argc, char** argv)
{
  KAboutData about("DialogBaseTest", "DialogBaseTest", "version");
  KCmdLineArgs::init(argc, argv, &about);

  KApplication app;

  // -----
  QString text= // the explanation shown by the example dialog
    "<center><h1>DialogBase Example</h1></center><hr><br>"
    "This example shows the usage of the <i>DialogBase</i>  class. "
    "<i>DialogBase</i> is the KDE user interface class used to create "
    "dialogs with unique layout without having to define an own dialog "
    "style for your application. <br>"
    "It provides three standard buttons (<b>OK</b>, <b>Apply</b>, and "
    "<b>Cancel</b>) that are needed in most dialogs. Each one may be "
    "hidden, enabled or disabled, and tooltips and quickhelp texts might be"
    " added. And you do not need to bother about geometry management, this "
    "is all done automatically.<br>"
    "To polish your user interface even further, you might want to add "
    "textures to the inner and the outer frame of the dialog (the frame is "
    "created by the dialog object). This is done "
    "using the <tt>setMainFrameTile</tt> and the <tt>setBaseFrameTile</tt> "
    "methods. These tiles are added application-wide, so each dialog "
    "of you application uses the same tiles. This is a tribute to a "
    "comprehensable user interface.<br>"
    "The class supports the creation of dialogs without being forced "
    "to derive an own class for it, but you may derive your own class "
    "for a better code structure.<br>"
    "If you wrote a help chapter explaining what your dialog does, you "
    "should add a link to it to the dialog using <tt>setHelp</tt>. You do "
    "not have to take care about launching the help viewer, just set the "
    "help file and topic and of course copy it to your documentation "
    "directory during the program installation.";
  /* Create the dialog object. DialogBase is derived from QDialog, but
     you do not need to derive it to create a nice-looking dialog. Mostly,
     you already have a widget class representing the core of your dialog,
     and you only need to add a frame around it and the default buttons.

     If you want to derive it, you still can, moving all code shown here
     inside of your new class. */
  KDialogBase dialog;
  /* Set a help chapter. If you do not set one, the link is not shown, and the
     upper part of the frame shrinks as much as possible. The help window "
     "will of course only pop up if you correctly installed kdebase. */
  // I disabled it, as khcclient did not run for me.
  // dialog.setHelp("kdehelp/intro.html", "", "");
  /* This QTextView is intended to be the main widget of our dialog. The
     main widget is placed inside the dialogs frame, with the buttons below
     it. You do not have to take care about the size handling, but it is a
     good idea to set the main wigdets minimum size, since the sizes Qt and
     the DialogBase class guess are sometimes ugly.

     It is important that your main widget is created with the dialog object
     as its parent! */
  Q3TextView view(text, QString(), &dialog);
  //view.setMinimumSize(400, view.heightForWidth(400)+20);
  view.setMinimumSize( 250, 300 );
  dialog.setMainWidget(&view);
  /* After finishing the setup of your main widget, the dialog needs to be
     adjusted. It is not done automatically, since the layout of the main
     widget may change before the dialog is shown. Additionally, setting a
     help chapter may cause a need for adjustment since it modifies the height
     of the upper frame. */
  dialog.resize(dialog.minimumSize());
  /* The dialog object is used just as any other QDialog: */
  if(dialog.exec())
    {
      qDebug("Accepted.");
    } else {
      qDebug("Rejected.");
    }
  return 0;
}

