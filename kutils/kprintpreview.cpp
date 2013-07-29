/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2007 Alex Merry <alex.merry@kdemail.net>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kprintpreview.h"

#include <QDialogButtonBox>
#include <QtCore/QFile>
#include <QLabel>
#include <QPrinter>
#include <QShowEvent>
#include <QVBoxLayout>
#include <qtemporarydir.h>

#include <klocalizedstring.h>
#include <kmimetypetrader.h>
#include <kparts/part.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kservice.h>
#include <QDebug>


class KPrintPreviewPrivate
{
public:
    KPrintPreviewPrivate(KPrintPreview *host, QPrinter * _printer)
        : q(host)
        , printer(_printer)
        , mainWidget(new QWidget(host))
        , previewPart(0)
        , failMessage(0)
    {
        mainWidget->setLayout(new QVBoxLayout);

        if ( tempdir.isValid() ) {
            filename = tempdir.path() + '/' + "print_preview.pdf";
        } else {
            // XXX: not portable!
            qWarning() << "Failed to create temporary directory";
            filename = "/dev/null";
        }
    }

    void getPart();
    bool doPreview();
    void fail();
    void setCentralWidget(QWidget *widget);

    KPrintPreview *q;

    QPrinter *printer;
    QWidget *mainWidget;

    QTemporaryDir tempdir;
    QString filename;

    KParts::ReadOnlyPart *previewPart;
    QWidget *failMessage;
};

void KPrintPreviewPrivate::getPart()
{
    if (previewPart) {
        // qDebug() << "already got a part";
        return;
    }
    // qDebug() << "querying trader for application/pdf service";

    KPluginFactory *factory(0);
    const KService::List offers =
        KMimeTypeTrader::self()->query("application/pdf", "KParts/ReadOnlyPart");

    KService::List::ConstIterator it = offers.begin();
    while (!factory && it != offers.end()) {
        KPluginLoader loader(**it);
        factory = loader.factory();
        if (!factory) {
            // qDebug() << "Loading failed:" << loader.errorString();
        }
        ++it;
    }
    if (factory) {
        // qDebug() << "Trying to create a part";
        previewPart = factory->create<KParts::ReadOnlyPart>(q, (QVariantList() << "Print/Preview"));
        if (!previewPart) {
            // qDebug() << "Part creation failed";
        }
    }
}

bool KPrintPreviewPrivate::doPreview()
{
    if (!QFile::exists(filename)) {
        qWarning() << "Nothing was produced to be previewed";
        return false;
    }

    getPart();
    if (!previewPart) {
        //TODO: error dialog
        qWarning() << "Could not find a PDF viewer for the preview dialog";
        fail();
        return false;
    } else {
        setCentralWidget(previewPart->widget());
        return previewPart->openUrl(QUrl::fromLocalFile(filename));
    }
}

void KPrintPreviewPrivate::fail()
{
    if (!failMessage) {
        failMessage = new QLabel(i18n("Could not load print preview part"), q);
    }
    setCentralWidget(failMessage);
}

void KPrintPreviewPrivate::setCentralWidget(QWidget *widget)
{
    mainWidget->layout()->addWidget(widget);
}



KPrintPreview::KPrintPreview(QPrinter *printer, QWidget *parent)
    : QDialog(parent)
    , d(new KPrintPreviewPrivate(this, printer))
{
    // qDebug() << "kdeprint: creating preview dialog";

    //There is no printing on wince
#ifndef _WIN32_WCE
    // Set up the dialog
    setWindowTitle(i18n("Print Preview"));

    // Set up the printer
    // qDebug() << "Will print to" << d->filename;
    printer->setOutputFileName(d->filename);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    setLayout(layout);

    layout->addWidget(d->mainWidget, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Close);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(buttonBox);

    resize(QSize(600, 500));
    adjustSize();
#endif
}

KPrintPreview::~KPrintPreview()
{
    delete d;
}

void KPrintPreview::showEvent(QShowEvent *event)
{
    if (!event->spontaneous()) {
        // being shown for the first time
        if (!d->doPreview()) {
            event->accept();
            return;
        }
    }
    QDialog::showEvent(event);
}

bool KPrintPreview::isAvailable()
{
    return !KMimeTypeTrader::self()->query("application/pdf", "KParts/ReadOnlyPart").isEmpty();
}




