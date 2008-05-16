/**
 * KDE Rich Text Editor
 *
 * Copyright 2008 Stephen Kelly <steveire@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#include "krichtexteditor.h"
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

int main(int argc, char **argv)
{
    const QByteArray& ba = QByteArray("krichtexteditor");
    const KLocalizedString name = ki18n("KDE Rich Text Editor");
    KAboutData aboutData(ba, ba, name, ba, name);
    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;
    KRichTextEditor* mw = new KRichTextEditor();
    mw->show();
    app.exec();
}
