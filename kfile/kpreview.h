// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
    
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

#ifndef KPREVIEW_H
#define KPREVIEW_H

#include <qpixmap.h>
#include <qgrpbox.h>
#include <qlabel.h>
#include <qmlined.h>
#include <qdict.h>

#include "kfileinfo.h"
#include "kdir.h"
#include "kfiledialog.h"

class KPreviewObject;

class KPreview : public QWidget {

    Q_OBJECT
    
public:

    KPreview( const KDir *inDir= 0, QWidget *parent= 0, const char *name= 0);
    //static KPreview *getKPreview(KDir *inDir, QWidget *parent= 0, const char *name= 0);
    //static KPreview *getKPreview();
    virtual ~KPreview(); 
    
public:
    static void registerPreviewModule( const char * format, PreviewHandler readPreview,
                                       PreviewType inType);
    
public slots:
    virtual void previewFile(const KFileInfo *i);

private:
    QLabel *myName;
    QLabel *mySize;
    QLabel *myDate;
    QLabel *myOwner;
    QLabel *myGroup;
    QLabel *myType;
    QLabel *myPreviewPicture;
    QMultiLineEdit *myPreviewText;
    QGroupBox *myBox;
    const KDir *myDir;
    bool showedText;

    KPreview *_myKPreview;    

};


#define KPreviewer (*KPreview::getKPreview())

#endif
