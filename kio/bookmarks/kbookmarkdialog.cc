//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE libraries
   Copyright 2007 Daniel Teske <teske@squorn.de>

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

#include <QInputDialog>
#include <QLineEdit>

#include "kbookmarkdialog.h"
#include "kbookmarkdialog_p.h"
#include "kbookmarkmanager.h"
#include "kbookmarkmenu.h"
#include "kbookmarkmenu_p.h"
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <QHeaderView>
#include <kiconloader.h>
#include <kguiitem.h>

KBookmarkDialogPrivate::KBookmarkDialogPrivate(KBookmarkDialog *q)
    : q(q)
    , folderTree(0)
    , layout(false)
{
}

KBookmarkDialogPrivate::~KBookmarkDialogPrivate()
{
}

void KBookmarkDialogPrivate::initLayout()
{
    QBoxLayout *vbox = new QVBoxLayout;

    QFormLayout * form = new QFormLayout();
    vbox->addLayout(form);

    form->addRow( titleLabel, title );
    form->addRow( urlLabel, url );
    form->addRow( commentLabel, comment );

    vbox->addWidget(folderTree);
    vbox->addWidget(buttonBox);

    q->setLayout(vbox);
}

void KBookmarkDialogPrivate::initLayoutPrivate()
{
    title = new QLineEdit(q);
    title->setMinimumWidth(300);
    titleLabel = new QLabel(QObject::tr("Name:", "@label:textbox"), q);
    titleLabel->setBuddy( title );

    url = new QLineEdit(q);
    url->setMinimumWidth(300);
    urlLabel = new QLabel(QObject::tr("Location:", "@label:textbox"), q);
    urlLabel->setBuddy( url );

    comment = new QLineEdit(q);
    comment->setMinimumWidth(300);
    commentLabel = new QLabel(QObject::tr("Comment:", "@label:textbox"), q);
    commentLabel->setBuddy( comment );

    folderTree = new QTreeWidget(q);
    folderTree->setColumnCount(1);
    folderTree->header()->hide();
    folderTree->setSortingEnabled(false);
    folderTree->setSelectionMode( QTreeWidget::SingleSelection );
    folderTree->setSelectionBehavior( QTreeWidget::SelectRows );
    folderTree->setMinimumSize( 60, 100 );
    QTreeWidgetItem *root = new KBookmarkTreeItem(folderTree);
    fillGroup(root, mgr->root());

    buttonBox = new QDialogButtonBox(q);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    q->connect(buttonBox, SIGNAL(accepted()), q, SLOT(accept()));
    q->connect(buttonBox, SIGNAL(rejected()), q, SLOT(reject()));

    initLayout();
    layout = true;
}

void KBookmarkDialogPrivate::fillGroup(QTreeWidgetItem * parentItem, const KBookmarkGroup &group)
{
  for (KBookmark bk = group.first() ; !bk.isNull() ; bk = group.next(bk))
  {
    if (bk.isGroup()) {
      QTreeWidgetItem* item = new KBookmarkTreeItem(parentItem, folderTree, bk.toGroup());
      fillGroup(item, bk.toGroup());
    }
  }
}

void KBookmarkDialogPrivate::setParentBookmark(const KBookmark & bm)
{
    QString address = bm.address();
    KBookmarkTreeItem * item = static_cast<KBookmarkTreeItem *>(folderTree->topLevelItem(0));
    while(true)
    {
        if (item->address() == bm.address()) {
            folderTree->setCurrentItem(item);
            return;
        }
        for (int i=0; i<item->childCount(); ++i) {
            KBookmarkTreeItem * child = static_cast<KBookmarkTreeItem *>(item->child(i));
            if (KBookmark::commonParent(child->address(), address) == child->address()) {
                item = child;
                break;
            }
        }
    }
}

KBookmarkGroup KBookmarkDialogPrivate::parentBookmark()
{
    KBookmarkTreeItem *item = dynamic_cast<KBookmarkTreeItem *>(folderTree->currentItem());
    if (!item)
        return mgr->root();
    const QString &address = item->address();
    return mgr->findByAddress(address).toGroup();
}

