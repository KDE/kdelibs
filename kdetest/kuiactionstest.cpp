/*
* acttest.cpp -- Implementation of class KActTest.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Fri May  7 02:50:50 EST 1999
*/

#include<qmultilinedit.h>
#include<qpopupmenu.h>
#include<qmenubar.h>
#include<qapplication.h>
#include<qnamespace.h>

#include<kuiactions.h>
#include<kactmenubuilder.h>

/**
* Test harness for KAction and KUIAction
* @author Sirtaj Singh Kang (taj@kde.org)
* @version $Id$
*/
class KActTest : public QWidget
{
	Q_OBJECT

public:
	/**
	* KActTest Constructor
	*/
	KActTest( QWidget *parent = 0 );

	/**
	* KActTest Destructor
	*/
	virtual ~KActTest() {}
	
public slots:
	void save();
	void load();
	
private:
	KActTest& operator=( const KActTest& );
	KActTest( const KActTest& );

	QMultiLineEdit *_mlined;
	KUIActions *_act;
};

KActTest::KActTest( QWidget *parent )
	: QWidget( parent )
{
	// menus
	QMenuBar *mymenu = new QMenuBar( this );

	debug( "built menus" );

	// actions
	_act = new KUIActions( this );
	debug( "created act" );

	_act->newAction( "Save", "Taj", this, SLOT(save()), CTRL + Key_S );
	_act->newAction( "Load", "Beavis", this, SLOT(load()), CTRL + Key_L );
	_act->newAction( "Quit", "Arrr", qApp, SLOT(quit()), CTRL + Key_Q );
	debug( "created actions" );
	
	QPopupMenu *file = new QPopupMenu( this );
	mymenu->insertItem( "File", file );
	
	KActionMenuBuilder mb( _act );
	mb.setMenu( file, KActionMenuBuilder::Text );

	mb.insert( "Save" );
	mb.insert( "Load" );
	mb.insert( "Quit" );
	
	debug( "connected actions" );

	_mlined = new QMultiLineEdit( this );
	_mlined->setGeometry( 0, mymenu->height(), width(), 
		height() - mymenu->height() );
}

void KActTest::save()
{
	_mlined->append( "Save!" );
}

void KActTest::load()
{
	_mlined->append( "Load!" );
	_act->action( "Load" )->setAccel( CTRL + Key_K );
}

int main( int argc, char **argv )
{
	QApplication app( argc, argv );
	debug( "created app" );

	KActTest tester;
	app.setMainWidget( &tester );
	debug( "created tester" );
	tester.show();

	app.exec();
}

#include"acttest.moc"
