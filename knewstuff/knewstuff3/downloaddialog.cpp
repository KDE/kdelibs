/*
    knewstuff3/ui/downloaddialog.cpp.
    Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>
    Copyright (C) 2005 - 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2007 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2007-2009 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (C) 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

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

#include <QtCore/QTimer>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QScrollBar>
#include <QtGui/QKeyEvent>

#include <kmessagebox.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kpushbutton.h>
#include <ktitlewidget.h>
#include <kdebug.h>

#include "downloadwidget.h"
#include "downloadwidget_p.h"

namespace KNS3 {
class DownloadDialogPrivate
{
public:
    ~DownloadDialogPrivate()
    {
        delete downloadWidget;
    }

    DownloadWidget* downloadWidget;
};
}

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
    // load the last size from config
    KConfigGroup group(KGlobal::config(), ConfigGroup);
    restoreDialogSize(group);
    setMinimumSize(700, 400);

    setCaption(i18n("Get Hot New Stuff"));
    setButtons(KDialog::None);

    d->downloadWidget = new DownloadWidget(configFile ,this);
    setMainWidget(d->downloadWidget);

    d->downloadWidget->d->ui.m_titleWidget->setText(i18nc("Program name followed by 'Add On Installer'",
        "%1 Add-On Installer",
        KGlobal::activeComponent().aboutData()->programName()));
    d->downloadWidget->d->ui.m_titleWidget->setPixmap(KIcon(KGlobal::activeComponent().aboutData()->programIconName()));
    d->downloadWidget->d->ui.m_titleWidget->setVisible(true);
    d->downloadWidget->d->ui.closeButton->setVisible(true);
    d->downloadWidget->d->ui.closeButton->setGuiItem(KStandardGuiItem::Close);
    d->downloadWidget->d->dialogMode = true;
    connect(d->downloadWidget->d->ui.closeButton, SIGNAL(clicked()), this, SLOT(accept()));
}

DownloadDialog::~DownloadDialog()
{
    KConfigGroup group(KGlobal::config(), ConfigGroup);
    saveDialogSize(group, KConfigBase::Persistent);
    delete d;
}

Entry::List DownloadDialog::changedEntries()
{
    return d->downloadWidget->changedEntries();
}

Entry::List DownloadDialog::installedEntries()
{
    return d->downloadWidget->installedEntries();
}


#include "downloaddialog.moc"
