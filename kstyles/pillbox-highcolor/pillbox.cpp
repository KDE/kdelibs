#ifndef INCLUDE_MENUITEM_DEF
#define INCLUDE_MENUITEM_DEF
#endif

#include <qmenudata.h>
#include <kconfig.h>
#include <kglobal.h>
#include "pillbox.h"
#include <kapp.h>
#include <kdrawutil.h>
#include <kpixmapeffect.h>
#include <qimage.h>
#include <qpalette.h>
#include <qbitmap.h>
#include <qtabbar.h>
#include <qobjectlist.h>
#include "../highcolor/paperbits.h"


#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)

static const unsigned char combodeco_bits[] = {
    0xff, 0xff, 0x00, 0xff, 0xff, 0x7e, 0x3c, 0x18};

static const unsigned char checkfill_bits[] = {
    0x00, 0x00, 0x80, 0x01, 0x80, 0x00, 0xc0, 0x00, 0x40, 0x00, 0x60, 0x00,
    0x22, 0x00, 0x36, 0x00, 0x1c, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00};

static const unsigned char checkoutline_bits[] = {
    0x80, 0x03, 0x40, 0x02, 0x40, 0x01, 0x20, 0x01, 0xa1, 0x00, 0x93, 0x00,
    0x55, 0x00, 0x49, 0x00, 0x22, 0x00, 0x24, 0x00, 0x18, 0x00, 0x10, 0x00};

static const unsigned char up_bits[] = {
    0x00, 0x18, 0x3c, 0x7e, 0xff, 0xff, 0x00, 0x00};

static const unsigned char down_bits[] = {
    0x00, 0x00, 0xff, 0xff, 0x7e, 0x3c, 0x18, 0x00};

static const unsigned char left_bits[] = {
    0x30, 0x38, 0x3c, 0x3e, 0x3e, 0x3c, 0x38, 0x30};

static const unsigned char right_bits[] = {
    0x0c, 0x1c, 0x3c, 0x7c, 0x7c, 0x3c, 0x1c, 0x0c};
    
PillBoxStyle::PillBoxStyle()
    :KStyle()
{
    setButtonDefaultIndicatorWidth(0);

    if(QPixmap::defaultDepth() > 8){
        vSmall = new KPixmap;
        vSmall->resize(20, 24);
        vMed = new KPixmap;
        vMed->resize(20, 34);
        vLarge = new KPixmap;
        vLarge->resize(20, 64);
        hSmall = new KPixmap;
        hSmall->resize(24, 20);
        hMed = new KPixmap;
        hMed->resize(34, 20);
        hLarge = new KPixmap;
        hLarge->resize(64, 20);
    }
    else{
        vSmall = vMed = vLarge = NULL;
        hSmall = hMed = hLarge = NULL;
    }

}

PillBoxStyle::~PillBoxStyle()
{
    if(vSmall){
        delete vSmall;
        delete vMed;
        delete vLarge;
        delete hSmall;
        delete hMed;
        delete hLarge;
    }
}


bool PillBoxStyle::eventFilter(QObject *obj, QEvent *ev)
{
    if(ev->type() == QEvent::Resize){
        // must be a toolbar resize
        QObjectList *tbChildList = obj->queryList("KToolBarButton", NULL,
                                                  false, false);
        QObjectListIt it(*tbChildList);
        QObject *child;
        while((child = it.current()) != NULL){
            ++it;
            if(child->isWidgetType())
                ((QWidget *)child)->repaint(false);
        }
	delete tbChildList;
    }
    return(false);
}


