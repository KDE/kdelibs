/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>, Alexander Neundorf <neundorf@kde.org>
   2000, 2002 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include "keditlistbox.h"

#include <QtCore/QStringList>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QListView>
#include <QtGui/QPushButton>

#include <kcombobox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <knotification.h>

#include <assert.h>

class KEditListBoxPrivate
{
public:
    QListView *m_listView;
    QPushButton *servUpButton, *servDownButton;
    QPushButton *servNewButton, *servRemoveButton;
    KLineEdit *m_lineEdit;
    QStringListModel *m_model;

    bool m_checkAtEntering;
    KEditListBox::Buttons buttons;
};

class KEditListBox::CustomEditorPrivate
{
public:
    CustomEditorPrivate(KEditListBox::CustomEditor *q):
        q(q),
        m_representationWidget(0),
        m_lineEdit(0) {}

    KEditListBox::CustomEditor *q;
    QWidget *m_representationWidget;
    KLineEdit *m_lineEdit;
};

KEditListBox::CustomEditor::CustomEditor()
    : d(new CustomEditorPrivate(this))
{
}

KEditListBox::CustomEditor::CustomEditor( QWidget *repWidget, KLineEdit *edit )
    : d(new CustomEditorPrivate(this))
{
    d->m_representationWidget = repWidget;
    d->m_lineEdit = edit;
}

KEditListBox::CustomEditor::CustomEditor( KComboBox *combo )
    : d(new CustomEditorPrivate(this))
{
    d->m_representationWidget = combo;
    d->m_lineEdit = qobject_cast<KLineEdit*>( combo->lineEdit() );
    Q_ASSERT( d->m_lineEdit );
}

KEditListBox::CustomEditor::~CustomEditor()
{
    delete d;
}

void KEditListBox::CustomEditor::setRepresentationWidget( QWidget *repWidget )
{
    d->m_representationWidget = repWidget;
}

void KEditListBox::CustomEditor::setLineEdit( KLineEdit *edit )
{
    d->m_lineEdit = edit;
}

QWidget *KEditListBox::CustomEditor::representationWidget() const
{
    return d->m_representationWidget;
}

KLineEdit *KEditListBox::CustomEditor::lineEdit() const
{
    return d->m_lineEdit;
}

KEditListBox::KEditListBox(QWidget *parent)
    :QGroupBox(parent), d(new KEditListBoxPrivate)
{
    init();
}

KEditListBox::KEditListBox(const QString &title, QWidget *parent)
    :QGroupBox(title, parent), d(new KEditListBoxPrivate)
{
    init();
}

KEditListBox::KEditListBox(QWidget *parent, const char *name,
                           bool checkAtEntering, Buttons buttons )
    :QGroupBox(parent ), d(new KEditListBoxPrivate)
{
    setObjectName(name);
    init( checkAtEntering, buttons );
}

KEditListBox::KEditListBox(const QString& title, QWidget *parent,
                           const char *name, bool checkAtEntering, Buttons buttons)
    :QGroupBox(title, parent ), d(new KEditListBoxPrivate)
{
    setObjectName(name);
    init( checkAtEntering, buttons );
}

KEditListBox::KEditListBox(const QString& title, const CustomEditor& custom,
                           QWidget *parent, const char *name,
                           bool checkAtEntering, Buttons buttons)
    :QGroupBox(title, parent), d(new KEditListBoxPrivate)
{
    setObjectName(name);
    d->m_lineEdit = custom.lineEdit();
    init( checkAtEntering, buttons, custom.representationWidget() );
}

KEditListBox::~KEditListBox()
{
    delete d;
}

