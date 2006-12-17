/* This file is part of the KDE libraries
   Copyright (C) 2001 - 2004 Anders Lund <anders@alweb.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kmimetypechooser.h"

#include <kconfig.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kprocess.h>
#include <krun.h>
#include <ksycoca.h>

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QTreeWidget>

//BEGIN KMimeTypeChooserPrivate
class KMimeTypeChooserPrivate
{
  public:
    QTreeWidget *mimeTypeTree;
    QPushButton *btnEditMimeType;

    QString defaultgroup;
    QStringList groups;
    int visuals;
};
//END

//BEGIN KMimeTypeChooser
KMimeTypeChooser::KMimeTypeChooser( const QString &text,
                              const QStringList &selMimeTypes,
                              const QString &defaultGroup,
                              const QStringList &groupsToShow,
                              int visuals,
                              QWidget *parent )
    : KVBox( parent )
{
  d = new KMimeTypeChooserPrivate();
  d->mimeTypeTree = 0;
  d->btnEditMimeType = 0;
  d->defaultgroup = defaultGroup;
  d->groups = groupsToShow;
  d->visuals = visuals;

  setSpacing( KDialog::spacingHint() );

  if ( !text.isEmpty() )
  {
    new QLabel( text, this );
  }

  d->mimeTypeTree = new QTreeWidget( this );
  QStringList headerLabels;
  headerLabels.append( i18n("Mime Type") );
//   d->mimeTypeTree->setColumnWidthMode( 0, QListView::Manual );

  if ( visuals & Comments ) {
      headerLabels.append( i18n("Comment") );
      //d->mimeTypeTree->setColumnWidthMode( 1, Q3ListView::Manual );
  }
  if ( visuals & Patterns ) {
      headerLabels.append( i18n("Patterns") );
  }

//  d->mimeTypeTree->setRootIsDecorated( true );
  d->mimeTypeTree->setColumnCount(headerLabels.count());
  d->mimeTypeTree->setHeaderLabels(headerLabels);

  loadMimeTypes( selMimeTypes );

  if (visuals & EditButton)
  {
    KHBox *btns = new KHBox( this );
    ((QBoxLayout*)btns->layout())->addStretch(1);
    d->btnEditMimeType = new QPushButton( i18n("&Edit..."), btns );

    connect( d->btnEditMimeType, SIGNAL(clicked()), this, SLOT(editMimeType()) );
    d->btnEditMimeType->setEnabled( false );
    connect( d->mimeTypeTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
             this, SLOT(editMimeType()));
    connect( d->mimeTypeTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
             this, SLOT(slotCurrentChanged(QTreeWidgetItem*)) );

    d->btnEditMimeType->setWhatsThis(i18n(
        "Click this button to display the familiar KDE mime type editor.") );
  }
}

KMimeTypeChooser::~KMimeTypeChooser()
{
  delete d;
}

void KMimeTypeChooser::loadMimeTypes( const QStringList &_selectedMimeTypes )
{
  QStringList selMimeTypes;

  if ( !_selectedMimeTypes.isEmpty() )
    selMimeTypes = _selectedMimeTypes;
  else
    selMimeTypes = mimeTypes();

  d->mimeTypeTree->clear();

  QMap<QString, QTreeWidgetItem*> groupItems;
  const KMimeType::List mimetypes = KMimeType::allMimeTypes();

  bool agroupisopen = false;
  QTreeWidgetItem *idefault = 0; //open this, if all other fails
  QTreeWidgetItem *firstChecked = 0; // make this one visible after the loop

  foreach (const KMimeType::Ptr& mt, mimetypes)
  {
    const QString mimetype = mt->name();
    const int index = mimetype.indexOf('/');
    const QString maj = mimetype.left(index);

    if ( !d->groups.isEmpty() && !d->groups.contains( maj ) )
      continue;

    QTreeWidgetItem *groupItem;
    QMap<QString,QTreeWidgetItem*>::Iterator mit = groupItems.find( maj );
    if ( mit == groupItems.end() )
    {
        groupItem = new QTreeWidgetItem( d->mimeTypeTree, QStringList(maj) );
        groupItems.insert( maj, groupItem );
        if ( maj == d->defaultgroup )
            idefault = groupItem;
    }
    else
        groupItem = mit.value();

    const QString min = mimetype.mid(index+1);
    QTreeWidgetItem *item = new QTreeWidgetItem( groupItem, QStringList(min) );
    item->setIcon( 0, SmallIcon( mt->iconName() ) );

    int cl = 1;

    if ( d->visuals & Comments )
    {
      item->setText( cl, mt->comment() );
      cl++;
    }

    if ( d->visuals & Patterns )
      item->setText( cl, mt->patterns().join("; ") );

    if ( selMimeTypes.contains(mimetype) ) {
      item->setCheckState( 0, Qt::Checked );
      groupItem->setExpanded( true );
      agroupisopen = true;
      if ( !firstChecked )
        firstChecked = item;
    } else {
      item->setCheckState( 0, Qt::Unchecked );
    }
  }

  if ( firstChecked )
    d->mimeTypeTree->scrollToItem( firstChecked );

  if ( !agroupisopen && idefault )
  {
    idefault->setExpanded( true );
    d->mimeTypeTree->scrollToItem( idefault );
  }
}

void KMimeTypeChooser::editMimeType()
{
    QTreeWidgetItem* item = d->mimeTypeTree->currentItem();
    if ( !item || !item->parent() )
        return;
    QString mt = (item->parent())->text(0) + '/' + item->text(0);
    // thanks to libkonq/konq_operations.cc
    connect( KSycoca::self(), SIGNAL(databaseChanged()),
             this, SLOT(slotSycocaDatabaseChanged()) );
    QString keditfiletype = QString::fromLatin1("keditfiletype");
    KRun::runCommand( keditfiletype
#ifndef Q_OS_WIN
                      + " --parent " + QString::number( (ulong)topLevelWidget()->winId())
#endif
                      + ' ' + KProcess::quote(mt),
                      keditfiletype, keditfiletype /*unused*/);
}

