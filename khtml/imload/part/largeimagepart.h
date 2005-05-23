/*
    Large image displaying library -- or rather a KPart using that

    Copyright (C) 2004 Maks Orlovich (maksim@kde.org)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
    AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
    AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#ifndef LARGE_IMAGE_PART_H
#define LARGE_IMAGE_PART_H

#include <qpainter.h>
#include <qscrollview.h>

#include <kaboutdata.h>
#include <klibloader.h>
#include <kparts/genericfactory.h>
#include <kparts/part.h> 
#include <kstdaction.h>

#include <kshortcut.h>
#include <qstyle.h>

#include <kio/global.h>
#include <kio/jobclasses.h> 
#include <kio/job.h>

#include <qdatetime.h>

#include "imagemanager.h"
#include "image.h"
#include "imageowner.h"


class LargeImageWidget: public QScrollView, public ImageOwner
{
    Q_OBJECT;
    
    double zoom;
    Image* image;
public:
    LargeImageWidget(QWidget* parent, const char* name);
    ~LargeImageWidget();

    //Image owner interface
    virtual void imageHasGeometry(Image* im, int width, int height);    
    virtual void imageChange(Image* im, QRect r);
    virtual void imageError(Image*);
    
    //Painting of the view contents
    virtual void drawContents(QPainter* p, int clipx, int clipy, int clipw, int cliph);

    //Interface to loading/destroying images from the part
    void createImage();
    void destroyImage();
    void imageData(uchar* data, unsigned int size);
    void imageEOF();
public slots:
    void zoomIn();
    void zoomOut();

    /* "fit to width" zoom */
    void zoomWidth();
private:
    void applySize(unsigned int width, unsigned int height); //Adjust the image, and the scrollview to the size
    void applyZoom(); //Applies the zoom ratio
    unsigned int calcHeight(unsigned int goalWidth, unsigned int iWidth, unsigned int iHeight);
};

class LargeImagePart: public KParts::ReadOnlyPart
{
    Q_OBJECT
private:
    KIO::TransferJob*  job;
    LargeImageWidget*  widget;
public:
    LargeImagePart(QWidget* wParent, const char* wName,
                   QObject* parent,  const char* name, 
                   const QStringList&);

    static KAboutData* createAboutData()
    {
        return new KAboutData("largeimagepart", I18N_NOOP("Large image viewer"), "0.1");
    }
    
    virtual bool openFile()
    {
        return true; /* we use openURL instead */
    }
    
    void cleanup()
    {
        widget->destroyImage();
        
        if (job)
        {
            job->kill();
            job = 0;
        }
    }
    
    virtual bool openURL( const KURL& url )
    {
        widget->createImage(); //cleans up the old one if need be
        
        //Start a job
        job = KIO::get(url, false, false);
        connect(job,  SIGNAL(data(KIO::Job*, const QByteArray&)), 
                this, SLOT  (data(KIO::Job*, const QByteArray&)));
        connect(job,  SIGNAL(result(KIO::Job*)),
                this, SLOT  (result(KIO::Job*)));
        
        return true;                
    }
    
    virtual bool closeURL()
    {
        cleanup();
        return KParts::ReadOnlyPart::closeURL();
    }
    
    ~LargeImagePart()
    {
        cleanup();
    }
public slots:    
    void data(KIO::Job*, const QByteArray& data)
    {
        widget->imageData((uchar*)data.data(), data.size());
    }
    
    void result(KIO::Job*)
    {
        widget->imageEOF();
        job = 0;
    }
};

#endif

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
