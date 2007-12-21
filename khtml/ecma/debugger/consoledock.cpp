/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2006 Matt Broadstone (mbroadst@gmail.com)
 *  Copyright (C) 2007 Maks Orlovich   (maksim@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "consoledock.h"
#include "consoledock.moc"

#include <QVBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QTextDocument>
#include <QPushButton>

#include <khistorycombobox.h>
#include <klocale.h>



namespace KJSDebugger {

ConsoleDock::ConsoleDock(QWidget *parent)
    : QDockWidget(i18n("Console"), parent)
{
    setFeatures(DockWidgetMovable | DockWidgetFloatable);

    QWidget* mainFrame = new QWidget(this);

    consoleView = new QTextEdit(mainFrame);
    consoleView->setReadOnly(true);

    consoleInput = new KHistoryComboBox(mainFrame);
    consoleInput->setSizePolicy(QSizePolicy::MinimumExpanding,
                                QSizePolicy::Fixed);

    connect(consoleInput, SIGNAL(returnPressed()),
           this, SLOT(slotUserRequestedEval()));

    consoleInputButton = new QPushButton(i18n("Enter"), mainFrame);
    connect(consoleInputButton, SIGNAL(clicked(bool)),
            this, SLOT(slotUserRequestedEval()));

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->setSpacing(0);
    bottomLayout->setMargin(0);
    bottomLayout->addWidget(consoleInput);
    bottomLayout->addWidget(consoleInputButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->addWidget(consoleView);
    mainLayout->addLayout(bottomLayout);
    mainFrame->setLayout(mainLayout);

    setWidget(mainFrame);

    oddLine = true;
}

ConsoleDock::~ConsoleDock()
{
}

void ConsoleDock::reportResult(const QString& src, const QString& msg)
{
    QTextCursor cursor(consoleView->document());
    cursor.movePosition(QTextCursor::End);

    const int marginSize = 6;

    // Append input..
    QTextCharFormat textFormat;
    QTextBlockFormat blockFormat;
    blockFormat.setTopMargin   (marginSize);
    blockFormat.setLeftMargin  (marginSize);
    blockFormat.setRightMargin (marginSize);
    blockFormat.setBottomMargin(0);
    
    blockFormat.setAlignment(Qt::AlignLeft);
    if (oddLine)
        blockFormat.setBackground(palette().base());
    else
        blockFormat.setBackground(palette().alternateBase());
    cursor.insertBlock(blockFormat);
    textFormat.setFontWeight(QFont::Normal);
    cursor.insertText(src, textFormat);

    // Append result.
    blockFormat.setAlignment(Qt::AlignRight);
    blockFormat.setTopMargin   (0);
    blockFormat.setBottomMargin(marginSize);
    cursor.insertBlock(blockFormat);
    textFormat.setFontWeight(QFont::Bold);
    cursor.insertText(msg, textFormat);

    oddLine = !oddLine;

    // Scroll into view..
    consoleView->moveCursor(QTextCursor::End);
}

void ConsoleDock::slotUserRequestedEval()
{
    QString src = consoleInput->lineEdit()->text();
    consoleInput->addToHistory(src);
    consoleInput->lineEdit()->clear();

    emit requestEval(src);
}

}
