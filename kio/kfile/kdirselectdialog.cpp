/*
    Copyright (C) 2001 Carsten Pfeiffer <pfeiffer@kde.org>
    Copyright (C) 2001 Michael Jarrett <michaelj@corel.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qdir.h>
#include <qlayout.h>

#include <klocale.h>
#include <kurl.h>

#include "kfiletreeview.h"
#include "kdirselectdialog.h"

// ### add mutator for treeview!

KDirSelectDialog::KDirSelectDialog(const QString &startDir, bool localOnly,
                                   QWidget *parent, const char *name,
                                   bool modal)
    : KDialogBase( parent, name, modal, i18n("Select a Directory"), Ok|Cancel),
      m_startDir( startDir ),
      m_localOnly( localOnly )
{
    QFrame *page = makeMainWidget();
    m_mainLayout = new QVBoxLayout(page, marginHint(), spacingHint());

    if ( m_startDir.isEmpty() )
        m_startDir = "/";

    // Create dir list
    m_treeView = new KFileTreeView( page );
    m_treeView->addColumn( i18n("Directory") );
    m_treeView->setColumnWidthMode( 0, QListView::Maximum );
    m_treeView->setResizeMode( QListView::AllColumns );
    m_mainLayout->addWidget(m_treeView, 1);
}


KDirSelectDialog::~KDirSelectDialog()
{
}

KURL KDirSelectDialog::url() const
{
    return m_treeView->currentURL();
}

KURL KDirSelectDialog::selectDirectory( const QString& startDir,
                                        bool localOnly,
                                        QWidget *parent,
                                        const QString& caption)
{
    KDirSelectDialog myDialog( startDir, localOnly, parent,
                               "kdirselect dialog", true );
    KURL root;
    root.setPath(myDialog.startDir());

    KFileTreeView *view = myDialog.view();
    KFileTreeBranch *rootBranch = view->addBranch( root, myDialog.startDir() );
    view->setDirOnlyMode( rootBranch, true );

    rootBranch->setOpen(true);
    
    if ( !caption.isNull() )
        myDialog.setCaption( caption );

    if ( myDialog.exec() == QDialog::Accepted )
        return myDialog.url();
    else
        return KURL();
}

void KDirSelectDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

#include "kdirselectdialog.moc"
