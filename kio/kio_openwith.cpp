/*  This file is part of the KDE libraries
    
    Copyright (C) 1997 Torben Weis (weis@stud.uni-frankfurt.de)
    Copyright (C) 1999 Dirk A. Mueller (dmuell@gmx.net)
 
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

// $Id$

#include <qapplication.h>
#include <qfile.h>
#include <qdir.h>
#include <qdialog.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

#include <kbuttonbox.h>
#include <kglobal.h>
#include <klined.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmimemagic.h>
#include <kstddirs.h>

#include "kio_openwith.h"
#include "krun.h"

#define SORT_SPEC (QDir::DirsFirst | QDir::Name | QDir::IgnoreCase)


// ----------------------------------------------------------------------

KAppTreeListItem::KAppTreeListItem( QListView* parent, const char *name,
                                    const QPixmap& pixmap, bool parse, bool dir, QString p, QString c )
    : QListViewItem( parent, name )
{
    init(pixmap, parse, dir, p, c);
}


// ----------------------------------------------------------------------

KAppTreeListItem::KAppTreeListItem( QListViewItem* parent, const char *name,
                                    const QPixmap& pixmap, bool parse, bool dir, QString p, QString c )
    : QListViewItem( parent, name )
{
    init(pixmap, parse, dir, p, c);
}


// ----------------------------------------------------------------------

void KAppTreeListItem::init(const QPixmap& pixmap, bool parse, bool dir, QString _path, QString _exec)
{
    setPixmap(0, pixmap);
    parsed = parse;
    directory = dir;
    path = _path; // relative path
    exec = _exec;
    exec.simplifyWhiteSpace();
    exec.truncate(exec.find(' '));
}


// ----------------------------------------------------------------------
// Ensure that dirs are sorted in front of files and case is ignored

QString KAppTreeListItem::key(int column, bool /*ascending*/) const
{
    if(directory)
        return " " + text(column).upper();
    else
        return text(column).upper();
}

void KAppTreeListItem::activate()
{
    if ( directory )
        setOpen(!isOpen());
}

void KAppTreeListItem::setOpen( bool o )
{
    if( o && !parsed ) { // fill the children before opening
        ((KApplicationTree *) parent())->parseDesktopDir( path + "/", this );
        parsed = true;
    }
    QListViewItem::setOpen( o );
}

// ----------------------------------------------------------------------

KApplicationTree::KApplicationTree( QWidget *parent )
    : QListView( parent )
{
    addColumn( i18n("Known Applications") );
    setRootIsDecorated( true );
    
    parseDesktopDir( "" );
    
    connect( this, SIGNAL( currentChanged(QListViewItem*) ), SLOT( slotItemHighlighted(QListViewItem*) ) );
    connect( this, SIGNAL( selectionChanged(QListViewItem*) ), SLOT( slotSelectionChanged(QListViewItem*) ) );
}


// ----------------------------------------------------------------------

bool KApplicationTree::isDesktopFile( const QString& filename )
{
    FILE *f;
    f = fopen( filename, "rb" );
    if ( f == 0L )
        return false;
    
    char buff[ 100 ];
    buff[ 0 ] = 0;
    fgets( buff, 100, f );
    fclose( f );
    
    KMimeMagicResult *result = KMimeMagic::self()->findBufferType( buff, 100 );
    if (!result)
        return false;

    if (result->mimeType() != "application/x-desktop")
        return false;

    return true;
}


// ----------------------------------------------------------------------

