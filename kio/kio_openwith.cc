#include <qfile.h>
#include <qdir.h>
#include <qdialog.h>
#include <qpixmap.h>

#include <kapp.h>
#include <kglobal.h>
#include <klined.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <ktreelist.h>
#include <ktopwidget.h>
#include <kiconloader.h>
#include <kservices.h>
#include <kmimemagic.h>
#include <kdebug.h>
#include <kstddirs.h>

#include <string.h>
#include <stdio.h>

#include "kio_openwith.h"

#define SORT_SPEC (QDir::DirsFirst | QDir::Name | QDir::IgnoreCase)

// ----------------------------------------------------------------------

KAppTreeListItem::KAppTreeListItem( const char *name, QPixmap *pixmap,
     bool d, bool parse, bool dir, QString p, QString c )
    : KTreeListItem( name, pixmap )
{
  dummy = d;
  parsed = parse;
  directory = dir;
  path = p;
  c.simplifyWhiteSpace();
  int pos;
  if( (pos = c.find( ' ')) > 0 )
    exec = c.left( pos );
  else
    exec = c;
  appname = name;
}

// ----------------------------------------------------------------------

KApplicationTree::KApplicationTree( QWidget *parent ) : QWidget( parent )
{
  tree = new KTreeList( this );
  tree->setSmoothScrolling( true );
  setFocusProxy( tree );

  QStringList list = KGlobal::dirs()->getResourceDirs("apps");
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); it++)
      parseDesktopDir( QDir(*it), tree );

  tree->show();
  connect( tree, SIGNAL( expanded(int) ), SLOT( expanded(int) ) );
  connect( tree, SIGNAL( highlighted(int) ), SLOT( highlighted(int) ) );
  connect( tree, SIGNAL( selected(int) ), SLOT( selected(int) ) );
}

bool KApplicationTree::isDesktopFile( const char* filename )
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

