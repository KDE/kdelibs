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

#include <QButtonGroup>
#include <QComboBox>
#include <QGroupBox>
#include <QHash>
#include <QMap>

#include <kabc/ldapconfigwidget.h>
#include <kabc/ldif.h>
#include <kdialogbase.h>
#include <kresources/configwidget.h>

class QCheckBox;
class QPushButton;
class QSpinBox;
class QString;

class KComboBox;
class KLineEdit;

namespace KABC {

class KABC_LDAPKIO_EXPORT ResourceLDAPKIOConfig : public KRES::ConfigWidget
{
  Q_OBJECT

  public:
    ResourceLDAPKIOConfig( QWidget* parent = 0 );

  public Q_SLOTS:
    void loadSettings( KRES::Resource* );
    void saveSettings( KRES::Resource* );

  private Q_SLOTS:
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

  private Q_SLOTS:
    void mapChanged( int pos );

  private:
    enum { UserMap, KolabMap, NetscapeMap, EvolutionMap, OutlookMap };

    KComboBox *mMapCombo, *mRDNCombo;
    QList< QMap<QString, QString> > mMapList;
    QMap<QString, QString> mDefaultMap;
    QHash<QString, KLineEdit*> mLineEditDict;
    QHash<QString, QString> mNameDict;
};

class OfflineDialog : public KDialogBase
{
  Q_OBJECT

  public:
    OfflineDialog( bool autoCache, int cachePolicy, const KUrl &src,
      const QString &dst, QWidget *parent, const char *name = 0 );
    ~OfflineDialog();

    int cachePolicy() const;
    bool autoCache() const;

  private Q_SLOTS:
    void loadCache();

  private:
    KUrl mSrc;
    QString mDst;
    QGroupBox *mCacheBox;
    QButtonGroup *mCacheGroup;
    QCheckBox *mAutoCache;
};

}

#endif
