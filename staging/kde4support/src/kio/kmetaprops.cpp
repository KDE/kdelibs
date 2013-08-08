/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Rolf Magnus <ramagnus@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
 */

#include "kmetaprops.h"
#include "kpropertiesdialog_p.h"

#include <kfileitem.h>
#include <kfilemetadatawidget.h>
#include <kfilemetadataconfigurationwidget.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kwindowconfig.h>

#include <QDialog>
#include <QDialogButtonBox>
#include <QtCore/QPointer>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

using namespace KDEPrivate;

class KFileMetaPropsPlugin::KFileMetaPropsPluginPrivate
{
public:
    KFileMetaPropsPluginPrivate();
    ~KFileMetaPropsPluginPrivate();
    void configureShownMetaData();

    KFileMetaDataWidget* m_fileMetaDataWidget;
};

KFileMetaPropsPlugin::KFileMetaPropsPluginPrivate::KFileMetaPropsPluginPrivate() :
    m_fileMetaDataWidget(0)
{
}

KFileMetaPropsPlugin::KFileMetaPropsPluginPrivate::~KFileMetaPropsPluginPrivate()
{
}

void KFileMetaPropsPlugin::KFileMetaPropsPluginPrivate::configureShownMetaData()
{
    QPointer<QDialog> dialog = new QDialog();
    dialog->setWindowTitle(i18nc("@title:window", "Configure Shown Data"));

    QLabel* descriptionLabel  = new QLabel(i18nc("@label::textbox",
                                                 "Select which data should "
                                                 "be shown:"));
    descriptionLabel->setWordWrap(true);

    KFileMetaDataConfigurationWidget* configWidget = new KFileMetaDataConfigurationWidget();
    const KFileItemList items = m_fileMetaDataWidget->items();
    configWidget->setItems(items);

    QDialogButtonBox *buttonBox = new QDialogButtonBox();
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));

    QVBoxLayout* topLayout = new QVBoxLayout;
    topLayout->addWidget(descriptionLabel);
    topLayout->addWidget(configWidget);
    topLayout->addWidget(buttonBox);
    dialog->setLayout(topLayout);

    KConfigGroup dialogConfig(KSharedConfig::openConfig(), "KFileMetaPropsPlugin");
    KWindowConfig::restoreWindowSize(dialog, dialogConfig);

    if ((dialog->exec() == QDialog::Accepted) && (dialog != 0)) {
        configWidget->save();

        // TODO: Check whether a kind of refresh() method might make sense
        // for KFileMetaDataWidget or whether the widget can verify internally
        // whether a change has been done
        m_fileMetaDataWidget->setItems(KFileItemList());
        m_fileMetaDataWidget->setItems(items);
    }

    if (dialog != 0) {
        KWindowConfig::saveWindowSize(dialog, dialogConfig);
        delete dialog;
        dialog = 0;
    }
}

KFileMetaPropsPlugin::KFileMetaPropsPlugin(KPropertiesDialog* props)
  : KPropertiesDialogPlugin(props),d(new KFileMetaPropsPluginPrivate)
{
    d->m_fileMetaDataWidget = new KFileMetaDataWidget();
    d->m_fileMetaDataWidget->setItems(properties->items());

    // Embed the FileMetaDataWidget inside a container that has a dummy widget
    // at the bottom. This prevents that the file meta data widget gets vertically stretched.
    QWidget* metaDataWidgetContainer = new QWidget();
    QVBoxLayout* containerLayout = new QVBoxLayout(metaDataWidgetContainer);
    containerLayout->addWidget(d->m_fileMetaDataWidget);
    QWidget* stretchWidget = new QWidget(metaDataWidgetContainer);
    stretchWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    containerLayout->addWidget(stretchWidget);

    // The height of FileMetaDataWidget can get very large, so it is embedded
    // into a scrollarea
    QScrollArea* metaDataArea = new QScrollArea();
    metaDataArea->setWidget(metaDataWidgetContainer);
    metaDataArea->setWidgetResizable(true);
    metaDataArea->setFrameShape(QFrame::NoFrame);

    // Add label 'Configure...' to be able to adjust which meta data should be shown
    QLabel* configureLabel = new QLabel("<a href=\"configure\">" +
                                        i18nc("@action:button", "Configure...") +
                                        "</a>");
    connect(configureLabel, SIGNAL(linkActivated(QString)),
            this, SLOT(configureShownMetaData()));

    QWidget* mainWidget = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->addWidget(metaDataArea);
    mainLayout->addWidget(configureLabel, 0, Qt::AlignRight);

    properties->addPage(mainWidget, i18nc("@title:tab", "Information"));
}

KFileMetaPropsPlugin::~KFileMetaPropsPlugin()
{
    delete d;
}

bool KFileMetaPropsPlugin::supports( const KFileItemList& _items )
{
    Q_UNUSED(_items);
    return true;
}

void KFileMetaPropsPlugin::applyChanges()
{
}

#include "moc_kmetaprops.cpp"
