/* This file is part of the KDE libraries
    Copyright (c) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "kapplication.h"
#include "kclipboard.h"

#include <qclipboard.h>

static QClipboard *clip = 0L;
static bool allOk = true;

static void printData()
{
    int i = 0;
    const char *format = 0;
    QMimeSource *data = clip->data();

    qDebug("  Serialnumber: %i", data->serialNumber() );

    while ( (format = data->format( i++ ) ) ) {
        QByteArray array = data->encodedData( format );
        qDebug("  Format: %s: size of data: %i", format, array.size());
    }
}

static void printContents()
{
    bool oldMode = clip->selectionModeEnabled();

    qDebug("::Text:");

    clip->setSelectionMode( true );
    qDebug("Selection Text: %s", clip->text().local8Bit().data() );
    clip->setSelectionMode( false );
    qDebug("Clipboard Text: %s", clip->text().local8Bit().data() );

    qDebug("\n::Data:");

    qDebug("-> Selection:");
    clip->setSelectionMode( true );
    printData();

    qDebug("\n-> Clipboard:");
    clip->setSelectionMode( false );
    printData();

    qDebug("\n\n");

    clip->setSelectionMode( oldMode );
}

static void checkResult( const QString& text )
{
    bool oldMode = clip->selectionModeEnabled();
    bool selectionMode = oldMode;
    bool clipboardMode = !oldMode;

    clip->setSelectionMode( false );
    QString clipboard = clip->text();
    clip->setSelectionMode( true );
    QString selection = clip->text();


    if ( KClipboard::isSynchronizing() )
    {
        if ( text != clipboard || text != selection )
        {
            qDebug("*** Synchronize mode: Bug: clipboard is != selection");
            allOk = false;
            printContents();
        }
    }

    if ( KClipboard::implicitSelection() && !KClipboard::isSynchronizing() )
    {
        if ( clipboardMode && clipboard != selection )
        {
            qDebug("*** Implicit selection: Bug: clipboard is != selection");
            allOk = false;
            printContents();
        }
        else if ( selectionMode && clipboard == selection )
        {
            qDebug("*** Implicit selection: Bug: clipboard is == selection");
            allOk = false;
            printContents();
        }
    }
    else if ( !KClipboard::implicitSelection() &&
              !KClipboard::isSynchronizing())
    {
        if ( clipboard == selection )
        {
            qDebug("*** No implicit selection: Bug: clipboard is == selection");
            allOk = false;
            printContents();
        }
    }

    clip->setSelectionMode( oldMode );
}

static void initClipboard( bool sync, bool implicit, bool selectionMode )
{
    qDebug("Now testing: sync: %i, implicit: %i, selectionMode: %i",
           sync, implicit, selectionMode);
    clip->setSelectionMode( false );
    clip->clear();
    clip->setSelectionMode( true );
    clip->clear();
    clip->setSelectionMode( selectionMode );

    KClipboard::setSynchronizing( sync );
    KClipboard::setImplicitSelection( implicit );
}

int main(int argc, char *argv[])
{
    KApplication a(argc, argv, "kclipboardtest");
    clip = QApplication::clipboard();

    QString text = QString::fromLatin1("This is a test. Unbelievable, eh?");

    initClipboard( true, true, false );
    clip->setText( text );
    checkResult( text );

    initClipboard( false, false, false );
    clip->setText( text );
    checkResult( text );

    initClipboard( false, true, false );
    clip->setText( text );
    checkResult( text );

    initClipboard( false, true, true );
    clip->setText( text );
    checkResult( text );
    if ( allOk )
        qDebug("\n\n*** All tests passed without errors.");
    else
        qDebug("\n\n### EEEK, Errors occurred!");

    return allOk ? 0 : 1;
}