void KEditListBox::init( bool checkAtEntering, Buttons buttons,
                         QWidget *representationWidget )
{
    d->m_checkAtEntering = checkAtEntering;

    d->servNewButton = d->servRemoveButton = d->servUpButton = d->servDownButton = 0L;
    setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
                              QSizePolicy::MinimumExpanding));

    QGridLayout * grid = new QGridLayout(this);
    grid->setMargin( KDialog::marginHint() );
    grid->setSpacing( KDialog::spacingHint() );
    grid->setRowStretch( 6, 1 );

    grid->setMargin(15);

    if ( representationWidget )
        representationWidget->setParent(this);
    else
        d->m_lineEdit = new KLineEdit(this);

    d->m_model = new QStringListModel();
    d->m_listView = new QListView(this);

    d->m_listView->setModel(d->m_model);

    QWidget *editingWidget = representationWidget ?
                             representationWidget : d->m_lineEdit;
    grid->addWidget(editingWidget,1,0,1,2);
    grid->addWidget(d->m_listView, 2, 0, 4, 1);

    d->buttons = 0;
    setButtons( buttons );

    connect(d->m_lineEdit,SIGNAL(textChanged(const QString&)),this,SLOT(typedSomething(const QString&)));
    d->m_lineEdit->setTrapReturnKey(true);
    connect(d->m_lineEdit,SIGNAL(returnPressed()),this,SLOT(addItem()));
    connect(d->m_listView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
            this,SLOT(enableMoveButtons(const QModelIndex&, const QModelIndex&)));

    // maybe supplied lineedit has some text already
    typedSomething( d->m_lineEdit->text() );
}

QListView *KEditListBox::listView() const
{
    return d->m_listView;
}

KLineEdit *KEditListBox::lineEdit() const
{
    return d->m_lineEdit;
}

QPushButton *KEditListBox::addButton() const
{
    return d->servNewButton;
}

QPushButton *KEditListBox::removeButton() const
{
    return d->servRemoveButton;
}

QPushButton *KEditListBox::upButton() const
{
    return d->servUpButton;
}

QPushButton *KEditListBox::downButton() const
{
    return d->servDownButton;
}

int KEditListBox::count() const
{
    return int(d->m_model->rowCount());
}

void KEditListBox::setButtons( Buttons buttons )
{
    if ( d->buttons == buttons )
        return;

    QGridLayout* grid = static_cast<QGridLayout *>( layout() );
    if ( ( buttons & Add ) && !d->servNewButton ) {
        d->servNewButton = new QPushButton(i18n("&Add"), this);
        d->servNewButton->setEnabled(false);
        d->servNewButton->show();
        connect(d->servNewButton, SIGNAL(clicked()), SLOT(addItem()));

        grid->addWidget(d->servNewButton, 2, 1);
    } else if ( ( buttons & Add ) == 0 && d->servNewButton ) {
        delete d->servNewButton;
        d->servNewButton = 0;
    }

    if ( ( buttons & Remove ) && !d->servRemoveButton ) {
        d->servRemoveButton = new QPushButton(i18n("&Remove"), this);
        d->servRemoveButton->setEnabled(false);
        d->servRemoveButton->show();
        connect(d->servRemoveButton, SIGNAL(clicked()), SLOT(removeItem()));

        grid->addWidget(d->servRemoveButton, 3, 1);
    } else if ( ( buttons & Remove ) == 0 && d->servRemoveButton ) {
        delete d->servRemoveButton;
        d->servRemoveButton = 0;
    }

    if ( ( buttons & UpDown ) && !d->servUpButton ) {
        d->servUpButton = new QPushButton(i18n("Move &Up"), this);
        d->servUpButton->setEnabled(false);
        d->servUpButton->show();
        connect(d->servUpButton, SIGNAL(clicked()), SLOT(moveItemUp()));

        d->servDownButton = new QPushButton(i18n("Move &Down"), this);
        d->servDownButton->setEnabled(false);
        d->servDownButton->show();
        connect(d->servDownButton, SIGNAL(clicked()), SLOT(moveItemDown()));

        grid->addWidget(d->servUpButton, 4, 1);
        grid->addWidget(d->servDownButton, 5, 1);
    } else if ( ( buttons & UpDown ) == 0 && d->servUpButton ) {
        delete d->servUpButton; d->servUpButton = 0;
        delete d->servDownButton; d->servDownButton = 0;
    }

    d->buttons = buttons;
}

void KEditListBox::setCheckAtEntering(bool check)
{
    d->m_checkAtEntering = check;
}

bool KEditListBox::checkAtEntering()
{
    return d->m_checkAtEntering;
}

