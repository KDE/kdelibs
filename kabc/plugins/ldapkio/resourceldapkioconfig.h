/*
    This file is part of libkabc.
    Copyright (c) 2002 - 2003 Tobias Koenig <tokoe@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef RESOURCELDAPCONFIG_H
#define RESOURCELDAPCONFIG_H

#include <qmap.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qdict.h>

#include <kdialogbase.h>
#include <kresources/configwidget.h>
#include <kabc/ldif.h>
#include <kabc/ldapconfigwidget.h>


class QCheckBox;
class QPushButton;
class QSpinBox;
class QString;

class KComboBox;
class KLineEdit;

namespace KABC {

class KABC_EXPORT ResourceLDAPKIOConfig : public KRES::ConfigWidget
{ 
  Q_OBJECT

  public:
    ResourceLDAPKIOConfig( QWidget* parent = 0, const char* name = 0 );

  public slots:
    void loadSettings( KRES::Resource* );
    void saveSettings( KRES::Resource* );

  private slots:
    void editAttributes();
    void editCache();
  private:
    QPushButton *mEditButton, *mCacheButton;
    LdapConfigWidget *cfg;
    QCheckBox *mSubTree;
    QMap<QString, QString> mAttributes;
    int mRDNPrefix, mCachePolicy;
    bool mAutoCache;
    QString mCacheDst;
};

class AttributesDialog : public KDialogBase
{
  Q_OBJECT

  public:
    AttributesDialog( const QMap<QString, QString> &attributes, int rdnprefix,
                      QWidget *parent, const char *name = 0 );
    ~AttributesDialog();

    QMap<QString, QString> attributes() const;
    int rdnprefix() const;

  private slots:
    void mapChanged( int pos );

  private:
    enum { UserMap, KolabMap, NetscapeMap, EvolutionMap, OutlookMap };

    KComboBox *mMapCombo, *mRDNCombo;
    QValueList< QMap<QString, QString> > mMapList;
    QMap<QString, QString> mDefaultMap;
    QDict<KLineEdit> mLineEditDict;
    QDict<QString> mNameDict;
};

class OfflineDialog : public KDialogBase
{
  Q_OBJECT

  public:
    OfflineDialog( bool autoCache, int cachePolicy, const KURL &src, 
      const QString &dst, QWidget *parent, const char *name = 0 );
    ~OfflineDialog();

    int cachePolicy() const;
    bool autoCache() const;

  private slots:
    void loadCache();

  private:
    KURL mSrc;
    QString mDst;
    QButtonGroup *mCacheGroup;
    QCheckBox *mAutoCache;
};

}

#endif
