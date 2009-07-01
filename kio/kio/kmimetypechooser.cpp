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
#include <kshell.h>
#include <krun.h>
#include <ksycoca.h>

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QTreeWidget>
#include <kconfiggroup.h>

//BEGIN KMimeTypeChooserPrivate
class KMimeTypeChooserPrivate
{
  public:
    KMimeTypeChooserPrivate( KMimeTypeChooser *parent )
      : q(parent),
        mimeTypeTree(0),
        btnEditMimeType(0)
    {
    }

    void loadMimeTypes( const QStringList &selected = QStringList() );

    void _k_editMimeType();
    void _k_slotCurrentChanged(QTreeWidgetItem*);
    void _k_slotSycocaDatabaseChanged(const QStringList&);

    KMimeTypeChooser *q;
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
    : KVBox( parent ),
      d(new KMimeTypeChooserPrivate(this))
{
  d->defaultgroup = defaultGroup;
  d->groups = groupsToShow;
  d->visuals = visuals;
  setSpacing(-1);

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

  d->loadMimeTypes( selMimeTypes );

  if (visuals & EditButton)
  {
    KHBox *btns = new KHBox( this );
    ((QBoxLayout*)btns->layout())->addStretch(1);
    d->btnEditMimeType = new QPushButton( i18n("&Edit..."), btns );

    connect( d->btnEditMimeType, SIGNAL(clicked()), this, SLOT(_k_editMimeType()) );
    d->btnEditMimeType->setEnabled( false );
    connect( d->mimeTypeTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
             this, SLOT(_k_editMimeType()));
    connect( d->mimeTypeTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
             this, SLOT(_k_slotCurrentChanged(QTreeWidgetItem*)) );

    d->btnEditMimeType->setWhatsThis(i18n(
        "Click this button to display the familiar KDE mime type editor.") );
  }
}

KMimeTypeChooser::~KMimeTypeChooser()
{
  delete d;
}

void KMimeTypeChooserPrivate::loadMimeTypes( const QStringList &_selectedMimeTypes )
{
  QStringList selMimeTypes;

  if ( !_selectedMimeTypes.isEmpty() )
    selMimeTypes = _selectedMimeTypes;
  else
    selMimeTypes = q->mimeTypes();

  mimeTypeTree->clear();

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

    if ( !groups.isEmpty() && !groups.contains( maj ) )
      continue;

    QTreeWidgetItem *groupItem;
    QMap<QString,QTreeWidgetItem*>::Iterator mit = groupItems.find( maj );
    if ( mit == groupItems.end() )
    {
        groupItem = new QTreeWidgetItem( mimeTypeTree, QStringList(maj) );
        groupItems.insert( maj, groupItem );
        if ( maj == defaultgroup )
            idefault = groupItem;
    }
    else
        groupItem = mit.value();

    const QString min = mimetype.mid(index+1);
    QTreeWidgetItem *item = new QTreeWidgetItem( groupItem, QStringList(min) );
    item->setIcon( 0, KIcon( mt->iconName() ) );

    int cl = 1;

    if ( visuals & KMimeTypeChooser::Comments )
    {
      item->setText( cl, mt->comment() );
      cl++;
    }

    if ( visuals & KMimeTypeChooser::Patterns )
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
    mimeTypeTree->scrollToItem( firstChecked );

  if ( !agroupisopen && idefault )
  {
    idefault->setExpanded( true );
    mimeTypeTree->scrollToItem( idefault );
  }
}

void KMimeTypeChooserPrivate::_k_editMimeType()
{
    QTreeWidgetItem* item = mimeTypeTree->currentItem();
    if ( !item || !item->parent() )
        return;
    QString mt = (item->parent())->text(0) + '/' + item->text(0);
    // thanks to libkonq/konq_operations.cc
    q->connect( KSycoca::self(), SIGNAL(databaseChanged(QStringList)),
                q, SLOT(_k_slotSycocaDatabaseChanged(QStringList)) );
    QString keditfiletype = QString::fromLatin1("keditfiletype");
    KRun::runCommand( keditfiletype
#ifndef Q_OS_WIN
                      + " --parent " + QString::number( (ulong)q->topLevelWidget()->winId())
#endif
                      + ' ' + KShell::quoteArg(mt),
                      keditfiletype, keditfiletype /*unused*/, q->topLevelWidget());
}

void KMimeTypeChooserPrivate::_k_slotCurrentChanged(QTreeWidgetItem* item)
{
  if ( btnEditMimeType )
    btnEditMimeType->setEnabled( item->parent() );
}

void KMimeTypeChooserPrivate::_k_slotSycocaDatabaseChanged(const QStringList& changedResources)
{
    if (changedResources.contains("xdgdata-mime"))
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

//BEGIN KMimeTypeChooserDialog::Private

class KMimeTypeChooserDialog::Private
{
    public:
        Private( KMimeTypeChooserDialog *parent )
            : q(parent)
        {
        }

        void init();

        KMimeTypeChooserDialog *q;
        KMimeTypeChooser *m_chooser;
};

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
    : KDialog( parent ), d(new Private(this))
{
  setCaption( caption );
  d->init();

  d->m_chooser = new KMimeTypeChooser( text, selMimeTypes,
                                       defaultGroup, groupsToShow, visuals,
                                       this );
  setMainWidget(d->m_chooser);
}

KMimeTypeChooserDialog::KMimeTypeChooserDialog(
                         const QString &caption,
                         const QString& text,
                         const QStringList &selMimeTypes,
                         const QString &defaultGroup,
                         QWidget *parent )
    : KDialog( parent ), d(new Private(this))
{
  setCaption( caption );
  d->init();

  d->m_chooser = new KMimeTypeChooser( text, selMimeTypes,
                                       defaultGroup, QStringList(),
                                       KMimeTypeChooser::Comments|KMimeTypeChooser::Patterns|KMimeTypeChooser::EditButton,
                                       this );
  setMainWidget(d->m_chooser);
}

KMimeTypeChooser* KMimeTypeChooserDialog::chooser()
{
    return d->m_chooser;
}

void KMimeTypeChooserDialog::Private::init()
{
  q->setButtons( Cancel | Ok );
  q->setModal( true );
  q->setDefaultButton( Ok );

  KConfigGroup group( KGlobal::config(), "KMimeTypeChooserDialog");
  q->resize( group.readEntry("size", QSize(500,400)));
}

KMimeTypeChooserDialog::~KMimeTypeChooserDialog()
{
  KConfigGroup group( KGlobal::config(), "KMimeTypeChooserDialog");
  group.writeEntry("size", size());

  delete d;
}

//END KMimeTypeChooserDialog

// kate: space-indent on; indent-width 2; replace-tabs on;
#include "kmimetypechooser.moc"
