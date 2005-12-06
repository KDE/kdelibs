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
#include <q3popupmenu.h>
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
#include <q3paintdevicemetrics.h>

#include <ctype.h>

HelpWindow::HelpWindow( const QString& home_, const QString& _path,
			QWidget* parent, const char *name )
    : KMainWindow( parent, name, Qt::WDestructiveClose ),
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
    file->insertItem( "&New Window", this, SLOT( newWindow() ), Qt::ALT | Qt::Key_N );
    file->insertItem( "&Open File", this, SLOT( openFile() ), Qt::ALT | Qt::Key_O );
    file->insertItem( "&Print", this, SLOT( print() ), Qt::ALT | Qt::Key_P );
    file->insertSeparator();
    file->insertItem( "&Close", this, SLOT( close() ), Qt::ALT | Qt::Key_Q );
    file->insertItem( "E&xit", qApp, SLOT( closeAllWindows() ), Qt::ALT | Qt::Key_X );

    // The same three icons are used twice each.
    QIcon icon_back( QPixmap("back.xpm") );
    QIcon icon_forward( QPixmap("forward.xpm") );
    QIcon icon_home( QPixmap("home.xpm") );

    QMenu* go = new QMenu( this );
    backwardId = go->insertItem( icon_back,
				 "&Backward", browser, SLOT( backward() ),
				 Qt::ALT | Qt::Key_Left );
    forwardId = go->insertItem( icon_forward,
				"&Forward", browser, SLOT( forward() ),
				Qt::ALT | Qt::Key_Right );
    go->insertItem( icon_home, "&Home", browser, SLOT( home() ) );

    QMenu* help = new QMenu( this );
    help->insertItem( "&About ...", this, SLOT( about() ) );
    help->insertItem( "About &Qt ...", this, SLOT( aboutQt() ) );

    hist = new QMenu( this );
    QStringList::Iterator it = history.begin();
    for ( ; it != history.end(); ++it )
	mHistory[ hist->insertItem( *it ) ] = *it;
    connect( hist, SIGNAL( activated( int ) ),
	     this, SLOT( histChosen( int ) ) );

    bookm = new QMenu( this );
    bookm->insertItem( tr( "Add Bookmark" ), this, SLOT( addBookmark() ) );
    bookm->insertSeparator();

    QStringList::Iterator it2 = bookmarks.begin();
    for ( ; it2 != bookmarks.end(); ++it2 )
	mBookmarks[ bookm->insertItem( *it2 ) ] = *it2;
    connect( bookm, SIGNAL( activated( int ) ),
	     this, SLOT( bookmChosen( int ) ) );

    menuBar()->insertItem( tr("&File"), file );
    menuBar()->insertItem( tr("&Go"), go );
    menuBar()->insertItem( tr( "History" ), hist );
    menuBar()->insertItem( tr( "Bookmarks" ), bookm );
    menuBar()->insertSeparator();
    menuBar()->insertItem( tr("&Help"), help );

    menuBar()->setItemEnabled( forwardId, false);
    menuBar()->setItemEnabled( backwardId, false);
    connect( browser, SIGNAL( backwardAvailable( bool ) ),
	     this, SLOT( setBackwardAvailable( bool ) ) );
    connect( browser, SIGNAL( forwardAvailable( bool ) ),
	     this, SLOT( setForwardAvailable( bool ) ) );


    Q3ToolBar* toolbar = new Q3ToolBar( this );
    addToolBar( toolbar, "Toolbar");
    QToolButton* button;

    button = new QToolButton( icon_back, tr("Backward"), "", browser, SLOT(backward()), toolbar );
    connect( browser, SIGNAL( backwardAvailable(bool) ), button, SLOT( setEnabled(bool) ) );
    button->setEnabled( false );
    button = new QToolButton( icon_forward, tr("Forward"), "", browser, SLOT(forward()), toolbar );
    connect( browser, SIGNAL( forwardAvailable(bool) ), button, SLOT( setEnabled(bool) ) );
    button->setEnabled( false );
    button = new QToolButton( icon_home, tr("Home"), "", browser, SLOT(home()), toolbar );

    toolbar->addSeparator();

    pathCombo = new QComboBox( true, toolbar );
    connect( pathCombo, SIGNAL( activated( const QString & ) ),
	     this, SLOT( pathSelected( const QString & ) ) );
    toolbar->setStretchableWidget( pathCombo );
    setRightJustification( true );
    setDockEnabled( Qt::DockLeft, false );
    setDockEnabled( Qt::DockRight, false );

    pathCombo->insertItem( home_ );

    browser->setFocus();
}


