// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1998 Daniel Grana <grana@ie.iwi.unibe.ch>
                  1998 Stephan Kulow <coolo@kde.org>
    
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

#include <qlayout.h>
#include <qfile.h>
#include <qtstream.h>
#include <qpixmap.h>
#include <qwmatrix.h>
#include <qstring.h>

#include "kpreview.h"
#include "xview.h"

#include <kdir.h>
#include <kapp.h>

static void cleanUpHandlers();
static void initHandlers();

static const int cMaxLines = 20;
static const int cMaxColumns = 79;

bool previewTextFile( const KFileInfo *, const QString inFilename,
                      QString &outText, QPixmap & )
{
    bool loaded = false;
    QFile lFile( inFilename );
    if ( lFile.open(IO_ReadOnly) ) {
        QTextStream t( &lFile );
        QString line;
        int n = 1;
        while ( !t.eof() && (n<cMaxLines) ) { 
            line = t.readLine();              
            outText.append(line.mid(0,cMaxColumns));
            if (line.mid(cMaxColumns,1) != "\n") outText.append("\n");
            n++;
        }
        lFile.close();
        loaded = true;
    }
    return loaded;
}

bool previewXVPicFile( const KFileInfo *i, const QString inFilename,
                       QString &, QPixmap &outPixmap )
{
    bool loaded = false;
    QString iconName(inFilename);
    iconName.detach();
    int index = iconName.find( i->fileName() );
    iconName.insert(index,".xvpics/");
    QFile miniPic( iconName );
    if ( miniPic.exists() ) {
        outPixmap = QPixmap( iconName );
        QWMatrix m;
        m.scale ( 2.0, 2.0 );
        outPixmap = outPixmap.xForm( m );  // scale it to double size
        loaded = true;
    }
    return loaded;
}

/*
 *
 */
class KPreviewObject {
public:
    KPreviewObject ( const char *inFormat, 
                     PreviewHandler inPreviewModule );

    QString format;
    PreviewHandler preview;

};

KPreviewObject::KPreviewObject ( const char *inFormat, 
                                 PreviewHandler inPreviewModule )
 : format(inFormat)
{
    preview = inPreviewModule;
}
/*
 *
 */
 
static QDict<KPreviewObject> *myTextPreviewerStorage = 0;
static QDict<KPreviewObject> *myPicturePreviewerStorage = 0;

static void cleanUpHandlers()
{
    delete myTextPreviewerStorage;
    delete myPicturePreviewerStorage;
    
    myTextPreviewerStorage = 0L;
    myPicturePreviewerStorage = 0L;

}

static void initHandlers()
{
    if ( myTextPreviewerStorage )
        return;
        
    myTextPreviewerStorage = new QDict<KPreviewObject>;
    myPicturePreviewerStorage = new QDict<KPreviewObject>;

    myTextPreviewerStorage->setAutoDelete( true );
    myPicturePreviewerStorage->setAutoDelete( true );

    qAddPostRoutine(cleanUpHandlers);
    
    KPreview::registerPreviewModule( "TEXT", previewTextFile, PreviewText);    
    KPreview::registerPreviewModule( "XVPIC", previewXVPicFile, PreviewPixmap);    
}

//
// NOTE: was static, not sure if it has t be static or not ...
//       commented it out for now
// 
//KPreview *KPreview::_myKPreview = 0;
/*
KPreview *KPreview::getKPreview(KDir *inDir, QWidget *parent, const char *name)
{
    if ( _myKPreview == 0 ) {
        _myKPreview = new KPreview( inDir, parent, name );
    }
    return _myKPreview;
}

KPreview *KPreview::getKPreview()
{
    if ( _myKPreview == 0 )
       debug("oops");
    return _myKPreview;
}
*/


KPreview::~KPreview()
{}

