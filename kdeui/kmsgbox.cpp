/* This file is part of the KDE libraries
    Copyright (C) 1997 Alexander Sanda (alex@darkstar.ping.at)

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
/*                                                    cd source
 * $Id$
 *
 * $Log$
 * Revision 1.10  1998/02/24 15:54:35  kulow
 * replaced some hard coded paths with the kapp->kde_ methodes.
 * I'm not sure, if kde_datadir() is optimal for /share/apps ;)
 *
 * Revision 1.9  1998/01/03 19:47:28  kulow
 * changed the defaults for yesNo and co. It's no longer "Yes" and co, but
 * 0 to enable translation in case.
 * This is source and binary compatible. It's just, that it will behave
 * differently after recompiling (as far I understood the C++ language ;)
 *
 * Revision 1.8  1997/11/16 20:46:51  mark
 * MD: Reworked look of KMessageBox dialogs.
 *   Originally I was going to make KMessageBox a wrapper for
 *   QMessageBox. However, the classes are too different. I
 *   settled for a reworking the look of KMessageBox to make it
 *   conpatible with QMessageBox.
 *   Also fixed a possible bug in KButtonBox to do with the
 *   calculation of the size of Motif default buttons. This
 *   calculation depends on the undocumented behaviour of
 *   QButton::sizeHint so I suppose it could change from version to
 *   version of Qt. The fix is clearly marked should it need to
 *   be revoked.
 *
 * Revision 1.7  1997/10/21 20:45:02  kulow
 * removed all NULLs and replaced it with 0L or "".
 * There are some left in mediatool, but this is not C++
 *
 * Revision 1.6  1997/10/16 11:15:26  torben
 * Kalle: Copyright headers
 * kdoctoolbar removed
 *
 * Revision 1.5  1997/09/10 12:07:48  kdecvs
 * Kalle: fixed large meomry leak (bug reported by Joerg Habenicht)
 *
 * Revision 1.4  1997/09/10 09:04:17  kdecvs
 * Coolo: Torben's wish is my command ;)
 *
 * Revision 1.3  1997/09/04 22:36:26  kdecvs
 * Coolo: put the icons in share/icons, I hope, this is right ;)
 *
 * Revision 1.2  1997/05/08 22:53:20  kalle
 * Kalle:
 * KPixmap gone for good
 * Eliminated static objects from KApplication and HTML-Widget
 *
 * Revision 1.1.1.1  1997/04/13 14:42:42  cvsuser
 * Source imported
 *
 * Revision 1.1.1.1  1997/04/09 00:28:09  cvsuser
 * Sources imported
 *
 * Revision 1.1  1997/03/15 22:41:21  kalle
 * Initial revision
 *
 * Revision 1.3.2.1  1997/01/10 19:48:32  alex
 * public release 0.1
 *
 * Revision 1.3  1997/01/10 19:44:33  alex
 * *** empty log message ***
 *
 * Revision 1.2.4.1  1997/01/10 16:46:33  alex
 * rel 0.1a, not public
 *
 * Revision 1.2  1997/01/10 13:05:52  alex
 * *** empty log message ***
 *
 * Revision 1.1.1.1  1997/01/10 13:05:21  alex
 * imported
 *
 */

#include <stdlib.h>

#include <qpixmap.h>
#include <qlayout.h>

#include <kapp.h>
#include <kbuttonbox.h>

#include "kmsgbox.h"
#include "kmsgbox.moc"

