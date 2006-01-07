#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kdialogbase.h>

#include <QTextBrowser>
#include <QLabel>

int main(int argc, char** argv)
{
  KAboutData about("DialogTest", "DialogTest", "version");
  KCmdLineArgs::init(argc, argv, &about);

  KApplication app;

  // -----
  QString text= // the explanation shown by the example dialog
    "<center><h1>KDialog Example</h1></center><hr><br>"
    "This example shows the usage of the <i>KDialog</i>  class. "
    "<i>KDialog</i> is the KDE user interface class used to create "
    "dialogs with simple layout without having to define an own dialog "
    "style for your application. <br>"
    "It provides some standards buttons that are needed in most dialogs. Each one may be "
    "hidden, enabled or disabled, and tooltips and quickhelp texts might be"
    " added. And you do not need to bother about geometry management, this "
    "is all done automatically.<br>"
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
  KDialog dialog;
  dialog.setButtonMask( KDialog::Ok | KDialog::Cancel | KDialog::Details | KDialog::User1 | KDialog::Help , KGuiItem("Test") );
  dialog.enableButtonSeparator(true);
  dialog.setCaption("dialog!");
  /* Set a help chapter. If you do not set one, the link is not shown, and the
     upper part of the frame shrinks as much as possible. The help window "
     "will of course only pop up if you correctly installed kdebase. */
  // I disabled it, as khcclient did not run for me.
   dialog.setHelp("kdehelp/intro.html", "");
  /* This QTextView is intended to be the main widget of our dialog. The
     main widget is placed inside the dialogs frame, with the buttons below
     it. You do not have to take care about the size handling, but it is a
     good idea to set the main wigdets minimum size, since the sizes Qt and
     the DialogBase class guess are sometimes ugly.

     It is important that your main widget is created with the dialog object
     as its parent! */
  QTextBrowser view( &dialog);
 view.setHtml( text );

  QLabel label("this is a place for some advanced settings" ,&dialog);
  dialog.setDetailsWidget( &label);
	
  //view.setMinimumSize(400, view.heightForWidth(400)+20);
  view.setMinimumSize( 250, 300 );
  dialog.setMainWidget(&view);
  /* After finishing the setup of your main widget, the dialog needs to be
     adjusted. It is not done automatically, since the layout of the main
     widget may change before the dialog is shown. Additionally, setting a
     help chapter may cause a need for adjustment since it modifies the height
     of the upper frame. */
 // dialog.resize(dialog.minimumSize());
  /* The dialog object is used just as any other QDialog: */
  if(dialog.exec())
    {
      qDebug("Accepted.");
    } else {
      qDebug("Rejected.");
    }
  return 0;
}