void PillBoxStyle::polish(QPalette &pal)
{
    KConfig *config = KGlobal::config();
    QString oldGrp = config->group();
    config->setGroup("KDE");
    macMode = config->readBoolEntry("macStyle", true);

    QColorGroup g = pal.active();
    if(vSmall){
        KPixmapEffect::gradient(*vSmall, g.midlight(),
                                g.mid(),
                                KPixmapEffect::VerticalGradient);
        KPixmapEffect::gradient(*vMed, g.midlight(),
                                g.mid(),
                                KPixmapEffect::VerticalGradient);
        KPixmapEffect::gradient(*vLarge, g.midlight(),
                                g.mid(),
                                KPixmapEffect::VerticalGradient);
        KPixmapEffect::gradient(*hSmall, g.midlight(),
                                g.mid(),
                                KPixmapEffect::HorizontalGradient);
        KPixmapEffect::gradient(*hMed, g.midlight(),
                                g.mid(),
                                KPixmapEffect::HorizontalGradient);
        KPixmapEffect::gradient(*hLarge, g.midlight(),
                                g.mid(),
                                KPixmapEffect::HorizontalGradient);
    }
    config->setGroup("Highcolor");
    QString tmpStr = config->readEntry("CustomWallpaper", "");
    if(!tmpStr.isEmpty()){
        QPixmap wallPaper(tmpStr);
        if(!wallPaper.isNull())
            pal.setBrush(QColorGroup::Background,
                 QBrush(pal.color(QPalette::Active, QColorGroup::Background),
                        wallPaper));
        else
            qWarning("Highcolor PillBox: Unable to load wallpaper %s",
                    tmpStr.latin1());
    }
    else if(config->readBoolEntry("UseWallpaper", true)){
        QPixmap wallPaper;
        makeWallpaper(wallPaper,pal.color(QPalette::Active,
                                          QColorGroup::Background));
        pal.setBrush(QColorGroup::Background,
                     QBrush(pal.color(QPalette::Active, QColorGroup::Background),
                            wallPaper));
    }
    config->setGroup(oldGrp);

}

void PillBoxStyle::polish(QWidget *w)
{
    if ( !w->isTopLevel() ) {
        if (w->inherits("QPushButton")
            || w->inherits("QComboBox")
            //|| w->inherits("QSlider")
            || w->inherits("QRadioButton"))
            w->setAutoMask(true);
    }

    if(QPixmap::defaultDepth() > 8){
        if(w->inherits("KToolBar"))
            w->installEventFilter(this);
        if(w->inherits("KToolBarButton"))
            w->setBackgroundMode(QWidget::NoBackground);
    }

}

void PillBoxStyle::unPolish(QWidget *w)
{
    if ( !w->isTopLevel() ) {
        if (w->inherits("QPushButton")
            || w->inherits("QComboBox")
            //|| w->inherits("QSlider")
            || w->inherits("QRadioButton"))
            w->setAutoMask(false);
    }
    if(QPixmap::defaultDepth() > 8){
        if(w->inherits("KToolBar"))
            w->removeEventFilter(this);
        if(w->inherits("KToolBarButton"))
            w->setBackgroundMode(QWidget::PaletteBackground);
    }
}

void PillBoxStyle::drawPushButtonLabel(QPushButton *btn, QPainter *p)
{
    // hack to bring round button text up one pixel
    int x = btn->x();
    int y = btn->y();
    int w = btn->width();
    int h = btn->height();
    if(w > 16 && h > 16)
        --x, --y, w-=2, h-=2;
    bool act = btn->isOn() || btn->isDown();
    drawItem(p, x+act?1:0, y+act?1:0, w, h,
             AlignCenter | ShowPrefix, btn->colorGroup(), btn->isEnabled(),
             btn->pixmap(), btn->text(), -1,
             act ? &btn->colorGroup().light() : &btn->colorGroup().buttonText());
}

void PillBoxStyle::drawPushButton(QPushButton *btn, QPainter *p)
{
    QColorGroup cg = btn->colorGroup();
    bool sunken = btn->isOn() || btn->isDown();
    QRect r = btn->rect();

    drawVGradient(p, cg.brush(QColorGroup::Mid), r.x(), r.y(), r.width(),
                  r.height());
    kDrawRoundButton(p, r, cg, sunken);
}


void PillBoxStyle::drawButtonMask(QPainter *p, int x, int y, int w, int h)
{
    kDrawRoundMask(p, x, y, w, h);
}

void PillBoxStyle::drawButton(QPainter *p, int x, int y, int w, int h,
                             const QColorGroup &cg, bool sunken,
                             const QBrush *fill)
{
    drawBevelButton(p, x, y, w, h, cg, sunken, fill);
}

QRect PillBoxStyle::buttonRect(int x, int y, int w, int h)
{
    return(QRect(x+6, y+6, w-12, h-12));
}

