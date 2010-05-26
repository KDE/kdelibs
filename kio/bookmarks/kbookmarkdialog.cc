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

#include "kbookmarkdialog.h"
#include "kbookmarkmanager.h"
#include "kbookmarkmenu.h"
#include "kbookmarkmenu_p.h"
#include <QFormLayout>
#include <QLabel>
#include <QTreeWidget>
#include <QHeaderView>
#include <klineedit.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <kstandardguiitem.h>


KBookmark KBookmarkDialog::editBookmark(const KBookmark & bm)
{
    if(!m_layout)
        initLayoutPrivate();
    setButtons( Ok | Cancel );
    setButtonGuiItem( KDialog::Ok, KGuiItem(i18nc("@action:button", "Update" )) );
    setCaption( i18nc("@title:window","Bookmark Properties") );
    m_url->setVisible(!bm.isGroup());
    m_urlLabel->setVisible(!bm.isGroup());
    m_bm = bm;
    m_title->setText(bm.fullText());
    m_url->setText(bm.url().url());
    m_comment->setVisible(true);
    m_commentLabel->setVisible(true);
    m_comment->setText(bm.description());
    m_folderTree->setVisible(false);

    m_mode = EditBookmark;
    aboutToShow(m_mode);

    if(exec() == QDialog::Accepted)
        return m_bm;
    else
        return KBookmark();

}

KBookmark KBookmarkDialog::addBookmark(const QString & title, const KUrl & url, KBookmark parent)
{
    if(!m_layout)
        initLayoutPrivate();
    if(parent.isNull())
        parent = m_mgr->root();
    setButtons( User1 | Ok | Cancel );
    setButtonGuiItem( KDialog::Ok,  KGuiItem( i18nc("@action:button", "Add" ), "bookmark-new") );
    setCaption( i18nc("@title:window","Add Bookmark") );
    setButtonGuiItem( User1, KGuiItem( i18nc("@action:button", "&New Folder..." ), "folder-new") );
    m_url->setVisible(true);
    m_urlLabel->setVisible(true);
    m_title->setText(title);    
    m_url->setText(url.url());
    m_comment->setText(QString());
    m_comment->setVisible(true);
    m_commentLabel->setVisible(true);
    setParentBookmark(parent);
    m_folderTree->setVisible(true);

    m_mode = NewBookmark;
    aboutToShow(m_mode);

    if(exec() == QDialog::Accepted)
        return m_bm;
    else
        return KBookmark();
}

KBookmarkGroup KBookmarkDialog::addBookmarks(const QList<QPair<QString, QString> > & list, const QString & name, KBookmarkGroup parent)
{
    if(!m_layout)
        initLayoutPrivate();
    if(parent.isNull())
        parent = m_mgr->root();

    m_list = list;

    setButtons( User1 | Ok | Cancel);
    setButtonGuiItem( KDialog::Ok,  KGuiItem( i18nc("@action:button", "Add" ), "bookmark-new") );
    setCaption( i18nc("@title:window","Add Bookmarks") );
    setButtonGuiItem( User1, KGuiItem( i18nc("@action:button", "&New Folder..." ), "folder-new") );
    m_url->setVisible(false);
    m_urlLabel->setVisible(false);
    m_title->setText(name);
    m_comment->setVisible(true);
    m_commentLabel->setVisible(true);
    m_comment->setText(QString());
    setParentBookmark(parent);
    m_folderTree->setVisible(true);

    m_mode = NewMultipleBookmarks;
    aboutToShow(m_mode);
    
    if(exec() == QDialog::Accepted)
        return m_bm.toGroup();
    else
        return KBookmarkGroup();
}

KBookmarkGroup KBookmarkDialog::selectFolder(KBookmark parent)
{
    if(!m_layout)
        initLayoutPrivate();
    if(parent.isNull())
        parent = m_mgr->root();
    setButtons( User1 | Ok | Cancel );
    setButtonGuiItem( KDialog::Ok, KStandardGuiItem::ok() );
    setButtonGuiItem( User1, KGuiItem( i18nc("@action:button", "&New Folder..." ), "folder-new") );
    setCaption( i18nc("@title:window","Select Folder"));
    m_url->setVisible(false);
    m_urlLabel->setVisible(false);
    m_title->setVisible(false);
    m_titleLabel->setVisible(false);
    m_comment->setVisible(false);
    m_commentLabel->setVisible(false);
    setParentBookmark(parent);
    m_folderTree->setVisible(true);

    m_mode = SelectFolder;
    aboutToShow(m_mode);

    if(exec() == QDialog::Accepted)
        return m_bm.toGroup();
    else
        return KBookmarkGroup();
}

