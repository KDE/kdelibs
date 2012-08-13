#include "kdockwidgetdemo.h"

#include <QApplication>
#include <Qt3Support/Q3Header>
#include <QToolButton>

#include <Qt3Support/Q3TextView>
#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QLabel>
#include <Qt3Support/Q3MultiLineEdit>
#include <QKeyEvent>
#include <Qt3Support/Q3PopupMenu>
#include <QPushButton>
#include <QtCore/QPoint>
#include <QMessageBox>
#include <QMacMime>
#include <Qt3Support/Q3StrList>
#include <QPainter>
//Added by qt3to4:
#include <QPixmap>

#include <kconfiggroup.h>
#include <ksharedconfig.h>
//#include <kimgio.h>
#include <stdlib.h>

static const char *dir_tree_xpm[] = {
"16 16 8 1",
"  c Gray0",
". c #000080",
"X c Cyan",
"o c #808000",
"O c Yellow",
"+ c #808080",
"@ c None",
"# c #c1c1c1",
"@@@@@@@@@@@@@@@@",
"@@@+++++@@@@@@@@",
"@@+@O@O@+@@@@@@@",
"@+@O@O@O@++++++@",
"@+@@@@@@@@@@@@o ",
"@+@O@++ +O@O@Oo ",
"@+@@+@X@ +O@O@o ",
"@+@+@X@X@ +O@Oo ",
"@+@+X@X@X o@O@o ",
"@+@+@X@X@ oO@Oo ",
"@+@@+@X@ +ooO@o ",
"@+@O@+  +@..oOo ",
"@+ooooooo.X..oo ",
"@@        .X.. @",
"@@@@@@@@@@@.X.. ",
"@@@@@@@@@@@@.X. "
};

static const char *preview_xpm[] = {
"16 16 6 1",
"  c Gray0",
". c #000080",
"X c Yellow",
"o c #808080",
"O c None",
"+ c Gray100",
"OOOOOOOOOOOOOOOO",
"OOo     oOOOOOOO",
"Oo  oooo  OOOOOO",
"O  OOOOoo oOOOOO",
"O OOOO++oo OOOOO",
"O OOOO++Oo OOOOO",
"O O+OOOOOo OOOOO",
"O OO+OOOOo OOOOO",
"OooOO+OOo  OOOOO",
"OO oOOOo  + OOOO",
"OOOo     .X+ OOO",
"OOOOOOOOO .X+ OO",
"OOOOOOOOOO .X+ O",
"OOOOOOOOOOO .X O",
"OOOOOOOOOOOO  OO",
"OOOOOOOOOOOOOOOO"
};

