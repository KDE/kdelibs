/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef MYDIALOGBASE_H
#define MYDIALOGBASE_H

#include <kdialog.h>

class QListView;
class QListViewItem;
class QWidgetStack;
class QLabel;
class MyPage;

class MyDialogBase : public KDialog
{
        Q_OBJECT
public:
        MyDialogBase(QWidget *parent = 0, const char *name = 0);
        ~MyDialogBase();

        void addPage(const QStringList& path, const QString& header, const QPixmap& icon, QWidget *w);

protected slots:
        virtual void slotOk();
        virtual void slotCancel();
        virtual void slotApply();
        void slotItemSelected(QListViewItem*);
        virtual void slotHelp();

private:
        QListViewItem* findParent(const QStringList& path);

private:
        QListView       *tree_;
        QWidgetStack    *stack_;
        QLabel          *title_;
};

#endif
