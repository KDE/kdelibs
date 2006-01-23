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

#include <qstringlist.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <q3groupbox.h>
#include <qlabel.h>
#include <QListView>

#include <kcombobox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <knotification.h>

#include "keditlistbox.h"

#include <assert.h>

class KEditListBoxPrivate
{
public:
    bool m_checkAtEntering;
    KEditListBox::Buttons buttons;
};

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
    m_lineEdit = custom.lineEdit();
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

    servNewButton = servRemoveButton = servUpButton = servDownButton = 0L;
    setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
                              QSizePolicy::MinimumExpanding));

    QGridLayout * grid = new QGridLayout(this, 7, 2,
                                         KDialog::marginHint(),
                                         KDialog::spacingHint());
    grid->addRowSpacing(0, fontMetrics().lineSpacing());
    grid->setRowStretch( 6, 1 );

    grid->setMargin(15);

    if ( representationWidget )
        representationWidget->reparent( this, QPoint(0,0) );
    else
        m_lineEdit=new KLineEdit(this);

    m_model = new QStringListModel();
    m_listView = new QListView(this);

    m_listView->setModel(m_model);

    QWidget *editingWidget = representationWidget ?
                             representationWidget : m_lineEdit;
    grid->addMultiCellWidget(editingWidget,1,1,0,1);
    grid->addMultiCellWidget(m_listView, 2, 6, 0, 0);

    setButtons( buttons );

    connect(m_lineEdit,SIGNAL(textChanged(const QString&)),this,SLOT(typedSomething(const QString&)));
    m_lineEdit->setTrapReturnKey(true);
    connect(m_lineEdit,SIGNAL(returnPressed()),this,SLOT(addItem()));
    connect(m_listView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
            this,SLOT(enableMoveButtons(const QModelIndex&, const QModelIndex&)));

    // maybe supplied lineedit has some text already
    typedSomething( m_lineEdit->text() );
}

void KEditListBox::setButtons( Buttons buttons )
{
    if ( d->buttons == buttons )
        return;

    QGridLayout* grid = static_cast<QGridLayout *>( layout() );
    if ( ( buttons & Add ) && !servNewButton ) {
        servNewButton = new QPushButton(i18n("&Add"), this);
        servNewButton->setEnabled(false);
        servNewButton->show();
        connect(servNewButton, SIGNAL(clicked()), SLOT(addItem()));

        grid->addWidget(servNewButton, 2, 1);
    } else if ( ( buttons & Add ) == 0 && servNewButton ) {
        delete servNewButton;
        servNewButton = 0;
    }

    if ( ( buttons & Remove ) && !servRemoveButton ) {
        servRemoveButton = new QPushButton(i18n("&Remove"), this);
        servRemoveButton->setEnabled(false);
        servRemoveButton->show();
        connect(servRemoveButton, SIGNAL(clicked()), SLOT(removeItem()));

        grid->addWidget(servRemoveButton, 3, 1);
    } else if ( ( buttons & Remove ) == 0 && servRemoveButton ) {
        delete servRemoveButton;
        servRemoveButton = 0;
    }

    if ( ( buttons & UpDown ) && !servUpButton ) {
        servUpButton = new QPushButton(i18n("Move &Up"), this);
        servUpButton->setEnabled(false);
        servUpButton->show();
        connect(servUpButton, SIGNAL(clicked()), SLOT(moveItemUp()));

        servDownButton = new QPushButton(i18n("Move &Down"), this);
        servDownButton->setEnabled(false);
        servDownButton->show();
        connect(servDownButton, SIGNAL(clicked()), SLOT(moveItemDown()));

        grid->addWidget(servUpButton, 4, 1);
        grid->addWidget(servDownButton, 5, 1);
    } else if ( ( buttons & UpDown ) == 0 && servUpButton ) {
        delete servUpButton; servUpButton = 0;
        delete servDownButton; servDownButton = 0;
    }

    d->buttons = buttons;
}

