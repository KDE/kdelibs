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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KABC_DISTRIBUTIONLISTDIALOG_H
#define KABC_DISTRIBUTIONLISTDIALOG_H

#include <qwidget.h>

#include <kdialogbase.h>

class QListView;
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
 
  <pre>
  KABC::DistributionListDialog *dlg = new
          KABC::DistributionListDialog( KABC::StdAddressBook::self(), this );
 
  dlg->exec();
  </pre>
*/
class DistributionListDialog : public KDialogBase
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
class EmailSelector : public KDialogBase
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
class DistributionListEditorWidget : public QWidget
{
    Q_OBJECT

  public:
    DistributionListEditorWidget( AddressBook *, QWidget *parent );
    virtual ~DistributionListEditorWidget();

  private slots:
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
    QListView *mEntryView;
    QListView *mAddresseeView;

    AddressBook *mAddressBook;
    DistributionListManager *mManager;
    QPushButton *mNewButton, *mEditButton, *mRemoveButton;
    QPushButton *mChangeEmailButton, *mRemoveEntryButton, *mAddEntryButton;

    struct Data;
    Data *d;
};

}
#endif
