/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2006 Matt Broadstone (mbroadst@gmail.com)
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
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>

#include <klocale.h>

ConsoleDock::ConsoleDock(QWidget *parent)
    : QDockWidget(i18n("Console"), parent)
{
    setFeatures(DockWidgetMovable | DockWidgetFloatable);
    QFrame *mainFrame = new QFrame;

    consoleEdit = new QTextEdit;
    consoleEdit->setReadOnly(true);
    consoleInput = new QLineEdit;
    consoleInputButton = new QPushButton(i18n("Enter"));

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->setSpacing(0);
    bottomLayout->setMargin(0);
    bottomLayout->addWidget(consoleInput);
    bottomLayout->addWidget(consoleInputButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(mainFrame);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->addWidget(consoleEdit);
    mainLayout->addLayout(bottomLayout);

    setWidget(mainFrame);
}

ConsoleDock::~ConsoleDock()
{
}
