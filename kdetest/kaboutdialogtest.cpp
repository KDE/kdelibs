/* -*- C++ -*-
 * This file shows an example for the KDE about dialog.
 *
 * copyright:  (C) Mirko Sucker, 1999
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko@kde.org>
 * requires:   recent C++-compiler, at least Qt 1.4
 * $Revision$
 */

#include "kaboutdialog.h"
#include <kimgio.h>
#include <qimage.h>
#include <kapp.h>
#include <klocale.h>

int main(int argc, char** argv)
{
  KApplication *app=new KApplication(argc, argv);
  QImage logo;
  QImage bg;
  QPixmap pix;
  KAboutDialog about;
  // -----
  // kimgioRegister();
  if(logo.load("RayTracedGear.png"))
    {
      pix=logo;
      about.setLogo(pix);
    } else {
      debug("main: No logo loaded.");
    }
  if(bg.load("background_1.png"))
    {
      pix=bg;
      about.setBackgroundTile(&pix);
      about.showBaseFrameTile(true);
    } else {
      debug("main: No tile loaded.");
    }
//    about.setButtonOKText
//      (i18n("Cl&ose"), i18n("Close this dialog."),
//       i18n("<b>Close</b> the dialog<br>"
//  	  "<i>(there are no settings to save).</i>"));
  about.setCaption(i18n("KAboutDialog example"));
  about.setVersion(i18n("KAboutDialog, Draft Study"));
  about.setHelp("kdehelp/intro.html", "intro", "");
  // ----- set the application author:
  about.setAuthor
    ("Mirko Sucker", "mirko@kde.org", "", i18n("Initial developer."));
  // ----- set the application maintainer:
  about.setMaintainer("Any One", // name
		      "anyone@kde.org", // email address 
		      "http://www.anyhere.com", // URL
		      i18n("Current maintainer.")); // description
  // ----- add some contributors:
  about.addContributor("Some One", "someone@kde.org", "http://www.somehere.com", 
		       i18n("Making coffee"));
  about.addContributor("Another One", 
		       "anotherone@kde.org", 
		       "http://www.blablax.com", 
		       i18n("Pizza donator"));
  // ----- contents of the dialog have changed, adapt sizes:
  about.adjust(); 
  about.exec();
  // -----
  return 0;
}

