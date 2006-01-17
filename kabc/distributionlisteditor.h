/*
    This file is part of libkabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KABC_DISTRIBUTIONLISTEDITOR_H
#define KABC_DISTRIBUTIONLISTEDITOR_H

#include <qwidget.h>

#include <kdialogbase.h>

class QTreeWidget;
class QComboBox;
class QButtonGroup;

namespace KABC {

class AddressBook;
class DistributionListManager;

class KABC_EXPORT EmailSelectDialog : public KDialogBase
{
  public:
    EmailSelectDialog( const QStringList &emails, const QString &current,
                       QWidget *parent );
    
    QString selected();

    static QString getEmail( const QStringList &emails, const QString &current,
                             QWidget *parent );

  private:
    QButtonGroup *mButtonGroup;
};

/**
  @obsolete
*/
class DistributionListEditor : public QWidget
{
    Q_OBJECT
  public:
    DistributionListEditor( AddressBook *, QWidget *parent );
    virtual ~DistributionListEditor();

  private Q_SLOTS:
    void newList();
    void removeList();
    void addEntry();
    void removeEntry();
    void changeEmail();
    void updateEntryView();
    void updateAddresseeView();
    void updateNameCombo();
    void slotSelectionEntryViewChanged();
    void slotSelectionAddresseeViewChanged();

  private:
    QComboBox *mNameCombo;  
    QTreeWidget *mEntryView;
    QTreeWidget *mAddresseeView;

    AddressBook *mAddressBook;
    DistributionListManager *mManager;
    QPushButton *newButton, *removeButton;
    QPushButton *changeEmailButton,*removeEntryButton,*addEntryButton;
};

}

#endif
