//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE libraries
   Copyright 2013 Jignesh Kakadiya <jigneshhk1992@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef __kbookmarkdialog_p_h__
#define __kbookmarkdialog_p_h__

#include "kbookmark.h"
#include <QDialog>

class KBookmarkDialog;
class KBookmarkManager;
class QDialogButtonBox;
class QLabel;
class QTreeWidget;
class QLineEdit;
class QTreeWidgetItem;

class KBookmarkDialogPrivate
{
public:
  KBookmarkDialogPrivate(KBookmarkDialog *q);
  ~KBookmarkDialogPrivate();

  typedef enum { NewFolder, NewBookmark, EditBookmark, NewMultipleBookmarks, SelectFolder } BookmarkDialogMode;

  void initLayout();
  void initLayoutPrivate();
  // selects the specified bookmark in the folder tree
  void setParentBookmark(const KBookmark & bm);
  KBookmarkGroup parentBookmark();
  void fillGroup (QTreeWidgetItem * parentItem, const KBookmarkGroup &group);

  KBookmarkDialog *q;
  BookmarkDialogMode mode;
  QDialogButtonBox *buttonBox;
  QLineEdit *url;
  QLineEdit *title;
  QLineEdit *comment;
  QLabel *titleLabel;
  QLabel *urlLabel;
  QLabel *commentLabel;
  QTreeWidget *folderTree;
  KBookmarkManager *mgr;
  KBookmark bm;
  QList<QPair<QString, QString> > list;
  bool layout;
};

#endif
