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
	void saveToggle();

private:
	KActTest& operator=( const KActTest& );
	KActTest( const KActTest& );

	QMultiLineEdit *_mlined;
	KUIActions *_act;
};

KActTest::KActTest( QWidget *parent )
	: QWidget( parent )
{
	// actions
	_act = new KUIActions( this );
	debug( "created act" );

	_act->newAction( "Save", "Bachaa", this, SLOT(save()), CTRL + Key_S );
	_act->newAction( "Load", "Khol", this, SLOT(load()), CTRL + Key_L );
	_act->newAction( "SaveEn", "Toggle Save", this, SLOT(saveToggle()),
			CTRL + Key_T );
	_act->newAction( "Quit", "Band Kar", qApp, 
			SLOT(quit()), CTRL + Key_Q );
	debug( "created actions" );
	
	// menus
	QMenuBar *mymenu = new QMenuBar( this );
	QPopupMenu *file = new QPopupMenu( this );
	mymenu->insertItem( "File", file );
	
	KActionMenuBuilder mb( _act );
	mb.setMenu( file, KActionMenuBuilder::Text );

	mb << "Save" << "Load" << "Quit" << "SaveEn";

	debug( "connected actions" );

	_mlined = new QMultiLineEdit( this );
	_mlined->setGeometry( 0, mymenu->height(), width(), 
		height() - mymenu->height() );
}

void KActTest::saveToggle()
{
	_act->setEnabled( "Save", !_act->enabled( "Save" ) );
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