void KApplicationTree::parseDesktopFile( QFileInfo *fi, KAppTreeListItem *item, QString relPath )
{
    QPixmap pixmap;
    QString text_name, pixmap_name, mini_pixmap_name, big_pixmap_name, command_name, comment;
    
    QString file = fi->absFilePath();
    
    if( fi->isDir() ) {
        text_name = fi->fileName();
        file += "/.directory";
    }
    else {
        int pos = fi->fileName().find( ".desktop" );
        if( pos >= 0 )
            text_name = fi->fileName().left( pos );
        else {
            pos = fi->fileName().find(".kdelnk");
            if (pos >= 0)
                text_name = fi->fileName().left(pos);
            else
                text_name = fi->fileName();
        }
    }

    QFile config( file );
    
    if( config.exists() ) {
        KSimpleConfig kconfig( file, true );
        kconfig.setDesktopGroup();
        command_name      = kconfig.readEntry("Exec");
        mini_pixmap_name  = kconfig.readEntry("MiniIcon");
        big_pixmap_name   = kconfig.readEntry("Icon");
        comment           = kconfig.readEntry("Comment");
        text_name         = kconfig.readEntry("Name", text_name);
        
        if( !mini_pixmap_name.isEmpty() )
            pixmap = KGlobal::iconLoader()->loadApplicationMiniIcon(mini_pixmap_name, 16, 16);
        if( pixmap.isNull() && !big_pixmap_name.isEmpty() )
            pixmap = KGlobal::iconLoader()->loadApplicationMiniIcon(big_pixmap_name, 16, 16);
        if( pixmap.isNull() )
            pixmap = KGlobal::iconLoader()->loadApplicationMiniIcon("default.png", 16, 16);
    }
    else {
        command_name = text_name;
        pixmap = KGlobal::iconLoader()->loadApplicationMiniIcon("default.png", 16, 16);
    }

    KAppTreeListItem * newItem;
    if(item)
      newItem = new KAppTreeListItem( item, text_name.ascii(), pixmap, false, fi->isDir(),
                                      relPath + fi->fileName(), command_name );
    else
      newItem = new KAppTreeListItem( this, text_name.ascii(), pixmap, false, fi->isDir(),
                                      relPath + fi->fileName(), command_name );

    if ( fi->isDir() )
      newItem->setExpandable( true );
}


// ----------------------------------------------------------------------

void KApplicationTree::parseDesktopDir( QString relPath, KAppTreeListItem *item)
{
  // for one relative path, like "Applications", there can be several real
  // dirs (ex : a global one and a local one). Parse them both.
    QStringList list = KGlobal::dirs()->findDirs("apps", relPath);
    for (QStringList::ConstIterator dirsit = list.begin(); dirsit != list.end(); dirsit++)
    {
      //debug(QString("(*dirsit): '%1'").arg((*dirsit))); 
      QDir d( (*dirsit) );
      if( d.exists() )
      {
        d.setSorting( SORT_SPEC );
        QList <QString> item_list;
	
        const QFileInfoList *list = d.entryInfoList();
        QFileInfoListIterator it( *list );
        QFileInfo *fi;
        
        if( it.count() < 3 ) // empty dir
          return;
        
        while( ( fi = it.current() ) ) {
          if( fi->fileName() == "." || fi->fileName() == ".." ) {
            ++it;
            continue;
          }
          // check if item already in list (e.g. parsed from ~/.kde)
          QListViewItem * pChild = item ? item->firstChild() : this->firstChild();
          bool found = false;
          while( pChild && !found ) {
            found = ( ((KAppTreeListItem *)pChild)->path == relPath + fi->fileName() );
            pChild = pChild->nextSibling();
          }
          if ( found )
          {
            //debug(QString("skipping %1 | %2").arg(*dirsit).arg(fi->fileName()));
            ++it;
            continue;
          }
          
          if( fi->isDir() ) {
            parseDesktopFile( fi, item, relPath );
          }
          else {
            if( !isDesktopFile( fi->absFilePath() ) ) {
              ++it;
              continue;
            }
            parseDesktopFile( fi, item, relPath );
          }
          ++it;
        }
      }
    }
}


// ----------------------------------------------------------------------

void KApplicationTree::slotItemHighlighted(QListViewItem* i)
{
    // i may be 0 (see documentation)
    if(!i)
        return;
    
    KAppTreeListItem *item = (KAppTreeListItem *) i;
    
    if( (!item->directory ) && (!item->exec.isEmpty()) )
        emit highlighted( item->text(0), item->exec );
}


// ----------------------------------------------------------------------