void KApplicationTree::parseDesktopFile( QFileInfo *fi, KTreeList *tree, KAppTreeListItem *item )
{
  QPixmap pixmap;
  QString text_name, pixmap_name, mini_pixmap_name, big_pixmap_name, command_name, comment;

  QString file = fi->absFilePath();

  if( fi->isDir() )
  {
    text_name = fi->fileName();
    file += "/.directory";
  }
  else
  {
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

  kdebug(KDEBUG_INFO, 7007, "Processing desktop file %s",file.ascii());

  QFile config( file );

  if( config.exists() )
  {
    KSimpleConfig kconfig( file, true );
    kconfig.setDesktopGroup();
    command_name      = kconfig.readEntry("Exec");
    mini_pixmap_name  = kconfig.readEntry("MiniIcon");
    big_pixmap_name   = kconfig.readEntry("Icon");
    comment           = kconfig.readEntry("Comment");
    text_name         = kconfig.readEntry("Name", text_name);

    if( !mini_pixmap_name.isEmpty() )
      pixmap = KGlobal::iconLoader()->loadApplicationMiniIcon(mini_pixmap_name.ascii(), 16, 16);
    if( pixmap.isNull() && !big_pixmap_name.isEmpty() )
      pixmap = KGlobal::iconLoader()->loadApplicationMiniIcon(big_pixmap_name.ascii(), 16, 16);
    if( pixmap.isNull() )
      pixmap = KGlobal::iconLoader()->loadApplicationMiniIcon("mini-default.xpm", 16, 16);	
  }
  else
  {
     command_name = text_name;
     pixmap = KGlobal::iconLoader()->loadApplicationMiniIcon("mini-default.xpm", 16, 16);
  }

  it2 = new KAppTreeListItem( text_name.data(), &pixmap, false, false, fi->isDir(), fi->absFilePath(), command_name );	

    if( item == 0 )
      tree->insertItem( it2 );
    else
      item->appendChild( it2 );

  if( fi->isDir() )
  {
    dummy = new KAppTreeListItem( i18n("This group is empty!").ascii(), 0, true, false, false, "", "" );
    it2->appendChild( dummy );
  }
}

short KApplicationTree::parseDesktopDir( QDir d, KTreeList *tree, KAppTreeListItem *item)
{
  if( !d.exists() )
    return -1;

  d.setSorting( SORT_SPEC );
  QList <QString> item_list;
	
  const QFileInfoList *list = d.entryInfoList();
  QFileInfoListIterator it( *list );
  QFileInfo *fi;

  if( it.count() < 3 )
    return -1;

  while( ( fi = it.current() ) )
  {
    if( fi->fileName() == "." || fi->fileName() == ".." )
    {
      ++it;
      continue;
    }
    if( fi->isDir() )
    {
      parseDesktopFile( fi, tree, item );
    }
    else
    {
      if( !isDesktopFile( fi->absFilePath().ascii() ) )
      {
	++it;
	continue;
      }
      parseDesktopFile( fi, tree, item );
    }
    ++it;
  }

  return 0;
}

void KApplicationTree::expanded(int index)
{
  KAppTreeListItem *item = (KAppTreeListItem *)tree->itemAt( index );

  if( !item->parsed && !(item->dummy ) )
  {
     parseDesktopDir( QDir(item->path), tree, item );
     item->parsed = true;
     if( item->childCount() > 1 )
        item->removeChild( item->getChild() );
  }
}


void KApplicationTree::highlighted(int index)
{

  KAppTreeListItem *item = (KAppTreeListItem *)tree->itemAt( index );

  if( ( !item->directory ) &&
      !( item->exec.isEmpty() ) &&
      !( item->appname.isEmpty() ) )

    emit highlighted( item->appname.data(), item->exec.data() );

}


void KApplicationTree::selected(int index)
{
  KAppTreeListItem *item = (KAppTreeListItem *)tree->itemAt( index );

  if( ( !item->directory ) && !( item->exec.isEmpty() ) && !( item->appname.isEmpty() ) )
     emit selected( item->appname.data(), item->exec.data() );
  else
    tree->expandOrCollapseItem( index );
}

void KApplicationTree::resizeEvent( QResizeEvent * )
{
  tree->setGeometry( 0, 0, width(), height() );
}

/***************************************************************
 *
 * OpenWithDlg
 *
 ***************************************************************/

OpenWithDlg::OpenWithDlg( const QString&_text, const QString& _value, QWidget *parent, bool _file_mode )
        : QDialog( parent, 0L, true )
{
  m_pTree = 0L;
  m_pService = 0L;
  haveApp = false;

  setGeometry( x(), y(), 370, 100 );

  label = new QLabel( _text , this );
  label->setGeometry( 10, 10, 350, 15 );

  edit = new KLineEdit( this, 0L );

  if ( _file_mode )
  {
    completion = new KURLCompletion();
    connect ( edit, SIGNAL (completion()),
	      completion, SLOT (make_completion()));
    connect ( edit, SIGNAL (rotation()),
	      completion, SLOT (make_rotation()));
    connect ( edit, SIGNAL (textChanged(const QString&)),
	      completion, SLOT (edited(const QString&)));
    connect ( completion, SIGNAL (setText (const QString&)),
	      edit, SLOT (setText (const QString&)));
  }
  else
    completion = 0L;

  edit->setGeometry( 10, 35, 350, 25 );
  connect( edit, SIGNAL(returnPressed()), SLOT(accept()) );

  ok = new QPushButton( i18n("OK"), this );
  ok->setGeometry( 10,70, 80,25 );
  connect( ok, SIGNAL(clicked()), SLOT(slotOK()) );

  browse = new QPushButton( i18n("&Browser"), this );
  browse->setGeometry( 100, 70, 80, 25 );
  connect( browse, SIGNAL(clicked()), SLOT(slotBrowse()) );

  clear = new QPushButton( i18n("Clear"), this );
  clear->setGeometry( 190, 70, 80, 25 );
  connect( clear, SIGNAL(clicked()), SLOT(slotClear()) );

  cancel = new QPushButton( i18n("Cancel"), this );
  cancel->setGeometry( 280, 70, 80, 25 );
  connect( cancel, SIGNAL(clicked()), SLOT(reject()) );

  edit->setText( _value );
  edit->setFocus();
  haveApp = false;
}

OpenWithDlg::~OpenWithDlg()
{
  delete completion;
}

void OpenWithDlg::slotClear()
{
  edit->setText("");
}

void OpenWithDlg::slotBrowse()
{
  if ( m_pTree )
    return;

  browse->setEnabled( false );

  ok->setGeometry( 10,280, 80,25 );
  browse->setGeometry( 100, 280, 80, 25 );
  clear->setGeometry( 190, 280, 80, 25 );
  cancel->setGeometry( 280, 280, 80, 25 );

  m_pTree = new KApplicationTree( this );
  connect( m_pTree, SIGNAL( selected( const QString&, const QString& ) ), this, SLOT( slotSelected( const QString&, const QString& ) ) );

  connect( m_pTree, SIGNAL( highlighted( const QString&, const QString& ) ), this, SLOT( slotHighlighted( const QString&, const QString& ) ) );

  m_pTree->setGeometry( 10, 70, 350, 200 );
  m_pTree->show();
  m_pTree->setFocus();

  resize( width(), height() + 210 );
}

void OpenWithDlg::resizeEvent(QResizeEvent *)
{
  // someone will have to write proper geometry management
  // but for now this will have to do ....

  if( m_pTree )
  {
    label->setGeometry( 10, 10, width() - 20, 15 );
    edit->setGeometry( 10, 35, width() - 20, 25 );
    ok->setGeometry( 10,height() - 30, (width()-20-30)/4,25 );
    browse->setGeometry( 10 + (width()-20-30)/4 + 10
			 ,height() - 30, (width()-20-30)/4, 25 );
    clear->setGeometry(10 + 2*((width()-20-30)/4) + 2*10
		       ,height() - 30, (width()-20-30)/4, 25 );
    cancel->setGeometry( 10 + 3*((width()-20-30)/4) + 3*10 ,
			 height() - 30, (width()-20-30)/4, 25 );
    m_pTree->setGeometry( 10, 70, width() - 20, height() - 110 );

  }
  else
  {
    label->setGeometry( 10, 10, width() - 20, 15 );
    edit->setGeometry( 10, 35, width() - 20, 25 );
    ok->setGeometry( 10,height() - 30, (width()-20-30)/4,25);
    browse->setGeometry( 10 + (width()-20-30)/4 + 10,
			 height() - 30, (width()-20-30)/4, 25 );
    clear->setGeometry( 10 + 2*((width()-20-30)/4) + 2*10,
			height() - 30, (width()-20-30)/4, 25 );
    cancel->setGeometry( 10 + 3*((width()-20-30)/4) + 3*10,
			 height() - 30, (width()-20-30)/4, 25 );
  }
}

void OpenWithDlg::slotSelected( const QString& _name, const QString& _exec )
{
  m_pService = KService::service( _name );
  if ( !m_pService )
    edit->setText( _exec );
  else
    edit->setText( "" );

  accept();
}

void OpenWithDlg::slotHighlighted( const QString& _name, const QString& )
{
  qName = _name;
  haveApp = true;
}

void OpenWithDlg::slotOK()
{
  if( haveApp )
  {
    m_pService = KService::service( qName );
  }

  haveApp = false;
  accept();
}
#include "kio_openwith.moc"

