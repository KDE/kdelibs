/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>                   
                                                                        
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or   
    (at your option) any later version.                                 
                                                                        
    This program is distributed in the hope that it will be useful,     
    but WITHOUT ANY WARRANTY; without even the implied warranty of      
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the        
    GNU General Public License for more details.                        
                                                                        
    You should have received a copy of the GNU General Public License   
    along with this program; if not, write to the Free Software         
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include <qlayout.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qheader.h>

#include <klistview.h>
#include <kiconloader.h>
#include <kdialog.h>
#include <klocale.h>
#include <kdebug.h>

#include "addressbook.h"
#include "distributionlisteditorwidget.h"

namespace KABC {

/////////////////////////////
// AddresseeListViewItem
class AddresseeListViewItem : public KListViewItem
{
  public:
    AddresseeListViewItem(const Addressee &a, KListView *parent, 
                          const QString &text)
      : KListViewItem(parent, text)
    {
      mAddressee = a;
    }
    
    ~AddresseeListViewItem() {}
    
    const Addressee &addressee() const { return mAddressee; }
  
  private:
    Addressee mAddressee;
                          
};

//////////////////////////////
// DistributionListEditorWidget

class DistributionListEditorWidgetPrivate
{
  public:
    DistributionListEditorWidgetPrivate() {}
    ~DistributionListEditorWidgetPrivate() {}
    
    void setEnabled(bool enabled);
    
    DistributionList *mList;
    AddressBook *mAddressBook;
    