SFileDialog::SFileDialog( QString initially, const QStringList& filter, const char* name )
:QDialog(0L)
{
  setObjectName(name);
  setModal(true);
  KConfigGroup config( KSharedConfig::openConfig(), QLatin1String("SFileDialogData:") + name );
  if ( initially.isNull() ){
    initially = config.readPathEntry( "InitiallyDir", QDir::currentPath() );
  }

  QStringList bookmark;
  bookmark = config.readEntry( "Bookmarks",QStringList() );

  dockManager = new K3DockManager(this);

  d_dirView = new K3DockWidget( dockManager, "Dock_DirView", QPixmap(dir_tree_xpm) );
  d_dirView->setWindowTitle("Tree");

  dirView = new DirectoryView( d_dirView, 0 );
  dirView->addColumn( "" );
  dirView->header()->hide();
  d_dirView->setWidget( dirView );

  Directory* root = new Directory( dirView, "/" );
  root->setOpen(true);

  d_preview = new K3DockWidget( dockManager, "Dock_Preview", QPixmap(preview_xpm) );
  d_preview->setWindowTitle("Preview");
  preview = new Preview( d_preview );
  d_preview->setWidget( preview );

  d_fd = new K3DockWidget( dockManager, "Dock_QFileDialog", QPixmap(), this );
  fd = new CustomFileDialog( d_fd );
  fd->setDir( initially );
  fd->setFilters( filter );
  fd->setBookmark( bookmark );
  fd->setParent( d_fd );
  d_fd->setWidget( fd );

  connect( dirView, SIGNAL(folderSelected(QString)), fd, SLOT(setDir2(QString)) );
  connect( fd, SIGNAL(dirEntered(QString)), dirView, SLOT(setDir(QString)) );

  d_fd->setDockSite( K3DockWidget::DockTop|K3DockWidget::DockLeft|K3DockWidget::DockRight|K3DockWidget::DockBottom );
  d_fd->setEnableDocking(K3DockWidget::DockNone);

  d_dirView->setEnableDocking(K3DockWidget::DockTop|K3DockWidget::DockLeft|K3DockWidget::DockRight|K3DockWidget::DockBottom|K3DockWidget::DockCenter);
  d_preview->setEnableDocking(K3DockWidget::DockTop|K3DockWidget::DockLeft|K3DockWidget::DockRight|K3DockWidget::DockBottom|K3DockWidget::DockCenter);

  d_dirView->manualDock( d_fd, K3DockWidget::DockLeft, 20 );
  d_preview->manualDock( d_fd, K3DockWidget::DockBottom, 70 );

  connect(fd, SIGNAL(fileHighlighted(QString)), preview, SLOT(showPreview(QString)));
  connect(fd, SIGNAL(signalDone(int)), this, SLOT(done(int)));

  connect(fd, SIGNAL(dirEntered(QString)), this, SLOT(changeDir(QString)));
  connect(dirView, SIGNAL(folderSelected(QString)), this, SLOT(changeDir(QString)));

  b_tree = new QToolButton( fd );
  b_tree->setToolTip( "Show/Hide Tree" );
  b_tree->setIcon( QIcon( QPixmap( dir_tree_xpm ) ) );
  connect( b_tree, SIGNAL(clicked()), d_dirView, SLOT(changeHideShowState()) );
  b_tree->setCheckable(true);
  b_tree->setChecked(true);
  fd->addToolButton( b_tree, true );

  b_preview = new QToolButton( fd );
  b_preview->setToolTip( "Show/Hide Preview" );
  b_preview->setIcon( QIcon( QPixmap( preview_xpm ) ) );
  connect( b_preview, SIGNAL(clicked()), d_preview, SLOT(changeHideShowState()) );
  b_preview->setCheckable(true);
  b_preview->setChecked(true);
  fd->addToolButton( b_preview );

  connect( dockManager, SIGNAL(change()), this, SLOT(dockChange()));
  connect( dockManager, SIGNAL(setDockDefaultPos(K3DockWidget*)), this, SLOT(setDockDefaultPos(K3DockWidget*)));
  setWindowTitle("Open File");
  resize(550,450);
  qDebug("read config");
  dockManager->readConfig( 0L , name );
}

void SFileDialog::dockChange()
{
  b_preview->setChecked( d_preview->isVisible() );
  b_tree->setChecked( d_dirView->isVisible() );
}

SFileDialog::~SFileDialog()
{
  KConfigGroup config(KSharedConfig::openConfig(), QString("SFileDialogData:") + objectName() );
  config.writeEntry( "Bookmarks", fd->getBookmark() );

  qDebug("write config");
  dockManager->writeConfig( 0L , objectName() );
}

void SFileDialog::setDockDefaultPos( K3DockWidget* d )
{
  if ( d == d_dirView ){
    d_dirView->manualDock( d_fd, K3DockWidget::DockLeft, 20 );
  }

  if ( d == d_preview ){
    d_preview->manualDock( d_fd, K3DockWidget::DockBottom, 70 );
  }
}

void SFileDialog::changeDir( const QString& f )
{
  if ( !f.isEmpty() ){
    KConfigGroup config( KSharedConfig::openConfig(), QString("SFileDialogData:") + objectName() );
    config.writePathEntry( "InitiallyDir", f );
  }
}

QString SFileDialog::getOpenFileName( QString initially,
                                      const QStringList& filter,
                                      const QString caption, const char* name )
{
  SFileDialog* fd = new SFileDialog( initially, filter, name );
  if ( !caption.isNull() ) fd->setWindowTitle( caption );
  QString result = ( fd->exec() == QDialog::Accepted ) ? fd->fd->selectedFile():QString();
  delete fd;

  return result;
}