KMsgBox::KMsgBox( QWidget *parent, const char *caption,
	const char *message, int type,
	const char *b1text, const char *b2text,
	const char *b3text, const char *b4text )
	: QDialog ( parent, caption, TRUE, 0 ),
	msg( 0L ), picture( 0L ),
	b1( 0L ), b2( 0L ), b3( 0L ), b4( 0L ),
	f1( 0L )
{
    int icon;
    static int icons_initialized = 0;
    static QPixmap icons[4];

    if( !icons_initialized ) {
    	QString ipath = kapp->kde_datadir() + "/kde/pics/";
        QString ifile = ipath + "info.xpm";
        icons[0].load( ifile );
        icons[1].load( ipath + "exclamation.xpm" );
        icons[2].load( ipath + "stopsign.xpm" );
        icons[3].load( ipath + "question.xpm" );
        icons_initialized = 1;
    }

    int icon_index = type & 0x0000000f;   // mask the lower 4 bits (icon style)
    
	if( icon_index <= 4 )
        icon = icon_index >> 1;
    else
        icon = 3;
	
	//
	// The separate call to initMe is now obsolete.
	// The following code replaces all of it.
	// 
    //initMe( caption, message, b1text, b2text, b3text, b4text, icons[icon] );
	//debug("KMsgBox::initMe");
	
	setCaption( caption );
	
	int border = 8;
	int vertSpacing = 0;
	
	QBoxLayout *topLayout = new QVBoxLayout( this, border );
	
	QBoxLayout *labels = new QHBoxLayout( border );
	topLayout->addLayout( labels );
	
	topLayout->addSpacing( vertSpacing );
	
	// CREATE THE ICON AND MESSAGE LABELS
	
	picture = new QLabel( this, "_pict" );
    //picture->setAutoResize( TRUE );
    picture->setPixmap( icons[icon] );
	picture->adjustSize();
	picture->setFixedSize( picture->size() );
	
	msg = new QLabel( message, this, "_msg" );
    //msg->setAlignment( AlignCenter );
    msg->adjustSize();
	msg->setFixedSize( msg->size().width()+10, msg->size().height() );
	
	// Add the labels to their layout manager
	
	labels->addWidget( picture );
	labels->addWidget( msg );
    
	//
	// Following is rendered obsolete by automatic geometry management
	// and new styling.
	//
	//calcOptimalSize();
    //setMinimumSize(w + 20, h);
    //f1 = new QFrame(this);
    //f1->setLineWidth(1);
    //f1->setFrameStyle(QFrame::NoFrame);
	//f1->resize(0,0);
	//

	topLayout->addStretch( 10 );
	
	// CREATE BUTTONS
	
	nr_buttons = 0;
    b1 = b2 = b3 = b4 = 0;
	
	KButtonBox *bbox = new KButtonBox( this );
	bbox->addStretch( 10 );
	
    if( b1text ) {
        //b1 = new QPushButton( b1text, this, "_b1" );
		//b1->resize(80, 25);
		
		b1 = bbox->addButton( b1text );
		connect( b1, SIGNAL( clicked() ), this, SLOT( b1Pressed() ) );
        nr_buttons++;
    }

    if( b2text ) {
        //b2 = new QPushButton( b2text, this, "_b2" );
		//b2->resize(80, 25);
		
        b2 = bbox->addButton( b2text );
		connect( b2, SIGNAL( clicked() ), this, SLOT( b2Pressed() ) );
        nr_buttons++;
    }

    if( b3text ) {
        //b3 = new QPushButton( b3text, this, "_b3" );
		//b3->resize(80, 25);
        
		b3 = bbox->addButton( b3text );
		connect( b3, SIGNAL( clicked() ), this, SLOT( b3Pressed() ) );
        nr_buttons++;
    }

    if( b4text ) {
        //b4 = new QPushButton( b4text, this, "_b4" );
		//b4->resize(80, 25);
        
		b4 = bbox->addButton( b4text );
		connect( b4, SIGNAL( clicked() ), this, SLOT( b4Pressed() ) );
        nr_buttons++;
    }
	
	bbox->addStretch( 10 );
	
	//
	// Check, if we have a default button
	// If not, set the left button to default
    //
	if( !(type & 0x000000f0) )
        type |= DB_FIRST;
    if( b1 )
        b1->setDefault( type & DB_FIRST );
    if( b2 )
        b2->setDefault( type & DB_SECOND );
    if( b3 )
        b3->setDefault( type & DB_THIRD );
    if( b4 )
        b4->setDefault( type & DB_FOURTH );
    
	bbox->layout();
	topLayout->addWidget( bbox );
	
    topLayout->freeze();
}

void KMsgBox::initMe( const char */*caption*/, 
		      const char */*message*/,
		      const char */*b1text*/, 
		      const char */*b2text*/,
		      const char */*b3text*/, 
		      const char */*b4text*/,
		      const QPixmap &/* icon*/ )
{
   /*
   
   debug("KMsgBox::initMe");
	
	setCaption( caption );
	
	QBoxLayout *topLayout = new QVBoxLayout( this, 10 );
	
	QBoxLayout *labels = new QHBoxLayout();
	topLayout->addLayout( labels );
	
	// CREATE THE ICON AND MESSAGE LABELS
	
	picture = new QLabel( this, "_pict" );
    picture->setAutoResize( TRUE );
    picture->setPixmap( icon );
	
	msg = new QLabel( message, this, "_msg" );
    //msg->setAlignment( AlignCenter );
    msg->adjustSize();
	
	// Add the labels to their layout manager
	
	labels->addWidget( picture );
	labels->addWidget( msg );
    
	// Following is rendered obsolete by automatic geometry management
	// and new styling.
	
	//calcOptimalSize();
    //setMinimumSize(w + 20, h);
    f1 = new QFrame(this);
    f1->setLineWidth(1);
    f1->setFrameStyle(QFrame::NoFrame);
	
	debug("created labels");
	
	// CREATE BUTTONS
	
	nr_buttons = 0;
    b1 = b2 = b3 = b4 = 0;
	
	QBoxLayout *buttons = new QHBoxLayout();
	topLayout->addLayout( buttons );
	
	int widget_width = 0;
	
    if( b1text ) {
        b1 = new QPushButton( b1text, this, "_b1" );
        
		//b1->resize(80, 25);
        
		connect( b1, SIGNAL( clicked() ), this, SLOT( b1Pressed() ) );
		if( b1->sizeHint().width() > widget_width )
			widget_width =  b1->sizeHint().width();
        nr_buttons++;
    }

    if( b2text ) {
        b2 = new QPushButton( b2text, this, "_b2" );
        
		//b2->resize(80, 25);
        
		connect( b2, SIGNAL( clicked() ), this, SLOT( b2Pressed() ) );
		if( b2->sizeHint().width() > widget_width )
			widget_width =  b2->sizeHint().width();
        nr_buttons++;
    }

    if( b3text ) {
        b3 = new QPushButton( b3text, this, "_b3" );
        
		//b3->resize(80, 25);
        
		connect( b3, SIGNAL( clicked() ), this, SLOT( b3Pressed() ) );
		if( b3->sizeHint().width() > widget_width )
			widget_width =  b3->sizeHint().width();
        nr_buttons++;
    }

    if( b4text ) {
        b4 = new QPushButton( b4text, this, "_b4" );
        
		//b4->resize(80, 25);
        
		connect( b4, SIGNAL( clicked() ), this, SLOT( b4Pressed() ) );
		if( b4->sizeHint().width() > widget_width )
			widget_width =  b4->sizeHint().width();
        nr_buttons++;
    }
    
	// Calculate appropriate buttons sizes
	// Add the buttons to their layout manager
	// The stretch pushes the buttons to the right
	
	debug("created buttons");
	
	
	buttons->addStretch( 10 );
	
	if ( b1 ) {
		b1->setSize( widget_width, b1->sizeHint().height() );
		buttons->addWidget( b1, 0, AlignBottom );
	}
	
	if ( b2 ) {
		b2->setSize( widget_width, b2->sizeHint().height() );
		buttons->addWidget( b2, 0, AlignBottom );
	}
	
	if ( b3 ) {
		b3->setSize( widget_width, b3->sizeHint().height() );
		buttons->addWidget( b3, 0, AlignBottom );
	}
	
	if ( b4 ) {
		b4->setSize( widget_width, b4->sizeHint().height() );
		buttons->addWidget( b4, 0, AlignBottom );
	}
		
	
	debug("activate topLayout");
	
	topLayout->activate();
	
	*/
}


