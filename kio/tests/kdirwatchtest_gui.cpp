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
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdirwatch.h>
#include <kfiletreeview.h>
#include <qdatetime.h>
#include <qdialog.h>
#include <qdir.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>

int main(int argc, char **argv)
{
    KAboutData about("KDirWatchTest_GUI", "KDirWatchTest_GUI", "0.1", "KDirWatchTest_GUI",
        KAboutData::License_LGPL, "(C) 2006 Dirk Stoecker", 0, 0, "kde@dstoecker.de");
    about.addAuthor( "Dirk Stoecker", 0, "kde@dstoecker.de" );
    KCmdLineArgs::init(argc, argv, &about);
    KApplication app;
    KDirWatchTest_GUI *mainWin = new KDirWatchTest_GUI();
    app.setMainWidget( mainWin );
    mainWin->show();
    return app.exec();
}

KDirWatchTest_GUI::KDirWatchTest_GUI() : QWidget()
{
    QPushButton *e,*f;
    KFileTreeView *tree;

    QVBoxLayout *lay = new QVBoxLayout(this, 0, -1);
    lay->addWidget(l1 = new QLineEdit( "Test 1", this));
    lay->addWidget(l2 = new QLineEdit( "Test 2", this));
    lay->addWidget(l3 = new QLineEdit( "Test 3", this));
    lay->addWidget(d = new QLineEdit( "Status", this));
    lay->addWidget(e = new QPushButton("new file", this));
    lay->addWidget(f = new QPushButton("delete file", this));

    dir = QDir::currentDirPath();
    file = dir + "/testfile_kdirwatchtest_gui";

    lay->addWidget(tree = new KFileTreeView(this));
    QString view = "file:" + dir;
    KFileTreeBranch *branch = tree->addBranch(KURL(view.latin1()), "Our data dir");
    tree->addColumn("Folders");
    branch->setOpen(true);

    w1 = new KDirWatch();
    w2 = new KDirWatch();
    w3 = new KDirWatch();
    connect(w1, SIGNAL(dirty(const QString &)), this, SLOT(slotDir1(const QString &)));
    connect(w2, SIGNAL(dirty(const QString &)), this, SLOT(slotDir2(const QString &)));
    connect(w3, SIGNAL(dirty(const QString &)), this, SLOT(slotDir3(const QString &)));
    w1->addDir(dir.latin1());
    w2->addDir(dir.latin1());
    w3->addDir(dir.latin1());

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
  w2->stopDirScan(a.latin1());
  w2->restartDirScan(a.latin1());
#endif
  l2->setText("Test 2 changed " + a + " at " + QTime::currentTime().toString());
}

void KDirWatchTest_GUI::slotDir3(const QString &a)
{
  l3->setText("Test 3 changed " + a + " at " + QTime::currentTime().toString());
}

void KDirWatchTest_GUI::slotDeleteClicked()
{
  remove(file.latin1());
  d->setText("Delete clicked at " + QTime::currentTime().toString());
}

void KDirWatchTest_GUI::slotNewClicked()
{
  fclose(fopen(file.latin1(), "wb"));
  d->setText("New clicked at " + QTime::currentTime().toString());
}

#include "kdirwatchtest_gui.moc"
