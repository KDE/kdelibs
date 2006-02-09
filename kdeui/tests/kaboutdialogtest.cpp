/* -*- C++ -*-
 * This file shows an example for the KDE about dialog.
 *
 * copyright:  (C) Mirko Boehm, 1999
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Boehm <mirko@kde.org>
 * requires:   recent C++-compiler, at least Qt 1.4
 */

#include "kaboutdialog.h"
// #include <kimgio.h>
#include <qimage.h>
//Added by qt3to4:
#include <QPixmap>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kaboutapplication.h>
#include <kapplication.h>
#include <klocale.h>

int main(int argc, char** argv)
{
  KAboutData aboutData("KAboutDialogTest", "KAboutDialogTest", "version", "KAboutDialog Test, Enjoy!", KAboutData::License_GPL, I18N_NOOP("(c) 1997-2006, The KDE Developers"));
  aboutData.addAuthor("Bob Dobbs", I18N_NOOP("Current Maintainer"), "bob@subgenius.com");
  aboutData.addAuthor("Gregory S. Hayes", I18N_NOOP("Developer"), "syncomm@gmail.com");
  aboutData.addAuthor("Random Authors from KFind");
  aboutData.addAuthor("Beppe Grimaldi", I18N_NOOP("UI Design & more search options"), "grimalkin@ciaoweb.it");
  aboutData.addAuthor("Martin Hartig");
  aboutData.addAuthor("Stephan Kulow", 0, "coolo@kde.org");
  aboutData.addAuthor("Mario Weilguni",0, "mweilguni@sime.com");
  aboutData.addAuthor("Alex Zepeda",0, "zipzippy@sonic.net");
  aboutData.addAuthor("Miroslav Fl�r",0, "flidr@kky.zcu.cz");
  aboutData.addAuthor("Harri Porten",0, "porten@kde.org");
  aboutData.addAuthor("Dima Rogozin",0, "dima@mercury.co.il");
  aboutData.addAuthor("Carsten Pfeiffer",0, "pfeiffer@kde.org");
  aboutData.addAuthor("Hans Petter Bieker", 0, "bieker@kde.org");
  aboutData.addAuthor("Waldo Bastian", I18N_NOOP("UI Design"), "bastian@kde.org");
  aboutData.addAuthor("Alexander Neundorf", 0, "neundorf@kde.org");
  aboutData.addAuthor("Clarence Dang", 0, "dang@kde.org");

  KCmdLineArgs::init(argc, argv, &aboutData);

  KApplication app;

  QImage logo;
  QImage bg;
  QPixmap pix;
  KAboutDialog about;
  // -----
  // kimgioRegister();
  if(logo.load("RayTracedGear.png"))
    {
      aboutData.setProgramLogo( logo );
      pix=logo;
      about.setLogo(pix);
    } else {
      qDebug("main: No logo loaded.");
    }
  if(bg.load("background_1.png"))
    {
      pix=bg;
      //about.setBackgroundTile(&pix);
      //about.showTile( true );
      about.enableLinkedHelp( true );

      //about.showBaseFrameTile(true);
    } else {
      qDebug("main: No tile loaded.");
    }
//    about.setButtonOKText
//      ("Cl&ose", "Close this dialog.",
//       "<b>Close</b> the dialog<br>"
//  	  "<i>(there are no settings to save).</i>");
  about.setCaption("KAboutDialog example");
  about.setVersion("KAboutDialog, Draft Study");
  about.setHelp("kdehelp/intro.html", "intro");
  // ----- set the application author:
  about.setAuthor
    ("Mirko Boehm", "mirko@kde.org", "", "Initial developer.");
  // ----- set the application maintainer:
  about.setMaintainer("Any One", // name
		      "anyone@kde.org", // email address
		      "http://www.anyhere.com", // URL
		      "Current maintainer."); // description
  // ----- add some contributors:
  about.addContributor("Some One", "someone@kde.org", "http://www.somehere.com",
		       "Making coffee");
  about.addContributor("Another One",
		       "anotherone@kde.org",
		       "http://www.blablax.com",
		       "Pizza donator");
  // ----- contents of the dialog have changed, adapt sizes:
  about.adjust();
  about.exec();
  // -----
  KAboutApplication aboutApp(&aboutData, app.activeWindow(), true);
  aboutApp.exec();
  return 0;
}