void HelpWindow::setBackwardAvailable( bool b)
{
    menuBar()->setItemEnabled( backwardId, b);
}

void HelpWindow::setForwardAvailable( bool b)
{
    menuBar()->setItemEnabled( forwardId, b);
}


void HelpWindow::textChanged()
{
    if ( browser->documentTitle().isNull() )
	setCaption( browser->context() );
    else
	setCaption( browser->documentTitle() ) ;

    selectedURL = caption();
    if ( !selectedURL.isEmpty() && pathCombo ) {
	bool exists = false;
	int i;
	for ( i = 0; i < pathCombo->count(); ++i ) {
	    if ( pathCombo->text( i ) == selectedURL ) {
		exists = true;
		break;
	    }
	}
	if ( !exists ) {
	    pathCombo->insertItem( selectedURL, 0 );
	    pathCombo->setCurrentItem( 0 );
	    mHistory[ hist->insertItem( selectedURL ) ] = selectedURL;
	} else
	    pathCombo->setCurrentItem( i );
	selectedURL = QString::null;
    }
}

HelpWindow::~HelpWindow()
{
    history.clear();
    QMap<int, QString>::Iterator it = mHistory.begin();
    for ( ; it != mHistory.end(); ++it )
	history.append( *it );

    QFile f( QDir::currentPath() + "/.history" );
    f.open( QIODevice::WriteOnly );
    QDataStream s( &f );
    s << history;
    f.close();

    bookmarks.clear();
    QMap<int, QString>::Iterator it2 = mBookmarks.begin();
    for ( ; it2 != mBookmarks.end(); ++it2 )
	bookmarks.append( *it2 );

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
    QString fn = QFileDialog::getOpenFileName( QString::null, QString::null, this );
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
	Q3PaintDeviceMetrics metrics(p.device());
	int dpix = metrics.logicalDpiX();
	int dpiy = metrics.logicalDpiY();
#ifdef KDE_PRINT
	const int margin = printer.option("app-rich-margin").toInt(); // pt
#else
	const int margin = 72; // pt
#endif
	QRect body(margin*dpix/72, margin*dpiy/72,
		   metrics.width()-margin*dpix/72*2,
		   metrics.height()-margin*dpiy/72*2 );
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
	    richText.draw( &p, body.left(), body.top(), view, colorGroup() );
	    view.moveBy( 0, body.height() );
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
    QMap<int, QString>::Iterator it = mHistory.begin();
    bool exists = false;
    for ( ; it != mHistory.end(); ++it ) {
	if ( *it == _path ) {
	    exists = true;
	    break;
	}
    }
    if ( !exists )
	mHistory[ hist->insertItem( _path ) ] = _path;
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
	    history.remove( history.begin() );
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

void HelpWindow::histChosen( int i )
{
    if ( mHistory.contains( i ) )
	browser->setSource( mHistory[ i ] );
}

void HelpWindow::bookmChosen( int i )
{
    if ( mBookmarks.contains( i ) )
	browser->setSource( mBookmarks[ i ] );
}

void HelpWindow::addBookmark()
{
    mBookmarks[ bookm->insertItem( caption() ) ] = caption();
}

#include "helpwindow.moc"