KPreview::KPreview( const KDir *inDir, QWidget *parent, const char *name)
    : QWidget(parent,name), myDir(inDir), showedText(false)
{

    QImageIO::defineIOHandler( "XV", "^P7 332", 0, read_xv_file, 0L );

    if ( !myTextPreviewerStorage )
        initHandlers();
    
    QHBoxLayout *top = new QHBoxLayout( this, 0, 5, "_top" );

    myBox = new QGroupBox(this,"_previewbox");
    top->addSpacing(5);
    top->addWidget( myBox, 10 );
    
    QGridLayout *vertical = new QGridLayout( myBox, 
                                             10,       // rows
                                             8,        // columns
                                             10,       // border
                                             5,        // autoborder 
                                             "_vertical" );
    
    for (int i=0; i<4; i++)
        vertical->setRowStretch ( i, 0 );
    for (int i=4; i<10; i++)
        vertical->setRowStretch ( i, 5 );
    
    QLabel *myNameLabel = new QLabel( i18n("Name:"), myBox, "_namelabel");
    myNameLabel->adjustSize();
    myNameLabel->setMinimumHeight( myNameLabel->sizeHint().height() );
    myNameLabel->setAlignment( AlignVCenter | AlignLeft );
    vertical->addWidget( myNameLabel, 0, 0 );
    myName = new QLabel( myBox, "_name" );
    myName->setAlignment( AlignVCenter | AlignLeft );
    vertical->addMultiCellWidget( myName, 0, 0, 1, 7 );

    QLabel *mySizeLabel = new QLabel( i18n("Size:"), myBox, "_sizelabel");
    mySizeLabel->setAlignment( AlignVCenter | AlignLeft );
    mySizeLabel->adjustSize();
    mySizeLabel->setMinimumHeight( mySizeLabel->sizeHint().height() );
    vertical->addWidget( mySizeLabel, 1, 0 );
    mySize = new QLabel( myBox, "_size" );
    mySize->setAlignment( AlignVCenter | AlignLeft );
    vertical->addMultiCellWidget( mySize, 1, 1, 1, 3 ); 

    QLabel *myDateLabel = new QLabel( i18n("Date:"), myBox, "_datelabel");
    myDateLabel->setAlignment( AlignVCenter | AlignLeft );
    myDateLabel->adjustSize();
    myDateLabel->setMinimumHeight( myDateLabel->sizeHint().height() );
    vertical->addWidget( myDateLabel, 1, 4 );
    myDate = new QLabel( myBox, "_date" );
    myDate->setAlignment( AlignVCenter | AlignLeft );
    vertical->addMultiCellWidget( myDate, 1, 1, 5, 7 ); 

    QLabel *myOwnerLabel = new QLabel( i18n("Owner:"), myBox, "_ownerlabel");
    myOwnerLabel->setAlignment( AlignVCenter | AlignLeft );
    myOwnerLabel->adjustSize();
    myOwnerLabel->setMinimumHeight( myOwnerLabel->sizeHint().height() );
    vertical->addWidget( myOwnerLabel, 2, 0 );
    myOwner = new QLabel( myBox, "_owner" );
    myOwner->setAlignment( AlignVCenter | AlignLeft );
    vertical->addMultiCellWidget( myOwner, 2, 2, 1, 3 ); 

    QLabel *myGroupLabel = new QLabel( i18n("Group:"), myBox, "_grouplabel");
    myGroupLabel->setAlignment( AlignVCenter | AlignLeft );
    myGroupLabel->adjustSize();
    myGroupLabel->setMinimumHeight( myGroupLabel->sizeHint().height() );
    vertical->addWidget( myGroupLabel, 2, 4 );
    myGroup = new QLabel( myBox, "_owner" );
    myGroup->setAlignment( AlignVCenter | AlignLeft );
    vertical->addMultiCellWidget( myGroup, 2, 2, 5, 7 ); 

    QLabel *myTypeLabel = new QLabel( i18n("Type:"), myBox, "_typelabel");
    myTypeLabel->setAlignment( AlignVCenter | AlignLeft );
    myTypeLabel->adjustSize();
    myTypeLabel->setMinimumHeight( myTypeLabel->sizeHint().height() );
    vertical->addWidget( myTypeLabel, 3, 0 );
    myType = new QLabel( myBox, "_name" );
    myType->setAlignment( AlignVCenter | AlignLeft );
    vertical->addMultiCellWidget( myType, 3, 3, 1, 7 );

    myPreviewText = new QMultiLineEdit( myBox, "_previewpart" );
    myPreviewText->setReadOnly(true);
    vertical->addMultiCellWidget( myPreviewText, 4, 9, 0, 7 );

    myPreviewPicture = new QLabel( myBox, "_previewpart" );
    // myPreviewText->setBackgroundColor( white );
    myPreviewPicture->setAlignment( AlignCenter );
    myPreviewPicture->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    myPreviewPicture->setLineWidth( 2 );
    myPreviewPicture->setMinimumHeight( 80 );  // this is the size of the mini pictures
    vertical->addMultiCellWidget( myPreviewPicture, 4, 9, 0, 7 );

}

void KPreview::registerPreviewModule(const char * format, PreviewHandler readPreview,
                                     PreviewType inType)
{   
    // debug("registering preview module (%s)",format);
    if ( !myTextPreviewerStorage ) {
	initHandlers();
    }

    KPreviewObject *po;
    switch ( inType ) {
        case (PreviewText) :   po = new KPreviewObject( format, readPreview );
                                    myTextPreviewerStorage->insert( format, po );
                                    break;
        case (PreviewPixmap) : po = new KPreviewObject( format, readPreview );
                                    myPicturePreviewerStorage->insert( format, po );
                                    break;
        default :                   break;
    }
}

void KPreview::previewFile(const KFileInfo *i)
{   
    if ( !myTextPreviewerStorage )
        initHandlers();
        
    // upper text part
    //
    bool isRegularFile = !i->isDir(); 
    bool canOpen = i->isReadable() && isRegularFile;
    QString lType = "";
    
    if ( !i->isReadable() )
        lType += i18n("locked");
            
    myName->setText(i->fileName());
    mySize->setNum((int)i->size());
    myDate->setText(i->date());
    myOwner->setText(i->owner());
    myGroup->setText(i->group());

    // preview part
    //
    QString fullPath;
    QString lTextOutput;
    QPixmap lPictOutput;
    QDictIterator<KPreviewObject> lTextIterator( *myTextPreviewerStorage );
    QDictIterator<KPreviewObject> lPictIterator( *myPicturePreviewerStorage );
    bool found = false;
    if ( canOpen ) {
        fullPath = myDir->path();
        fullPath += i->fileName();
 
        lPictIterator.toFirst();
        while ( lPictIterator.current() && (!found) ) {
            if ( lPictIterator.current()->preview( i, fullPath, lTextOutput, lPictOutput ) ) {
                myPreviewPicture->setPixmap( lPictOutput );
                lType += i18n("picture");
                found = true;
                if ( showedText ) {
                    showedText = false;
                    myPreviewPicture->raise();
                }
            }
            ++lPictIterator;
        }
        lTextIterator.toFirst();
        while ( !found && lTextIterator.current() ) {
            if ( lTextIterator.current()->preview( i, fullPath, lTextOutput, lPictOutput ) ) {
                myPreviewText->setText( lTextOutput );
                lType += i18n("file");
                found = true;
                if ( !showedText ) {
                    showedText = true;
                    myPreviewText->raise();
                }
            }
            ++lTextIterator;
        }
    } else {
        myPreviewText->clear();
        myPreviewPicture->setText("");
    }

    if ( i->isDir() )
        lType += i18n("folder");
    myType->setText(lType);
    
}

#include "kpreview.moc"