void KApplicationTree::slotSelectionChanged(QListViewItem* i)
{
    // i may be 0 (see documentation)
    if(!i)
        return;
    
    KAppTreeListItem *item = (KAppTreeListItem *) i;

    if( ( !item->directory ) && (!item->exec.isEmpty() ) )
        emit selected( item->text(0), item->exec );
}

// ----------------------------------------------------------------------

void KApplicationTree::resizeEvent( QResizeEvent * e)
{
    setColumnWidth(0, width()-QApplication::style().scrollBarExtent().width());
    QListView::resizeEvent(e);
}


/***************************************************************
 *
 * KOpenWithDlg
 *
 ***************************************************************/

KOpenWithDlg::KOpenWithDlg( const QStringList& _url, const QString&_text, const QString& _value, QWidget *parent)
    : QDialog( parent, 0L, true )
{
    m_pTree = 0L;
    m_pService = 0L;
    haveApp = false;

    setCaption(_url.first());

    QBoxLayout* topLayout = new QVBoxLayout(this, 15, 10);
  
    label = new QLabel( _text , this );
    topLayout->addWidget(label);
    
    QBoxLayout* l = new QHBoxLayout(topLayout, 10);    
    edit = new KLineEdit( this, 0L );
    edit->setMinimumWidth(200);
    l->addWidget(edit);
    
    completion = new KURLCompletion();
    connect ( edit, SIGNAL (completion()), completion, SLOT (make_completion()));
    connect ( edit, SIGNAL (rotation()), completion, SLOT (make_rotation()));
    connect ( edit, SIGNAL (textChanged(const QString&)), completion, SLOT (edited(const QString&)));
    connect ( completion, SIGNAL (setText (const QString&)), edit, SLOT (setText (const QString&)));
    connect ( edit, SIGNAL(returnPressed()), SLOT(accept()) );

    terminal = new QCheckBox( i18n("Run in terminal"), this );
    l->addWidget(terminal);
    
    m_pTree = new KApplicationTree( this );
    topLayout->addWidget(m_pTree);
        
    connect( m_pTree, SIGNAL( selected( const QString&, const QString& ) ), this, SLOT( slotSelected( const QString&, const QString& ) ) );
    connect( m_pTree, SIGNAL( highlighted( const QString&, const QString& ) ), this, SLOT( slotHighlighted( const QString&, const QString& ) ) );

    // Use KButtonBox for the aligning pushbuttons nicely
    KButtonBox* b = new KButtonBox(this);
    clear = b->addButton( i18n("C&lear") );
    b->addStretch(1);
    connect( clear, SIGNAL(clicked()), SLOT(slotClear()) );

    ok = b->addButton( i18n ("&OK") );
    ok->setDefault(true);
    connect( ok, SIGNAL(clicked()), SLOT(slotOK()) );

    cancel = b->addButton( i18n("&Cancel") );
    connect( cancel, SIGNAL(clicked()), SLOT(reject()) );

    b->layout();
    topLayout->addWidget(b);
    
    edit->setText( _value );
    edit->setFocus();
    haveApp = false;
}


// ----------------------------------------------------------------------

KOpenWithDlg::~KOpenWithDlg()
{
    delete completion;
}

// ----------------------------------------------------------------------

void KOpenWithDlg::slotClear()
{
    edit->setText("");
}


// ----------------------------------------------------------------------

void KOpenWithDlg::slotSelected( const QString& /*_name*/, const QString& _exec )
{
    edit->setText( _exec );
}


// ----------------------------------------------------------------------

void KOpenWithDlg::slotHighlighted( const QString& _name, const QString& )
{
    qName = _name;
    haveApp = true;
}


// ----------------------------------------------------------------------

void KOpenWithDlg::slotOK()
{
    if( haveApp ) 
        m_pService = KService::service( qName );

    if( terminal->isChecked() ) {
        KConfig conf("konquerorrc", true, false);
        conf.setGroup("Misc Defaults");
        QString t = conf.readEntry("Terminal", "konsole");

        t += " -e ";
        t += edit->text();
        edit->setText(t);
    }

    haveApp = false;
    accept();
}

#include "kio_openwith.moc"

