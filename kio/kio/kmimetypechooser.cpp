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
#include <klistview.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kprocess.h>
#include <krun.h>
#include <ksycoca.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>

//BEGIN KMimeTypeChooserPrivate
class KMimeTypeChooserPrivate
{
  public:
    KListView *lvMimeTypes;
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
                              QWidget *parent,
                              const char *name )
    : QVBox( parent, name )
{
  d = new KMimeTypeChooserPrivate();
  d->lvMimeTypes = 0;
  d->btnEditMimeType = 0;
  d->defaultgroup = defaultGroup;
  d->groups = groupsToShow;
  d->visuals = visuals;

  setSpacing( KDialogBase::spacingHint() );

  if ( !text.isEmpty() )
  {
    new QLabel( text, this );
  }

  d->lvMimeTypes = new KListView( this );

  d->lvMimeTypes->addColumn( i18n("Mime Type") );
//   d->lvMimeTypes->setColumnWidthMode( 0, QListView::Manual );

  if ( visuals & Comments )
  {
    d->lvMimeTypes->addColumn( i18n("Comment") );
    d->lvMimeTypes->setColumnWidthMode( 1, QListView::Manual );
  }
  if ( visuals & Patterns )
    d->lvMimeTypes->addColumn( i18n("Patterns") );

  d->lvMimeTypes->setRootIsDecorated( true );

  loadMimeTypes( selMimeTypes );

  if (visuals & KMimeTypeChooser::EditButton)
  {
    QHBox *btns = new QHBox( this );
    ((QBoxLayout*)btns->layout())->addStretch(1);
    d->btnEditMimeType = new QPushButton( i18n("&Edit..."), btns );

    connect( d->btnEditMimeType, SIGNAL(clicked()), this, SLOT(editMimeType()) );
    d->btnEditMimeType->setEnabled( false );
    connect( d->lvMimeTypes, SIGNAL( doubleClicked ( QListViewItem * )),
             this, SLOT( editMimeType()));
    connect( d->lvMimeTypes, SIGNAL(currentChanged(QListViewItem*)),
             this, SLOT(slotCurrentChanged(QListViewItem*)) );

    QWhatsThis::add( d->btnEditMimeType, i18n(
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

  d->lvMimeTypes->clear();

  QMap<QString,QListViewItem*> groups;
  // thanks to kdebase/kcontrol/filetypes/filetypesview
  KMimeType::List mimetypes = KMimeType::allMimeTypes();
  QValueListIterator<KMimeType::Ptr> it(mimetypes.begin());

  QListViewItem *groupItem;
  bool agroupisopen = false;
  QListViewItem *idefault = 0; //open this, if all other fails
  QListViewItem *firstChecked = 0; // make this one visible after the loop

  for (; it != mimetypes.end(); ++it)
  {
    QString mimetype = (*it)->name();
    int index = mimetype.find("/");
    QString maj = mimetype.left(index);

    if ( d->groups.count() && !d->groups.contains( maj ) )
      continue;

    QString min = mimetype.right(mimetype.length() - (index+1));

    QMapIterator<QString,QListViewItem*> mit = groups.find( maj );
    if ( mit == groups.end() )
    {
      groupItem = new QListViewItem( d->lvMimeTypes, maj );
      groups.insert( maj, groupItem );
       if ( maj == d->defaultgroup )
         idefault = groupItem;
    }
    else
        groupItem = mit.data();

    QCheckListItem *item = new QCheckListItem( groupItem, min, QCheckListItem::CheckBox );
    item->setPixmap( 0, SmallIcon( (*it)->icon(QString::null,false) ) );

    int cl = 1;

    if ( d->visuals & Comments )
    {
      item->setText( cl, (*it)->comment(QString::null, false) );
      cl++;
    }

    if ( d->visuals & Patterns )
      item->setText( cl, (*it)->patterns().join("; ") );

    if ( selMimeTypes.contains(mimetype) )
    {
      item->setOn( true );
      groupItem->setOpen( true );
      agroupisopen = true;
      if ( !firstChecked )
        firstChecked = item;
    }
  }

  if ( firstChecked )
    d->lvMimeTypes->ensureItemVisible( firstChecked );

  if ( !agroupisopen && idefault )
  {
    idefault->setOpen( true );
    d->lvMimeTypes->ensureItemVisible( idefault );
  }
}

void KMimeTypeChooser::editMimeType()
{
  if ( !(d->lvMimeTypes->currentItem() && (d->lvMimeTypes->currentItem())->parent()) )
    return;
  QString mt = (d->lvMimeTypes->currentItem()->parent())->text( 0 ) + "/" + (d->lvMimeTypes->currentItem())->text( 0 );
  // thanks to libkonq/konq_operations.cc
  connect( KSycoca::self(), SIGNAL(databaseChanged()),
           this, SLOT(slotSycocaDatabaseChanged()) );
  QString keditfiletype = QString::fromLatin1("keditfiletype");
  KRun::runCommand( keditfiletype
                    + " --parent " + QString::number( (ulong)topLevelWidget()->winId())
                    + " " + KProcess::quote(mt),
                    keditfiletype, keditfiletype /*unused*/);
}

void KMimeTypeChooser::slotCurrentChanged(QListViewItem* i)
{
  if ( d->btnEditMimeType )
    d->btnEditMimeType->setEnabled( i->parent() );
}

void KMimeTypeChooser::slotSycocaDatabaseChanged()
{
  if ( KSycoca::self()->isChanged("mime") )
    loadMimeTypes();
}

QStringList KMimeTypeChooser::mimeTypes() const
{
  QStringList l;
  QListViewItemIterator it( d->lvMimeTypes );
  for (; it.current(); ++it)
  {
    if ( it.current()->parent() && ((QCheckListItem*)it.current())->isOn() )
      l << it.current()->parent()->text(0) + "/" + it.current()->text(0); // FIXME uncecked, should be Ok unless someone changes mimetypes during this!
  }
  return l;
}

QStringList KMimeTypeChooser::patterns() const
{
  QStringList l;
  KMimeType::Ptr p;
  QString defMT = KMimeType::defaultMimeType();
  QListViewItemIterator it( d->lvMimeTypes );
  for (; it.current(); ++it)
  {
    if ( it.current()->parent() && ((QCheckListItem*)it.current())->isOn() )
    {
      p = KMimeType::mimeType( it.current()->parent()->text(0) + "/" + it.current()->text(0) );
      if ( p->name() != defMT )
        l += p->patterns();
    }
  }
  return l;
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
                         QWidget *parent, const char *name )
    : KDialogBase(parent, name, true, caption, Cancel|Ok, Ok)
{
  m_chooser = new KMimeTypeChooser( text, selMimeTypes,
                                  defaultGroup, groupsToShow, visuals,
                                  this, "chooser" );
  setMainWidget(m_chooser);

  KConfigGroup group( KGlobal::config(), "KMimeTypeChooserDialog");
  QSize defaultSize( 400, 300 );
  resize( group.readSizeEntry("size", &defaultSize) );
}

KMimeTypeChooserDialog::KMimeTypeChooserDialog(
                         const QString &caption,
                         const QString& text,
                         const QStringList &selMimeTypes,
                         const QString &defaultGroup,
                         QWidget *parent, const char *name )
    : KDialogBase(parent, name, true, caption, Cancel|Ok, Ok)
{
  m_chooser = new KMimeTypeChooser( text, selMimeTypes,
                                  defaultGroup, QStringList(),
                                  KMimeTypeChooser::Comments|KMimeTypeChooser::Patterns|KMimeTypeChooser::EditButton,
                                  this, "chooser" );
  setMainWidget(m_chooser);

  KConfigGroup group( KGlobal::config(), "KMimeTypeChooserDialog");
  QSize defaultSize( 400, 300 );
  resize( group.readSizeEntry("size", &defaultSize) );
}


KMimeTypeChooserDialog::~KMimeTypeChooserDialog()
{
  KConfigGroup group( KGlobal::config(), "KMimeTypeChooserDialog");
  group.writeEntry("size", size());
}

//END KMimeTypeChooserDialog

// kate: space-indent on; indent-width 2; replace-tabs on;
#include "kmimetypechooser.moc"
