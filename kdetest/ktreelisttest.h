/* outline.h
-------------------------------------------------------------------
$Id$
  
outline class declarations
An example using the KTreeListWidget

Copyright (C) 1996 Keith Brown and KtSoft

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABLILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details. You should have received a
copy of the GNU General Public License along with this program; if
not, write to the Free Software Foundation, Inc, 675 Mass Ave, 
Cambridge, MA 02139, USA.
  
-------------------------------------------------------------------
*/

#ifndef OUTLINE_H
#define OUTLINE_H

#include <kapp.h>
#include <qchkbox.h>
#include <qgrpbox.h>
#include <qlabel.h>
#include <qlined.h>
#include <ktreelist.h>
#include <qpushbt.h>
#include <stdio.h>

class OutlineWidget : public QWidget
{
  Q_OBJECT
public:
    OutlineWidget(QWidget* parent = 0);
private slots:
    void addItem();
    void addSubItem();
    void changeItem();
    void collapse();
    void expand();
    void insertItem();
    void itemDown();
    void itemIn();
    void itemOut();
    void itemUp();
    void removeItem();
    void toggleExpandButtons(bool on);
    void toggleTree(bool on);
  private:
    KTreeList *outlineTree;
    QPushButton *collapseButton;
    QPushButton *expandButton;
    QLabel *expandLevelLabel;
    QPushButton *upButton;
    QPushButton *outButton;
    QPushButton *inButton;
    QPushButton *downButton;
    QCheckBox *expandOption;
    QCheckBox *treeOption;
    QPushButton *quitButton;
    QLineEdit *itemEdit;
    QPushButton *addButton;
    QPushButton *subButton;
    QPushButton *changeButton;
    QPushButton *delButton;
    QPushButton *insertButton;
};


#endif  OUTLINE_H
