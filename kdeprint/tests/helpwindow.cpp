/****************************************************************************
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "helpwindow.h"
#include "richpage.h"
#include <qstatusbar.h>
#include <qpixmap.h>
#include <kmenubar.h>
#include <ktoolbar.h>
#include <kstatusbar.h>
#include <qtoolbutton.h>
#include <qicon.h>
#include <qfile.h>
#include <qtextstream.h>
#include <q3stylesheet.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qapplication.h>
#include <qcombobox.h>
#include <qevent.h>
#include <qlineedit.h>
#include <qobject.h>
#include <qfileinfo.h>
#include <qdatastream.h>
#include <kprinter.h>
#include <q3simplerichtext.h>
#include <qpainter.h>

#include <ctype.h>

HelpWindow::HelpWindow( const QString& home_, const QString& _path,
			QWidget* parent )
    : KMainWindow( parent, Qt::WDestructiveClose ),
      pathCombo( 0 ), selectedURL()
{
    readHistory();
    readBookmarks();

    browser = new Q3TextBrowser( this );
    browser->mimeSourceFactory()->setFilePath( _path );
    browser->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    connect( browser, SIGNAL( textChanged() ),
	     this, SLOT( textChanged() ) );

    setCentralWidget( browser );

    if ( !home_.isEmpty() )
	browser->setSource( home_ );

    connect( browser, SIGNAL( highlighted( const QString&) ),
	     statusBar(), SLOT( message( const QString&)) );

    resize( 640,700 );

    QMenu* file = new QMenu( this );
    file->addAction( "&New Window", this, SLOT( newWindow() ), Qt::ALT | Qt::Key_N );
	file->addAction( "&Open File", this, SLOT( openFile() ), Qt::ALT | Qt::Key_O );
	QAction * before = file->addAction( "&Print", this, SLOT( print() ), Qt::ALT | Qt::Key_P );
    file->insertSeparator( before );
	file->addAction( "&Close", this, SLOT( close() ), Qt::ALT | Qt::Key_Q );
	file->addAction( "E&xit", qApp, SLOT( closeAllWindows() ), Qt::ALT | Qt::Key_X );

    // The same three icons are used twice each.
    QIcon icon_back( QPixmap("back.xpm") );
    QIcon icon_forward( QPixmap("forward.xpm") );
    QIcon icon_home( QPixmap("home.xpm") );

    QMenu* go = new QMenu( this );
    backwardAction = go->addAction( icon_back,
				 "&Backward", browser, SLOT( backward() ),
				 Qt::ALT | Qt::Key_Left );
    forwardAction = go->addAction( icon_forward,
				"&Forward", browser, SLOT( forward() ),
				Qt::ALT | Qt::Key_Right );
    go->addAction( icon_home, "&Home", browser, SLOT( home() ) );

    QMenu* help = new QMenu( this );
    help->addAction( "&About ...", this, SLOT( about() ) );
    help->addAction( "About &Qt ...", this, SLOT( aboutQt() ) );

    hist = new QMenu( this );
    QStringList::Iterator it = history.begin();
    for ( ; it != history.end(); ++it )
	mHistory[ hist->addAction( *it ) ] = *it;
	connect( hist, SIGNAL( triggered( QAction* ) ),
			 this, SLOT( histChosen( QAction* ) ) );

    bookm = new QMenu( this );
    before = bookm->addAction( tr( "Add Bookmark" ), this, SLOT( addBookmark() ) );
    bookm->insertSeparator( before );

    QStringList::Iterator it2 = bookmarks.begin();
    for ( ; it2 != bookmarks.end(); ++it2 )
	mBookmarks[ bookm->addAction( *it2 ) ] = *it2;
	connect( bookm, SIGNAL( triggered( QAction* ) ),
			 this, SLOT( bookmChosen( QAction* ) ) );

	menuBar()->addMenu( file )->setText( tr("&File") );
	menuBar()->addMenu( go )->setText( tr("&Go") );
	menuBar()->addMenu( hist )->setText( tr( "History" ) );
	before = menuBar()->addMenu( bookm );
	before->setText( tr( "Bookmarks" ) );
    menuBar()->insertSeparator( before );
	menuBar()->addMenu( help )->setText( tr("&Help") );

	forwardAction->setEnabled( false );
	backwardAction->setEnabled( false );
    connect( browser, SIGNAL( backwardAvailable( bool ) ),
	     this, SLOT( setBackwardAvailable( bool ) ) );
    connect( browser, SIGNAL( forwardAvailable( bool ) ),
	     this, SLOT( setForwardAvailable( bool ) ) );


    QToolBar* toolbar = addToolBar("Toolbar");
    QToolButton* button;

    button = new QToolButton( toolbar );
    button->setIcon( icon_back );
    button->setText( tr("Backward") );
    connect( button, SIGNAL( clicked() ), browser, SLOT(backward()) );
    connect( browser, SIGNAL( backwardAvailable(bool) ), button, SLOT( setEnabled(bool) ) );
    button->setEnabled( false );
    button = new QToolButton( toolbar );
    button->setIcon( icon_forward );
    button->setText( tr("Forward") );
    connect( button, SIGNAL( clicked() ), browser, SLOT(forward()) );
    connect( browser, SIGNAL( forwardAvailable(bool) ), button, SLOT( setEnabled(bool) ) );
    button->setEnabled( false );
    button = new QToolButton( toolbar );
    button->setIcon( icon_home );
    button->setText( tr("Home") );
    connect( button, SIGNAL( clicked() ), browser, SLOT(home()) );

    toolbar->addSeparator();

    pathCombo = new QComboBox(toolbar );
    pathCombo->setEditable( true );
    connect( pathCombo, SIGNAL( activated( const QString & ) ),
	     this, SLOT( pathSelected( const QString & ) ) );
    //toolbar->setStretchableWidget( pathCombo );
    //setRightJustification( true );
    //setDockEnabled( Qt::DockLeft, false );
    //setDockEnabled( Qt::DockRight, false );

    pathCombo->addItem( home_ );

    browser->setFocus();
}


void HelpWindow::setBackwardAvailable( bool b)
{
	backwardAction->setEnabled(b);
}

void HelpWindow::setForwardAvailable( bool b)
{
	forwardAction->setEnabled(b);
}


void HelpWindow::textChanged()
{
    if ( browser->documentTitle().isNull() )
	setCaption( browser->context() );
    else
	setCaption( browser->documentTitle() ) ;

    selectedURL = windowTitle();
    if ( !selectedURL.isEmpty() && pathCombo ) {
	bool exists = false;
	int i;
	for ( i = 0; i < pathCombo->count(); ++i ) {
	    if ( pathCombo->itemText( i ) == selectedURL ) {
		exists = true;
		break;
	    }
	}
	if ( !exists ) {
	    pathCombo->insertItem( 0, selectedURL );
	    pathCombo->setCurrentIndex( 0 );
	    mHistory[ hist->addAction( selectedURL ) ] = selectedURL;
	} else
	    pathCombo->setCurrentIndex( i );
	selectedURL.clear();
    }
}

HelpWindow::~HelpWindow()
{
	history = mHistory.values();

    QFile f( QDir::currentPath() + "/.history" );
    f.open( QIODevice::WriteOnly );
    QDataStream s( &f );
    s << history;
    f.close();

	bookmarks = mBookmarks.values();

    QFile f2( QDir::currentPath() + "/.bookmarks" );
    f2.open( QIODevice::WriteOnly );
    QDataStream s2( &f2 );
    s2 << bookmarks;
    f2.close();
}

void HelpWindow::about()
{
    QMessageBox::about( this, "HelpViewer Example",
			"<p>This example implements a simple HTML help viewer "
			"using Qt's rich text capabilities</p>"
			"<p>It's just about 100 lines of C++ code, so don't expect too much :-)</p>"
			);
}


void HelpWindow::aboutQt()
{
    QMessageBox::aboutQt( this, "QBrowser" );
}

void HelpWindow::openFile()
{
#ifndef QT_NO_FILEDIALOG
    QString fn = QFileDialog::getOpenFileName( this );

    if ( !fn.isEmpty() )
	browser->setSource( fn );
#endif
}

void HelpWindow::newWindow()
{
    ( new HelpWindow(browser->source(), "qbrowser") )->show();
}

#define KDE_PRINT
void HelpWindow::print()
{
#ifdef KDE_PRINT
    KPrinter printer;
#else
    QPrinter printer;
#endif
    printer.setFullPage(true);
    printer.setDocName("Help Viewer");
    printer.setDocFileName("my_document");
#ifdef KDE_PRINT
    printer.addDialogPage(new RichPage());
    printer.addStandardPage(KPrinter::FilesPage);
#endif
    if ( printer.setup(this) ) {
	QPainter p( &printer );
	int dpix = p.device()->logicalDpiX();
	int dpiy = p.device()->logicalDpiY();
#ifdef KDE_PRINT
	const int margin = printer.option("app-rich-margin").toInt(); // pt
#else
	const int margin = 72; // pt
#endif
	QRect body(margin*dpix/72, margin*dpiy/72,
		   p.device()->width()-margin*dpix/72*2,
		   p.device()->height()-margin*dpiy/72*2 );
#ifdef KDE_PRINT
	QFont font(printer.option("app-rich-fontname"), printer.option("app-rich-fontsize").toInt());
#else
	QFont font("times",10);
#endif
	Q3SimpleRichText richText( browser->text(), font, browser->context(), browser->styleSheet(),
				  browser->mimeSourceFactory(), body.height() );
	richText.setWidth( &p, body.width() );
	QRect view( body );
	int page = 1;
	do {
	    richText.draw( &p, body.left(), body.top(), view, palette() );
	    view.translate( 0, body.height() );
	    p.translate( 0 , -body.height() );
	    p.setFont( font );
	    p.drawText( view.right() - p.fontMetrics().width( QString::number(page) ),
			view.bottom() + p.fontMetrics().ascent() + 5, QString::number(page) );
	    if ( view.top()  >= richText.height() )
		break;
	    printer.newPage();
	    page++;
	} while (true);
    }
}

void HelpWindow::pathSelected( const QString &_path )
{
    browser->setSource( _path );
	
	if ( !mHistory.values().contains( _path ) )
		mHistory[ hist->addAction( _path ) ] = _path;
}

void HelpWindow::readHistory()
{
    if ( QFile::exists( QDir::currentPath() + "/.history" ) ) {
	QFile f( QDir::currentPath() + "/.history" );
	f.open( QIODevice::ReadOnly );
	QDataStream s( &f );
	s >> history;
	f.close();
	while ( history.count() > 20 )
	    history.removeFirst();
    }
}

void HelpWindow::readBookmarks()
{
    if ( QFile::exists( QDir::currentPath() + "/.bookmarks" ) ) {
	QFile f( QDir::currentPath() + "/.bookmarks" );
	f.open( QIODevice::ReadOnly );
	QDataStream s( &f );
	s >> bookmarks;
	f.close();
    }
}

void HelpWindow::histChosen( QAction *action )
{
	if ( mHistory.contains( action ) )
		browser->setSource( mHistory[ action ] );
}

void HelpWindow::bookmChosen( QAction *action )
{
	if ( mBookmarks.contains( action ) )
		browser->setSource( mBookmarks[ action ] );
}

void HelpWindow::addBookmark()
{
	mBookmarks[ bookm->addAction( windowTitle() ) ] = windowTitle();
}

#include "helpwindow.moc"
