/**
 * KStyle for KF5
 * Copyright (C) 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
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
 * Many thanks to Bradley T. Hughes for the 3 button scrollbar code.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialog>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QLayout>
#include "kstyle.h"

void showDialog()
{
    QScopedPointer<QDialog> dialog(new QDialog);
    dialog->setLayout(new QVBoxLayout);

    QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, dialog.data());
    QObject::connect(box, SIGNAL(accepted()), dialog.data(), SLOT(accept()));
    QObject::connect(box, SIGNAL(rejected()), dialog.data(), SLOT(reject()));

    dialog->layout()->addWidget(new QTextEdit(dialog.data()));
    dialog->layout()->addWidget(box);

    dialog->resize(200, 200);
    dialog->exec();
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setStyle(new KStyle);

    QWidget w;
    w.setLayout(new QVBoxLayout);

    QPushButton* showDialogButton = new QPushButton(QLatin1Literal("Dialog"), &w);
    QObject::connect(showDialogButton, &QPushButton::clicked, &showDialog);
    w.layout()->addWidget(showDialogButton);
    w.resize(200, 200);
    w.show();

    return app.exec();
}