void KBookmarkDialog::accept()
{
    if (d->mode == KBookmarkDialogPrivate::NewFolder) {
        KBookmarkGroup parent = d->parentBookmark();
        if (d->title->text().isEmpty())
            d->title->setText("New Folder");
        d->bm = parent.createNewFolder(d->title->text());
        d->bm.setDescription(d->comment->text());
        d->mgr->emitChanged(parent);
    } else if (d->mode == KBookmarkDialogPrivate::NewBookmark) {
        KBookmarkGroup parent = d->parentBookmark();
        if (d->title->text().isEmpty())
            d->title->setText("New Bookmark");
        d->bm = parent.addBookmark(d->title->text(), QUrl(d->url->text()));
        d->bm.setDescription(d->comment->text());
        d->mgr->emitChanged(parent);
    } else if (d->mode == KBookmarkDialogPrivate::NewMultipleBookmarks) {
        KBookmarkGroup parent = d->parentBookmark();
        if (d->title->text().isEmpty())
            d->title->setText("New Folder");
        d->bm = parent.createNewFolder(d->title->text());
        d->bm.setDescription(d->comment->text());
        QList< QPair<QString, QString> >::iterator  it, end;
        end = d->list.end();
        for (it = d->list.begin(); it!= d->list.end(); ++it) {
            d->bm.toGroup().addBookmark( (*it).first, QUrl((*it).second));
        }
        d->mgr->emitChanged(parent);
    } else if (d->mode == KBookmarkDialogPrivate::EditBookmark) {
        d->bm.setFullText(d->title->text());
        d->bm.setUrl(QUrl(d->url->text()));
        d->bm.setDescription(d->comment->text());
        d->mgr->emitChanged(d->bm.parentGroup());
    } else if (d->mode == d->SelectFolder) {
        d->bm = d->parentBookmark();
    }
    QDialog::accept();
}

KBookmark KBookmarkDialog::editBookmark(const KBookmark & bm)
{
    if (!d->layout)
        d->initLayoutPrivate();

    KGuiItem::assign(d->buttonBox->button(QDialogButtonBox::Ok), KGuiItem(tr("Update", "@action:button")));
    setWindowTitle(tr("Bookmark Properties", "@title:window"));
    d->url->setVisible(!bm.isGroup());
    d->urlLabel->setVisible(!bm.isGroup());
    d->bm = bm;
    d->title->setText(bm.fullText());
    d->url->setText(bm.url().toString());
    d->comment->setVisible(true);
    d->commentLabel->setVisible(true);
    d->comment->setText(bm.description());
    d->folderTree->setVisible(false);

    d->mode = KBookmarkDialogPrivate::EditBookmark;

    if (exec() == QDialog::Accepted)
        return d->bm;
    else
        return KBookmark();

}

KBookmark KBookmarkDialog::addBookmark(const QString & title, const QUrl & url, KBookmark parent)
{
    if (!d->layout)
        d->initLayoutPrivate();
    if (parent.isNull())
        parent = d->mgr->root();

    QPushButton *newButton = new QPushButton;
    KGuiItem::assign(newButton, KGuiItem(tr("&New Folder...", "@action:button"), "folder-new"));
    d->buttonBox->addButton(newButton, QDialogButtonBox::ActionRole);
    connect(newButton, SIGNAL(clicked()), SLOT(newFolderButton()));

    KGuiItem::assign(d->buttonBox->button(QDialogButtonBox::Ok), KGuiItem(tr("Add", "@action:button"), "bookmark-new"));
    setWindowTitle(tr("Add Bookmark", "@title:window"));
    d->url->setVisible(true);
    d->urlLabel->setVisible(true);
    d->title->setText(title);
    d->url->setText(url.toString());
    d->comment->setText(QString());
    d->comment->setVisible(true);
    d->commentLabel->setVisible(true);
    d->setParentBookmark(parent);
    d->folderTree->setVisible(true);

    d->mode = KBookmarkDialogPrivate::NewBookmark;

    if (exec() == QDialog::Accepted)
        return d->bm;
    else
        return KBookmark();
}

KBookmarkGroup KBookmarkDialog::addBookmarks(const QList<QPair<QString, QString> > & list, const QString & name, KBookmarkGroup parent)
{
    if (!d->layout)
        d->initLayoutPrivate();
    if (parent.isNull())
        parent = d->mgr->root();

    d->list = list;

    QPushButton *newButton = new QPushButton;
    KGuiItem::assign(newButton, KGuiItem(tr("&New Folder...", "@action:button"), "folder-new"));
    d->buttonBox->addButton(newButton, QDialogButtonBox::ActionRole);
    connect(newButton, SIGNAL(clicked()), SLOT(newFolderButton()));

    KGuiItem::assign(d->buttonBox->button(QDialogButtonBox::Ok), KGuiItem(tr("Add", "@action:button"), "bookmark-new"));
    setWindowTitle(tr("Add Bookmarks", "@title:window"));
    d->url->setVisible(false);
    d->urlLabel->setVisible(false);
    d->title->setText(name);
    d->comment->setVisible(true);
    d->commentLabel->setVisible(true);
    d->comment->setText(QString());
    d->setParentBookmark(parent);
    d->folderTree->setVisible(true);

    d->mode = KBookmarkDialogPrivate::NewMultipleBookmarks;

    if (exec() == QDialog::Accepted)
        return d->bm.toGroup();
    else
        return KBookmarkGroup();
}

