/*                                                                      
    This file is part of libkabc.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>                   
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

#include <qlayout.h>
#include <qgroupbox.h>
#include <qtooltip.h>
#include <qtoolbutton.h>
#include <qlabel.h>

#include <klocale.h>
#include <kiconloader.h>
#include <klistbox.h>
#include <klineedit.h>

#include "addressbook.h"
#include "distributionlist.h"
#include "distributionlisteditorwidget.h"
#include "distributionlisteditor.h"

namespace KABC {

/////////////////////////////////
// NameEditDialog

/** Internal class that has a line edit. This should probably
* be generalized into a simple dialog in kdelibs/kdeui
*/
class NameDialog : public KDialogBase
{
  public:
    NameDialog(QWidget *parent) 
      : KDialogBase(Plain, i18n("Distribution List Name"), Ok | Cancel, 
                    Ok, parent)
      {
        QWidget *page = plainPage();
        QVBoxLayout *layout = new QVBoxLayout(page);
        layout->setMargin(marginHint());
        layout->setSpacing(spacingHint());
        layout->setAutoAdd(true);
        
        (void) new QLabel(i18n("Please enter a name for the list:"), page);
        mNameEdit = new KLineEdit(page, "mNameEdit");
        mNameEdit->setFocus();
      }
      
    QString name() const { return mNameEdit->text(); }
    void setName(const QString &name)
    {
      mNameEdit->setText(name);
      mNameEdit->home(false);
      mNameEdit->end(true); // select all
    }
  
  private:
   KLineEdit *mNameEdit;
};

  
/////////////////////////////////
// DistributionListEditor
  
class DistributionListEditorPrivate
{
  public:
    DistributionListEditorPrivate() {}
    ~DistributionListEditorPrivate() {}
    
    AddressBook *mAddressBook;
    DistributionListManager *mManager;
    int mIndex;
    
    // GUI
    QToolButton *mRemoveButton;
    QToolButton *mRenameButton;
    KListBox *mDistListBox;
    DistributionListEditorWidget *mEditorWidget;
}; 

DistributionListEditor::DistributionListEditor(AddressBook *book,
                                               QWidget *parent,
                                               const char *name)
  : KDialogBase(Plain, i18n("Distribution List Editor"),
                Ok | Apply | Cancel, Ok, parent, name)
{
  d = new DistributionListEditorPrivate();
  d->mAddressBook = book;
  d->mIndex = -1;
  
  initGUI();
  
  // Fill in the values
  d->mManager = new DistributionListManager(book);
  d->mManager->load();
  
  d->mDistListBox->insertStringList(d->mManager->listNames());
  
  itemSelected(-1);
  
  enableButton(Apply, false);
}

DistributionListEditor::~DistributionListEditor()
{
  delete d;
}

