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

#include <klocalizedstring.h>
#include <qmimedatabase.h>
#include <ksharedconfig.h>

#include <QApplication>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLayout>
#include <QProcess>
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
    : QWidget( parent ),
      d(new KMimeTypeChooserPrivate(this))
{
  d->defaultgroup = defaultGroup;
  d->groups = groupsToShow;
  d->visuals = visuals;

  QVBoxLayout* vboxLayout = new QVBoxLayout(this);
  vboxLayout->setMargin(0);
  if ( !text.isEmpty() )
  {
    vboxLayout->addWidget(new QLabel( text, this ));
  }

  d->mimeTypeTree = new QTreeWidget( this );
  vboxLayout->addWidget(d->mimeTypeTree);
  QStringList headerLabels;
  headerLabels.append( i18n("Mime Type") );

  if ( visuals & Comments ) {
      headerLabels.append( i18n("Comment") );
  }
  if ( visuals & Patterns ) {
      headerLabels.append( i18n("Patterns") );
  }

  d->mimeTypeTree->setColumnCount(headerLabels.count());
  d->mimeTypeTree->setHeaderLabels(headerLabels);
  QFontMetrics fm(d->mimeTypeTree->fontMetrics());
  d->mimeTypeTree->setColumnWidth(0, 20 * fm.height()); // big enough for most names, but not for the insanely long ones

  d->loadMimeTypes( selMimeTypes );

  if (visuals & EditButton)
  {
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch(1);
    d->btnEditMimeType = new QPushButton( i18n("&Edit..."), this );
    buttonLayout->addWidget(d->btnEditMimeType);

    connect( d->btnEditMimeType, SIGNAL(clicked()), this, SLOT(_k_editMimeType()) );
    d->btnEditMimeType->setEnabled( false );
    connect( d->mimeTypeTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
             this, SLOT(_k_editMimeType()));
    connect( d->mimeTypeTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
             this, SLOT(_k_slotCurrentChanged(QTreeWidgetItem*)) );

    d->btnEditMimeType->setWhatsThis(i18n(
        "Click this button to display the familiar KDE mime type editor.") );

    vboxLayout->addLayout(buttonLayout);
  }
  setLayout(vboxLayout);
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
  QMimeDatabase db;
  const QList<QMimeType> mimetypes = db.allMimeTypes();

  bool agroupisopen = false;
  QTreeWidgetItem *idefault = 0; //open this, if all other fails
  QTreeWidgetItem *firstChecked = 0; // make this one visible after the loop

  foreach (const QMimeType& mt, mimetypes)
  {
    const QString mimetype = mt.name();
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
    item->setIcon(0, QIcon::fromTheme(mt.iconName()));

    int cl = 1;

    if ( visuals & KMimeTypeChooser::Comments )
    {
      item->setText(cl, mt.comment());
      cl++;
    }

    if ( visuals & KMimeTypeChooser::Patterns )
      item->setText(cl, mt.globPatterns().join("; "));

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
  mimeTypeTree->resizeColumnToContents(1);
}

void KMimeTypeChooserPrivate::_k_editMimeType()
{
    QTreeWidgetItem* item = mimeTypeTree->currentItem();
    if ( !item || !item->parent() )
        return;
    QString mt = (item->parent())->text(0) + '/' + item->text(0);
    // KF5 TODO: use a QFileSystemWatcher on one of the shared-mime-info generated files, instead.
    //q->connect( KSycoca::self(), SIGNAL(databaseChanged(QStringList)),
    //            q, SLOT(_k_slotSycocaDatabaseChanged(QStringList)) );
#pragma message("KF5 TODO: use QFileSystemWatcher to be told when keditfiletype changed a mimetype")

    // TODO: Move this class to the kwidgets framework, no more kio dependency.

    QStringList args;
#ifndef Q_OS_WIN
    args << "--parent" << QString::number((ulong)q->topLevelWidget()->winId());
#endif
    args << "--caption" << QGuiApplication::applicationDisplayName();
    args << mt;

    QProcess::startDetached(QStringLiteral("keditfiletype"), args);
}

void KMimeTypeChooserPrivate::_k_slotCurrentChanged(QTreeWidgetItem* item)
{
  if ( btnEditMimeType )
    btnEditMimeType->setEnabled(item && item->parent());
}

// TODO: see _k_editMimeType
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
    QMimeDatabase db;
    foreach(QTreeWidgetItem* item, checkedItems) {
        QMimeType mime = db.mimeTypeForName(item->parent()->text(0) + '/' + item->text(0));
        Q_ASSERT(mime.isValid());
        patternList += mime.globPatterns();
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
    : QDialog( parent ), d(new Private(this))
{
  setWindowTitle( caption );

  d->m_chooser = new KMimeTypeChooser( text, selMimeTypes,
                                       defaultGroup, groupsToShow, visuals,
                                       this );
  d->init();
}

KMimeTypeChooserDialog::KMimeTypeChooserDialog(
                         const QString &caption,
                         const QString& text,
                         const QStringList &selMimeTypes,
                         const QString &defaultGroup,
                         QWidget *parent )
    : QDialog( parent ), d(new Private(this))
{
  setWindowTitle( caption );

  d->m_chooser = new KMimeTypeChooser( text, selMimeTypes,
                                       defaultGroup, QStringList(),
                                       KMimeTypeChooser::Comments|KMimeTypeChooser::Patterns|KMimeTypeChooser::EditButton,
                                       this );
  d->init();
}

KMimeTypeChooser* KMimeTypeChooserDialog::chooser()
{
    return d->m_chooser;
}

void KMimeTypeChooserDialog::Private::init()
{
    QVBoxLayout *layout = new QVBoxLayout;
    q->setLayout(layout);

    layout->addWidget(m_chooser);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(q);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QObject::connect(buttonBox, SIGNAL(accepted()), q, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), q, SLOT(reject()));
    layout->addWidget(buttonBox);

  KConfigGroup group( KSharedConfig::openConfig(), "KMimeTypeChooserDialog");
  q->resize(group.readEntry("size", QSize(600,500)));
}

KMimeTypeChooserDialog::~KMimeTypeChooserDialog()
{
  KConfigGroup group( KSharedConfig::openConfig(), "KMimeTypeChooserDialog");
  group.writeEntry("size", size());

  delete d;
}

//END KMimeTypeChooserDialog

// kate: space-indent on; indent-width 2; replace-tabs on;
#include "moc_kmimetypechooser.cpp"