void KEditListBox::typedSomething(const QString& text)
{
    if(currentItem() >= 0) {
        if(currentText() != d->m_lineEdit->text())
        {
            // IMHO changeItem() shouldn't do anything with the value
            // of currentItem() ... like changing it or emitting signals ...
            // but TT disagree with me on this one (it's been that way since ages ... grrr)
            bool block = d->m_listView->signalsBlocked();
            d->m_listView->blockSignals( true );
            QItemSelectionModel *selection = d->m_listView->selectionModel();
            QModelIndex currentIndex = selection->currentIndex();
            if ( currentIndex.isValid() )
              d->m_model->setData(currentIndex,text);
            d->m_listView->blockSignals( block );
            emit changed();
        }
    }

    if ( !d->servNewButton )
        return;

    if (!d->m_checkAtEntering)
        d->servNewButton->setEnabled(!text.isEmpty());
    else
    {
        if (text.isEmpty())
        {
            d->servNewButton->setEnabled(false);
        }
        else
        {
            QStringList list = d->m_model->stringList();
            bool enable = !list.contains( text, Qt::CaseSensitive );
            d->servNewButton->setEnabled( enable );
        }
    }
}

void KEditListBox::moveItemUp()
{
    if (!d->m_listView->isEnabled())
    {
        KNotification::beep();
        return;
    }

    QItemSelectionModel *selection = d->m_listView->selectionModel();
    QModelIndex index = selection->currentIndex();
    if ( index.isValid() ) {
      if (index.row() == 0) {
          KNotification::beep();
          return;
      }

      QModelIndex aboveIndex = d->m_model->index( index.row() - 1, index.column() );

      QString tmp = d->m_model->data( aboveIndex, Qt::DisplayRole ).toString();
      d->m_model->setData( aboveIndex, d->m_model->data( index, Qt::DisplayRole ) );
      d->m_model->setData( index, tmp );

      selection->setCurrentIndex( aboveIndex, QItemSelectionModel::Select | QItemSelectionModel::Clear );
    }

    emit changed();
}

void KEditListBox::moveItemDown()
{
    if (!d->m_listView->isEnabled())
    {
        KNotification::beep();
        return;
    }

    QItemSelectionModel *selection = d->m_listView->selectionModel();
    QModelIndex index = selection->currentIndex();
    if ( index.isValid() ) {
      if (index.row() == d->m_model->rowCount() - 1) {
          KNotification::beep();
          return;
      }

      QModelIndex belowIndex = d->m_model->index( index.row() + 1, index.column() );

      QString tmp = d->m_model->data( belowIndex, Qt::DisplayRole ).toString();
      d->m_model->setData( belowIndex, d->m_model->data( index, Qt::DisplayRole ) );
      d->m_model->setData( index, tmp );

      selection->setCurrentIndex( belowIndex, QItemSelectionModel::Select | QItemSelectionModel::Clear );
    }

    emit changed();
}

void KEditListBox::addItem()
{
    // when m_checkAtEntering is true, the add-button is disabled, but this
    // slot can still be called through Key_Return/Key_Enter. So we guard
    // against this.
    if ( !d->servNewButton || !d->servNewButton->isEnabled() )
        return;


    QItemSelectionModel *selection = d->m_listView->selectionModel();
    QModelIndex currentIndex = selection->currentIndex();

    const QString& currentTextLE=d->m_lineEdit->text();
    bool alreadyInList(false);
    //if we didn't check for dupes at the inserting we have to do it now
    if (!d->m_checkAtEntering)
    {
        // first check current item instead of dumb iterating the entire list
        if ( currentIndex.isValid() ) {
          if ( d->m_model->data( currentIndex, Qt::DisplayRole ).toString() == currentTextLE )
            alreadyInList = true;
        }
        else
        {
            alreadyInList = d->m_model->stringList().contains( currentTextLE, Qt::CaseSensitive );
        }
    }
    if ( d->servNewButton )
        d->servNewButton->setEnabled(false);

    bool block = d->m_lineEdit->signalsBlocked();
    d->m_lineEdit->blockSignals(true);
    d->m_lineEdit->clear();
    d->m_lineEdit->blockSignals(block);

    selection->setCurrentIndex(currentIndex, QItemSelectionModel::Deselect);

    if (!alreadyInList)
    {
        block = d->m_listView->signalsBlocked();

        if ( currentIndex.isValid() ) {
          d->m_model->setData(currentIndex, currentTextLE );
        } else {
            QStringList lst;
            lst<<currentTextLE;
            lst<<d->m_model->stringList();
            d->m_model->setStringList(lst);
        }
        emit changed();
        emit added( currentTextLE );
    }
}

