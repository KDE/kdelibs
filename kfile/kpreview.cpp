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

#include "kpreview.h"

#include <kdir.h>
#include <kapp.h>

static const int cMaxLines = 20;
static const int cMaxColumns = 79;

KPreview::KPreview( const KDir *inDir, QWidget *parent, const char *name)
    : QWidget(parent,name), myDir(inDir)
{
    // QWidget::setFocusPolicy(QWidget::StrongFocus);
    
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

    myPreview = new QMultiLineEdit( myBox, "_previewpart" );
    myPreview->setReadOnly(true);
    vertical->addMultiCellWidget( myPreview, 4, 9, 0, 7 );

}

void KPreview::previewFile(const KFileInfo *i)
{
    bool canOpen = false;
    QString lType = "";

    if (i->isDir())
	if (i->isReadable())
	    lType = "folder";
	else
	    lType = "locked folder";
    else
	if (i->isReadable()) {
	    lType = "file";
            canOpen = true;
        }
	else
	    lType = "locked file";
    
    if ( canOpen ) {
        QString fullPath = myDir->path();
        fullPath += i->fileName(); 
        QFile lFile( fullPath );
        if ( lFile.open(IO_ReadOnly) ) {  
            QTextStream t( &lFile );       
            QString s("");
            QString line;
            int n = 1;
            while ( !t.eof() && (n<cMaxLines) ) { // until end of file...
                line = t.readLine();        // line of text excluding '\n'
                s.append(line.mid(0,cMaxColumns));
                if (line.mid(cMaxColumns,1) != "\n") s.append("\n");
                n++;
            }
            lFile.close();
            myPreview->setText(s);
        }
    } else {
        myPreview->clear();
    }
    myName->setText(i->fileName());
    mySize->setNum((int)i->size());
    myDate->setText(i->date());
    myOwner->setText(i->owner());
    myGroup->setText(i->group());
    myType->setText(lType);

    // TODO: find out, if a repaint is really necessary
}


#include "kpreview.moc"