void PillBoxStyle::drawComboButton(QPainter *p, int x, int y, int w, int h,
                                  const QColorGroup &cg, bool sunken,
                                  bool, bool, const QBrush *)
{
    if (comboDeco.isNull())
    {
        comboDeco = QBitmap(8, 8, combodeco_bits, true);
        comboDeco.setMask(comboDeco);
    }
    
    p->setPen(Qt::black);
    int x2 = x+w-1;
    int y2 = y+h-1;
    p->drawLine(x+1, y, x2-1, y);
    p->drawLine(x, y+1, x, y2-1);
    p->drawLine(x+1, y2, x2-1, y2);
    p->drawLine(x2, y+1, x2, y2-1);
    qDrawShadeRect(p, x+1, y+1, w-2, h-2, cg, sunken, 1, 0);
    drawVGradient(p, cg.brush(QColorGroup::Mid), x+2, y+2, w-4, h-4);

    int right = x+w-1;
    int bottom = y+h-1;
    
    p->setPen(cg.light());
    p->drawLine(right-16, y+5, right-16, bottom-5);
    p->setPen(cg.dark());
    p->drawLine(right-15, y+5, right-15, bottom-5);

    p->setPen(Qt::black);
    p->drawPixmap(right-12, y+(h-8)/2, comboDeco);
}

QRect PillBoxStyle::comboButtonRect(int x, int y, int w, int h)
{
    return(QRect(x+4, y+4, w-23, h-8));
}

void PillBoxStyle::drawComboButtonMask(QPainter *p, int x, int y, int w, int h)
{
    p->fillRect(x, y, w, h, QBrush(color1, SolidPattern));
    p->setPen(Qt::color0);
    int x2 = x+y-1;
    int y2 = y+h-1;
    p->drawPoint(x, y);
    p->drawPoint(x2, y);
    p->drawPoint(x, y2);
    p->drawPoint(x2, y2);
    
}

void PillBoxStyle::drawBevelButton(QPainter *p, int x, int y, int w, int h,
                                 const QColorGroup &g, bool sunken,
                                 const QBrush *)
{
    if(w > 2 && h > 2){
        p->setPen(Qt::black);
        p->drawRect(x, y, w, h);
        qDrawShadeRect(p, x+1, y+1, w-2, h-2, g, sunken, 1, 0);
        drawVGradient(p, g.brush(QColorGroup::Mid), x+2, y+2, w-4, h-4);
        
    }
    else{
        qDrawShadeRect(p, x, y, w, h, g, sunken, 1, 0);
        drawVGradient(p, g.brush(QColorGroup::Mid), x+1, y+1, w-2, h-2);
    }
}


void PillBoxStyle::drawKToolBar(QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g, KToolBarPos, QBrush *)
{
    if(w > h)
        drawVGradient(p, g.brush(QColorGroup::Mid), x, y, w, h);
    else
        drawHGradient(p, g.brush(QColorGroup::Mid), x, y, w, h);
}

void PillBoxStyle::drawKMenuBar(QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g, bool, QBrush *)
{
    drawVGradient(p, g.brush(QColorGroup::Mid), x, y, w, h);
}

void PillBoxStyle::drawLightShadeRect(QPainter *p, int x, int y, int w, int h,
                                     const QColorGroup &g, const QBrush *fill)
{
    int x2 = x+w-1, y2 = y+h-1;
    p->setPen(g.dark());
    p->drawLine(x, y, x2, y);
    p->drawLine(x, y, x, y2);
    p->setPen(g.midlight());
    p->drawLine(x, y2, x2, y2);
    p->drawLine(x2, y, x2, y2);
    p->fillRect(x+1, y+1, x2-x-2, y2-y-2, fill ? *fill :
                g.brush(QColorGroup::Background));
}
    