KMsgBox::~KMsgBox() 
{
	delete f1;
	delete picture;
	delete msg;
	
	if( b1 )
		delete b1;
	if( b2 )
		delete b2;
	if( b3 )
		delete b3;
	if( b4 )
		delete b4;
}

void KMsgBox::resizeEvent(QResizeEvent *)
{
	// Resize event is rendered obsolete by automatic geometry management
	 
    /*
	
	int interval = B_WIDTH + B_SPACING;
    
    calcOptimalSize();

    f1->setGeometry(0, height() - 45, width(), 2);
    
    picture->move(15, 10 + (h1 - picture->height()) / 2);
    msg->move(text_offset + ((width() - text_offset) - msg->width()) / 2, 10 + (h1 - msg->height()) / 2);
    int left_offset = (width() - ((nr_buttons * 80) + (nr_buttons - 1) * 10)) / 2;
    int hw = height() - 30;
    if(b1) {
        b1->move(left_offset, hw);
        left_offset += interval;
    }
    if(b2) {
        b2->move(left_offset, hw);
        left_offset += interval;
    }
    if(b3) {
        b3->move(left_offset, hw);
        left_offset += interval;
    }

    if(b4)
        b4->move(left_offset, hw);
		
	*/
}

void KMsgBox::calcOptimalSize()
{
    int text_width = picture->width() + 10 + msg->width() + 30;  // label width + margins

    int button_width = (nr_buttons * 80) + 20 + (nr_buttons - 1) * B_SPACING;

    w = button_width > text_width ? button_width : text_width;
    
    h1 = msg->height() > picture->height() ? msg->height() : picture->height();

    h = h1 + 25 + 40;

    text_offset = 15 + picture->width() + 10;
}

void KMsgBox::b1Pressed() { done(1); }
void KMsgBox::b2Pressed() { done(2); }
void KMsgBox::b3Pressed() { done(3); }
void KMsgBox::b4Pressed() { done(4); }

/*
 * now, the common functions. They may be used for easy creation and usage of
 * some general message boxes
 */

int KMsgBox::message(QWidget *parent, const char *caption, const char *message, int type, const char *btext)

{
    if (!btext)
	btext = klocale->translate("OK");

    KMsgBox *mb = new KMsgBox(parent, caption, message, type, btext);

    int retcode = mb->exec();
    delete mb;
    return retcode;
}


int KMsgBox::yesNo(QWidget *parent, const char *caption, const char *message, int type, const char *yes, const char *no)
{
    if (!no)
	no = klocale->translate("No");
    if (!yes)
	yes = klocale->translate("Yes");

    KMsgBox *mb = new KMsgBox(parent, caption, message, type, yes, no);

    int retcode = mb->exec();
    delete mb;
    return retcode;
}

int KMsgBox::yesNoCancel(QWidget *parent, const char *caption, const char *message, int type,
                        const char *yes, const char *no, const char *cancel)
{
    if (!no)
	no = klocale->translate("No");
    if (!yes)
	yes = klocale->translate("Yes");
    if (!cancel)
	cancel = klocale->translate("Cancel");
    
    KMsgBox *mb = new KMsgBox(parent, caption, message, type, yes, no, cancel);

    int retcode = mb->exec();
    delete mb;
    return retcode;
}