KBookmarkGroup KBookmarkDialog::selectFolder(KBookmark parent)
{
    if (!d->layout)
        d->initLayoutPrivate();
    if (parent.isNull())
        parent = d->mgr->root();

    QPushButton *newButton = new QPushButton;
    KGuiItem::assign(newButton, KGuiItem(tr("&New Folder...", "@action:button"), "folder-new"));
    d->buttonBox->addButton(newButton, QDialogButtonBox::ActionRole);
    connect(newButton, SIGNAL(clicked()), SLOT(newFolderButton()));

    setWindowTitle(tr("Select Folder", "@title:window"));
    d->url->setVisible(false);
    d->urlLabel->setVisible(false);
    d->title->setVisible(false);
    d->titleLabel->setVisible(false);
    d->comment->setVisible(false);
    d->commentLabel->setVisible(false);
    d->setParentBookmark(parent);
    d->folderTree->setVisible(true);

    d->mode = d->SelectFolder;

    if (exec() == QDialog::Accepted)
        return d->bm.toGroup();
    else
        return KBookmarkGroup();
}

KBookmarkGroup KBookmarkDialog::createNewFolder(const QString & name, KBookmark parent)
{
    if (!d->layout)
        d->initLayoutPrivate();
    if (parent.isNull())
        parent = d->mgr->root();

    setWindowTitle(tr("New Folder", "@title:window"));
    d->url->setVisible(false);
    d->urlLabel->setVisible(false);
    d->comment->setVisible(true);
    d->commentLabel->setVisible(true);
    d->comment->setText(QString());
    d->title->setText(name);
    d->setParentBookmark(parent);
    d->folderTree->setVisible(true);

    d->mode = KBookmarkDialogPrivate::NewFolder;

    if (exec() == QDialog::Accepted)
        return d->bm.toGroup();
    else
        return KBookmarkGroup();
}

KBookmarkDialog::KBookmarkDialog(KBookmarkManager * mgr, QWidget * parent )
  : QDialog(parent)
  , d(new KBookmarkDialogPrivate(this))
{
    d->mgr = mgr;
}

KBookmarkDialog::~KBookmarkDialog()
{
   delete d;
}

void KBookmarkDialog::newFolderButton()
{

    QString caption = d->parentBookmark().fullText().isEmpty() ?
                      tr("Create New Bookmark Folder", "@title:window") :
                      tr("Create New Bookmark Folder in %1", "@title:window").arg(d->parentBookmark().text() );
    bool ok;
    QString text = QInputDialog::getText( this, caption, tr("New folder:", "@label:textbox"), 
                                          QLineEdit::Normal, QString(), &ok );
    if (!ok)
        return;

    KBookmarkGroup group = d->parentBookmark().createNewFolder(text);
    if (!group.isNull()) {
        KBookmarkGroup parentGroup = group.parentGroup();
        d->mgr->emitChanged( parentGroup );
        d->folderTree->clear();
        QTreeWidgetItem *root = new KBookmarkTreeItem(d->folderTree);
        d->fillGroup( root, d->mgr->root() );
    }
}

/********************************************************************/

KBookmarkTreeItem::KBookmarkTreeItem(QTreeWidget * tree)
    : QTreeWidgetItem(tree), m_address("")
{
    setText(0, KBookmarkDialog::tr("Bookmarks", "name of the container of all browser bookmarks"));
    setIcon(0, SmallIcon("bookmarks"));
    tree->expandItem(this);
    tree->setCurrentItem( this );
    tree->setItemSelected( this, true );
}

KBookmarkTreeItem::KBookmarkTreeItem(QTreeWidgetItem * parent, QTreeWidget * tree, const KBookmarkGroup &bk)
    : QTreeWidgetItem(parent)
{
    setIcon(0, SmallIcon(bk.icon()));
    setText(0, bk.fullText() );
    tree->expandItem(this);
    m_address = bk.address();
}

KBookmarkTreeItem::~KBookmarkTreeItem()
{
}

QString KBookmarkTreeItem::address()
{
    return m_address;
}