void PillBoxStyle::drawKBarHandle(QPainter *p, int x, int y, int w, int h,
                                 const QColorGroup &g, KToolBarPos,
                                 QBrush *)
{
    qDrawShadeRect(p, x, y, w, h, g, false, 1, 0);
    if(h > w){
        drawVGradient(p, g.brush(QColorGroup::Mid), x+1, y+1, w-2, h-2);
        x += 2;
        y += 3;
        w = 5;
        h = 9;
    }
    else{
        drawHGradient(p, g.brush(QColorGroup::Mid), x+1, y+1, w-2, h-2);
        x += 3;
        y += 2;
        w = 9;
        h = 5;
    }
    drawLightShadeRect(p, x, y, w, h, g, &g.brush(QColorGroup::Mid));
}

void PillBoxStyle::drawKMenuItem(QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g, bool active,
                                QMenuItem *mi, QBrush *)
{
    if(active){
        qDrawShadePanel(p, x, y, w, h, g, true, 1,
                        &g.brush(QColorGroup::Midlight));
        QApplication::style().drawItem(p, x, y, w, h,
                                       AlignCenter|ShowPrefix|DontClip|SingleLine,
                                       g, mi->isEnabled(), mi->pixmap(), mi->text(),
                                       -1, &g.text());
    }
    else
        QApplication::style().drawItem(p, x, y, w, h,
                                       AlignCenter|ShowPrefix|DontClip|SingleLine,
                                       g, mi->isEnabled(), mi->pixmap(), mi->text(),
                                       -1, &g.text() );
}

void PillBoxStyle::drawKToolBarButton(QPainter *p, int x, int y, int w, int h,
                                     const QColorGroup &g, bool sunken,
                                     bool raised, bool enabled, bool popup,
                                     KToolButtonType icontext,
                                     const QString& btext, const QPixmap *pixmap,
                                     QFont *font, QWidget *btn)
{
    int x2 = x+w-1;
    int y2 = y+h-1;
    int dx, dy;

    if(raised || sunken){
        p->setPen(sunken ? g.dark() : g.light());
        p->drawLine(x, y, x2, y);
        p->drawLine(x, y, x, y2);

        p->setPen(sunken ? g.light() : g.dark());
        p->drawLine(x, y2, x2-1, y2);
        p->drawLine(x2, y, x2, y2-1);
        drawVGradient(p, g.brush(QColorGroup::Mid), x+1, y+1, w-2, h-2);
    }
    else{
        if(btn->parent() && btn->parent()->isWidgetType()){
            QWidget *toolbar = (QWidget*)btn->parent();
            // horizontal toolbar
            if(toolbar->width() > toolbar->height()){
                // See if we are top row. Buttons are offset a few pixels
                // but not visibly.
                if(btn->y() <= 3){
                    if(toolbar->height() <= 24)
                        p->drawTiledPixmap(x, y, w, h, *vSmall);
                    else if(toolbar->height() <= 34)
                        p->drawTiledPixmap(x, y, w, h, *vMed);
                    else
                        p->drawTiledPixmap(x, y, w, h, *vLarge);

                }
                // See if we are in the gradient at all. Two rows always are
                // large.
                else if(btn->y() <= 64){
                    p->fillRect(x, y, w, h, g.mid());
                    p->drawTiledPixmap(x, y, w, 64-btn->y(),
                                       *vLarge, 0, btn->y());
                }
                // nope, we are not in the gradient
                else
                    p->fillRect(x, y, w, h, g.mid());
            }
            // vertical toolbar
            else{
                if(btn->x() <= 3){
                    if(toolbar->width() <= 24)
                        p->drawTiledPixmap(x, y, w, h, *hSmall);
                    else if(toolbar->width() <= 34)
                        p->drawTiledPixmap(x, y, w, h, *hMed);
                    else
                        p->drawTiledPixmap(x, y, w, h, *hLarge);

                }
                else if(btn->x() <= 64){
                    p->fillRect(x, y, w, h, g.mid());
                    p->drawTiledPixmap(x, y, 64-btn->x(), h,
                                       *hLarge, btn->x(), 0);
                }
                else
                    p->fillRect(x, y, w, h, g.mid());
            }
        }
    }
    p->setPen(g.text());
    
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
    if (popup){
        if (enabled)
            qDrawArrow (p, Qt::DownArrow, Qt::WindowsStyle, false, w-5, h-5, 0, 0,
                        g, true);
        else
            qDrawArrow (p, Qt::DownArrow, Qt::WindowsStyle, false, w-5, h-5,
                        0, 0, g, false);
    }
}

