/*
    Copyright (c) 2008 Pino Toscano <pino@kde.org>

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


#include "kanimatedbuttontest.h"

#include "kanimatedbutton.h"

#include <QtGui/QLabel>
#include <QtGui/qlayout.h>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>


AnimationGroup::AnimationGroup(const QString &name, int size, QWidget *parent)
    : QGroupBox(parent)
{
    QHBoxLayout *lay = new QHBoxLayout(this);
    m_animButton = new KAnimatedButton(this);
    lay->addWidget(m_animButton);
    QPushButton *start = new QPushButton("Start", this);
    lay->addWidget(start);
    QPushButton *stop = new QPushButton("Stop", this);
    lay->addWidget(stop);

    setTitle(QString("%1 (%2)").arg(name).arg(size));
    m_animButton->setIconSize(QSize(size, size));
    m_animButton->setIcons(name);

    connect(start, SIGNAL(clicked()), m_animButton, SLOT(start()));
    connect(stop, SIGNAL(clicked()), m_animButton, SLOT(stop()));
}


MainWindow::MainWindow(QWidget *parent)
    : KMainWindow(parent)
{
    QWidget *central = new QWidget(this);
    QVBoxLayout *lay = new QVBoxLayout(central);
    setCentralWidget(central);

    QWidget *top = new QWidget(central);
    lay->addWidget(top);
    QHBoxLayout *lay2 = new QHBoxLayout(top);
    m_name = new QLineEdit(top);
    lay2->addWidget(m_name);
    m_size = new QSpinBox(top);
    lay2->addWidget(m_size);

    m_size->setValue(22);

    connect(m_name, SIGNAL(returnPressed()), this, SLOT(slotAddNew()));
}

void MainWindow::slotAddNew()
{
    AnimationGroup *group = new AnimationGroup(m_name->text(), m_size->value(), centralWidget());
    centralWidget()->layout()->addWidget(group);
}


int main(int argc, char **argv)
{
    KAboutData aboutData("kanimatedbuttontest", 0, ki18n("kanimatedbuttontest"), "0.1");
    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    MainWindow *window = new MainWindow();
    window->show();

    return app.exec();
}

#include "kanimatedbuttontest.moc"