int KEditListBox::currentItem() const
{
    QItemSelectionModel *selection = d->m_listView->selectionModel();
    QModelIndex currentIndex = selection->currentIndex();
    if ( currentIndex.isValid() )
        return currentIndex.row();
    else
        return -1;
}

void KEditListBox::removeItem()
{
    QItemSelectionModel *selection = d->m_listView->selectionModel();
    QModelIndex currentIndex = selection->currentIndex();
    if ( !currentIndex.isValid() )
      return;

    if ( currentIndex.row() >= 0 )
    {
        QString removedText = d->m_model->data( currentIndex, Qt::DisplayRole ).toString();

        d->m_model->removeRows( currentIndex.row(), 1 );
        if ( d->m_model->rowCount() > 0 ) {
            QModelIndex aboveIndex = d->m_model->index( currentIndex.row() - 1, currentIndex.row() );
            selection->setCurrentIndex( aboveIndex, QItemSelectionModel::Select );
        }

        emit changed();

        emit removed( removedText );
    }

    currentIndex = selection->currentIndex();

    if ( d->servRemoveButton && !currentIndex.isValid() )
        d->servRemoveButton->setEnabled(false);
}

void KEditListBox::enableMoveButtons(const QModelIndex &newIndex, const QModelIndex&)
{
    int index = newIndex.row();

    // Update the lineEdit when we select a different line.
    if(currentText() != d->m_lineEdit->text())
        d->m_lineEdit->setText(currentText());

    bool moveEnabled = d->servUpButton && d->servDownButton;

    if (moveEnabled )
    {
        if (d->m_model->rowCount() <= 1)
        {
            d->servUpButton->setEnabled(false);
            d->servDownButton->setEnabled(false);
        }
        else if (index == (d->m_model->rowCount() - 1))
        {
            d->servUpButton->setEnabled(true);
            d->servDownButton->setEnabled(false);
        }
        else if (index == 0)
        {
            d->servUpButton->setEnabled(false);
            d->servDownButton->setEnabled(true);
        }
        else
        {
            d->servUpButton->setEnabled(true);
            d->servDownButton->setEnabled(true);
        }
    }

    if ( d->servRemoveButton )
        d->servRemoveButton->setEnabled(true);
}

void KEditListBox::clear()
{
    d->m_lineEdit->clear();
    d->m_model->setStringList( QStringList() );
    emit changed();
}

void KEditListBox::insertStringList(const QStringList& list, int index)
{
    QStringList content = d->m_model->stringList();
    if ( index > content.count() )
      content += list;
    else
      for ( int i = 0, j = index; i < list.count(); ++i, ++j )
        content.insert( j, list[ i ] );

    d->m_model->setStringList( content );
}

void KEditListBox::insertItem(const QString& text, int index)
{
  QStringList list = d->m_model->stringList();

  if ( index == -1 )
    list.append( text );
  else
    list.insert( index, text );

  d->m_model->setStringList(list);
}

QString KEditListBox::text(int index) const
{
  QStringList list = d->m_model->stringList();

  return list[ index ];
}

QString KEditListBox::currentText() const
{
  QItemSelectionModel *selection = d->m_listView->selectionModel();
  QModelIndex index = selection->currentIndex();
  if ( !index.isValid() )
    return QString();
  else
    return text( index.row() );
}

QStringList KEditListBox::items() const
{
    return d->m_model->stringList();
}

void KEditListBox::setItems(const QStringList& items)
{
  d->m_model->setStringList(items);
}

KEditListBox::Buttons KEditListBox::buttons() const
{
  return d->buttons;
}

#include "keditlistbox.moc"
