/* This file is part of the KDE libraries
 *
 * Copyright 2012 Aurélien Gâteau <agateau@kde.org>
 *
 * Based on test program by Dominik Haumann <dhaumann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QFrame>
#include <QCheckBox>

#include <kmessagewidget.h>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QWidget* mainWindow = new QWidget();

    QVBoxLayout* l = new QVBoxLayout(mainWindow);

    KMessageWidget* mw = new KMessageWidget(mainWindow);
    mw->setWordWrap(true);
    mw->setText(
        "Test KMessageWidget is properly sized when word-wrap is enabled by default."
    );
    // A frame to materialize the end of the KMessageWidget
    QFrame* frame = new QFrame(mainWindow);
    frame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QCheckBox* wordWrapCb = new QCheckBox("wordWrap", mainWindow);
    wordWrapCb->setChecked(true);
    QObject::connect(wordWrapCb, SIGNAL(toggled(bool)), mw, SLOT(setWordWrap(bool)));

    QCheckBox* closeButtonCb = new QCheckBox("closeButton", mainWindow);
    closeButtonCb->setChecked(true);
    QObject::connect(closeButtonCb, SIGNAL(toggled(bool)), mw, SLOT(setCloseButtonVisible(bool)));

    l->addWidget(wordWrapCb);
    l->addWidget(closeButtonCb);
    l->addWidget(mw);
    l->addWidget(frame);

    mainWindow->resize(400, 300);
    mainWindow->show();

    return app.exec();
    delete mainWindow;
}

// kate: replace-tabs on;