QStringList SFileDialog::getOpenFileNames( QString initially,
                                      const QStringList& filter,
                                      const QString caption, const char* name )
{
  SFileDialog* fd = new SFileDialog( initially, filter, name );
  if ( !caption.isNull() ) fd->setWindowTitle( caption );

  fd->fd->setMode( Q3FileDialog::ExistingFiles );
  fd->d_preview->undock();
  fd->b_preview->hide();

  fd->exec();
  QStringList result = fd->fd->selectedFiles();
  delete fd;

  return result;
}

void SFileDialog::showEvent( QShowEvent *e )
{
  QDialog::showEvent( e );
  dirView->setDir( fd->dirPath() );
}

/******************************************************************************************************/
PixmapView::PixmapView( QWidget *parent )
:Q3ScrollView( parent )
{
//  kimgioRegister();
}

void PixmapView::setPixmap( const QPixmap &pix )
{
    pixmap = pix;
    resizeContents( pixmap.size().width(), pixmap.size().height() );
    viewport()->repaint();
}

void PixmapView::drawContents( QPainter *p, int, int, int, int )
{
    p->drawPixmap( 0, 0, pixmap );
}

Preview::Preview( QWidget *parent )
:Q3WidgetStack( parent )
{
    normalText = new Q3MultiLineEdit( this );
    normalText->setReadOnly( true );
    html = new Q3TextView( this );
    pixmap = new PixmapView( this );
    raiseWidget( normalText );
}

void Preview::showPreview( const QString &str )
{
  Q3Url u(str);
  if ( u.isLocalFile() ){
  QString path = u.path();
  QFileInfo fi( path );
	if ( fi.isFile() && (int)fi.size() > 400 * 1024 ) {
	    normalText->setText( tr( "The File\n%1\nis too large, so I don't show it!" ).arg( path ) );
	    raiseWidget( normalText );
	    return;
	}

	QPixmap pix( path );
	if ( pix.isNull() ) {
	    if ( fi.isFile() ) {
		QFile f( path );
		if ( f.open( QIODevice::ReadOnly ) ) {
		    QTextStream ts( &f );
		    QString text = ts.readAll();
		    f.close();
		    if ( fi.suffix().toLower().contains( "htm" ) ) {
			QString url = html->mimeSourceFactory()->makeAbsolute( path, html->context() );
			html->setText( text, url );
			raiseWidget( html );
			return;
		    } else {
			normalText->setText( text );
			raiseWidget( normalText );
			return;
		    }
		}
	    }
	    normalText->setText( QString() );
	    raiseWidget( normalText );
	} else {
	    pixmap->setPixmap( pix );
	    raiseWidget( pixmap );
	}
    } else {
	normalText->setText( "I only show local files!" );
	raiseWidget( normalText );
    }
}

// ****************************************************************************************************
static const char* homepage_xpm[] = {
"24 24 9 1",
"  c #262660",
". c #383666",
"X c #62639c",
"o c #7e86a5",
"O c #a6a7dd",
"+ c #bbbaed",
"@ c #c4c4f2",
"# c #f8f9f9",
"$ c None",
"$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$o.X$$$$$",
"$$$$$$$$$$oOOOO$O.X$$$$$",
"$$$$$$$$$$oXXXX+O.X$$$$$",
"$$$$$$$$$X#XXXXXO.X$$$$$",
"$$$$$$$$X  #XXXXO.X$$$$$",
"$$$$$$$X XO #XXXO.X$$$$$",
"$$$$$$X XOOO #XXX.XX$$$$",
"$$$$$X XOOOOO #XXXXXX$$$",
"$$$$X XOOOOOOO #XXXXXX$$",
"$$$X XOOOOOOOOO #XXXXXX$",
"$$X XOOOOOOOOOOO #.....$",
"$$$$$OOOOXXXXOOOOX...$$$",
"$$$$$OOOOXXXXOOOOX...$$$",
"$$$$$OOOOXXXXOOOOX..o$$$",
"$$$$$OOOOXXXXOOOOX.oo$$$",
"$$$$$OOOOXXXXOOOOXXoo$$$",
"$$$$$OOOOXXXXOOOOXooo$$$",
"$$$ooOOOOXXXXOOOOXoooo$$",
"$ooooXXXXXXXXXXXXXooooo$",
"$ooooooooooooooooooooooo",
"$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$"
};

