/*  This file is part of the KDE libraries
    Copyright (C) 2002 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2 
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qhbox.h>
#include <qregexp.h>
#include <qimage.h>
#include <qpushbutton.h>

#include <kbuttonbox.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klistview.h>
#include <kservice.h>
#include <kconfigbase.h>
#include <kopenwith.h>

#include "kcustommenueditor.h"

class KCustomMenuEditor::Item : public KListViewItem
{
public:
   Item(QListView *parent, KService::Ptr service)
     : KListViewItem(parent),
       s(service)
   {
      init();
   }

   Item(QListViewItem *parent, KService::Ptr service)
     : KListViewItem(parent),
       s(service)
   {
      init();
   }

   void init()
   {
      QString serviceName = s->name();

      // item names may contain ampersands. To avoid them being converted
      // to accelators, replace them with two ampersands.
      serviceName.replace(QRegExp("&"), "&&");

      QPixmap normal = KGlobal::instance()->iconLoader()->loadIcon(s->icon(), KIcon::Small,
                              0, KIcon::DefaultState, 0L, true);

      // make sure they are not larger than 16x16
      if (normal.width() > 16 || normal.height() > 16) {
          QImage tmp = normal.convertToImage();
          tmp = tmp.smoothScale(16, 16);
          normal.convertFromImage(tmp);
      }
      setText(0, serviceName);
      setPixmap(0, normal);
   }

   KService::Ptr s;
};

KCustomMenuEditor::KCustomMenuEditor(QWidget *parent)
  : KDialogBase(parent, "custommenueditor", true, i18n("Menu Editor"), Ok|Cancel, Ok, true),
    m_listView(0)
{
}

void KCustomMenuEditor::init()
{  
   if (m_listView) return;
   QHBox *page = makeHBoxMainWidget();
   m_listView = new KListView(page);
   m_listView->addColumn(i18n("Menu"));
   m_listView->setFullWidth(true);
   KButtonBox *buttonBox = new KButtonBox(page, Vertical);
   buttonBox->addButton(i18n("New..."), this, SLOT(slotNewItem()));
   buttonBox->addButton(i18n("Remove"), this, SLOT(slotRemoveItem()));
   buttonBox->addButton(i18n("Move Up"), this, SLOT(slotMoveUp()));
   buttonBox->addButton(i18n("Move Down"), this, SLOT(slotMoveDown()));
}

void 
KCustomMenuEditor::load(KConfigBase *cfg)
{
  cfg->setGroup(QString::null);
  int count = cfg->readNumEntry("NrOfItems");
  qWarning("Count = %d", count);
  for(int i = 0; i < count; i++)
  {
     QString entry = cfg->readEntry(QString("Item%1").arg(i+1));
     if (entry.isEmpty())
        continue;

qWarning("Entry = %s", entry.latin1());
     // Try KSycoca first.
     KService::Ptr menuItem = KService::serviceByDesktopPath( entry );
     if (!menuItem)
        menuItem = KService::serviceByDesktopName( entry );
     if (!menuItem)
        menuItem = new KService( entry );

qWarning("Valid = %d", menuItem->isValid());
     if (!menuItem->isValid())
        continue;

qWarning("New Item."); 
     new Item(m_listView, menuItem);
  }
}

void 
KCustomMenuEditor::save(KConfigBase *cfg)
{
  // First clear the whole config file.
  QStringList groups = cfg->groupList();
  for(QStringList::ConstIterator it = groups.begin();
      it != groups.end(); ++it)
  {
     cfg->deleteGroup(*it);
  }

  cfg->setGroup(QString::null);
  Item * item = (Item *) m_listView->firstChild();
  int i = 0;
  while(item)
  {
    i++;
    cfg->writeEntry(QString("Item%1").arg(i), item->s->desktopEntryPath());
    item = (Item *) item->nextSibling();
  }
  cfg->writeEntry("NrOfItems", i);
}

void
KCustomMenuEditor::slotNewItem()
{
   QListViewItem *item = m_listView->currentItem();
   
   KOpenWithDlg dlg(this);
   
   if (dlg.exec())
   {
      KService::Ptr s = dlg.service();
      if (s && s->isValid())
      {
         Item *newItem = new Item(m_listView, s);
         newItem->moveItem(item);
      }
   }
}

void
KCustomMenuEditor::slotRemoveItem()
{
   QListViewItem *item = m_listView->currentItem();
   if (!item)
      return;
      
   delete item;
}

void
KCustomMenuEditor::slotMoveUp()
{
   QListViewItem *item = m_listView->currentItem();
   if (!item)
      return;
      
   QListViewItem *searchItem = m_listView->firstChild();
   while(searchItem)
   {
      QListViewItem *next = searchItem->nextSibling();
      if (next == item)
      {
         searchItem->moveItem(item);
         break;
      }
      searchItem = next;
   }      
}

void
KCustomMenuEditor::slotMoveDown()
{
   QListViewItem *item = m_listView->currentItem();
   if (!item)
      return;

   QListViewItem *after = item->nextSibling();
   if (!after)
      return;
      
   item->moveItem( after );
}

#include "kcustommenueditor.moc"