    // GUI
    KListView *mAvailableView;
    KListView *mSelectedView;
    QToolButton *mAddButton;
    QToolButton *mRemoveButton;
};
  
DistributionListEditorWidget::DistributionListEditorWidget(AddressBook *book,
                                                           QWidget *parent, 
                                                           const char *name)
  : QWidget(parent, name)
{
  d = new DistributionListEditorWidgetPrivate();
  d->mAddressBook = book;
  
  initGUI();
  
  availableSelectionChanged();
  selectedSelectionChanged();
}

DistributionListEditorWidget::~DistributionListEditorWidget()
{
  delete d;
}

void DistributionListEditorWidget::clear()
{
  // Clear all the widgets
  d->mAvailableView->clear();
  d->mSelectedView->clear();
  
  // Set the list to null
  d->mList = 0;
}

void DistributionListEditorWidget::setDistributionList(DistributionList *list)
{
  bool b = signalsBlocked();
  blockSignals(true);
  
  clear();
  
  d->mList = list;
  
  // populate the left and right list boxes
  QListViewItem *item;
  QStringList emailList;
  QStringList::Iterator emailIter;
  Addressee a;
  AddressBook::Iterator iter;
  for (iter = d->mAddressBook->begin(); iter != d->mAddressBook->end(); ++iter)
  {
    a = *iter;
    
    item = new AddresseeListViewItem(*iter, d->mAvailableView, a.realName());
    
    // Now add all the email addresses
    emailList = a.emails();
    for (emailIter = emailList.begin(); emailIter != emailList.end();
          ++emailIter)
    {
      (void) new KListViewItem(item, *emailIter);
    }
  }
  
  // now move all the selected addressees to the other list
  DistributionList::Entry::List entryList = d->mList->entries();
  DistributionList::Entry::List::Iterator entryIter;
  for (entryIter = entryList.begin(); entryIter != entryList.end(); ++entryIter)
  {
    if (selectEntry(*entryIter))
      add();
  }
  
  // now delselect everything
  d->mAvailableView->selectAll(false);
  d->mSelectedView->selectAll(false);
  
  // close all the items. Only the selected list will be expanded
  for (item = d->mSelectedView->firstChild(); item; item = item->nextSibling())
    item->setOpen(false);
  
  // scroll to the top
  d->mAvailableView->ensureItemVisible(d->mAvailableView->firstChild());
  d->mSelectedView->ensureItemVisible(d->mSelectedView->firstChild());
  
  availableSelectionChanged();
  selectedSelectionChanged();
  
  blockSignals(b);
}

DistributionList *DistributionListEditorWidget::distributionList()
{
  // Save the list from the gui and then return it
  // There is really nothing to save since the distribution list i modified
  // in real time.
  return d->mList;
}

void DistributionListEditorWidget::add()
{
  // update the dist list
  QListViewItem *item = d->mAvailableView->selectedItem();
  AddresseeListViewItem *aItem = 
                      dynamic_cast<AddresseeListViewItem*>(item->parent());
                      
  d->mList->insertEntry(aItem->addressee(), item->text(0));
  
  moveItem(d->mAvailableView, d->mSelectedView);
  
  emit modified();
}

void DistributionListEditorWidget::remove()
{
  // update the dist list
  QListViewItem *item = d->mSelectedView->selectedItem();
  AddresseeListViewItem *aItem = 
                      dynamic_cast<AddresseeListViewItem*>(item->parent());
  d->mList->removeEntry(aItem->addressee(), item->text(0));
  
  moveItem(d->mSelectedView, d->mAvailableView);
  
  emit modified();
}

void DistributionListEditorWidget::moveItem(KListView *source, 
                                            KListView *target)
{
  QListViewItem *item;
  AddresseeListViewItem *aItem;
  AddresseeListViewItem *targetItem;
  QString email;
  
  item = source->selectedItem();
  if (!item)
    return;
    
  aItem = dynamic_cast<AddresseeListViewItem*>(item->parent());
  
  // Find the root item
  targetItem = findItem(target, aItem->addressee().uid());
  
  if (!targetItem)
  {
    // Add a parent item
    targetItem = new AddresseeListViewItem(aItem->addressee(), target,
                                           aItem->addressee().realName());
  }
  
  // Now add the child item to the selected list
  email = item->text(0);
  aItem->takeItem(item);
  delete item;
  
  // If deleting the item leaves the parent with 0 children, get rid of the
  // parent
  if (aItem->childCount() == 0)
  {
    source->takeItem(aItem);
    delete aItem;
  }
  
  item = new KListViewItem(targetItem, email);
  target->setSelected(item, true);
  target->ensureItemVisible(item);
}

AddresseeListViewItem *DistributionListEditorWidget::findItem(KListView *view,
                                                           const QString &uid)
{
  bool found = false;
  QListViewItem *item;
  AddresseeListViewItem *aItem = 0;
  for (item = view->firstChild(); item && !found; )
  {
    aItem = dynamic_cast<AddresseeListViewItem*>(item);
    if (aItem->addressee().uid() == uid)
      found = true;
    else
      item = item->nextSibling();
  }
  
  aItem = dynamic_cast<AddresseeListViewItem*>(item);
  return aItem;
}
                                                     
bool DistributionListEditorWidget::selectEntry(DistributionList::Entry &e)
{
  // find the addressee in the list
  bool found = false;
  QListViewItem *item;
  AddresseeListViewItem *aItem = 0;
  QString uid = e.addressee.uid();
  for (item = d->mAvailableView->firstChild();
       item && !found;)
  {
    aItem = dynamic_cast<AddresseeListViewItem*>(item);
    if (aItem)
    {
      if (uid == aItem->addressee().uid())
        found = true;
    }
    
    if (!found)
      item = item->nextSibling();
  }
  
  if (found)
  {
    // We found the addressee, now try to find the email address
    found = false;
    for (item = aItem->firstChild(); item && !found; item = item->nextSibling())
    {
      if (item->text(0) == e.email)
      {
        found = true;
        d->mAvailableView->setSelected(item, true);
      }
    }
  }
  
  return found;
}

void DistributionListEditorWidget::availableSelectionChanged()
{
  QListViewItem *item = d->mAvailableView->selectedItem();
  d->mAddButton->setEnabled(item && item->parent());
}

void DistributionListEditorWidget::selectedSelectionChanged()
{
  QListViewItem *item = d->mSelectedView->selectedItem();
  d->mRemoveButton->setEnabled(item && item->parent());
}
    
void DistributionListEditorWidget::initGUI()
{
  QBoxLayout *topLayout = new QHBoxLayout(this);
  topLayout->setSpacing( KDialog::spacingHint() );
  
  // Available list
  QBoxLayout *listLayout = new QVBoxLayout();
  listLayout->setSpacing( KDialog::spacingHint() );
  topLayout->addLayout( listLayout );
  topLayout->setStretchFactor(listLayout, 1);
  
  QLabel *label = new QLabel(i18n("&Available Contacts:"), this);
  listLayout->addWidget(label);
  d->mAvailableView = new KListView(this, "mAvailableView");
  d->mAvailableView->setSelectionModeExt(KListView::Single);
  d->mAvailableView->addColumn(i18n("Contacts"));
  d->mAvailableView->header()->hide();
  d->mAvailableView->setRootIsDecorated(true);
  connect(d->mAvailableView, SIGNAL(selectionChanged()),
          SLOT(availableSelectionChanged()));
  label->setBuddy(d->mAvailableView);
  listLayout->addWidget(d->mAvailableView);
  
  
  // Buttons
  listLayout = new QVBoxLayout();
  listLayout->setSpacing( KDialog::spacingHint() );
  topLayout->addLayout( listLayout );
  
  QWidget *spacer = new QWidget(this);
  spacer->setMinimumHeight(30);
  listLayout->addWidget(spacer);
  listLayout->setStretchFactor(spacer, 1);
  
  d->mAddButton = new QToolButton(this, "mAddButton");
  QToolTip::add(d->mAddButton, i18n("Add selected contact"));
  d->mAddButton->setIconSet(SmallIconSet("forward"));
  connect(d->mAddButton, SIGNAL(clicked()), SLOT(add()));
  listLayout->addWidget(d->mAddButton);
  
  d->mRemoveButton = new QToolButton(this, "mRemoveButton");
  QToolTip::add(d->mRemoveButton, i18n("Remove selected contact"));
  d->mRemoveButton->setIconSet(SmallIconSet("back"));
  connect(d->mRemoveButton, SIGNAL(clicked()), SLOT(remove()));
  listLayout->addWidget(d->mRemoveButton);
  
  spacer = new QWidget(this);
  spacer->setMinimumHeight(30);
  listLayout->addWidget(spacer);
  listLayout->setStretchFactor(spacer, 1);
  
  // selected list
  listLayout = new QVBoxLayout();
  listLayout->setSpacing( KDialog::spacingHint() );
  topLayout->addLayout( listLayout );
  topLayout->setStretchFactor(listLayout, 1);
  
  label = new QLabel(i18n("&Selected Contacts:"), this);
  listLayout->addWidget(label);
  d->mSelectedView = new KListView(this, "mSelectedView");
  d->mSelectedView->setSelectionModeExt(KListView::Single);
  d->mSelectedView->addColumn(i18n("Contacts"));
  d->mSelectedView->header()->hide();
  d->mSelectedView->setRootIsDecorated(true);
  connect(d->mSelectedView, SIGNAL(selectionChanged()),
          SLOT(selectedSelectionChanged()));
  label->setBuddy(d->mSelectedView);
  listLayout->addWidget(d->mSelectedView);
  
}

}

#include "distributionlisteditorwidget.moc"
