#include <kstyle.h>
#include <qdrawutil.h>

void KStyle::drawKBarHandle(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, bool horizontal,
                            QBrush *fill)
{
    int stipple_height;
    
    if(horizontal){
        qDrawShadePanel(p, x, y, w, h, g , false, 1, fill);
        p->setPen(g.light());
        stipple_height = 3;
        while(stipple_height < h-4) {
            p->drawPoint(x+1, y+stipple_height+1);
            p->drawPoint(x+4, y+stipple_height);
        stipple_height+=3;
        }
        p->setPen(g.dark());
        stipple_height = 4;
        while(stipple_height < h-4) {
            p->drawPoint(x+2, y+stipple_height+1);
            p->drawPoint(x+5, y+stipple_height);
            stipple_height+=3;
        }
    }
    else{
        qDrawShadePanel(p, x, y, w, h, g , false, 1, fill);
        p->setPen(g.light());
        stipple_height = 3;
        while(stipple_height < w-4) {
            p->drawPoint(x+stipple_height+1, y+1);
            p->drawPoint(x+stipple_height, y+4);
            stipple_height+=3;
        }
        p->setPen(g.dark());
        stipple_height = 4;
        while(stipple_height < w-4) {
            p->drawPoint(x+stipple_height+1, y+2);
            p->drawPoint(x+stipple_height, y+5);
            stipple_height+=3;
        }
    }
}

void KStyle::drawKToolBar(QPainter *p, int x, int y, int w, int h,
                          const QColorGroup &g, bool floating)
{
    if(!floating)
        qDrawShadePanel(p, x, y, w, h, g , false, 1);
}

void KStyle::drawKToolBarButton(QPainter *p, int x, int y, int w, int h, const
                                QColorGroup &g, bool sunken, bool raised,
                                bool enabled, bool popup,
                                KToolButtonType icontext, const QString btext,
                                const QPixmap *pixmap,
                                QFont *font)
{
    if ( sunken )
        qDrawWinButton(p, x, y, w, h, g, true );
    else if ( raised )
        qDrawWinButton(p, x, y, w, h, g, false );

    int dx, dy;

    if (icontext == Icon){ // icon only
        if (pixmap){
            dx = ( w - pixmap->width() ) / 2;
            dy = ( h - pixmap->height() ) / 2;
            if ( sunken )
            {
                ++dx;
                ++dy;
            }
            p->drawPixmap( x+dx, y+dy, *pixmap );
        }
    }
    else if (icontext == IconTextRight){ // icon and text (if any)
        if (pixmap){
            dx = 1;
            dy = ( h - pixmap->height() ) / 2;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            p->drawPixmap( x+dx, y+dy, *pixmap );
        }
        if (!btext.isNull()){
            int tf = AlignVCenter|AlignLeft;
            if (!enabled)
                p->setPen(g.dark());
            if (pixmap)
                dx= pixmap->width();
            else
                dx= 1;
            dy = 0;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            if (font)
                p->setFont(*font);
            if(raised)
                p->setPen(Qt::blue);
            p->drawText(x+dx, y+dy, w-dx, h, tf, btext);
        }
    }
    else if (icontext == Text){ // only text, even if there is a icon
        if (!btext.isNull()){
            int tf = AlignVCenter|AlignLeft;
            if (!enabled)
                p->setPen(g.dark());
            dx= 1;
            dy= 0;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            if (font)
                p->setFont(*font);
            if(raised)
                p->setPen(Qt::blue);
            p->drawText(x+dx, y+dy, w-dx, h, tf, btext);
        }
    }
    else if (icontext == IconTextBottom){
        if (pixmap){
            dx = (w - pixmap->width()) / 2;
            dy = 1;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            p->drawPixmap( x+dx, y+dy, *pixmap );
        }
        if (!btext.isNull()){
            int tf = AlignBottom|AlignHCenter;
            if (!enabled)
                p->setPen(g.dark());
            dy= pixmap->height();
            dx = 2;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            if (font)
                p->setFont(*font);
            if(raised)
                p->setPen(Qt::blue);
            p->drawText(x, y, w, h-4, tf, btext);
        }
    }
    //#warning What about Icontext=3

    if (popup){
        if (enabled)
            qDrawArrow (p, DownArrow, WindowsStyle, false, w-5, h-5, 0, 0,
                        g, true);
        else
            qDrawArrow (p, DownArrow, WindowsStyle, false, w-5, h-5,
                        0, 0, g, false);
    }
}

#include "kstyle.moc"

