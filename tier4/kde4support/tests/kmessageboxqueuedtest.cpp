/* This file is part of the KDE libraries
    Copyright (C) 2013 Kevin Ottens <ervin+bluesystems@kde.org>

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

#include <QApplication>
#include <QDebug>
#include <QPushButton>

#include "kmessagebox_queued.h"

class TriggerObject : public QObject
{
    Q_OBJECT
public:
    TriggerObject(QWidget *parent = 0)
        : QObject(parent), m_widget(parent) {}

public Q_SLOTS:
    void showMessages()
    {
        const int total = 10;

        for (int i = 0; i < total; i++) {
            QString message = "You should see only one of these (%1/%2)";
            KMessageBox::queuedMessageBox(m_widget, KMessageBox::Information, message.arg(i+1).arg(total));
            qDebug() << message.arg(i+1).arg(total);
        }
    }

private:
    QWidget * const m_widget;
};

int main( int argc, char *argv[] )
{
    QApplication app(argc, argv);

    QPushButton *button = new QPushButton("Click me to see messages");
    TriggerObject *trigger = new TriggerObject(button);
    QObject::connect(button, SIGNAL(clicked()), trigger, SLOT(showMessages()));
    button->show();

    return app.exec();
}

#include "kmessageboxqueuedtest.moc"