static const char* folder_trash[] = {
"16 16 10 1",
"  c Gray0",
". c #222222",
"X c #333366",
"o c #6666cc",
"O c Magenta",
"+ c #808080",
"@ c #9999ff",
"# c #c0c0c0",
"$ c Gray100",
"% c None",
"%%%%%%oo%%%%%%%%",
"%%%%%o$$o%%%%%%%",
"%%%%o$$$$o%%%%%%",
"%%%o$+$+$$o o%%%",
"%%oo$$+$+$$o .%%",
"%%.oo$$+$+$$o.%%",
"%% Xoo$$+$$o +%%",
"%%o Xoo$$oo  o%%",
"%%%X.XX    +X%%%",
"%%%o@X+X#X+X+%%%",
"%%%+.$X#X+X %%%%",
"%%%%.o$o#X+X%%%%",
"%%%%.X@$X+ +%%%%",
"%%%%+Xo.#. %%%%%",
"%%%%%.Xo. o%%%%%",
"%%%%%+.   %%%%%%"
};

static const char* globalbookmark_xpm[]={
"12 16 3 1",
". c None",
"a c #000000",
"# c #008080",
"............",
"............",
"........##..",
".......#####",
"......#####a",
".....#####a.",
"....#####a..",
"...#####a...",
"..#####a....",
".#####a.....",
"aaa##a......",
"...#a.......",
"...a........",
"............",
"............",
"............"};

CustomFileDialog::CustomFileDialog( QWidget* parent )
: Q3FileDialog( parent, 0, false )
{
  QToolButton *p = new QToolButton( this );

  p->setIcon( QIcon( QPixmap( globalbookmark_xpm ) ) );
  p->setToolTip( tr("Bookmarks") );

  bookmarkMenu = new Q3PopupMenu( this );
  connect( bookmarkMenu, SIGNAL(activated(int)), this, SLOT(bookmarkChosen(int)) );
  addId = bookmarkMenu->insertItem( "Add bookmark" );
  clearId = bookmarkMenu->insertItem( QPixmap(folder_trash), "Clear bookmarks" );
  bookmarkMenu->addSeparator();

  p->setMenu( bookmarkMenu );
  p->setPopupDelay(0);
  addToolButton( p, true );

  QToolButton *b = new QToolButton( this );
  b->setToolTip( tr("Go Home!") );

  b->setIcon( QIcon( QPixmap( homepage_xpm ) ) );
  connect( b, SIGNAL(clicked()), this, SLOT(goHome()) );

  addToolButton( b );
}


void CustomFileDialog::setBookmark( QStringList &s )
{
	QStringList::Iterator it = s.begin();
	for ( ; it != s.end(); ++it ){
    bookmarkList << (*it);

    const char* book_pix[]={
    "12 16 3 1",
    ". c None",
    "a c #000000",
    "# c #008080",
    "............",
    "............",
    "........##..",
    ".......#####",
    "......#####a",
    ".....#####a.",
    "....#####a..",
    "...#####a...",
    "..#####a....",
    ".#####a.....",
    "aaa##a......",
    "...#a.......",
    "...a........",
    "............",
    "............",
    "............"};
    bookmarkMenu->insertItem( QIcon( book_pix ), (*it) );
	}
}

CustomFileDialog::~CustomFileDialog()
{
}

void CustomFileDialog::setDir2( const QString &s )
{
  blockSignals( true );
  setDir( s );
  blockSignals( false );
}

void CustomFileDialog::bookmarkChosen( int i )
{
  if ( i == clearId ){
    bookmarkList.clear();
    bookmarkMenu->clear();
    addId = bookmarkMenu->insertItem( "Add bookmark" );
    clearId = bookmarkMenu->insertItem( "Clear bookmarks" );
    bookmarkMenu->addSeparator();
    return;
  }

  if ( i == addId ){
    bookmarkList << dirPath();

    const char* book_pix[]={
    "12 16 3 1",
    ". c None",
    "a c #000000",
    "# c #008080",
    "............",
    "............",
    "........##..",
    ".......#####",
    "......#####a",
    ".....#####a.",
    "....#####a..",
    "...#####a...",
    "..#####a....",
    ".#####a.....",
    "aaa##a......",
    "...#a.......",
    "...a........",
    "............",
    "............",
    "............"};
    bookmarkMenu->insertItem( QIcon( book_pix ), dirPath() );
    return;
  }

  setDir( bookmarkMenu->text( i ) );
}