KBookmarkGroup KBookmarkDialog::createNewFolder(const QString & name, KBookmark parent)
{
    if(!m_layout)
        initLayoutPrivate();
    if(parent.isNull())
        parent = m_mgr->root();
    setButtons( Ok | Cancel );
    setButtonGuiItem( KDialog::Ok, KStandardGuiItem::ok() );
    setCaption( i18nc("@title:window","New Folder"));
    m_url->setVisible(false);
    m_urlLabel->setVisible(false);
    m_comment->setVisible(true);
    m_commentLabel->setVisible(true);
    m_comment->setText(QString());
    m_title->setText(name);
    setParentBookmark(parent);
    m_folderTree->setVisible(true);

    m_mode = NewFolder;
    aboutToShow(m_mode);

    if(exec() == QDialog::Accepted)
        return m_bm.toGroup();
    else
        return KBookmarkGroup();
}

void KBookmarkDialog::setParentBookmark(const KBookmark & bm)
{
    QString address = bm.address();
    KBookmarkTreeItem * item = static_cast<KBookmarkTreeItem *>(m_folderTree->topLevelItem(0));
    while(true)
    {
        if(item->address() == bm.address())
        {
            m_folderTree->setCurrentItem(item);
            return;
        }
        for(int i=0; i<item->childCount(); ++i)
        {
            KBookmarkTreeItem * child = static_cast<KBookmarkTreeItem *>(item->child(i));
            if( KBookmark::commonParent(child->address(), address) == child->address())
            {
                item = child;
                break;
            }
        }
    }
}

KBookmarkGroup KBookmarkDialog::parentBookmark()
{
    KBookmarkTreeItem *item = dynamic_cast<KBookmarkTreeItem *>(m_folderTree->currentItem());
    if(!item)
        return m_mgr->root();
    const QString &address = item->address();
    return m_mgr->findByAddress(address).toGroup();
}

void KBookmarkDialog::slotButtonClicked(int button)
{
    if(button == Ok)
    {
        if(m_mode == NewFolder)
        {
            KBookmarkGroup parent = parentBookmark();
            if(m_title->text().isEmpty())
                m_title->setText("New Folder");
            m_bm = parent.createNewFolder(m_title->text());
            m_bm.setDescription(m_comment->text());
            save(m_mode, m_bm);
            m_mgr->emitChanged(parent);
        } else if(m_mode == NewBookmark) {
            KBookmarkGroup parent = parentBookmark();
            if(m_title->text().isEmpty())
                m_title->setText("New Bookmark");
            m_bm = parent.addBookmark(m_title->text(), KUrl(m_url->text()));
            m_bm.setDescription(m_comment->text());
            save(m_mode, m_bm);
            m_mgr->emitChanged(parent);
        } else if(m_mode == NewMultipleBookmarks) {
            KBookmarkGroup parent = parentBookmark();
            if(m_title->text().isEmpty())
                m_title->setText("New Folder");
            m_bm = parent.createNewFolder(m_title->text());
            m_bm.setDescription(m_comment->text());
            QList< QPair<QString, QString> >::iterator  it, end;
            end = m_list.end();
            for(it = m_list.begin(); it!= m_list.end(); ++it)
            {
                m_bm.toGroup().addBookmark( (*it).first, KUrl((*it).second));
            }
            save(m_mode, m_bm);
            m_mgr->emitChanged(parent);
        } else if(m_mode == EditBookmark) {
            m_bm.setFullText(m_title->text());
            m_bm.setUrl(KUrl(m_url->text()));
            m_bm.setDescription(m_comment->text());
            save(m_mode, m_bm);
            m_mgr->emitChanged(m_bm.parentGroup());
        } else if(m_mode == SelectFolder) {
            m_bm = parentBookmark();
            save(m_mode, m_bm);
        }
    }
    KDialog::slotButtonClicked(button);
}

