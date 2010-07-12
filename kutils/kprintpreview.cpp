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

#include <QtCore/QFile>
#include <QtGui/QLabel>
#include <QtGui/QPrinter>
#include <QtGui/QShowEvent>

#include <kmimetypetrader.h>
#include <kparts/part.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kservice.h>
#include <ktempdir.h>
#include <kdebug.h>


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
        if ( tempdir.exists() ) {
            filename = tempdir.name() + "print_preview.pdf";
        } else {
            // XXX: not portable!
            kWarning() << "Failed to create temporary directory";
            filename = "/dev/null";
        }
    }

    void getPart();
    bool doPreview();
    void fail();

    KPrintPreview *q;

    QPrinter *printer;
    QWidget *mainWidget;

    KTempDir tempdir;
    QString filename;

    KParts::ReadOnlyPart *previewPart;
    QWidget *failMessage;
};

void KPrintPreviewPrivate::getPart()
{
    if (previewPart) {
        kDebug(500) << "already got a part";
        return;
    }
    kDebug(500) << "querying trader for application/pdf service";

    KPluginFactory *factory(0);
    const KService::List offers =
        KMimeTypeTrader::self()->query("application/pdf", "KParts/ReadOnlyPart");

    KService::List::ConstIterator it = offers.begin();
    while (!factory && it != offers.end()) {
        KPluginLoader loader(**it);
        factory = loader.factory();
        if (!factory) {
            kDebug(500) << "Loading failed:" << loader.errorString();
        }
        ++it;
    }
    if (factory) {
        kDebug(500) << "Trying to create a part";
        previewPart = factory->create<KParts::ReadOnlyPart>(q, (QVariantList() << "Print/Preview"));
        if (!previewPart) {
            kDebug(500) << "Part creation failed";
        }
    }
}

bool KPrintPreviewPrivate::doPreview()
{
    if (!QFile::exists(filename)) {
        kWarning() << "Nothing was produced to be previewed";
        return false;
    }

    getPart();
    if (!previewPart) {
        //TODO: error dialog
        kWarning() << "Could not find a PDF viewer for the preview dialog";
        fail();
        return false;
    } else {
        q->setMainWidget(previewPart->widget());
        return previewPart->openUrl(filename);
    }
}

void KPrintPreviewPrivate::fail()
{
    if (!failMessage) {
        failMessage = new QLabel(i18n("Could not load print preview part"), q);
    }
    q->setMainWidget(failMessage);
}




KPrintPreview::KPrintPreview(QPrinter *printer, QWidget *parent)
    : KDialog(parent)
    , d(new KPrintPreviewPrivate(this, printer))
{
    kDebug(500) << "kdeprint: creating preview dialog";

    //There is no printing on wince
#ifndef _WIN32_WCE
    // Set up the dialog
    setCaption(i18n("Print Preview"));
    setButtons(KDialog::Close);

    // Set up the printer
    kDebug(500) << "Will print to" << d->filename;
    printer->setOutputFileName(d->filename);

    setInitialSize(QSize(600, 500));
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
    KDialog::showEvent(event);
}

bool KPrintPreview::isAvailable()
{
    return !KMimeTypeTrader::self()->query("application/pdf", "KParts/ReadOnlyPart").isEmpty();
}

#include "kprintpreview.moc"



