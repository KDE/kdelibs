/* This file is part of the KDE libraries
    Copyright (C) 2000 David Faure <faure@kde.org>, Alexander Neundorf <neundorf@kde.org>

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

#include <qstringlist.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qlistbox.h>
#include <qwhatsthis.h>
#include <qlabel.h>

#include <kdebug.h>
#include <kdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kapplication.h>
#include <knotifyclient.h>

#include "keditlistbox.h"

class KEditListBoxPrivate
{
   public:
      bool m_checkAtEntering;
      int buttons;
};

KEditListBox::KEditListBox(QWidget *parent, const char *name,
			   bool checkAtEntering, int buttons )
    :QGroupBox(parent, name )
{
   init( checkAtEntering, buttons );
}

KEditListBox::KEditListBox(const QString& title, QWidget *parent,
			   const char *name, bool checkAtEntering, int buttons)
    :QGroupBox(title, parent, name )
{
   init( checkAtEntering, buttons );
}

KEditListBox::~KEditListBox()
{
   delete d;
   d=0;
}

void KEditListBox::init( bool checkAtEntering, int buttons )
{
   d=new KEditListBoxPrivate;
   d->m_checkAtEntering=checkAtEntering;
   d->buttons = buttons;

   int lostButtons = 0;
   if ( (buttons & Add) == 0 )
       lostButtons++;
   if ( (buttons & Remove) == 0 )
       lostButtons++;
   if ( (buttons & UpDown) == 0 )
       lostButtons += 2;


   servNewButton = servRemoveButton = servUpButton = servDownButton = 0L;
   setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
			     QSizePolicy::MinimumExpanding));

   QWidget * gb = this;
   QGridLayout * grid = new QGridLayout(gb, 7 - lostButtons, 2,
					KDialog::marginHint(),
					KDialog::spacingHint());
   grid->addRowSpacing(0, fontMetrics().lineSpacing());
   for ( int i = 1; i < 7 - lostButtons; i++ )
       grid->setRowStretch(i, 1);

   grid->setMargin(15);

   m_lineEdit=new KLineEdit(gb);
   m_listBox = new QListBox(gb);

   grid->addMultiCellWidget(m_lineEdit,1,1,0,1);
   grid->addMultiCellWidget(m_listBox, 2, 6 - lostButtons, 0, 0);
   int row = 2;
   if ( buttons & Add ) {
       servNewButton = new QPushButton(i18n("&Add"), gb);
       servNewButton->setEnabled(false);
       connect(servNewButton, SIGNAL(clicked()), SLOT(addItem()));

       grid->addWidget(servNewButton, row++, 1);
   }

   if ( buttons & Remove ) {
       servRemoveButton = new QPushButton(i18n("&Remove"), gb);
       servRemoveButton->setEnabled(false);
       connect(servRemoveButton, SIGNAL(clicked()), SLOT(removeItem()));

       grid->addWidget(servRemoveButton, row++, 1);
   }

   if ( buttons & UpDown ) {
       servUpButton = new QPushButton(i18n("Move &Up"), gb);
       servUpButton->setEnabled(false);
       connect(servUpButton, SIGNAL(clicked()), SLOT(moveItemUp()));

       servDownButton = new QPushButton(i18n("Move &Down"), gb);
       servDownButton->setEnabled(false);
       connect(servDownButton, SIGNAL(clicked()), SLOT(moveItemDown()));

       grid->addWidget(servUpButton, row++, 1);
       grid->addWidget(servDownButton, row++, 1);
   }

   connect(m_lineEdit,SIGNAL(textChanged(const QString&)),this,SLOT(typedSomething(const QString&)));
   m_lineEdit->setTrapReturnKey(true);
   connect(m_lineEdit,SIGNAL(returnPressed()),this,SLOT(addItem()));
   connect(m_listBox, SIGNAL(highlighted(int)), SLOT(enableMoveButtons(int)));
}

void KEditListBox::typedSomething(const QString& text)
{
    if(currentItem() >= 0) {
        if(currentText() != m_lineEdit->text()) 
        {
            m_listBox->changeItem(text, currentItem());
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
         const QString& currentText=m_lineEdit->text();
         for (int i=0; i<count(); i++)
         {
            if (m_listBox->text(i)==currentText)
            {
               servNewButton->setEnabled(false);
               return;
            }
         }
         servNewButton->setEnabled(true);
      }
   }
}

void KEditListBox::moveItemUp()
{
   if (!m_listBox->isEnabled())
   {
      KNotifyClient::beep();
      return;
   }

   unsigned int selIndex = m_listBox->currentItem();
   if (selIndex == 0)
   {
      KNotifyClient::beep();
      return;
   }

   QListBoxItem *selItem = m_listBox->item(selIndex);
   m_listBox->takeItem(selItem);
   m_listBox->insertItem(selItem, selIndex-1);
   m_listBox->setCurrentItem(selIndex - 1);

   emit changed();
}

void KEditListBox::moveItemDown()
{
   if (!m_listBox->isEnabled())
   {
      KNotifyClient::beep();
      return;
   }

   unsigned int selIndex = m_listBox->currentItem();
   if (selIndex == m_listBox->count() - 1)
   {
      KNotifyClient::beep();
      return;
   }

   QListBoxItem *selItem = m_listBox->item(selIndex);
   m_listBox->takeItem(selItem);
   m_listBox->insertItem(selItem, selIndex+1);
   m_listBox->setCurrentItem(selIndex + 1);

   emit changed();
}

void KEditListBox::addItem()
{
   const QString& currentTextLE=m_lineEdit->text();
   bool alreadyInList(false);
   //if we didn't check for dupes at the inserting we have to do it now
   if (!d->m_checkAtEntering)
   {
      for (int i=0; i<count(); i++)
      {
         if (m_listBox->text(i)==currentTextLE)
         {
            alreadyInList=true;
            break;
         };
      };
   };

   if ( servNewButton )
       servNewButton->setEnabled(false);

   m_lineEdit->blockSignals(true);
   m_lineEdit->clear();
   m_lineEdit->blockSignals(false);
   m_listBox->setSelected(currentItem(), false);
   if (!alreadyInList)
   {
      m_listBox->insertItem(currentTextLE);
      emit changed();
   }
}

int KEditListBox::currentItem() const {
    int nr = m_listBox->currentItem();
    if(nr >= 0 && !m_listBox->item(nr)->isSelected()) return -1;
    return nr;
}

void KEditListBox::removeItem()
{
   int selected = m_listBox->currentItem();

   if ( selected >= 0 )
   {
      m_listBox->removeItem( selected );
      if ( count() > 0 )
          m_listBox->setSelected( QMIN( selected, count() - 1 ), true );
      
      emit changed();
   }

   if ( servRemoveButton && m_listBox->currentItem() == -1 )
      servRemoveButton->setEnabled(false);
}

void KEditListBox::enableMoveButtons(int index)
{
    // Update the lineEdit when we select a different line.
    if(currentText() != m_lineEdit->text()) 
        m_lineEdit->setText(currentText());

    bool moveEnabled = servUpButton && servDownButton;

   if (moveEnabled )
   {
       if (m_listBox->count() <= 1)
       {
	   servUpButton->setEnabled(false);
	   servDownButton->setEnabled(false);
       }
       else if ((uint) index == (m_listBox->count() - 1))
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
   m_listBox->clear();
   emit changed();
}

void KEditListBox::insertStringList(const QStringList& list, int index)
{
   m_listBox->insertStringList(list,index);
}

void KEditListBox::insertStrList(const QStrList* list, int index)
{
   m_listBox->insertStrList(list,index);
}

void KEditListBox::insertStrList(const QStrList& list, int index)
{
   m_listBox->insertStrList(list,index);
}

void KEditListBox::insertStrList(const char ** list, int numStrings, int index)
{
   m_listBox->insertStrList(list,numStrings,index);
}

QStringList KEditListBox::items() const
{
    QStringList list;
    for ( uint i = 0; i < m_listBox->count(); i++ )
	list.append( m_listBox->text( i ));

    return list;
}

void KEditListBox::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "keditlistbox.moc"