void CustomFileDialog::goHome()
{
  const QByteArray home = qgetenv( "HOME" );
  setDir( home.isEmpty() ? QLatin1String( "/" ) : QString(home) );
}

void CustomFileDialog::done( int i )
{
  emit signalDone(i);
}
/******************************************************************************************************/
static const char* folder_closed_xpm[] = {
"16 16 9 1",
"  c Gray0",
". c #222222",
"X c #6666cc",
"o c #9999ff",
"O c #c0c0c0",
"+ c #ccccff",
"@ c #ffffcc",
"# c Gray100",
"$ c None",
"$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$",
"$$XXXXo$$$$$$$$$",
"$X#++o@XXXXXX$$$",
"X#+++++++++++o$$",
"X#o+o+o+o+o+oX$$",
"X#+o+o+o+o+o+X $",
"X#o+o+o+o+o+oX $",
"X#+o+o+o+o+o+X $",
"X#o+o+o+o+o+oX $",
"X#+o+o+o+o+o+X $",
"X+o+o+o+o+o+oX $",
"XXXXXXXXXXXXXX $",
"$ ..           $"
};

static const char* folder_open_xpm[] = {
"16 16 10 1",
"  c Gray0",
". c #222222",
"X c #6666cc",
"o c Magenta",
"O c #9999ff",
"+ c #c0c0c0",
"@ c #ccccff",
"# c #ffffcc",
"$ c Gray100",
"% c None",
"%%%%%%%%%%%%%%%%",
"%%%%%%%%%%%%%%%%",
"%%%%%%%%%%%%%%%%",
"%%%%%%%%%%%%%%%%",
"%%XXXX%%%%%%%%%%",
"%X$$$XXXXXXXXX%%",
"%X$O+#######.OX%",
"%X$+#######.O@X%",
"%O$#######.O@OX ",
"XXXXXXXXXX.XO@X ",
"X$$$$$$$$$+ @OX ",
"%XO+O+O+O+O+ @X ",
"%X+O+O+O+O+O. X ",
"%%X+O+O+O+O+O.  ",
"%%XXXXXXXXXXXXX ",
"%%%             "
};

static const char* folder_locked_xpm[] = {
"16 16 8 1",
"  c Gray0",
". c #333366",
"X c #6666cc",
"o c Magenta",
"O c #9999ff",
"+ c #c0c0c0",
"@ c Gray100",
"# c None",
"###O.O##########",
"###...##########",
"###O.O##########",
"##.#X#.#OX######",
"#...X...@@X#####",
"#.......OOOOOOX#",
"#.......@@@@@@X#",
"#@@@@@@@O+O+O+X ",
"#O@O+O+O+O+O+OX ",
"#O@+O+O+O+O+O+X ",
"#O@O+O+O+O+O+OX ",
"#O@+O+O+O+O+O+X ",
"#O@O+O+O+O+O+OX ",
"#OXXXXXXXXXXXXX ",
"##              ",
"################"
};

Directory::Directory( Directory * parent, const QString& filename )
:Q3ListViewItem( parent ), f(filename)
{
  p = parent;
  readable = QDir( fullName() ).isReadable();

  if ( !readable )
    setPixmap( 0, QPixmap( folder_locked_xpm ) );
  else
    setPixmap( 0, QPixmap( folder_closed_xpm ) );
}


Directory::Directory( Q3ListView * parent, const QString& filename )
:Q3ListViewItem( parent ), f(filename)
{
  p = 0;
  readable = QDir( fullName() ).isReadable();
}