void PillBoxStyle::drawScrollBarControls( QPainter *p, const QScrollBar *sb,
                                         int sliderStart,
                                         unsigned int controls,
                                         unsigned int activeControl )
{
    QRect addB, subB, addPageR, subPageR, sliderR;
    bool horiz = sb->orientation() == QScrollBar::Horizontal;
    int sliderMin, sliderMax, sliderLength, buttonDim;
    int addX, addY, subX, subY;
    int length = horiz ? sb->width()  : sb->height();
    int extent = horiz ? sb->height() : sb->width();
    QColorGroup g  = sb->colorGroup();

    scrollBarMetrics( sb, sliderMin, sliderMax, sliderLength, buttonDim );

    if(sliderStart > sliderMax)
        sliderStart = sliderMax;

    if(horiz){
        subY = addY = (extent - buttonDim)/2;
        subX = length - (buttonDim*2);
        addX = length - buttonDim;
    }
    else{
        subX = addX = (extent - buttonDim) / 2;
        subY = length - (buttonDim*2);
        addY = length - buttonDim;
    }

    subB.setRect(subX, subY, buttonDim, buttonDim);
    addB.setRect(addX, addY, buttonDim, buttonDim);

    int sliderEnd = sliderStart + sliderLength;
    int sliderW = extent;
    if(horiz){
        subPageR.setRect(1, 0, sliderStart - 1 , sliderW);
	addPageR.setRect(sliderEnd, 0, subX - sliderEnd, sliderW);
	sliderR.setRect(sliderStart, 0, sliderLength, sliderW);
    }
    else{
        subPageR.setRect(0, 1, sliderW, sliderStart - 1);
	addPageR.setRect(0, sliderEnd, sliderW, subY - sliderEnd);
	sliderR.setRect(0, sliderStart, sliderW, sliderLength);
    }

    bool maxedOut = (sb->maxValue() == sb->minValue());
    if(controls & AddLine){
        drawBevelButton(p, addB.x(), addB.y(), addB.width(), addB.height(), g,
                        activeControl == AddLine, &g.brush(QColorGroup::Background));
        p->setPen(g.shadow());
        p->drawRect(addB);
        drawArrow(p, horiz ? RightArrow : DownArrow, false, addB.x()+2,
                  addB.y()+2,  addB.width()-4, addB.height()-4, g,
                  !maxedOut, activeControl == AddLine ?
                  &g.brush( QColorGroup::Mid) : &g.brush(QColorGroup::Background));
    }
    if(controls & SubLine){
        drawBevelButton(p, subB.x(), subB.y(), subB.width(), subB.height(), g,
                        activeControl == SubLine, &g.brush(QColorGroup::Background));
        p->setPen(g.shadow());
	p->drawRect(subB);
        drawArrow(p, horiz ? LeftArrow : UpArrow, false, subB.x()+2,
                  subB.y()+2, subB.width()-4, subB.height()-4, g, !maxedOut,
                  activeControl == SubLine ? &g.brush(QColorGroup::Mid) :
                  &g.brush(QColorGroup::Background));
    }
    if(controls & SubPage)
        drawScrollBarBackground(p, subPageR.x(), subPageR.y(), subPageR.width(),
                                subPageR.height(), g, horiz );
    if(controls & AddPage)
        drawScrollBarBackground(p, addPageR.x(), addPageR.y(), addPageR.width(),
                                addPageR.height(), g, horiz );
    if(controls & Slider){
        if(sliderR.width() > 4 && sliderR.height() > 4){
            p->setPen(Qt::black);
            p->drawRect(sliderR);
            qDrawShadeRect(p, sliderR.x()+1, sliderR.y()+1, sliderR.width()-2,
                           sliderR.height()-2, g, false, 1, 0);
            if(horiz)
                drawVGradient(p, g.brush(QColorGroup::Mid), sliderR.x()+2,
                              sliderR.y()+2, sliderR.width()-4,
                              sliderR.height()-4);
            else
                drawHGradient(p, g.brush(QColorGroup::Mid), sliderR.x()+2,
                              sliderR.y()+2, sliderR.width()-4,
                              sliderR.height()-4);
        }
        else{
            drawBevelButton(p, sliderR.x(), sliderR.y(), sliderR.width(),
                            sliderR.height(), g, false,
                            &g.brush(QColorGroup::Background));
        }

        if(sliderR.width() >= 14 && sliderR.height() >= 14){
            int x = sliderR.x() + (sliderR.width()-8)/2;
            int y = sliderR.y() + (sliderR.height()-8)/2;
            drawLightShadeRect(p, x, y, 8, 8, g, activeControl == Slider ?
                               &g.brush(QColorGroup::Midlight) :
                               &g.brush(QColorGroup::Background));
        }
    }
}