void KEditListBox::typedSomething(const QString& text)
{
    if(currentItem() >= 0) {
        if(currentText() != m_lineEdit->text())
        {
            // IMHO changeItem() shouldn't do anything with the value
            // of currentItem() ... like changing it or emitting signals ...
            // but TT disagree with me on this one (it's been that way since ages ... grrr)
            bool block = m_listView->signalsBlocked();
            m_listView->blockSignals( true );
            QItemSelectionModel *selection = m_listView->selectionModel();
            QModelIndex currentIndex = selection->currentIndex();
            if ( currentIndex.isValid() )
              m_model->setData(currentIndex,text);
            m_listView->blockSignals( block );
            emit changed();
        }
    }

    if ( !servNewButton )
        return;

    if (!d->m_checkAtEntering)
        servNewButton->setEnabled(!text.isEmpty());
    else
    {
        if (text.isEmpty())
        {
            servNewButton->setEnabled(false);
        }
        else
        {
            QStringList list = m_model->stringList();
            bool enable = list.contains( text, Qt::CaseSensitive );
            servNewButton->setEnabled( enable );
        }
    }
}

void KEditListBox::moveItemUp()
{
    if (!m_listView->isEnabled())
    {
        KNotification::beep();
        return;
    }

    QItemSelectionModel *selection = m_listView->selectionModel();
    QModelIndex index = selection->currentIndex();
    if ( index.isValid() ) {
      if (index.row() == 0) {
          KNotification::beep();
          return;
      }

      QModelIndex aboveIndex = m_model->index( index.row() - 1, index.column() );

      QString tmp = m_model->data( aboveIndex, Qt::DisplayRole ).toString();
      m_model->setData( aboveIndex, m_model->data( index, Qt::DisplayRole ) );
      m_model->setData( index, tmp );

      selection->setCurrentIndex( aboveIndex, QItemSelectionModel::Select | QItemSelectionModel::Clear );
    }

    emit changed();
}

void KEditListBox::moveItemDown()
{
    if (!m_listView->isEnabled())
    {
        KNotification::beep();
        return;
    }

    QItemSelectionModel *selection = m_listView->selectionModel();
    QModelIndex index = selection->currentIndex();
    if ( index.isValid() ) {
      if (index.row() == m_model->rowCount() - 1) {
          KNotification::beep();
          return;
      }

      QModelIndex belowIndex = m_model->index( index.row() + 1, index.column() );

      QString tmp = m_model->data( belowIndex, Qt::DisplayRole ).toString();
      m_model->setData( belowIndex, m_model->data( index, Qt::DisplayRole ) );
      m_model->setData( index, tmp );

      selection->setCurrentIndex( belowIndex, QItemSelectionModel::Select | QItemSelectionModel::Clear );
    }

    emit changed();
}

void KEditListBox::addItem()
{
    // when m_checkAtEntering is true, the add-button is disabled, but this
    // slot can still be called through Key_Return/Key_Enter. So we guard
    // against this.
    if ( !servNewButton || !servNewButton->isEnabled() )
        return;


    QItemSelectionModel *selection = m_listView->selectionModel();
    QModelIndex currentIndex = selection->currentIndex();

    const QString& currentTextLE=m_lineEdit->text();
    bool alreadyInList(false);
    //if we didn't check for dupes at the inserting we have to do it now
    if (!d->m_checkAtEntering)
    {
        // first check current item instead of dumb iterating the entire list
        if ( currentIndex.isValid() ) {
          if ( m_model->data( currentIndex, Qt::DisplayRole ).toString() == currentTextLE )
            alreadyInList = true;
        }
        else
        {
            alreadyInList = m_model->stringList().contains( currentTextLE, Qt::CaseSensitive );
        }
    }

    if ( servNewButton )
        servNewButton->setEnabled(false);

    bool block = m_lineEdit->signalsBlocked();
    m_lineEdit->blockSignals(true);
    m_lineEdit->clear();
    m_lineEdit->blockSignals(block);

    selection->setCurrentIndex(currentIndex, QItemSelectionModel::Deselect);

    if (!alreadyInList)
    {
        block = m_listView->signalsBlocked();
        if ( currentIndex.isValid() ) {
          m_model->setData( currentIndex, currentTextLE );
        }
        emit changed();
        emit added( currentTextLE );
    }
}

