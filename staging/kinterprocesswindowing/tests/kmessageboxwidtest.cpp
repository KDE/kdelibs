/* This file is part of the KDE libraries
    Copyright (c) 2013 Aurélien Gâteau <agateau@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include <kmessageboxwidtest.h>

#include <kmessagebox_kiw.h>
#include <kwindowsystem.h>

#include <QApplication>
#include <QHBoxLayout>
#include <QPushButton>

Window::Window()
{
    QPushButton *internalBoxButton = new QPushButton("Show Message Box");
    connect(internalBoxButton, &QPushButton::clicked, this, &Window::showMessageBox);

    QPushButton *externalBoxButton = new QPushButton("Show Message Box on Window from Another Process");
    connect(externalBoxButton, &QPushButton::clicked, this, &Window::showMessageBoxExternal);

    m_label = new QLabel;

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(internalBoxButton);
    layout->addWidget(externalBoxButton);
    layout->addWidget(m_label);
}

void Window::showMessageBox()
{
    int ret = KMessageBox::warningContinueCancelWId(winId(),
        "Are you sure you want to continue?",
        "Dangerous stuff");
    updateLabel(ret);
}

void Window::showMessageBoxExternal()
{
    m_label->setText("Click on another window to show a KMessageBox on it");
    WId us = winId();
    while (KWindowSystem::activeWindow() == us) {
        QApplication::processEvents();
    }
    WId id = KWindowSystem::activeWindow();
    m_label->setText(QString("Showing dialog on window with id: %1").arg(id));

    int ret = KMessageBox::warningContinueCancelWId(id,
        "Are you sure you want to continue?",
        "Dangerous stuff");
    updateLabel(ret);
}

void Window::updateLabel(int ret)
{
    QString txt;
    switch (KMessageBox::ButtonCode(ret)) {
    case KMessageBox::Ok:
        txt = "Ok";
        break;
    case KMessageBox::Cancel:
        txt = "Cancel";
        break;
    case KMessageBox::Yes:
        txt = "Yes";
        break;
    case KMessageBox::No:
        txt = "No";
        break;
    case KMessageBox::Continue:
        txt = "Continue";
        break;
    }
    m_label->setText(QString("'%1' button clicked.").arg(txt));
}

int main(int argc, char **argv)
{
    QApplication::setApplicationName("kmessageboxwidtest");
    QApplication app(argc, argv);
    Window window;
    window.show();
    return app.exec();
}
