/* This file is part of the KDE libraries
   Copyright (C) 2004 George Staikos <staikos@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef KBETTERTHANKDIALOG_H
#define KBETTERTHANKDIALOG_H

#include <QtGui/QDialog>

#include "ui_kbetterthankdialogbase.h"

class KBetterThanKDialog : public QDialog, private Ui_KBetterThanKDialogBase
{
  Q_OBJECT

  public:
    KBetterThanKDialog( QWidget *parent = 0 );

    void init();
    void setLabel( const QString &label );

  protected Q_SLOTS:
    virtual void accept();
    virtual void reject();
  
  private Q_SLOTS:
    void allowOnceClicked();
    void allowAlwaysClicked();
    void denyClicked();
    void denyForeverClicked();
};

#endif
