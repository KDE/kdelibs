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

#ifndef KDIRWATCHTEST_GUI_H
#define KDIRWATCHTEST_GUI_H

#include <kmainwindow.h>
#include <qdialog.h>

class KDirWatchTest_GUI : public QWidget
{
    Q_OBJECT
public:
    KDirWatchTest_GUI();
protected slots:
    void slotNewClicked();
    void slotDeleteClicked();
    void slotDir1(const QString &path);
    void slotDir2(const QString &path);
    void slotDir3(const QString &path);
private:
    class QLineEdit *d;
    QString file, dir;
    class KDirWatch *w1;
    class KDirWatch *w2;
    class KDirWatch *w3;
    class QLineEdit *l1, *l2, *l3;
};

#endif
