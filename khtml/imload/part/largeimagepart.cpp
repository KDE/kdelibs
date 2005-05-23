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
#include "largeimagepart.h"

#include <kaction.h>

/*************************************
 The widget
**************************************/

LargeImageWidget::LargeImageWidget(QWidget* parent, const char* name):QScrollView(parent, name)
{
    image = 0;
    zoom  = 1.0;
    setFocusPolicy(ClickFocus);
    viewport()->setBackgroundMode(NoBackground);
}

LargeImageWidget::~LargeImageWidget()
{
    destroyImage();
}

//BEGIN Interface to the part
void LargeImageWidget::createImage()
{
    destroyImage();
    image = new Image(this);
}

void LargeImageWidget::destroyImage()
{
    delete image;
    image = 0;
    zoom  = 1.0;
}

void LargeImageWidget::imageData(uchar* data, unsigned int size)
{
    if (image) image->processData(data, size);
}

void LargeImageWidget::imageEOF()
{
    if (image) image->processEOF();
}

//END Interface to the part

//BEGIN Interface to the image
void LargeImageWidget::imageHasGeometry(Image*, int width, int height)
{
    zoom  = 1.0;
    resizeContents(width, height);
    qDebug("Image size:%d, %d", width, height);
}
    
void LargeImageWidget::imageChange(Image*, QRect r)
{
    updateContents(r);
}
    
void LargeImageWidget::imageError(Image*)
{}

//END Interface to the image

//Drawing code
void LargeImageWidget::drawContents(QPainter* p, int clipx, int clipy, int clipw, int cliph)
{
    int iw = 0, ih = 0;
    if (image)
    {
        //### this seems like it could be flicker-prone -- may want to add a background mode notion
        //p->fillRect(clipx, clipy, clipw, cliph, palette().active().dark());
        image->paint(clipx, clipy, p,
                        clipx, clipy, clipw, cliph, 350);
        iw = image->size().width();
        ih = image->size().height();
    }

    //Fill in the background to the right + bottom of the image
    int clipr = clipx + clipw - 1;

    if (clipr >= iw)
        p->fillRect(iw, clipy, clipr - iw  + 1, cliph, palette().active().dark());

    //Now fill in to the bottom
    int clipb = clipy + cliph - 1;
    if (clipb >= ih)
        p->fillRect(clipx, ih, clipw, clipb - ih + 1, palette().active().dark());
}


//BEGIN Zoomy things
void LargeImageWidget::applySize(unsigned int width, unsigned int height)
{
    image->scale  (width, height);
    resizeContents(width, height);
    updateContents();
}

void LargeImageWidget::applyZoom()
{
    if (!image) return;
    
    QSize cur = image->originalSize();
    applySize(qRound(cur.width() * zoom), qRound(cur.height() * zoom));
}

void LargeImageWidget::zoomIn()
{
    zoom *= 1.1;
    applyZoom();
}

void LargeImageWidget::zoomOut()
{
    zoom /= 1.1;
    applyZoom();
}

unsigned int LargeImageWidget::calcHeight(unsigned int goalWidth, unsigned int iWidth, unsigned int iHeight)
{
    double zoom  = double(goalWidth) / iWidth;
    return qRound(zoom * iHeight);
}

void LargeImageWidget::zoomWidth()
{
    if (!image) return;
        
    unsigned int iWidth       = image->originalSize().width();
    unsigned int iHeight      = image->originalSize().height();

    unsigned int canvasWidth  = contentsRect().width();
    unsigned int canvasHeight = contentsRect().height();

    //First, try to fit w/o a vertical scrollbar (we always avoid horizontal scrollbar here)
    unsigned int goalWidth    = canvasWidth;
    unsigned int goalHeight   = calcHeight(goalWidth, iWidth, iHeight);

    if (goalHeight > canvasHeight)
    {
        //Will need vertical scrollbar, and hence a horizontal one -- allocate less
        //space horizontally
        goalWidth  = canvasWidth - style().pixelMetric(QStyle::PM_ScrollBarExtent);
        goalHeight = calcHeight(goalWidth, iWidth, iHeight);
    }

    //Store the zoom
    zoom = double(goalWidth)/iWidth;

    applySize(goalWidth, goalHeight);
}

//END Zoomy things
    
/*************************************
 The part
**************************************/
typedef KParts::GenericFactory<LargeImagePart> LargeImagePartFactory;

LargeImagePart::LargeImagePart(QWidget* wParent, const char* wName,
                                QObject* parent,  const char* name, 
                                const QStringList&)
                                : KParts::ReadOnlyPart(wParent, wName)
{
    setInstance( LargeImagePartFactory::instance() );        
    job    = 0;   
    
    widget = new LargeImageWidget(wParent, wName);
    setWidget(widget);
    
    setXMLFile("largeimagepart.rc");
    
    (void)KStdAction::zoomIn (widget, SLOT(zoomIn ()), actionCollection());
    (void)new KAction(i18n("Fit to Width"), KShortcut::null(), widget, SLOT(zoomWidth()), actionCollection(), "zoomWidth");
    (void)KStdAction::zoomOut(widget, SLOT(zoomOut()), actionCollection());           
}

K_EXPORT_COMPONENT_FACTORY(largeimagepart, LargeImagePartFactory)


#include "largeimagepart.moc"

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
