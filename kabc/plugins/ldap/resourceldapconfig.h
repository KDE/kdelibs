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

#ifndef RESOURCELDAPCONFIG_H
#define RESOURCELDAPCONFIG_H

#include <qmap.h>

#include <kdialogbase.h>
#include <kresources/resourceconfigwidget.h>

class QCheckBox;
class QPushButton;
class QSpinBox;
class QString;

class KComboBox;
class KLineEdit;

namespace KABC {

class ResourceLDAPConfig : public KRES::ResourceConfigWidget
{ 
  Q_OBJECT

  public:
    ResourceLDAPConfig( QWidget* parent = 0, const char* name = 0 );

  public slots:
    void loadSettings( KRES::Resource* );
    void saveSettings( KRES::Resource* );

  private slots:
    void editAttributes();

  private:
    KLineEdit *mUser;
    KLineEdit *mPassword;
    KLineEdit *mHost;
    QSpinBox  *mPort;
    KLineEdit *mDn;
    KLineEdit *mFilter;
    QCheckBox *mAnonymous;
    QPushButton *mEditButton;
    QMap<QString, QString> mAttributes;
};

class AttributesDialog : public KDialogBase
{
  Q_OBJECT

  public:
    AttributesDialog( const QMap<QString, QString> &attributes, QWidget *parent,
                      const char *name = 0 );
    ~AttributesDialog();

    QMap<QString, QString> attributes() const;

  private slots:
    void mapChanged( int pos );

  private:
    enum { UserMap, KolabMap, NetscapeMap, EvolutionMap, OutlookMap };

    KComboBox *mMapCombo;
    QValueList< QMap<QString, QString> > mMapList;

    QDict<KLineEdit> mLineEditDict;
    QDict<QString> mNameDict;
};

}

#endif
