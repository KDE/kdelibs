/*
    This file is part of libkabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef RESOURCECONFIGDLG_H
#define RESOURCECONFIGDLG_H

#include <kdialog.h>

#include "resourceconfigwidget.h"

class QPushButton;
class KLineEdit;
class KConfig;
class QCheckBox;

class ResourceConfigDlg : KDialog
{
  Q_OBJECT

  public:
    ResourceConfigDlg( QWidget *parent, const QString& type,
                       KConfig *config, const char *name = 0 );

    bool readOnly();
    bool fast();
    QString resourceName();
    void setEditMode( bool value );

  public slots:
    int exec();

    void setReadOnly( bool value );
    void setFast( bool value );
    void setResourceName( const QString &name );
    void slotNameChanged( const QString &text);

  protected slots:
    void accept();

  private:
    KABC::ResourceConfigWidget *mConfigWidget;
    KConfig *mConfig;

    KLineEdit *mName;
    QCheckBox *mReadOnly;
    QCheckBox *mFast;
    QPushButton *mButtonOk;
};

#endif