QSize PillBoxStyle::indicatorSize() const
{
    return(QSize(16, 16));
}

void PillBoxStyle::drawIndicator(QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g, int state, bool down,
                                bool)
{
    if (checkOutline.isNull())
    {
        checkOutline = QBitmap(10, 12, checkoutline_bits, true);
        checkFill = QBitmap(10, 12, checkfill_bits, true);
        checkFill.setMask(checkFill);
        checkOutline.setMask(checkOutline);
    }
    drawButton(p, x, y, w, h, g, down);
    if(state != QButton::Off){
        p->setPen(g.dark());
        p->drawPixmap(4, 2, checkFill);
        p->setPen(Qt::black);
        p->drawPixmap(4, 2, checkOutline);
    }
}

QSize PillBoxStyle::exclusiveIndicatorSize() const
{
    return(QSize(15,15));
}

void PillBoxStyle::drawExclusiveIndicator(QPainter *p, int x, int y, int w,
                                         int h, const QColorGroup &g, bool on,
                                         bool down, bool)
{
    static const QCOORD circle_dark[] = {5,1, 6,1, 7,1, 8,1, 9,1, 10,1,
    3,2, 4,2,
    2,3,
    2,4,
    1,5,
    1,6,
    1,7,
    1,8,
    1,9,
    2,10
    };

    static const QCOORD circle_mid[] = {5,0, 6,0, 7,0, 8,0, 9,0,
    3,1, 4,1, 11,1,
    2,2, 10,2, 12,2,
    1,3, 3,3,
    1,4,
    0,5,
    0,6,
    0,7,
    0,8,
    0,9,
    1,10,
    1,11,
    2,12
    };

    static const QCOORD circle_light[] = {14,5,
    14,6,
    14,7,
    14,8,
    14,9,
    13,10,
    13,11,
    12,12,
    4,13, 5,13, 10,13, 11,13,
    5,14, 6,14, 7,14, 8,14, 9,14
    };

    static const QCOORD fill_lines[] = {6,3, 9,3, 3,6, 3,9, 12,6, 12,9, 6,12, 9,12};

    QPen oldPen = p->pen();
    p->fillRect( x, y, w, h, g.brush(QColorGroup::Background));
    QPointArray a( QCOORDARRLEN(circle_dark), circle_dark );
    a.translate( x, y );
    p->setPen(Qt::black);
    p->drawPoints( a );
    a.setPoints( QCOORDARRLEN(circle_mid), circle_mid );
    a.translate( x, y );
    p->setPen(g.mid());
    p->drawPoints( a );
    a.setPoints( QCOORDARRLEN(circle_light), circle_light );
    a.translate( x, y );
    p->setPen(g.light() );
    p->drawPoints( a );

    if(on || down){
        p->setPen(down ? g.mid() : Qt::black);
        a.setPoints(QCOORDARRLEN(fill_lines), fill_lines);
        a.translate(x,y);
        p->drawLineSegments(a);
        p->fillRect(4, 4, 8, 8, (down)? g.brush(QColorGroup::Mid) :
                    QBrush(Qt::black, SolidPattern));
    }
    p->setPen(oldPen);
}

void PillBoxStyle::drawSliderGroove(QPainter *p, int x, int y, int w, int h,
                                    const QColorGroup &g, QCOORD,
                                    Orientation orient)
{
    p->setPen(Qt::black);
    p->drawRect(x, y, w, h);
    qDrawShadeRect(p, x+1, y+1, w-2, h-2, g, true, 1, 0);
    if(orient == Horizontal)
        drawVGradient(p, g.brush(QColorGroup::Mid), x+2, y+2, w-4, h-4);
    else
        drawHGradient(p, g.brush(QColorGroup::Mid), x+2, y+2, w-4, h-4);
}

