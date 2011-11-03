//krazy:excludeall=license (it's a program, not a library)
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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <kapplication.h>
#include <ktoolinvocation.h>
#include <kauthorized.h>
#include <kmessagebox.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kurl.h>

int main(int argc, char **argv)
{
    KCmdLineOptions options;
    options.add("+url");

    KCmdLineArgs::init(argc, argv, "ktelnetservice", "kdelibs4", qi18n("telnet service"),
               "unknown", qi18n("telnet protocol handler"));
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if (args->count() != 1)
        return 1;

    KConfig config("kdeglobals");
    KConfigGroup cg(&config, "General");
    QString terminal = cg.readPathEntry("TerminalApplication", "konsole");

    KUrl url(args->arg(0));
    QStringList cmd;
    if (terminal == "konsole")
        cmd << "--noclose";

    cmd << "-e";
        if ( url.protocol() == "telnet" )
            cmd << "telnet";
        else if ( url.protocol() == "ssh" )
            cmd << "ssh";
        else if ( url.protocol() == "rlogin" )
            cmd << "rlogin";
        else {
            kError() << "Invalid protocol " << url.protocol() << endl;
            return 2;
        }

        if (!KAuthorized::authorize("shell_access"))
        {
            KMessageBox::sorry(0,
                i18n("You do not have permission to access the %1 protocol.", url.protocol()));
            return 3;
        }

    if (!url.user().isEmpty())
    {
        cmd << "-l";
        cmd << url.user();
    }

        QString host;
        if (!url.host().isEmpty())
           host = url.host(); // telnet://host
        else if (!url.path().isEmpty())
           host = url.path(); // telnet:host

        if (host.isEmpty() || host.startsWith('-'))
        {
            kError() << "Invalid hostname " << host << endl;
            return 2;
        }

        cmd << host;

    if (url.port() > 0){
            if ( url.protocol() == "ssh" )
        cmd << "-p" << QString::number(url.port());
        else
        cmd << QString::number(url.port());
    }

    KToolInvocation::kdeinitExec(terminal, cmd);

    return 0;
}