void KBookmarkDialog::save(BookmarkDialogMode , const KBookmark & )
{

}

void KBookmarkDialog::aboutToShow(BookmarkDialogMode mode)
{
    Q_UNUSED(mode);
}

void KBookmarkDialog::initLayout()
{
    QBoxLayout *vbox = new QVBoxLayout( m_main );
    vbox->setMargin(0);
    QFormLayout * form = new QFormLayout();
    vbox->addLayout(form);

    form->addRow( m_titleLabel, m_title );
    form->addRow( m_urlLabel, m_url );
    form->addRow( m_commentLabel, m_comment );

    vbox->addWidget(m_folderTree);
}


void KBookmarkDialog::initLayoutPrivate()
{
    m_main = new QWidget( this );
    setMainWidget( m_main );
    connect( this, SIGNAL( user1Clicked() ), SLOT( newFolderButton() ) );

    m_title = new KLineEdit( m_main );
    m_title->setMinimumWidth(300);
    m_titleLabel = new QLabel( i18nc("@label:textbox", "Name:" ), m_main );
    m_titleLabel->setBuddy( m_title );

    m_url = new KLineEdit( m_main );
    m_url->setMinimumWidth(300);
    m_urlLabel = new QLabel( i18nc("@label:textbox", "Location:" ), m_main );
    m_urlLabel->setBuddy( m_url );

    m_comment = new KLineEdit( m_main );
    m_comment->setMinimumWidth(300);
    m_commentLabel = new QLabel( i18nc("@label:textbox", "Comment:" ), m_main );
    m_commentLabel->setBuddy( m_comment );

    m_folderTree = new QTreeWidget(m_main);
    m_folderTree->setColumnCount(1);
    m_folderTree->header()->hide();
    m_folderTree->setSortingEnabled(false);
    m_folderTree->setSelectionMode( QTreeWidget::SingleSelection );
    m_folderTree->setSelectionBehavior( QTreeWidget::SelectRows );
    m_folderTree->setMinimumSize( 60, 100 );
    QTreeWidgetItem *root = new KBookmarkTreeItem(m_folderTree);    
    fillGroup( root, m_mgr->root() );

    initLayout();
    m_layout = true;
}


KBookmarkDialog::KBookmarkDialog(KBookmarkManager * mgr, QWidget * parent )
  : KDialog(parent),
    m_folderTree(0), m_mgr(mgr), m_layout(false)
{
 
}

void KBookmarkDialog::newFolderButton()
{

    QString caption = parentBookmark().fullText().isEmpty() ?
                      i18nc("@title:window","Create New Bookmark Folder" ) :
                      i18nc("@title:window","Create New Bookmark Folder in %1" ,
                        parentBookmark().text() );
    bool ok;
    QString text = KInputDialog::getText( caption, i18nc("@label:textbox", "New folder:" ), QString(), &ok );
    if ( !ok )
        return;

    KBookmarkGroup group = parentBookmark().createNewFolder(text);
    if ( !group.isNull() )
    {
        KBookmarkGroup parentGroup = group.parentGroup();
        m_mgr->emitChanged( parentGroup );
        m_folderTree->clear();
        QTreeWidgetItem *root = new KBookmarkTreeItem(m_folderTree);
        fillGroup( root, m_mgr->root() );
    }
}

void KBookmarkDialog::fillGroup( QTreeWidgetItem * parentItem, const KBookmarkGroup &group)
{
  for ( KBookmark bk = group.first() ; !bk.isNull() ; bk = group.next(bk) )
  {
    if ( bk.isGroup() )
    {
      QTreeWidgetItem* item = new KBookmarkTreeItem(parentItem, m_folderTree, bk.toGroup() );
      fillGroup( item, bk.toGroup() );
    }
  }
}

/********************************************************************/

KBookmarkTreeItem::KBookmarkTreeItem(QTreeWidget * tree)
    : QTreeWidgetItem(tree), m_address("")
{
    setText(0, i18nc("name of the container of all browser bookmarks","Bookmarks"));
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