void Directory::setOpen( bool o )
{
  if ( o )
    setPixmap( 0, QPixmap( folder_open_xpm ) );
  else
    setPixmap( 0, QPixmap( folder_closed_xpm ) );

  if ( o && !childCount() ){
    QString s( fullName() );
    QDir thisDir( s );
    if ( !thisDir.isReadable() ) {
      readable = false;
      setExpandable( false );
      return;
    }

    listView()->setUpdatesEnabled( false );
    QFileInfoList files = thisDir.entryInfoList();
    foreach ( const QFileInfo &f, files ){
      if ( f.fileName() != "." && f.fileName() != ".." && f.isDir() )
        (void)new Directory( this, f.fileName() );
    }
    listView()->setUpdatesEnabled( true );
  }
  Q3ListViewItem::setOpen( o );
}


void Directory::setup()
{
  setExpandable( true );
  Q3ListViewItem::setup();
}


QString Directory::fullName()
{
  QString s;
  if ( p ) {
    s = p->fullName();
    s.append( f.fileName() );
    s.append( "/" );
  } else {
    s = f.fileName();
  }
  return s;
}


QString Directory::text( int column ) const
{
  if ( column == 0 )
    return f.fileName();
  else
    if ( readable )
      return "Directory";
    else
      return "Unreadable Directory";
}

DirectoryView::DirectoryView( QWidget *parent, const char *name )
:Q3ListView( parent, name )
{
  connect( this, SIGNAL(clicked(Q3ListViewItem*)),
           this, SLOT(slotFolderSelected(Q3ListViewItem*)) );
  connect( this, SIGNAL(doubleClicked(Q3ListViewItem*)),
           this, SLOT(slotFolderSelected(Q3ListViewItem*)) );
  connect( this, SIGNAL(returnPressed(Q3ListViewItem*)),
           this, SLOT(slotFolderSelected(Q3ListViewItem*)) );

  setAcceptDrops( true );
  viewport()->setAcceptDrops( true );
}

void DirectoryView::setOpen( Q3ListViewItem* i, bool b )
{
  Q3ListView::setOpen(i,b);
  setCurrentItem(i);
  slotFolderSelected(i);
}

void DirectoryView::slotFolderSelected( Q3ListViewItem *i )
{
  if ( !i )	return;

  Directory *dir = (Directory*)i;
  emit folderSelected( dir->fullName() );
}

QString DirectoryView::fullPath(Q3ListViewItem* item)
{
  QString fullpath = item->text(0);
  while ( (item=item->parent()) ) {
    if ( item->parent() )
      fullpath = item->text(0) + "/" + fullpath;
    else
      fullpath = item->text(0) + fullpath;
  }
  return fullpath;
}

void DirectoryView::setDir( const QString &s )
{
  Q3ListViewItemIterator it( this );
  ++it;
  for ( ; it.current(); ++it ) {
    it.current()->setOpen( false );
  }

  QStringList lst( s.split( "/", QString::SkipEmptyParts ) );
  Q3ListViewItem *item = firstChild();
  QStringList::Iterator it2 = lst.begin();
  for ( ; it2 != lst.end(); ++it2 ) {
    while ( item ) {
      if ( item->text( 0 ) == *it2 ) {
        item->setOpen( true );
        break;
      }
      item = item->itemBelow();
    }
  }

  if ( item ){
    setSelected( item, true );
    setCurrentItem( item );
  }
}

QString DirectoryView::selectedDir()
{
  Directory *dir = (Directory*)currentItem();
  return dir->fullName();
}
/**********************************************************************************************/

int main(int argc, char* argv[]) {
  QApplication::setApplicationName("kdockwidgetdemo");

  QApplication app(argc, argv);

#if 0
  SFileDialog* openfile = new SFileDialog();
  openfile->exec();
  qDebug( openfile->fileName() );
#endif

#if 0
  qDebug ( SFileDialog::getOpenFileName( QString(), QLatin1String("All (*)"),
                                         QLatin1String("DockWidget Demo"), "dialog1" ) );
#endif

#if 1
  QStringList s = SFileDialog::getOpenFileNames( QString(),
          QStringList(QLatin1String("All (*)")),
          QLatin1String("DockWidget Demo"), "dialog1" );
  QStringList::Iterator it = s.begin();
  for ( ; it != s.end(); ++it ){
    qDebug( "%s", (*it).toLocal8Bit().data() );
  }
#endif
  return 0;
}

#include "moc_kdockwidgetdemo.cpp"

