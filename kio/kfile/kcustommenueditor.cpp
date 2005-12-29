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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <khbox.h>
#include <qregexp.h>
#include <qimage.h>
#include <qpushbutton.h>
#include <qdir.h>

#include <kbuttonbox.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <klistview.h>
#include <kservice.h>
#include <kstandarddirs.h>
#include <kconfigbase.h>
#include <kopenwith.h>

#include "kcustommenueditor.h"

class KCustomMenuEditor::Item : public Q3ListViewItem
{
public:
   Item(Q3ListView *parent, KService::Ptr service)
     : Q3ListViewItem(parent),
       s(service)
   {
      init();
   }

   Item(Q3ListViewItem *parent, KService::Ptr service)
     : Q3ListViewItem(parent),
       s(service)
   {
      init();
   }

   void init()
   {
      QString serviceName = s->name();

      // item names may contain ampersands. To avoid them being converted
      // to accelators, replace them with two ampersands.
      serviceName.replace("&", "&&");

      QPixmap normal = KGlobal::instance()->iconLoader()->loadIcon(s->icon(), KIcon::Small,
                              0, KIcon::DefaultState, 0L, true);

      // make sure they are not larger than 16x16
      if (normal.width() > 16 || normal.height() > 16) {
          QImage tmp = normal.toImage();
          tmp = tmp.smoothScale(16, 16);
          normal.convertFromImage(tmp);
      }
      setText(0, serviceName);
      setPixmap(0, normal);
   }

   KService::Ptr s;
};

class KCustomMenuEditor::KCustomMenuEditorPrivate
{
public:
    QPushButton * pbRemove;
    QPushButton * pbMoveUp;
    QPushButton * pbMoveDown;
};

KCustomMenuEditor::KCustomMenuEditor(QWidget *parent)
  : KDialogBase(parent, "custommenueditor", true, i18n("Menu Editor"), Ok|Cancel, Ok, true),
    m_listView(0)
{
    d = new KCustomMenuEditorPrivate;
   KHBox *page = makeHBoxMainWidget();
   m_listView = new KListView(page);
   m_listView->addColumn(i18n("Menu"));
   m_listView->setFullWidth(true);
   m_listView->setSorting(-1);
   KButtonBox *buttonBox = new KButtonBox(page, Qt::Vertical);
   buttonBox->addButton(i18n("New..."), this, SLOT(slotNewItem()));
   d->pbRemove=buttonBox->addButton(i18n("Remove"), this, SLOT(slotRemoveItem()));
   d->pbMoveUp=buttonBox->addButton(i18n("Move Up"), this, SLOT(slotMoveUp()));
   d->pbMoveDown=buttonBox->addButton(i18n("Move Down"), this, SLOT(slotMoveDown()));
   buttonBox->layout();
   connect( m_listView, SIGNAL( selectionChanged () ), this, SLOT( refreshButton() ) );
   refreshButton();
}

KCustomMenuEditor::~KCustomMenuEditor()
{
    delete d;
    d=0;
}

void KCustomMenuEditor::refreshButton()
{
    Q3ListViewItem *item = m_listView->currentItem();
    d->pbRemove->setEnabled( item );
    d->pbMoveUp->setEnabled( item && item->itemAbove() );
    d->pbMoveDown->setEnabled( item && item->itemBelow() );
}

void
KCustomMenuEditor::load(KConfigBase *cfg)
{
   cfg->setGroup(QString::null);
   int count = cfg->readEntry("NrOfItems", QVariant(0)).toInt();
   Q3ListViewItem *last = 0;
   for(int i = 0; i < count; i++)
   {
      QString entry = cfg->readPathEntry(QString("Item%1").arg(i+1));
      if (entry.isEmpty())
         continue;

      // Try KSycoca first.
      KService::Ptr menuItem = KService::serviceByDesktopPath( entry );
      if (!menuItem)
         menuItem = KService::serviceByDesktopName( entry );
      if (!menuItem)
         menuItem = new KService( entry );

      if (!menuItem->isValid())
         continue;

      Q3ListViewItem *item = new Item(m_listView, menuItem);
      item->moveItem(last);
      last = item;
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
      QString path = item->s->desktopEntryPath();
      if (QDir::isRelativePath(path) || QDir::isRelativePath(KGlobal::dirs()->relativeLocation("xdgdata-apps", path)))
         path = item->s->desktopEntryName();
      cfg->writePathEntry(QString("Item%1").arg(i), path);
      item = (Item *) item->nextSibling();
   }
   cfg->writeEntry("NrOfItems", i);
}

void
KCustomMenuEditor::slotNewItem()
{
   Q3ListViewItem *item = m_listView->currentItem();

   KOpenWithDlg dlg(this);
   dlg.setSaveNewApplications(true);

   if (dlg.exec())
   {
      KService::Ptr s = dlg.service();
      if (s && s->isValid())
      {
         Item *newItem = new Item(m_listView, s);
         newItem->moveItem(item);
      }
      refreshButton();
   }
}

void
KCustomMenuEditor::slotRemoveItem()
{
   Q3ListViewItem *item = m_listView->currentItem();
   if (!item)
      return;

   delete item;
   refreshButton();
}

void
KCustomMenuEditor::slotMoveUp()
{
   Q3ListViewItem *item = m_listView->currentItem();
   if (!item)
      return;

   Q3ListViewItem *searchItem = m_listView->firstChild();
   while(searchItem)
   {
      Q3ListViewItem *next = searchItem->nextSibling();
      if (next == item)
      {
         searchItem->moveItem(item);
         break;
      }
      searchItem = next;
   }
   refreshButton();
}

void
KCustomMenuEditor::slotMoveDown()
{
   Q3ListViewItem *item = m_listView->currentItem();
   if (!item)
      return;

   Q3ListViewItem *after = item->nextSibling();
   if (!after)
      return;

   item->moveItem( after );
   refreshButton();
}

#include "kcustommenueditor.moc"
