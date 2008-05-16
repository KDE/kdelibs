/**
 * KDE Rich Text Editor
 *
 * Copyright 2008  Stephen Kelly <steveire@gmail.com>
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

#include <QtCore/QTextStream>

#include <kapplication.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <kfiledialog.h>
#include <KRichTextWidget>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include <ksavefile.h>
#include <kstatusbar.h>


KRichTextEditor::KRichTextEditor() : KXmlGuiWindow()
{
    setupActions();

    textArea = new KRichTextWidget(this);
    setCentralWidget(textArea);

    textArea->createActions(actionCollection());
    setupGUI();

    statusBar()->insertItem(QString(), 0, 1);
    statusBar()->setItemAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);

    connect(textArea, SIGNAL(cursorPositionChanged()),
            SLOT(cursorPositionChanged()));
}

KRichTextEditor::~KRichTextEditor()
{
}

void KRichTextEditor::setupActions()
{
    KStandardAction::quit(kapp, SLOT(quit()),
                          actionCollection());

    KStandardAction::open(this, SLOT(openFile()),
                          actionCollection());

    KStandardAction::save(this, SLOT(saveFile()),
                          actionCollection());

    KStandardAction::saveAs(this, SLOT(saveFileAs()),
                            actionCollection());

    KStandardAction::openNew(this, SLOT(newFile()),
                             actionCollection());
}

void KRichTextEditor::cursorPositionChanged()
{
// Show link target in status bar
    if (textArea->textCursor().charFormat().isAnchor()) {
        QString text = textArea->currentLinkText();
        QString url = textArea->currentLinkUrl();
        statusBar()->changeItem(text + " -> " + url, 0);
    } else {
        statusBar()->changeItem(QString(), 0);
    }
}

void KRichTextEditor::newFile()
{
//maybeSave
    fileName.clear();
    textArea->clear();
}

void KRichTextEditor::saveFileAs(const QString &outputFileName)
{
    KSaveFile file(outputFileName);
    file.open();

    QByteArray outputByteArray;
    outputByteArray.append(textArea->toHtml().toUtf8());
    file.write(outputByteArray);
    file.finalize();
    file.close();

    fileName = outputFileName;
}

void KRichTextEditor::saveFileAs()
{
    saveFileAs(KFileDialog::getSaveFileName());
}

void KRichTextEditor::saveFile()
{
    if (!fileName.isEmpty()) {
        saveFileAs(fileName);
    } else {
        saveFileAs();
    }
}

void KRichTextEditor::openFile()
{
    QString fileNameFromDialog = KFileDialog::getOpenFileName();
    if (fileNameFromDialog.isEmpty()) {
        return;
    }

    QString tmpFile;
    if (KIO::NetAccess::download(fileNameFromDialog, tmpFile,
                                 this)) {
        QFile file(tmpFile);
        file.open(QIODevice::ReadOnly);
        textArea->setTextOrHtml(QTextStream(&file).readAll());
        fileName = fileNameFromDialog;

        KIO::NetAccess::removeTempFile(tmpFile);
    } else {
        KMessageBox::error(this,
                           KIO::NetAccess::lastErrorString());
    }
}