int PillBoxStyle::sliderLength() const
{
    return(20);
}

void PillBoxStyle::drawSlider(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, Orientation orient,
                            bool, bool)
{
    p->setPen(Qt::black);
    p->drawRect(x+1, y+1, w-2, h-2);
    qDrawShadeRect(p, x+2, y+2, w-4, h-4, g, false, 1, 0);

    if(orient == Horizontal){
        int mid = x+w/2;
        drawVGradient(p, g.brush(QColorGroup::Mid), x+3, y+3, w-6, h-6);
        qDrawShadeLine(p, mid, y+3, mid, y+h-3, g, true, 1);
    }
    else{
        int mid = y+w/2;
        drawHGradient(p, g.brush(QColorGroup::Mid), x+3, y+3, w-6, h-6);
        qDrawShadeLine(p, x+3, mid, x+w-3, mid, g, true, 1);
    }
}

void PillBoxStyle::drawArrow(QPainter *p, Qt::ArrowType type, bool on, int x,
                            int y, int w, int h, const QColorGroup &g,
                            bool enabled, const QBrush *)
{
    if (up.isNull())
    {
        up = QBitmap(8, 8, up_bits, true);
        down = QBitmap(8, 8, down_bits, true);
        left = QBitmap(8, 8, left_bits, true);
        right = QBitmap(8, 8, right_bits, true);
        up.setMask(up);
        down.setMask(down);
        left.setMask(left);
        right.setMask(right);
    }
    
    p->setPen(enabled ? on ? g.light() : Qt::black : g.mid());
    if(w > 8){
        x = x + (w-8)/2;
        y = y + (h-8)/2;
    }
    switch(type){
    case Qt::UpArrow:
        p->drawPixmap(x, y, up);
        break;
    case Qt::DownArrow:
        p->drawPixmap(x, y, down);
        break;
    case Qt::LeftArrow:
        p->drawPixmap(x, y, left);
        break;
    default:
        p->drawPixmap(x, y, right);
        break;
    }
}

void PillBoxStyle::drawPopupMenuItem( QPainter* p, bool checkable, int maxpmw,
                                     int tab, QMenuItem* mi,
                                     const QPalette& pal, bool act,
                                     bool enabled, int x, int y, int w, int h)
{
static const int motifItemFrame         = 2;
static const int motifItemHMargin       = 3;
static const int motifItemVMargin       = 2;
static const int motifArrowHMargin      = 6;
static const int windowsRightBorder     = 12;

    if(act){
        bool dis = !enabled;
        QColorGroup itemg = dis ? pal.disabled() : pal.active();

        if (checkable)
            maxpmw = QMAX( maxpmw, 12 );
        int checkcol = maxpmw;

        int x2 = x+w-1;
        int y2 = y+h-1;
        p->setPen(itemg.light());
        p->drawLine(x, y, x2, y);
        p->drawLine(x, y, x, y2);
        p->setPen(itemg.dark());
        p->drawLine(x2, y, x2, y2);
        p->drawLine(x, y2, x2, y2);
        p->fillRect(x+1, y+1, w-2, h-2, itemg.mid());
        if ( mi->iconSet() ) {
            QIconSet::Mode mode = dis? QIconSet::Disabled : QIconSet::Normal;
            if (!dis)
                mode = QIconSet::Active;
            QPixmap pixmap = mi->iconSet()->pixmap(QIconSet::Small, mode);
            int pixw = pixmap.width();
            int pixh = pixmap.height();
            QRect cr(x, y, checkcol, h);
            QRect pmr(0, 0, pixw, pixh);
            pmr.moveCenter( cr.center() );
            p->setPen(itemg.text());
            p->drawPixmap(pmr.topLeft(), pixmap );

        }
        else if(checkable) {
            int mw = checkcol + motifItemFrame;
            int mh = h - 2*motifItemFrame;
            if (mi->isChecked()){
                drawCheckMark( p, x + motifItemFrame,
                               y+motifItemFrame, mw, mh, itemg, act, dis );
            }
        }
        p->setPen(itemg.light());
        QColor discol;
        if (dis) {
            discol = itemg.text();
            p->setPen(discol);
        }
        int xm = motifItemFrame + checkcol + motifItemHMargin;
        QString s = mi->text();
        if (!s.isNull()) {
            int t = s.find( '\t' );
            int m = motifItemVMargin;
            const int text_flags = AlignVCenter|ShowPrefix | DontClip | SingleLine;
            if (t >= 0) {
                p->drawText(x+w-tab-windowsRightBorder-motifItemHMargin-motifItemFrame,
                            y+m, tab, h-2*m, text_flags, s.mid( t+1 ));
            }
            p->drawText(x+xm, y+m, w-xm-tab+1, h-2*m, text_flags, s, t);
        } else if (mi->pixmap()) {
            QPixmap *pixmap = mi->pixmap();
            if (pixmap->depth() == 1)
                p->setBackgroundMode(OpaqueMode);
            p->drawPixmap( x+xm, y+motifItemFrame, *pixmap);
            if (pixmap->depth() == 1)
                p->setBackgroundMode(TransparentMode);
        }
        if (mi->popup()) {
            int dim = (h-2*motifItemFrame) / 2;
            if (!dis)
                discol = itemg.text();
            QColorGroup g2(discol, itemg.highlight(),
                           white, white,
                           dis ? discol : white,
                           discol, white);
            drawArrow(p, RightArrow, true,
                      x+w - motifArrowHMargin - motifItemFrame - dim,  y+h/2-dim/2,
                      dim, dim, itemg, TRUE);
        }
    }
    else
        KStyle::drawPopupMenuItem(p, checkable, maxpmw, tab, mi, pal, act,
                                  enabled, x, y, w, h);
}


