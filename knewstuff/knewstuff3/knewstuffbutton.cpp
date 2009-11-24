/*
    knewstuff3/ui/knewstuffbutton.cpp.
    Copyright (c) 2004 Aaron J. Seigo <aseigo@kde.org>

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

#include "knewstuffbutton.h"

#include <kiconloader.h>
#include <klocale.h>
#include <kicon.h>
#include "downloaddialog.h"

namespace KNS3
{
    class Button::Private {
    public:
        QString configFile;
    };

Button::Button(const QString& text,
               const QString& configFile,
               QWidget* parent)
        : KPushButton(parent),
        d(new Private)
{
    setButtonText(text);
    d->configFile = configFile;
    init();
}

Button::Button(QWidget* parent)
        : KPushButton(parent),
        d(new Private)
{
    setButtonText(i18n("Download New Stuff..."));
    init();
}

Button::~Button()
{
    delete d;
}

void Button::init()
{
    setIcon(KIcon("get-hot-new-stuff"));
    connect(this, SIGNAL(clicked()), SLOT(showDialog()));
}

void Button::setButtonText(const QString& what)
{
    setText(what);
}

void Button::setConfigFile(const QString& configFile)
{
    d->configFile = configFile;
}

void Button::showDialog()
{
    emit aboutToShowDialog();

    DownloadDialog dialog(d->configFile, this);
    dialog.exec();

    emit dialogFinished(dialog.changedEntries());
}

}

#include "knewstuffbutton.moc"
