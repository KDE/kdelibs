/*
    Large image displaying library -- test app

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

#include <qapplication.h>
#include <kinstance.h>
#include <qimage.h>
#include <qdir.h>
#include <qscrollview.h>
#include <qptrvector.h>
#include <qvaluevector.h>

#include <qtimer.h>

#include "image.h"
#include "imageowner.h"
#include "fileloader.h"
#include "imagemanager.h"
#include <math.h>

#define PATH "/home/Maksim/testset/"


class ImageWrapper;

QValueVector<ImageWrapper*> images;

class ImageViewer:public QScrollView
{
public:
    ImageViewer()
    {
    }
    
    void drawContents(QPainter* p, int clipx, int clipy, int clipw, int cliph);
};

class ImageWrapper: public ImageOwner
{
private:
    QString fname;
    ImageViewer* owner;
        
    FileLoader loader;
    
public:    
    int        width;
    int        height;
    Image      theImage;    

    ImageWrapper(ImageViewer* _owner,  QString fileName):fname(fileName), theImage(this),loader(&theImage, fileName)
    {
        owner = _owner;
        width  = 3;
        height = 3;
    }
    
    virtual void imageHasGeometry(Image*, int width, int height) 
    {
        //Do KHTML's size limiting thingy
/*        if ( width * height > 2048*2048 ) 
        {
            float scale = sqrt( width*height / ( 2048.*2048. ) );
            width = (int) (width/scale);
            height = (int) (height/scale);
            theImage.scale(width, height);
        }*/
        qDebug("fname:%s, width:%d, height:%d", fname.latin1(), width, height);
        /*width  = width / 2;
        height = height / 2;
        theImage.scale(width, height);*/

        
        this->width  = width;
        this->height = height;
        
        //Resize..        
        owner->updateContents(); //Cheapo relayout
    }
    
    virtual void imageChange(Image* img, QRect region)
    {
        //Calculate the region to update, by scanning all the images until we find us
        int y = 0;
        for (int img = 0; img < images.size(); img++)
        {
            if (images[img] == this)
                break;
            y += images[img]->height - 1 + 5;
        }
        owner->updateContents(0, y + region.y(), region.width(), region.height());
    }
    
    /**
     Called to notify the owner the the image is broken
    */
    virtual void imageError(Image* img)
    {        
        owner->updateContents();
    }
};

void ImageViewer::drawContents(QPainter* p, int clipx, int clipy, int clipw, int cliph)
{
    int sclip = clipy;
    int eclip = clipy + cliph - 1;

    //Walk through the images, see which ones intersect.
    int sy = 0;
    for (int img = 0; img < images.size(); img++)
    {
        int ey = sy + images[img]->height - 1;
        
        //qDebug("cl:%d-%d, img:%d-%d", sclip, eclip, sy, ey);
        //Check whether sClip -> eClip region overlaps with the sy - ey region
        if (sclip <= ey && sy <= eclip)
        {
            //Yep. Figure out where to draw, etc..
            int srcY = 0;
            if (sy < sclip)
                srcY = sclip - sy;

            int srcHeight = images[img]->height - srcY;
            //qDebug("original height:%d, eclip:%d, ey:%d", srcHeight, eclip, ey);
            if (eclip < ey)
                srcHeight -= (ey - eclip);
            //qDebug("modified height:%d", srcHeight);                    
                
            //Draw
            images[img]->theImage.paint(clipx, sy+srcY, p, clipx, srcY, clipw, srcHeight);
        }
        
        sy = ey + 5; //For next image..
    }
}



int main(int argc, char** argv)
{
    QApplication qapp(argc, argv);
    KInstance* inst = new KInstance("testapp");

    ImageViewer view;
    view.resize(1000, 740);
    view.resizeContents(3000, 100000);//### fudge!
    view.show();    
    
    QDir d(PATH);
    
    int width = 0;
    int height = 0;
 
    for ( int i = 0; i < d.count(); i++ )
    {
        QString name = d[i];
        qDebug("%s", name.latin1());
        if (!name.startsWith("."))
        {
            ImageWrapper* wrap = new ImageWrapper(&view, PATH + name);
           
            images.append(wrap);
        }
   }

   qapp.setMainWidget(&view);
   return qapp.exec();
}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
