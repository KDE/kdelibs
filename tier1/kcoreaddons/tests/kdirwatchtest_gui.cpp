/*
 *  Copyright (C) 2006 Dirk Stoecker <kde@dstoecker.de>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kdirwatchtest_gui.h"

#include <kdirwatch.h>
#include <qplatformdefs.h>
#include <QDir>
#include <QApplication>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextBrowser>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    KDirWatchTest_GUI *mainWin = new KDirWatchTest_GUI();
    mainWin->show();
    return app.exec();
}

KDirWatchTest_GUI::KDirWatchTest_GUI() : QWidget()
{
    QPushButton *e,*f;

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setMargin( 0 );
    lay->addWidget(l1 = new QLineEdit( QLatin1String("Test 1"), this));
    lay->addWidget(l2 = new QLineEdit( QLatin1String("Test 2"), this));
    lay->addWidget(l3 = new QLineEdit( QLatin1String("Test 3"), this));
    lay->addWidget(m_eventBrowser = new QTextBrowser(this));
    lay->addWidget(d = new QLineEdit( QLatin1String("Status"), this));
    lay->addWidget(e = new QPushButton(QLatin1String("new file"), this));
    lay->addWidget(f = new QPushButton(QLatin1String("delete file"), this));

    dir = QDir::currentPath();
    file = dir + QLatin1String("/testfile_kdirwatchtest_gui");

    w1 = new KDirWatch();
    w1->setObjectName(QLatin1String("w1"));
    w2 = new KDirWatch();
    w2->setObjectName(QLatin1String("w2"));
    w3 = new KDirWatch();
    w3->setObjectName(QLatin1String("w3"));
    connect(w1, SIGNAL(dirty(QString)), this, SLOT(slotDir1(QString)));
    connect(w2, SIGNAL(dirty(QString)), this, SLOT(slotDir2(QString)));
    connect(w3, SIGNAL(dirty(QString)), this, SLOT(slotDir3(QString)));
    w1->addDir(dir);
    w2->addDir(dir);
    w3->addDir(dir);

    KDirWatch* w4 = new KDirWatch(this);
    w4->setObjectName(QLatin1String("w4"));
    w4->addDir(dir, KDirWatch::WatchFiles|KDirWatch::WatchSubDirs);
    connect(w1, SIGNAL(dirty(QString)), this, SLOT(slotDirty(QString)));
    connect(w1, SIGNAL(created(QString)), this, SLOT(slotCreated(QString)));
    connect(w1, SIGNAL(deleted(QString)), this, SLOT(slotDeleted(QString)));

    KDirWatch* w5 = new KDirWatch(this);
    w5->setObjectName(QLatin1String(QLatin1String("w5")));
    w5->addFile(file);
    connect(w5, SIGNAL(dirty(QString)), this, SLOT(slotDirty(QString)));
    connect(w5, SIGNAL(created(QString)), this, SLOT(slotCreated(QString)));
    connect(w5, SIGNAL(deleted(QString)), this, SLOT(slotDeleted(QString)));

    lay->addWidget(new QLabel(QLatin1String("Directory = ") + dir, this));
    lay->addWidget(new QLabel(QLatin1String("File = ") + file, this));

    connect(e, SIGNAL(clicked()), this, SLOT(slotNewClicked()));
    connect(f, SIGNAL(clicked()), this, SLOT(slotDeleteClicked()));

    setMinimumWidth(800);
    setMinimumHeight(400);
}

void KDirWatchTest_GUI::slotDir1(const QString &a)
{
  l1->setText(QLatin1String("Test 1 changed ") + a + QLatin1String(" at ") + QTime::currentTime().toString());
}

void KDirWatchTest_GUI::slotDir2(const QString &a)
{
  // This used to cause bug #119341, fixed now
#if 1
  w2->stopDirScan(QLatin1String(a.toLatin1().constData()));
  w2->restartDirScan(QLatin1String(a.toLatin1().constData()));
#endif
  l2->setText(QLatin1String("Test 2 changed ") + a + QLatin1String(" at ") + QTime::currentTime().toString());
}

void KDirWatchTest_GUI::slotDir3(const QString &a)
{
  l3->setText(QLatin1String("Test 3 changed ") + a + QLatin1String(" at )") + QTime::currentTime().toString());
}

void KDirWatchTest_GUI::slotDeleteClicked()
{
  remove(file.toLatin1().constData());
  d->setText(QLatin1String("Delete clicked at ") + QTime::currentTime().toString());
}

void KDirWatchTest_GUI::slotNewClicked()
{
  fclose(QT_FOPEN(file.toLatin1().constData(), "wb"));
  d->setText(QLatin1String("New clicked at ") + QTime::currentTime().toString());
}

void KDirWatchTest_GUI::slotDirty(const QString& path)
{
    m_eventBrowser->append( QLatin1String("Dirty(") + sender()->objectName() + QLatin1String("): ") + path + QLatin1Char('\n') );
}

void KDirWatchTest_GUI::slotCreated(const QString& path)
{
    m_eventBrowser->append( QLatin1String("Created(") + sender()->objectName() + QLatin1String("): ") + path + QLatin1Char('\n') );
}

void KDirWatchTest_GUI::slotDeleted(const QString& path)
{
    m_eventBrowser->append( QLatin1String("Deleted(") + sender()->objectName() + QLatin1String("): ") + path + QLatin1Char('\n') );
}

