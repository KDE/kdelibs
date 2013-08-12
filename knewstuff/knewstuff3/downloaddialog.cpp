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
#include <QSortFilterProxyModel>
#include <QScrollBar>
#include <QKeyEvent>

#include <ksharedconfig.h>
#include <ktitlewidget.h>
#include <kwindowconfig.h>
#include <QCoreApplication>
#include <kstandardguiitem.h>

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
    : QDialog(parent)
    , d(new DownloadDialogPrivate)
{
    const QString name = QCoreApplication::applicationName();
    init(name + ".knsrc");
}

DownloadDialog::DownloadDialog(const QString& configFile, QWidget * parent)
        : QDialog(parent)
        , d(new DownloadDialogPrivate)
{
    init(configFile);
}

void DownloadDialog::init(const QString& configFile)
{
    // load the last size from config
    KConfigGroup group(KSharedConfig::openConfig(), ConfigGroup);
    KWindowConfig::restoreWindowSize(windowHandle(), group);
    setMinimumSize(700, 400);

    setWindowTitle(i18n("Get Hot New Stuff"));

    QVBoxLayout *layout = new QVBoxLayout;
    d->downloadWidget = new DownloadWidget(configFile ,this);
    layout->addWidget(d->downloadWidget);
    setLayout(layout);

    QString displayName = QGuiApplication::applicationDisplayName();
    if (displayName.isEmpty())
        displayName = QCoreApplication::applicationName();
    d->downloadWidget->d->ui.m_titleWidget->setText(i18nc("Program name followed by 'Add On Installer'",
                                                          "%1 Add-On Installer", displayName));
    //d->downloadWidget->d->ui.m_titleWidget->setPixmap(QIcon::fromTheme(KGlobal::activeComponent().aboutData()->programIconName()));
    d->downloadWidget->d->ui.m_titleWidget->setVisible(true);
    d->downloadWidget->d->ui.closeButton->setVisible(true);
    KStandardGuiItem::assign(d->downloadWidget->d->ui.closeButton, KStandardGuiItem::Close);
    d->downloadWidget->d->dialogMode = true;
    connect(d->downloadWidget->d->ui.closeButton, SIGNAL(clicked()), this, SLOT(accept()));
}

DownloadDialog::~DownloadDialog()
{
    KConfigGroup group(KSharedConfig::openConfig(), ConfigGroup);
    KWindowConfig::saveWindowSize(windowHandle(), group, KConfigBase::Persistent);
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


