/*
        Copyright (C) 2001 Jeff Tranter
                           tranter@kde.org
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


------------------------------------------------------------------------

This application displays an error, warning, or informational message
in a dialog. It is normally used by artsd in conjunction with the -m
option.  By abstracting this out of artsd, we keep it independent of
any particular graphics toolkit.

This version uses KDE. Equivalent versions could be written using Qt,
Gnome, etc. and used instead.

*/

#include <klocale.h>
#include <kglobal.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <kmessagebox.h>
#include <kcmdlineargs.h>

// command line options
static KCmdLineOptions options[] = 
  {
	  { "e", 0,0 },
	  { "error", I18N_NOOP("Display error message (default)"), 0 },
	  { "w", 0, 0},
	  { "warning", I18N_NOOP("Display warning message"), 0 },
	  { "i", 0, 0 },
	  { "info", I18N_NOOP("Display informational message"), 0 },
	  { "+message", I18N_NOOP("Message string to be displayed"), 0 },
	  { 0, 0, 0 } // End of options.
  };

KAboutData aboutData("artsmessage", I18N_NOOP("artsmessage"), "0.1",
					 I18N_NOOP("Utility to display aRts error messages."),
					 KAboutData::License_GPL, "(c) 2001, Jeff Tranter", 0, 0, "tranter@kde.org");

int main(int argc, char **argv) {
	aboutData.addAuthor("Jeff Tranter", 0, "tranter@kde.org");
	KGlobal::locale()->setMainCatalogue("kdelibs");
	KCmdLineArgs::init(argc, argv, &aboutData);
	KCmdLineArgs::addCmdLineOptions(options);
	KApplication app;
	
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	QString msg;

	// must be at least one argument
	if (args->count() == 0) {
		args->usage();
	}

	// build up message string from remaining arguments
	for (int i = 0; i < args->count(); i++) {
		if (i == 0)
			msg = args->arg(i);
		else
			msg += QString(" ") + args->arg(i);
	}

	if (args->isSet("w")) {
		KMessageBox::sorry(0, msg, i18n("Warning"));
	} else if (args->isSet("i")) {
		KMessageBox::information(0, msg, i18n("Informational"), "ShowInfoMessages");
	} else {
		KMessageBox::error(0, msg, i18n("Error"));
	}
	
	return 0;
}
