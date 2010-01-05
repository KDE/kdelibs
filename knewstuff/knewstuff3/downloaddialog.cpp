/*
    knewstuff3/ui/downloaddialog.cpp.
    Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>
    Copyright (C) 2005 - 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2007 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2007-2009 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (C) 2009 Frederik Gladhorn <gladhorn@kde.org>

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

#include "downloaddialog.h"
#include "downloaddialog_p.h"


#include <QtGui/QSortFilterProxyModel>
#include <QtCore/QTimer>
#include <QtGui/QScrollBar>

#include <kmessagebox.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <ktitlewidget.h>
#include <kdebug.h>

#include "ui/itemsmodel.h"
#include "ui/itemsviewdelegate.h"

#include "ui_downloaddialog.h"


using namespace KNS3;

const char * ConfigGroup = "DownloadDialog Settings";


DownloadDialog::DownloadDialog(QWidget* parent)
    : KDialog(parent)
    , d(new DownloadDialogPrivate)
{
    KComponentData component = KGlobal::activeComponent();
    QString name = component.componentName();
    init(name + ".knsrc");
}

DownloadDialog::DownloadDialog(const QString& configFile, QWidget * parent)
        : KDialog(parent)
        , d(new DownloadDialogPrivate)
{
    init(configFile);
}


void DownloadDialog::init(const QString& configFile)
{
    setButtons(KDialog::None);
    QWidget* _mainWidget = new QWidget(this);
    setMainWidget(_mainWidget);
    d->ui.setupUi(_mainWidget);

    d->ui.closeButton->setGuiItem(KStandardGuiItem::Close);
    connect(d->ui.closeButton, SIGNAL(clicked()), SLOT(accept()));

    d->init(configFile);

    // load the last size from config
    KConfigGroup group(KGlobal::config(), ConfigGroup);
    restoreDialogSize(group);
    setMinimumSize(700, 400);

    setCaption(i18n("Get Hot New Stuff"));
}

DownloadDialog::~DownloadDialog()
{
    KConfigGroup group(KGlobal::config(), ConfigGroup);
    saveDialogSize(group, KConfigBase::Persistent);
    delete d;
}

Entry::List DownloadDialog::changedEntries()
{
    Entry::List entries;
    foreach (const EntryInternal &e, d->changedEntries) {
        entries.append(e.toEntry());
    }
    return entries;
}

Entry::List DownloadDialog::installedEntries()
{
    Entry::List entries;
    foreach (const EntryInternal &e, d->changedEntries) {
        if (e.status() == EntryInternal::Installed) {
            entries.append(e.toEntry());
        }
    }
    return entries;
}

#include "downloaddialog.moc"
