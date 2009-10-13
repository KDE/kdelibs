/*
    Copyright (c) 2009 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kdebug.h>
#include <browseropenorsavequestion.h>
#include <kapplication.h>
#include <kcmdlineargs.h>

using namespace KParts;

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "openorsavequestion", QByteArray(), ki18n("Test for the 'open or save' question"), "0");
    KApplication app;

    BrowserOpenOrSaveQuestion questionOpen(KUrl("foo.pdf"),
                                           QString::fromLatin1("application/pdf"),
                                           QString::fromLatin1("file.pdf"));
    BrowserRun::AskSaveResult res = questionOpen.askOpenOrSave(0);
    kDebug() << res;
    if (questionOpen.selectedService())
        kDebug() << "Selected service:" << questionOpen.selectedService()->entryPath();

    return 0;
}
