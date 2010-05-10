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

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdirwatch.h>
#include <QtCore/QDate>
#include <QtGui/QDialog>
#include <QtCore/QDir>
#include <QtGui/QApplication>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QTextBrowser>

int main(int argc, char **argv)
{
    KAboutData about(QByteArray("KDirWatchTest_GUI"), QByteArray(), ki18n("KDirWatchTest_GUI"), 
            QByteArray("0.1"),
        ki18n("KDirWatchTest_GUI"), KAboutData::License_LGPL,
        ki18n("(C) 2006 Dirk Stoecker"), KLocalizedString(), QByteArray(),
        QByteArray("kde@dstoecker.de"));
    about.addAuthor( ki18n("Dirk Stoecker"), KLocalizedString(), QByteArray("kde@dstoecker.de") );
    KCmdLineArgs::init(argc, argv, &about);
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
    lay->addWidget(l1 = new QLineEdit( "Test 1", this));
    lay->addWidget(l2 = new QLineEdit( "Test 2", this));
    lay->addWidget(l3 = new QLineEdit( "Test 3", this));
    lay->addWidget(m_eventBrowser = new QTextBrowser(this));
    lay->addWidget(d = new QLineEdit( "Status", this));
    lay->addWidget(e = new QPushButton("new file", this));
    lay->addWidget(f = new QPushButton("delete file", this));

    dir = QDir::currentPath();
    file = dir + "/testfile_kdirwatchtest_gui";

    w1 = new KDirWatch();
    w1->setObjectName("w1");
    w2 = new KDirWatch();
    w2->setObjectName("w2");
    w3 = new KDirWatch();
    w3->setObjectName("w3");
    connect(w1, SIGNAL(dirty(const QString &)), this, SLOT(slotDir1(const QString &)));
    connect(w2, SIGNAL(dirty(const QString &)), this, SLOT(slotDir2(const QString &)));
    connect(w3, SIGNAL(dirty(const QString &)), this, SLOT(slotDir3(const QString &)));
    w1->addDir(dir);
    w2->addDir(dir);
    w3->addDir(dir);

    KDirWatch* w4 = new KDirWatch(this);
    w4->setObjectName("w4");
    w4->addDir(dir, KDirWatch::WatchFiles|KDirWatch::WatchSubDirs);
    connect(w1, SIGNAL(dirty(const QString &)), this, SLOT(slotDirty(const QString &)));
    connect(w1, SIGNAL(created(const QString &)), this, SLOT(slotCreated(const QString &)));
    connect(w1, SIGNAL(deleted(const QString &)), this, SLOT(slotDeleted(const QString &)));

    KDirWatch* w5 = new KDirWatch(this);
    w5->setObjectName("w5");
    w5->addFile(file);
    connect(w5, SIGNAL(dirty(const QString &)), this, SLOT(slotDirty(const QString &)));
    connect(w5, SIGNAL(created(const QString &)), this, SLOT(slotCreated(const QString &)));
    connect(w5, SIGNAL(deleted(const QString &)), this, SLOT(slotDeleted(const QString &)));

    lay->addWidget(new QLabel("Directory = " + dir, this));
    lay->addWidget(new QLabel("File = " + file, this));

    connect(e, SIGNAL(clicked()), this, SLOT(slotNewClicked()));
    connect(f, SIGNAL(clicked()), this, SLOT(slotDeleteClicked()));

    setMinimumWidth(800);
    setMinimumHeight(400);
}

void KDirWatchTest_GUI::slotDir1(const QString &a)
{
  l1->setText("Test 1 changed " + a + " at " + QTime::currentTime().toString());
}

void KDirWatchTest_GUI::slotDir2(const QString &a)
{
  // This used to cause bug #119341, fixed now
#if 1
  w2->stopDirScan(a.toLatin1().constData());
  w2->restartDirScan(a.toLatin1().constData());
#endif
  l2->setText("Test 2 changed " + a + " at " + QTime::currentTime().toString());
}

void KDirWatchTest_GUI::slotDir3(const QString &a)
{
  l3->setText("Test 3 changed " + a + " at " + QTime::currentTime().toString());
}

void KDirWatchTest_GUI::slotDeleteClicked()
{
  remove(file.toLatin1().constData());
  d->setText("Delete clicked at " + QTime::currentTime().toString());
}

void KDirWatchTest_GUI::slotNewClicked()
{
  fclose(fopen(file.toLatin1().constData(), "wb"));
  d->setText("New clicked at " + QTime::currentTime().toString());
}

void KDirWatchTest_GUI::slotDirty(const QString& path)
{
    m_eventBrowser->append( "Dirty(" + sender()->objectName() + "): " + path + "\n" );
}

void KDirWatchTest_GUI::slotCreated(const QString& path)
{
    m_eventBrowser->append( "Created(" + sender()->objectName() + "): " + path + "\n" );
}

void KDirWatchTest_GUI::slotDeleted(const QString& path)
{
    m_eventBrowser->append( "Deleted(" + sender()->objectName() + "): " + path + "\n" );
}

#include "kdirwatchtest_gui.moc"