void PillBoxStyle::drawFocusRect(QPainter *p, const QRect &r,
                                 const QColorGroup &g, const QColor *bg, bool f)
{
    QWindowsStyle::drawFocusRect(p, r, g, bg, f);
}

// no check here, make sure your highcolor beforehand ;-)
void PillBoxStyle::drawVGradient(QPainter *p, const QBrush &fill, int x, int y,
                                 int w, int h)
{
    if(h <= 24){
        p->drawTiledPixmap(x, y, w, h, *vSmall);
    }
    else if(h <= 34){
        p->drawTiledPixmap(x, y, w, h, *vMed);
    }
    else if(h <= 64){
        p->drawTiledPixmap(x, y, w, h, *vLarge);
    }
    else{
        p->fillRect(x, y+vLarge->height(), w, h-vLarge->height(), fill);
        p->drawTiledPixmap(x, y, w, vLarge->height(), *vLarge);
    }
}

void PillBoxStyle::drawHGradient(QPainter *p, const QBrush &fill, int x, int y,
                                 int w, int h)
{
    if(w <= 24){
        p->drawTiledPixmap(x, y, w, h, *hSmall);
    }
    else if(w <= 34){
        p->drawTiledPixmap(x, y, w, h, *hMed);
    }
    else if(w <= 64){
        p->drawTiledPixmap(x, y, w, h, *hLarge);
    }
    else{
        p->fillRect(x+hLarge->width(), y, w-hLarge->width(), h, fill);
        p->drawTiledPixmap(x, y, hLarge->width(), h, *hLarge);
    }
}

void PillBoxStyle::makeWallpaper(QPixmap &dest, const QColor &base)
{
    if (paper1.isNull())
    {
        paper1  = QBitmap(45, 45, paper_1_bits, true);
        paper2 = QBitmap(45, 45, paper_2_bits, true);
        paper3 = QBitmap(45, 45, paper_3_bits, true);
        paper1.setMask(paper1);
        paper2.setMask(paper2);
        paper3.setMask(paper3);
    }
    dest.resize(45, 45);
    dest.fill(base);
    QPainter p;
    p.begin(&dest);
    p.setPen(base.dark(104));
    p.drawPixmap(0, 0, paper1);
    p.setPen(base.light(104));
    p.drawPixmap(0, 0, paper2);
    p.setPen(base.light(106));
    p.drawPixmap(0, 0, paper3);
    p.end();

}


