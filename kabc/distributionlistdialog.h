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

#ifndef KABC_DISTRIBUTIONLISTDIALOG_H
#define KABC_DISTRIBUTIONLISTDIALOG_H

#include <QWidget>

#include <kdialogbase.h>

class QTreeWidget;
class QComboBox;
class QButtonGroup;

namespace KABC {

class AddressBook;
class DistributionListEditorWidget;
class DistributionListManager;

/**
  @short Frontend to create distribution lists
 
  Creating a new DistributionListDialog does automatically
  load all addressees and distribution lists from the config
  files. The changes will be saved when clicking the 'OK'
  button.
 
  Example:
 
  \code
  KABC::DistributionListDialog *dlg = new
          KABC::DistributionListDialog( KABC::StdAddressBook::self(), this );
 
  dlg->exec();
  \endcode
*/
class KABC_EXPORT DistributionListDialog : public KDialogBase
{
    Q_OBJECT

  public:
    /**
      Constructor.

      @param ab     The addressbook, the addressees should be used from
      @param parent The parent widget
    */
    DistributionListDialog( AddressBook *ab, QWidget *parent );

    /**
      Destructor.
    */
    virtual ~DistributionListDialog();

  private:
    DistributionListEditorWidget *mEditor;

    struct Data;
    Data *d;
};

/**
  @short Helper class
*/
class KABC_EXPORT EmailSelector : public KDialogBase
{
  public:
    EmailSelector( const QStringList &emails, const QString &current,
        QWidget *parent );

    QString selected();

    static QString getEmail( const QStringList &emails, const QString &current,
        QWidget *parent );

  private:
    QButtonGroup *mButtonGroup;
};

/**
  @short Helper class
*/
class KABC_EXPORT DistributionListEditorWidget : public QWidget
{
    Q_OBJECT

  public:
    DistributionListEditorWidget( AddressBook *, QWidget *parent );
    virtual ~DistributionListEditorWidget();

  private Q_SLOTS:
    void newList();
    void editList();
    void removeList();
    void addEntry();
    void removeEntry();
    void changeEmail();
    void updateEntryView();
    void updateAddresseeView();
    void updateNameCombo();
    void slotSelectionEntryViewChanged();
    void slotSelectionAddresseeViewChanged();
    void save();

  private:
    QComboBox *mNameCombo;  
    QLabel *mListLabel;
    QTreeWidget *mEntryView;
    QTreeWidget *mAddresseeView;

    AddressBook *mAddressBook;
    DistributionListManager *mManager;
    QPushButton *mNewButton, *mEditButton, *mRemoveButton;
    QPushButton *mChangeEmailButton, *mRemoveEntryButton, *mAddEntryButton;

    struct Data;
    Data *d;
};

}
#endif
