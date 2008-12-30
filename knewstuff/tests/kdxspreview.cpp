/*
    This file is part of KNewStuff2.
    Copyright (c) 2005, 2006 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kdxsview.h"

#include <kapplication.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kcmdlineargs.h>

int main(int argc, char **argv)
{
    KAboutData about("kdxspreview",
                     0,
                     ki18n("KDXS Preview"),
                     "0.1",
                     ki18n("KNewstuff2 DXS Preview"),
                     KAboutData::License_GPL,
                     ki18n("(C) 2005, 2006 Josef Spillner"),
                     KLocalizedString(),
                     0,
                     "spillner@kde.org");

    KCmdLineArgs::init(argc, argv, &about);
    KApplication app;

    KNS::KDXSView view;
    return view.exec();
}
