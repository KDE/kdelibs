/*
* kfbrowsebtn.cpp -- Implementation of class KBrowseButton.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Tue Aug  4 03:44:44 EST 1998
*/


#include<assert.h>
#include<kapp.h>
#include<qstring.h>
#include<kfiledialog.h>


#include"kfbrowsebtn.h"
#include"kfbrowsebtn.moc" // <-- oh, the pain! moc_, how I miss thee..

KBrowseButton::KBrowseButton( const char *caption, 
	const char *path , QWidget *parent, const char *name, 
	bool modal )
	: QPushButton( caption ? caption : i18n( "Browse.." ), 
			parent, name ),
	_dialog( 0 ),
	_path( new QString ),
	_modal ( modal )
{
	connect( this, SIGNAL(clicked()), this, SLOT(showDialog()) );
}

KBrowseButton::~KBrowseButton()
{
	delete _dialog;
	delete _path;
}

const char *KBrowseButton::path() const
{
	return _path->data();
}

void KBrowseButton::setPath( const char *url )
{
	*_path = url;

	if( _dialog ) {
		// change dialog path
		_dialog->setSelection( url );
	}

	debug( "path set to: %s", _path->data() );
}

void KBrowseButton::showDialog()
{
	// create dialog if needed

	if ( !_dialog ) {
		_dialog = newBrowserDialog( _modal );
		assert( _dialog != 0 );
		connect( _dialog, SIGNAL(fileSelected(const char *)),
			this, SLOT(setPathAndRaise(const char *)) );
	}
	
	_dialog->show();
}

void KBrowseButton::setPathAndRaise( const char *url )
{
	setPath( url );
	emit pathChanged( url );
}

KFileBaseDialog *KFileBrowseButton::newBrowserDialog( bool modal )
{
	return new KFileDialog( path() ? path() : "", 0, 0, 0, modal );
}

KFileBaseDialog *KDirBrowseButton::newBrowserDialog( bool )
{
	return new KDirDialog( path() ? path() : "", 0, 0 );
}