int KEditListBox::currentItem() const
{
    QItemSelectionModel *selection = m_listView->selectionModel();
    QModelIndex currentIndex = selection->currentIndex();
    if ( currentIndex.isValid() )
        return currentIndex.row();
    else
        return -1;
}

void KEditListBox::removeItem()
{
    QItemSelectionModel *selection = m_listView->selectionModel();
    QModelIndex currentIndex = selection->currentIndex();
    if ( !currentIndex.isValid() )
      return;

    if ( currentIndex.row() >= 0 )
    {
        QString removedText = m_model->data( currentIndex, Qt::DisplayRole ).toString();

        m_model->removeRows( currentIndex.row(), 1 );
        if ( m_model->rowCount() > 0 ) {
            QModelIndex aboveIndex = m_model->index( currentIndex.row() - 1, currentIndex.row() );
            selection->setCurrentIndex( aboveIndex, QItemSelectionModel::Select );
        }

        emit changed();

        emit removed( removedText );
    }

    currentIndex = selection->currentIndex();

    if ( servRemoveButton && !currentIndex.isValid() )
        servRemoveButton->setEnabled(false);
}

void KEditListBox::enableMoveButtons(const QModelIndex &newIndex, const QModelIndex&)
{
    int index = newIndex.row();

    // Update the lineEdit when we select a different line.
    if(currentText() != m_lineEdit->text())
        m_lineEdit->setText(currentText());

    bool moveEnabled = servUpButton && servDownButton;

    if (moveEnabled )
    {
        if (m_model->rowCount() <= 1)
        {
            servUpButton->setEnabled(false);
            servDownButton->setEnabled(false);
        }
        else if (index == (m_model->rowCount() - 1))
        {
            servUpButton->setEnabled(true);
            servDownButton->setEnabled(false);
        }
        else if (index == 0)
        {
            servUpButton->setEnabled(false);
            servDownButton->setEnabled(true);
        }
        else
        {
            servUpButton->setEnabled(true);
            servDownButton->setEnabled(true);
        }
    }

    if ( servRemoveButton )
        servRemoveButton->setEnabled(true);
}

void KEditListBox::clear()
{
    m_lineEdit->clear();
    m_model->setStringList( QStringList() );
    emit changed();
}

void KEditListBox::insertStringList(const QStringList& list, int index)
{
    QStringList content = m_model->stringList();
    if ( index > content.count() )
      content += list;
    else
      for ( int i = 0, j = index; i < list.count(); ++i, ++j )
        content.insert( j, list[ i ] );

    m_model->setStringList( content );
}

void KEditListBox::insertItem(const QString& text, int index)
{
  QStringList list = m_model->stringList();

  if ( index == -1 )
    list.append( text );
  else
    list.insert( index, text );

  m_model->setStringList(list);
}

QString KEditListBox::text(int index) const
{
  QStringList list = m_model->stringList();

  return list[ index ];
}

QString KEditListBox::currentText() const
{
  QItemSelectionModel *selection = m_listView->selectionModel();
  QModelIndex index = selection->currentIndex();
  if ( !index.isValid() )
    return QString();
  else
    return text( index.row() );
}

QStringList KEditListBox::items() const
{
    return m_model->stringList();
}

void KEditListBox::setItems(const QStringList& items)
{
  m_model->setStringList(items);
}

KEditListBox::Buttons KEditListBox::buttons() const
{
  return d->buttons;
}

void KEditListBox::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

KEditListBox::CustomEditor::CustomEditor( KComboBox *combo )
{
    m_representationWidget = combo;
    m_lineEdit = dynamic_cast<KLineEdit*>( combo->lineEdit() );
    assert( m_lineEdit );
}

#include "keditlistbox.moc"