void DistributionListEditor::initGUI()
{
  QWidget *page = plainPage();
  
  QBoxLayout *topLayout = new QHBoxLayout(page);
  topLayout->setSpacing( spacingHint() );
  topLayout->setMargin( marginHint() );
  
  QGroupBox *gb = new QGroupBox(i18n("Available Lists"), page);
  topLayout->addWidget(gb);
  topLayout->setStretchFactor(gb, 1);
  
  QBoxLayout *listLayout = new QVBoxLayout(gb);
  listLayout->setSpacing( spacingHint() );
  listLayout->setMargin( 15 );
  
  d->mDistListBox = new KListBox(gb, "mDistListBox");
  connect(d->mDistListBox, SIGNAL(highlighted(int)), SLOT(itemSelected(int)));
  listLayout->addWidget(d->mDistListBox);
  
  QBoxLayout *buttonLayout = new QHBoxLayout();
  buttonLayout->setSpacing( spacingHint() );
  listLayout->addLayout(buttonLayout);
  
  QToolButton *addButton = new QToolButton(gb);
  addButton->setIconSet(SmallIconSet("filenew"));
  QToolTip::add(addButton, i18n("Add a new distribution list"));
  connect(addButton, SIGNAL(clicked()), SLOT(add()));
  buttonLayout->addWidget(addButton);
  
  d->mRemoveButton = new QToolButton(gb, "mRemoveButton");
  d->mRemoveButton->setIconSet(SmallIconSet("remove"));
  QToolTip::add(d->mRemoveButton, i18n("Remove the selected distribution list"));
  connect(d->mRemoveButton, SIGNAL(clicked()), SLOT(remove()));
  buttonLayout->addWidget(d->mRemoveButton);
  
  d->mRenameButton = new QToolButton(gb, "mRenameButton");
  d->mRenameButton->setIconSet(SmallIconSet("edit"));
  QToolTip::add(d->mRenameButton, i18n("Rename the selected distribution list"));
  connect(d->mRenameButton, SIGNAL(clicked()), SLOT(rename()));
  buttonLayout->addWidget(d->mRenameButton);
  
  d->mEditorWidget = new DistributionListEditorWidget(d->mAddressBook,
                                                      page, "mEditorWidget");
  d->mEditorWidget->setMinimumWidth(300);
  connect(d->mEditorWidget, SIGNAL(modified()), SLOT(modified()));
  topLayout->addWidget(d->mEditorWidget);
  topLayout->setStretchFactor(d->mEditorWidget, 3);
}

void DistributionListEditor::add()
{
  NameDialog dialog(this);
  if (dialog.exec())
  {
    (void) new DistributionList(d->mManager, dialog.name());
    d->mDistListBox->insertItem(dialog.name());
    d->mDistListBox->setCurrentItem(d->mDistListBox->count()-1);
    d->mDistListBox->ensureCurrentVisible();
    
    modified();
  }
}

void DistributionListEditor::remove()
{
  QString name = d->mDistListBox->currentText();
  int index = d->mDistListBox->currentItem();
  DistributionList *list = d->mManager->list(name);
  
  // Remove it from the list view so current changes
  d->mDistListBox->removeItem(index);
  
  // If we have a valid list, remove it as well
  if (list)
  {
    d->mManager->remove(list);
    delete list;
    
    modified();
  }
}

void DistributionListEditor::rename()
{
  NameDialog dialog(this);
  QString name = d->mDistListBox->currentText();
  
  dialog.setName(name);
  if (dialog.exec())
  {
    DistributionList *list = d->mManager->list(name);
    if (list)
    {
      d->mManager->remove(list);
      list->setName(dialog.name());
      d->mManager->insert(list); 
    }
    
    d->mDistListBox->changeItem(dialog.name(), d->mDistListBox->currentItem());
    
    modified();
  }
}

void DistributionListEditor::itemSelected(int id)
{
  bool e = id >= 0;
  
  d->mRemoveButton->setEnabled(e);
  d->mRenameButton->setEnabled(e);
  d->mEditorWidget->setEnabled(e);
  
  // save the current one if there is one
  if (d->mIndex >= 0)
    (void) d->mEditorWidget->distributionList();
  
  // Now update the new one
  if (e)
  {
    d->mIndex = id;
    QString name = d->mDistListBox->currentText();
    d->mEditorWidget->setDistributionList(d->mManager->list(name));
  }
  else
  {
    d->mEditorWidget->clear();
  }
}
    
void DistributionListEditor::slotOk()
{
  slotApply();
  
  KDialogBase::slotOk();
}

void DistributionListEditor::slotApply()
{
  // Save the current one if there is one
  if (d->mIndex >= 0)
    (void) d->mEditorWidget->distributionList();
    
  // Save the entire dist list
  d->mManager->save();  
  
  enableButton(Apply, false);
  
  KDialogBase::slotApply();
}

void DistributionListEditor::modified()
{
  enableButton(Apply, true);
}

} // end namespace KABC

#include "distributionlisteditor.moc"
