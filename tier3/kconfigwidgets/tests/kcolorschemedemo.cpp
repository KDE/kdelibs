/* This file is part of the KDE project
 * Copyright (C) 2013 Martin Gräßlin <mgraesslin@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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
#include <kcolorschememanager.h>
#include <kactionmenu.h>

#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QListView>
#include <QToolButton>
#include <QVBoxLayout>

class KColorSchemeDemo : public QDialog
{
    Q_OBJECT
public:
    KColorSchemeDemo() : QDialog(0) {
        KColorSchemeManager *manager = new KColorSchemeManager(this);

        QListView *view = new QListView(this);
        view->setModel(manager->model());
        connect(view, &QListView::activated, manager, &KColorSchemeManager::activateScheme);

        QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Close, this);
        connect(box, &QDialogButtonBox::rejected, qApp, &QApplication::quit);

        QToolButton *button = new QToolButton(box);
        button->setIcon(QIcon::fromTheme("fill-color"));
        button->setMenu(manager->createSchemeSelectionMenu(QStringLiteral("Oxygen"), button)->menu());
        box->addButton(button, QDialogButtonBox::InvalidRole);

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(view);
        layout->addWidget(box);
        setLayout(layout);
    }
    ~KColorSchemeDemo() {}
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    KColorSchemeDemo *d = new KColorSchemeDemo;
    d->show();
    return app.exec();
}

#include "kcolorschemedemo.moc"
