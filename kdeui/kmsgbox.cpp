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
 * Revision 1.18  1999/05/07 15:42:55  kulow
 * making some changes to the code and partly to the API to make it
 * -DQT_NO_ASCII_CAST compatible.
 * The job is quite boring, but triggers some abuses of QString. BTW:
 * I added some TODOs to the code where I was too lazy to continue.
 * Someone should start a grep for TODO in the code on a regular base ;)
 *
 * Revision 1.17  1999/03/10 19:46:55  porten
 * ported to new qlayout
 *
 * Revision 1.16  1999/03/02 16:22:28  kulow
 * i18n is no longer a macro, but a function defined in klocale.h. So you
 * don't need to include kapp.h when you want to use i18n. I see klocale->translate
 * as obsolute (actually I seded it all over KDE :)
 * I wanted to remove the #include <klocale.h> from kapp.h, but this broke
 * too much, so I readded it after fixing half of kdeui. I guess I will
 * write a script once that fixed compilation problems (I like the qt20fix way :),
 * but for now it's only important to know that i18n works without kapp.h
 *
 * Revision 1.15  1999/03/01 23:34:53  kulow
 * CVS_SILENT ported to Qt 2.0
 *
 * Revision 1.14.2.1  1999/02/21 20:55:50  kulow
 * more porting to Qt 2.0. It compiles and links. Jucheisassa :)
 *
 * Revision 1.14  1999/01/18 10:56:50  kulow
 * .moc files are back in kdelibs. Built fine here using automake 1.3
 *
 * Revision 1.13  1999/01/15 09:31:09  kulow
 * it's official - kdelibs builds with srcdir != builddir. For this I
 * automocifized it, the generated rules are easier to maintain than
 * selfwritten rules. I have to fight with some bugs of this tool, but
 * generally it's better than keeping them updated by hand.
 *
 * Revision 1.12  1998/08/03 15:24:22  ssk
 * Wrote documentation.
 * Eliminated dead and commented-out code.
 *
 * Revision 1.11  1998/03/15 13:05:14  mario
 * Mario: fixed kb60
 *
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
#include <klocale.h>
#include <kbuttonbox.h>

#include "kmsgbox.h"

KMsgBox::KMsgBox( QWidget *parent, const QString& caption,
	const QString&message, int type,
	const QString&b1text, const QString&b2text,
	const QString&b3text, const QString&b4text )
	: QDialog ( parent, caption.ascii(), TRUE, 0 )
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

	topLayout->addStretch( 10 );
	
	// CREATE BUTTONS
	
	nr_buttons = 0;
    b1 = b2 = b3 = b4 = 0;
	
	KButtonBox *bbox = new KButtonBox( this );
	bbox->addStretch( 10 );
	
    if( !b1text.isNull() ) {
		b1 = bbox->addButton( b1text );
		connect( b1, SIGNAL( clicked() ), this, SLOT( b1Pressed() ) );
        nr_buttons++;
    }

    if( !b2text.isNull() ) {
	
        b2 = bbox->addButton( b2text );
		connect( b2, SIGNAL( clicked() ), this, SLOT( b2Pressed() ) );
        nr_buttons++;
    }

    if( !b3text.isNull() ) {
		b3 = bbox->addButton( b3text );
		connect( b3, SIGNAL( clicked() ), this, SLOT( b3Pressed() ) );
        nr_buttons++;
    }

    if( !b4text.isNull() ) {
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
	
        setFixedSize(sizeHint());
}

KMsgBox::~KMsgBox() 
{
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

int KMsgBox::message(QWidget *parent, const QString&caption, 
	const QString&message, int type, const QString&btext)

{
    KMsgBox *mb = new KMsgBox(parent, caption, message, type, 
			      btext.isNull() ? i18n("OK") : btext);

    int retcode = mb->exec();
    delete mb;
    return retcode;
}


int KMsgBox::yesNo(QWidget *parent, const QString&caption, 
	const QString&message, int type, 
	const QString&yes, const QString&no )
{
    KMsgBox *mb = new KMsgBox(parent, caption, message, type, 
			      yes.isNull() ? i18n("Yes") : yes,
			      no.isNull() ? i18n("No") : no);

    int retcode = mb->exec();
    delete mb;
    return retcode;
}

int KMsgBox::yesNoCancel(QWidget *parent, const QString& caption, 
			 const QString& message, int type, const QString& yes, 
			 const QString& no, const QString& cancel)
{
    KMsgBox *mb = new KMsgBox(parent, caption, message, type, 
			      yes.isNull() ? i18n("Yes") : yes,
			      no.isNull() ? i18n("No") : no,
			      cancel.isNull() ? i18n("Cancel") : cancel);

    int retcode = mb->exec();
    delete mb;
    return retcode;
}
#include "kmsgbox.moc"
