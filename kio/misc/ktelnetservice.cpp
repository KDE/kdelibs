/*
   Copyright (c) 2001 Malte Starostik <malte@kde.org>
   based on kmailservice.cpp,
   Copyright (c) 2000 Simon Hausmann <hausmann@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

// $Id$

#include <kapplication.h>
#include <kmessagebox.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <klocale.h>
#include <kprocess.h>

static const KCmdLineOptions options[] =
{
	{"+url", 0, 0},
	KCmdLineLastOption
};

int main(int argc, char **argv)
{
	KLocale::setMainCatalogue("kdelibs");
	KCmdLineArgs::init(argc, argv, "ktelnetservice", I18N_NOOP("telnet service"),
			   I18N_NOOP("telnet protocol handler"), "unknown");
	KCmdLineArgs::addCmdLineOptions(options);

	KApplication app;

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	
	if (args->count() != 1)
		return 1;

	KURL url(args->arg(0));
	QStringList cmd;
	cmd << "--noclose";

	cmd << "-e";
        if ( url.protocol() == "telnet" )
            cmd << "telnet";
        else if ( url.protocol() == "ssh" )
            cmd << "ssh";
        else if ( url.protocol() == "rlogin" )
            cmd << "rlogin";
        else {
            kdError() << "Invalid protocol " << url.protocol() << endl;
            return 2;
        }
        
        if (!app.authorize("shell_access"))
        {
            KMessageBox::sorry(0, 
            	i18n("You do not have permission to access the %1 protocol.").arg(url.protocol()));
            return 3;
        }
        
	if (!url.user().isEmpty())
	{
		cmd << "-l";
		cmd << url.user();
	}

        if (!url.host().isEmpty())
           cmd << url.host(); // telnet://host
        else if (!url.path().isEmpty())
           cmd << url.path(); // telnet:host
        
	if (url.port()){
            if ( url.protocol() == "ssh" )
		cmd << "-p" << QString::number(url.port());
	    else
		cmd << QString::number(url.port());
	}

	app.kdeinitExec("konsole", cmd);

	return 0;
}

// vim: ts=4 sw=4 noet