void KMimeTypeChooser::slotCurrentChanged(QTreeWidgetItem* i)
{
  if ( d->btnEditMimeType )
    d->btnEditMimeType->setEnabled( i->parent() );
}

void KMimeTypeChooser::slotSycocaDatabaseChanged()
{
  if ( KSycoca::self()->isChanged("mime") )
    loadMimeTypes();
}

// recursive helper for mimeTypes()
static void getCheckedItems(QList<QTreeWidgetItem *> &lst, QTreeWidgetItem* parent)
{
    for (int i = 0; i < parent->childCount(); ++i ) {
        QTreeWidgetItem* item = parent->child(i);
        if (item->checkState(0) == Qt::Checked)
            lst.append(item);
        getCheckedItems(lst, item);
    }
}

static void getCheckedItems(QList<QTreeWidgetItem *>& lst, QTreeWidget* tree)
{
    for (int i = 0; i < tree->topLevelItemCount(); ++i ) {
        QTreeWidgetItem* topItem = tree->topLevelItem(i);
        //if (topItem->checkState(0) == Qt::Checked)
        //    lst.append(topItem);
        getCheckedItems(lst, topItem);
    }
}

QStringList KMimeTypeChooser::mimeTypes() const
{
    QStringList mimeList;
    QList<QTreeWidgetItem *> checkedItems;
    getCheckedItems(checkedItems, d->mimeTypeTree);
    foreach(QTreeWidgetItem* item, checkedItems) {
        mimeList.append( item->parent()->text(0) + '/' + item->text(0) );
    }
    return mimeList;
}

QStringList KMimeTypeChooser::patterns() const
{
    QStringList patternList;
    QList<QTreeWidgetItem *> checkedItems;
    getCheckedItems(checkedItems, d->mimeTypeTree);
    foreach(QTreeWidgetItem* item, checkedItems) {
        KMimeType::Ptr p = KMimeType::mimeType( item->parent()->text(0) + '/' + item->text(0) );
        Q_ASSERT(p);
        patternList += p->patterns();
    }
    return patternList;
}
//END

//BEGIN KMimeTypeChooserDialog
KMimeTypeChooserDialog::KMimeTypeChooserDialog(
                         const QString &caption,
                         const QString& text,
                         const QStringList &selMimeTypes,
                         const QString &defaultGroup,
                         const QStringList &groupsToShow,
                         int visuals,
                         QWidget *parent )
    : KDialog( parent )
{
  setCaption( caption );
  init();

  m_chooser = new KMimeTypeChooser( text, selMimeTypes,
                                  defaultGroup, groupsToShow, visuals,
                                  this );
  setMainWidget(m_chooser);
}

KMimeTypeChooserDialog::KMimeTypeChooserDialog(
                         const QString &caption,
                         const QString& text,
                         const QStringList &selMimeTypes,
                         const QString &defaultGroup,
                         QWidget *parent )
    : KDialog( parent )
{
  setCaption( caption );
  init();

  m_chooser = new KMimeTypeChooser( text, selMimeTypes,
                                  defaultGroup, QStringList(),
                                  KMimeTypeChooser::Comments|KMimeTypeChooser::Patterns|KMimeTypeChooser::EditButton,
                                  this );
  setMainWidget(m_chooser);
}

void KMimeTypeChooserDialog::init()
{
  setButtons( Cancel | Ok );
  setModal( true );
  setDefaultButton( Ok );

  KConfigGroup group( KGlobal::config(), "KMimeTypeChooserDialog");
  resize( group.readEntry("size", QSize(500,400)));
}

KMimeTypeChooserDialog::~KMimeTypeChooserDialog()
{
  KConfigGroup group( KGlobal::config(), "KMimeTypeChooserDialog");
  group.writeEntry("size", size());
}

//END KMimeTypeChooserDialog

// kate: space-indent on; indent-width 2; replace-tabs on;
#include "kmimetypechooser.moc"
