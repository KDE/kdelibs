/*-
 * HCStyle (C) 2000 Daniel M. Duley  <mosfet@kde.org>
 *         (C) 2000 Dirk Mueller     <mueller@kde.org>
 *         (C) 2001 Martijn Klingens <mklingens@yahoo.com>
 *         (C) 2001 Roberto Teixeira <maragato@kde.org>
 *
 * Animated menu code based on code by Mario Weilguni <mweilguni@kde.org>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */


#ifndef INCLUDE_MENUITEM_DEF
#define INCLUDE_MENUITEM_DEF
#endif

#include <qmenudata.h>
#include "hcstyle.h"
#include <kapp.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kdrawutil.h>
#include <kglobalsettings.h>
#include <kpixmapeffect.h>
#include <qpalette.h>
#include <qbitmap.h>
#include <qtabbar.h>
#include <qpopupmenu.h>
#include <qobjectlist.h>
#include <qimage.h>
#include <qtimer.h>
#include <kimageeffect.h>
#include <ktoolbar.h>
#include <qdrawutil.h>
#include <unistd.h>
#include <klocale.h>
#include <kiconloader.h>

#include "bitmaps.h"

GradientSet::GradientSet(const QColor &baseColor)
{
    c = baseColor;

    int i;
    for(i=0; i < 5; ++i)
        gradients[i] = NULL;

}

KPixmap * GradientSet::gradient(GradientType type)
{
    if(gradients[type])
        return(gradients[type]);

    switch(type){
    case VSmall:
        gradients[VSmall] = new KPixmap;
        gradients[VSmall]->resize(18, 24);
        KPixmapEffect::gradient(*gradients[VSmall], c.light(110), c.dark(110),
                                KPixmapEffect::VerticalGradient);
        break;
    case VMed:
        gradients[VMed] = new KPixmap;
        gradients[VMed]->resize(18, 34);
        KPixmapEffect::gradient(*gradients[VMed], c.light(110), c.dark(110),
                                KPixmapEffect::VerticalGradient);
        break;
    case VLarge:
        gradients[VLarge] = new KPixmap;
        gradients[VLarge]->resize(18, 64);
        KPixmapEffect::gradient(*gradients[VLarge], c.light(110), c.dark(110),
                                KPixmapEffect::VerticalGradient);
        break;
    case HMed:
        gradients[HMed] = new KPixmap;
        gradients[HMed]->resize(34, 18);
        KPixmapEffect::gradient(*gradients[HMed], c.light(110), c.dark(110),
                                KPixmapEffect::HorizontalGradient);
        break;
    case HLarge:
        gradients[HLarge] = new KPixmap;
        gradients[HLarge]->resize(52, 18);
        KPixmapEffect::gradient(*gradients[HLarge], c.light(110), c.dark(110),
                                KPixmapEffect::HorizontalGradient);
        break;
    default:
        break;
    }
    return(gradients[type]);
}


GradientSet::~GradientSet()
{
    int i;
    for(i=0; i < 5; ++i){
        if(gradients[i])
            delete gradients[i];
    }
}


HCAniMenu::HCAniMenu(QPopupMenu *menu)
  : QObject(menu), mnu(menu), widget(0L)
{
  KConfig *config = KGlobal::config();
  QString oldGrp = config->group();
  config->setGroup("B2");
  animationDelay = config->readNumEntry("MenuAnimationDelay", 3);
  config->setGroup(oldGrp);

  menu->installEventFilter(this);

  // We should be cleaned up when the menu is destroyed..
//     connect(mnu, SIGNAL(destroyed()), this, SLOT(slotFinished()));
}

HCAniMenu::~HCAniMenu()
{
    if(widget)
        delete widget;
}

bool HCAniMenu::eventFilter(QObject *, QEvent *ev)
{
  if(ev->type() == QEvent::Show)
    scrollIn();
  else if(ev->type() == QEvent::Hide){
    scrollOut();
  }
  return(false);
}

void HCAniMenu::scrollIn()
{
  QTime t;
  int w = mnu->width();
  int h = mnu->height();
  int steps = QMIN(w, h) / 10;

  bgPix = QPixmap::grabWindow(QApplication::desktop()->winId(),
				    mnu->x(), mnu->y(), w, h);

  mnuPix.resize(w, h);
  mnuPix.fill(mnu->colorGroup().color(QColorGroup::Background));
  QPainter::redirect(mnu, &mnuPix);
  mnu->repaint(false);
  QPainter::redirect(mnu, 0);
  if(!widget)
    widget = new QWidget(0, 0, WStyle_Customize | WStyle_NoBorder |
			 WStyle_Tool | WType_Popup );
  widget->setFocusPolicy(QWidget::StrongFocus);
  widget->move(mnu->x(), mnu->y());
  widget->resize(w, h);
  widget->setBackgroundMode(QWidget::NoBackground);
  widget->show();
  bitBlt(widget, 0, 0, &bgPix);

  for(int x = 1; x <= steps; x++)
    {
      t.start();
      while(t.elapsed() <= animationDelay );

      int howMuch = (int)(float(x) / float(steps) * w);

      bitBlt(widget, 0, 0, &mnuPix, w - howMuch, 0, howMuch, h);
      kapp->syncX();
    }
  QTimer::singleShot(1, this, SLOT(slotDestroyFake()));
}

void HCAniMenu::scrollOut()
{
  QTime t;
  int w = mnu->width();
  int h = mnu->height();
  int steps = QMIN(w, h) / 10;

  if(!widget)
    widget = new QWidget(0, 0, WStyle_Customize | WStyle_NoBorder |
			 WStyle_Tool | WType_Popup );
  widget->move(mnu->x(), mnu->y());
  widget->resize(w, h);
  widget->setBackgroundMode(QWidget::NoBackground);
  widget->show();
  bitBlt(widget, 0, 0, &mnuPix);

  for(int x = steps; x >= 0; x--)
    {
      t.start();
      while(t.elapsed() <= animationDelay );

      int howMuch = (int)(float(x) / float(steps) * w);

      bitBlt(widget, 0 + howMuch, 0, &bgPix, 0 + howMuch, 0, w - howMuch, h);
      bitBlt(widget, 0, 0, &mnuPix, w - howMuch, 0, howMuch, h);
      kapp->syncX();
    }
  QTimer::singleShot(1, this, SLOT(slotDestroyFake()));
}

void HCAniMenu::slotDestroyFake()
{
    delete(widget);
    widget = NULL;
}

void HCAniMenu::slotFinished()
{
    delete(widget);
    // We'll be deleted when the menu is destroyed..
}


HCStyle::HCStyle()
    :KStyle()
{
    highlightWidget = 0L;
    setButtonDefaultIndicatorWidth(0);
    gDict.setAutoDelete(true);
    highcolor = QPixmap::defaultDepth() > 8;
}

HCStyle::~HCStyle()
{
    ;
}

void HCStyle::polish(QPalette &)
{
    KConfig *config = KGlobal::config();
    QString oldGrp = config->group();
    config->setGroup("B2");
    QPalette pal = QApplication::palette();

    // if the config has specific colors for items set use those and don't
    // worry about high color usage, otherwise use KDE standard colors for
    // everything except one color - the slider groove fill. That doesn't
    // really look good with any of the standard colors and one additional
    // color alloc shouldn't kill psudeocolor display users :P
    QColor tmpColor(0, 0, 192);
    if(config->hasKey("RadioOnColor")){
        tmpColor = config->readColorEntry("RadioOnColor", &tmpColor);
        radioOnGrp.setColor(QColorGroup::Mid, tmpColor);
        radioOnGrp.setColor(QColorGroup::Button, tmpColor);
        radioOnGrp.setColor(QColorGroup::Light, tmpColor.light(130));
        radioOnGrp.setColor(QColorGroup::Dark, tmpColor.dark(130));
    }
    else{
        radioOnGrp.setColor(QColorGroup::Mid, tmpColor);
        radioOnGrp.setColor(QColorGroup::Button, tmpColor);
        radioOnGrp.setColor(QColorGroup::Light, QColor(0, 0, 255));
        radioOnGrp.setColor(QColorGroup::Dark, QColor(0, 0, 128));
    }
    config->setGroup(oldGrp);
    gDict.clear();
}

void HCStyle::polish(QWidget *w)
{
    if(qstrcmp(w->name(), "qt_viewport") == 0 ||
       w->testWFlags(WType_Popup) || w->inherits("KDesktop")
       || w->inherits("PanelButtonBase") || w->isTopLevel())
        return;

    if(w->inherits("QPushButton"))
        w->installEventFilter(this);

    if(w->inherits("QMenuBar") || w->inherits("KToolBarButton")){
        w->setBackgroundMode(QWidget::NoBackground);
        return;
    }

    if(w->parent() && w->parent()->inherits("KToolBar"))
    {
        if(w->inherits("QLabel") || w->inherits("KToolBarSeparator")){
            w->setAutoMask(true);
        }
    }
    else if( w->inherits("QButton") || w->inherits("QComboBox"))
        w->setBackgroundOrigin(QWidget::ParentOrigin);

    if(w->inherits("KToolBar")){
        w->installEventFilter(this);
        w->setBackgroundMode(QWidget::NoBackground);
        return;
    }
}

void HCStyle::unPolish(QWidget *w)
{

    if(qstrcmp(w->name(), "qt_viewport") == 0 ||
       w->testWFlags(WType_Popup) || w->inherits("KDesktop")
       || w->inherits("PanelButtonBase") || w->isTopLevel())
        return;

    w->setBackgroundMode(QWidget::PaletteBackground);

    if(w->inherits("QPushButton")){
        w->removeEventFilter(this);
    }
    if(w->inherits("QMenuBar") || w->inherits("KToolBarButton")){
        w->setBackgroundMode(QWidget::PaletteBackground);
        return;
    }
    if(w->parent() && w->parent()->inherits("KToolBar"))
    {
        if(w->inherits("QLabel") || w->inherits("KToolBarSeparator"))
            w->setAutoMask(false);
    }
    else if( w->inherits("QButton") || w->inherits("QComboBox"))
        w->setBackgroundOrigin(QWidget::WidgetOrigin);

    if(w->inherits("KToolBar")){
        w->removeEventFilter(this);
        w->setBackgroundMode(QWidget::PaletteBackground);
        return;
    }
}


bool HCStyle::eventFilter(QObject *obj, QEvent *ev)
{
    if(obj->inherits("KToolBar")){
        if(ev->type() == QEvent::Resize){
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
    }
    else if(obj->inherits("QPushButton")){
        if(ev->type() == QEvent::Enter){
            QWidget *btn = (QWidget *)obj;
            if (btn->isEnabled()){
                highlightWidget = btn;
                btn->repaint(false);
            }
        }
        else if(ev->type() == QEvent::Leave){
            QWidget *btn = (QWidget *)obj;
            if (btn == highlightWidget)
                highlightWidget = 0L;
            btn->repaint(false);
        }
    }
    return(false);
}

void HCStyle::drawButton(QPainter *p, int x, int y, int w, int h,
                         const QColorGroup &g, bool sunken,
                         const QBrush *)
{
    kDrawBeButton(p, x, y, w, h, g, sunken, &g.brush(QColorGroup::Midlight));
#if 0
    if(g.highlight() != Qt::black){
        kDrawBeButton(p, x, y, w, h, g, sunken,
                      &g.brush(QColorGroup::Midlight));
    }
    else if(highcolor){
        int x2 = x+w-1;
        int y2 = y+h-1;
        p->setPen(g.dark());
        p->drawLine(x+1, y, x2-1, y);
        p->drawLine(x+1, y2, x2-1, y2);
        p->drawLine(x, y+1, x, y2-1);
        p->drawLine(x2, y+1, x2, y2-1);

        if(!sunken){
            p->setPen(g.light());
            p->drawLine(x+2, y+2, x2-1, y+2);
            p->drawLine(x+2, y+3, x2-2, y+3);
            p->drawLine(x+2, y+4, x+2, y2-1);
            p->drawLine(x+3, y+4, x+3, y2-2);
        }
        else{
            p->setPen(g.mid());
            p->drawLine(x+2, y+2, x2-1, y+2);
            p->drawLine(x+2, y+3, x2-2, y+3);
            p->drawLine(x+2, y+4, x+2, y2-1);
            p->drawLine(x+3, y+4, x+3, y2-2);
        }
        p->setPen(sunken? g.light() : g.mid());
        p->drawLine(x2-1, y+2, x2-1, y2-1);
        p->drawLine(x+2, y2-1, x2-1, y2-1);

        p->setPen(g.mid());
        p->drawLine(x+1, y+1, x2-1, y+1);
        p->drawLine(x+1, y+2, x+1, y2-1);
        p->drawLine(x2-2, y+3, x2-2, y2-2);

        drawVGradient(p, g, QColorGroup::Button, x+4, y+4, w-6, h-6);
    }
    else
        kDrawBeButton(p, x, y, w, h, g, sunken, fill);
#endif
}

void HCStyle::drawPushButton(QPushButton *btn, QPainter *p)
{
    QRect r = btn->rect();
    bool sunken = btn->isOn() || btn->isDown();
    QColorGroup g = btn->colorGroup();
    int x = r.x(), y = r.y(), w = r.width(), h = r.height();

    if(sunken)
        kDrawBeButton(p, x, y, w, h, g, true,
                      &g.brush(QColorGroup::Mid));
    else {
	if (btn != highlightWidget) {
	    // -----
	    if(highcolor){
		int x2 = x+w-1;
		int y2 = y+h-1;
		p->setPen(g.dark());
		p->drawLine(x+1, y, x2-1, y);
		p->drawLine(x+1, y2, x2-1, y2);
		p->drawLine(x, y+1, x, y2-1);
		p->drawLine(x2, y+1, x2, y2-1);

		if(!sunken){
		    p->setPen(g.light());
		    p->drawLine(x+2, y+2, x2-1, y+2);
		    p->drawLine(x+2, y+3, x2-2, y+3);
		    p->drawLine(x+2, y+4, x+2, y2-1);
		    p->drawLine(x+3, y+4, x+3, y2-2);
		}
		else{
		    p->setPen(g.mid());
		    p->drawLine(x+2, y+2, x2-1, y+2);
		    p->drawLine(x+2, y+3, x2-2, y+3);
		    p->drawLine(x+2, y+4, x+2, y2-1);
		    p->drawLine(x+3, y+4, x+3, y2-2);
		}
		p->setPen(sunken? g.light() : g.mid());
		p->drawLine(x2-1, y+2, x2-1, y2-1);
		p->drawLine(x+2, y2-1, x2-1, y2-1);

		p->setPen(g.mid());
		p->drawLine(x+1, y+1, x2-1, y+1);
		p->drawLine(x+1, y+2, x+1, y2-1);
		p->drawLine(x2-2, y+3, x2-2, y2-2);

		drawVGradient(p, g, QColorGroup::Button, x+4, y+4, w-6, h-6, 0, 0, w-6, h-6);
	    }
	    else
		drawButton(p, x, y, w, h, g, false);
	}
	else
	    drawButton(p, x, y, w, h, g, false);
    }

    if(btn->isDefault()){
        p->setPen(Qt::black);
        p->drawLine(r.x()+1, r.y(), r.right()-1, r.y());
        p->drawLine(r.x()+1, r.bottom(), r.right()-1, r.bottom());
        p->drawLine(r.x(), r.y()+1, r.x(), r.bottom()-1);
        p->drawLine(r.right(), r.y()+1, r.right(), r.bottom()-1);
    }

}

void HCStyle::drawPushButtonLabel(QPushButton *btn, QPainter *p)
{
    int x1, y1, x2, y2, w, h;
    btn->rect().coords(&x1, &y1, &x2, &y2);
    w = btn->width();
    h = btn->height();

    bool act = btn->isOn() || btn->isDown();
    if(act){
        ++x1, ++y1;
    }

    // If this is a button with an associated popup menu, draw an arrow first
    if ( btn->popup() )
    {
	int dx = menuButtonIndicatorWidth( btn->height() );

	QColorGroup g( btn->colorGroup() );
	int xx = x1 + w - dx - 4;
	int yy = y1 - 3;
	int hh = h + 6;

	if ( !act )
	{
	    p->setPen( g.light() );
	    p->drawLine( xx + 1, yy + 5, xx + 1, yy + hh - 6 );
	    p->setPen( g.mid() );
	    p->drawLine( xx, yy + 6, xx, yy + hh - 6 );
	}
	else
	{
	    p->setPen( g.button() );
	    p->drawLine( xx, yy + 4, xx, yy + hh - 4 );
	}
	drawArrow( p, DownArrow, FALSE,
		   x1 + w - dx - 2, y1 + 2, dx, h - 4,
		   btn->colorGroup(),
		   btn->isEnabled() );
	w -= dx;
    }

    // Next, draw iconset, if any
    if ( btn->iconSet() && !btn->iconSet()->isNull() )
    {
	QIconSet::Mode mode = btn->isEnabled()
			      ? QIconSet::Normal : QIconSet::Disabled;
	if ( mode == QIconSet::Normal && btn->hasFocus() )
	    mode = QIconSet::Active;
	QPixmap pixmap = btn->iconSet()->pixmap( QIconSet::Small, mode );
	int pixw = pixmap.width();
	int pixh = pixmap.height();

	p->drawPixmap( x1+6, y1+h/2-pixh/2, pixmap );
	x1 += pixw + 8;
	w -= pixw + 8;
    }

    if(act || btn->isDefault()){
        QFont font = btn->font();
        font.setBold(true);
        p->setFont(font);
        drawItem( p, x1+1, y1+1, w, h,
                 AlignCenter | ShowPrefix, btn->colorGroup(), btn->isEnabled(),
                 btn->pixmap(), btn->text(), -1, act ?
                 &btn->colorGroup().dark() : &btn->colorGroup().mid());

        drawItem( p, x1, y1, w, h,
                 AlignCenter | ShowPrefix, btn->colorGroup(), btn->isEnabled(),
                 btn->pixmap(), btn->text(), -1, act ?
                 &btn->colorGroup().light() : &btn->colorGroup().text());
    }
    else{
        drawItem( p, x1 + ( act ? 1 : 0 ), y1 + ( act ? 1 : 0 ), w, h,
                  AlignCenter | ShowPrefix,
		  btn->colorGroup(), btn->isEnabled(),
                  btn->pixmap(), btn->text(), -1,
                  act ? &btn->colorGroup().light()
		      : &btn->colorGroup().buttonText());
    }
}

void HCStyle::drawButtonMask(QPainter *p, int x, int y, int w, int h)
{
    int x2 = x+w-1;
    int y2 = y+h-1;
    p->fillRect(x, y, w, h, QBrush(color1, SolidPattern));
    p->setPen(color0);
    p->drawPoint(x, y);
    p->drawPoint(x2, y);
    p->drawPoint(x, y2);
    p->drawPoint(x2, y2);
}

void HCStyle::drawBevelButton(QPainter *p, int x, int y, int w, int h,
                              const QColorGroup &g, bool sunken,
                              const QBrush *fill)
{
    // I like black instead of dark() for the darkest shade
    int x2 = x+w-1;
    int y2 = y+h-1;
    p->setPen(sunken ? Qt::black : g.light());
    p->drawLine(x, y, x2, y);
    p->drawLine(x, y, x, y2);
    p->setPen(sunken ? g.light() : Qt::black);
    p->drawLine(x, y2, x2, y2);
    p->drawLine(x2, y, x2, y2);

    p->setPen(sunken ? g.dark() : g.midlight());
    p->drawLine(x+1, y+1, x2-1, y+1);
    p->drawLine(x+1, y+1, x+1, y2-1);
    p->setPen(sunken ? g.midlight() : g.dark());
    p->drawLine(x+1, y2-1, x2-1, y2-1);
    p->drawLine(x2-1, y+1, x2-1, y2-1);
    if(highcolor)
        drawVGradient(p, g, QColorGroup::Button, x+2, y+2, w-4, h-4, 0, 0, w-4, h-4);
    else
        p->fillRect(x+2, y+2, w-4, h-4, fill ? *fill :
                    g.brush(QColorGroup::Button));

}

QRect HCStyle::buttonRect(int x, int y, int w, int h)
{
    return(QRect(x+4, y+4, w-6, h-6));
}

void HCStyle::drawComboButton(QPainter *p, int x, int y, int w, int h,
                                 const QColorGroup &g, bool sunken,
                                 bool edit, bool, const QBrush *)
{
    int x2 = x+w-1;
    int y2 = y+h-1;

    p->setPen(g.dark());
    p->drawLine(x+1, y, x2-1, y);
    p->drawLine(x+1, y2, x2-1, y2);
    p->drawLine(x, y+1, x, y2-1);
    p->drawLine(x2, y+1, x2, y2-1);

    if(highcolor)
        drawVGradient(p, g, QColorGroup::Background, x+2, y+2, w-4, h-4, 0, 0, w-4, h-4);
    else
        p->fillRect(x+2, y+2, w-4, h-4, g.brush(QColorGroup::Background));

    p->setPen(sunken? g.light() : g.mid());
    p->drawLine(x2-1, y+2, x2-1, y2-1);
    p->drawLine(x+1, y2-1, x2-1, y2-1);

    p->setPen(sunken? g.mid() : g.light());
    p->drawLine(x+1, y+1, x2-1, y+1);
    p->drawLine(x+1, y+2, x+1, y2-2);

    p->setPen(g.dark());
    p->drawPoint(x+1, y+1);

    if(edit){
        x2 = x2-17;
        y2 = y2-3;
        p->setPen(g.dark());
        //p->drawRect(x+2, y+2, x2, y2);
        p->drawLine(x+3, y+2, x2, y+2);
        p->drawLine(x+2, y+3, x+2, y2);
    }

    int arrow_h = h / 3;
    int arrow_w = arrow_h;
    int arrow_x = w - arrow_w - 6;
    int arrow_y = (h - arrow_h) / 2;
    drawArrow(p, DownArrow, false, arrow_x, arrow_y, arrow_w, arrow_h, g, true);
}

void HCStyle::drawComboButtonMask(QPainter *p, int x, int y, int w, int h)
{
    int x2 = x+w-1;
    int y2 = y+h-1;
    p->fillRect(x, y, w, h, QBrush(color1, SolidPattern));
    p->setPen(color0);
    p->drawPoint(x, y);
    p->drawPoint(x, y+1);
    p->drawPoint(x+1, y);
    p->drawPoint(x2, y);
    p->drawPoint(x, y2);
    p->drawPoint(x2, y2);
}

QRect HCStyle::comboButtonRect(int x, int y, int w, int h)
{
    return(QRect(x+3, y+3, w - (h / 3) - 13, h-6));
}

QRect HCStyle::comboButtonFocusRect(int x, int y, int w, int h)
{
    return(QRect(x+3, y+3, w-(h/3)-13, h-6));
}


void HCStyle::drawScrollBarControls(QPainter *p, const QScrollBar *sb,
                                       int sliderStart, uint controls,
                                       uint activeControl)
{
    int sliderMin, sliderMax, sliderLength, buttonDim;
    scrollBarMetrics( sb, sliderMin, sliderMax, sliderLength, buttonDim );

    if (sliderStart > sliderMax)
        sliderStart = sliderMax;

    bool	horiz = sb->orientation() == QScrollBar::Horizontal;

    QColorGroup	g = sb->colorGroup();
    QRect	addB, subHC, subB;
    QRect       addPageR, subPageR, sliderR;
    int		addX, addY, subX, subY;
    int		len, extent;
    uint	numButtons = 3;

    if ( horiz )
    {
	len    = sb->width();
	extent = sb->height();

        subY = addY = ( extent - buttonDim ) / 2;
        subX = 0;
        addX = len - buttonDim;
    }
    else
    {
	len    = sb->height();
	extent = sb->width();

        subX = addX = ( extent - buttonDim ) / 2;
        subY = 0;
        addY = len - buttonDim;
    }

    // Calc number of used buttons:
    if( len < buttonDim * 4 )
	numButtons = 2;

    subB.setRect( subX, subY, buttonDim, buttonDim );
    addB.setRect( addX, addY, buttonDim, buttonDim );
    if( horiz )
        subHC.setRect( addX - buttonDim, addY, buttonDim, buttonDim );
    else
        subHC.setRect( addX, addY - buttonDim, buttonDim, buttonDim );

    int sliderEnd = sliderStart + sliderLength;
    int sliderW = extent;

    // Calculate sizes of the actual scrollbar parts
    if ( horiz )
    {
        subPageR.setRect( subB.right() + 1, 0,
                          sliderStart - subB.right() - 1 , sliderW );
        addPageR.setRect( sliderEnd, 0, addX - sliderEnd -
			  ( ( numButtons == 3 ) ? buttonDim : 0 ), sliderW );
        sliderR .setRect( sliderStart, 0, sliderLength, sliderW );
    }
    else
    {
        subPageR.setRect( 0, subB.bottom() + 1, sliderW,
                          sliderStart - subB.bottom() - 1 );
	addPageR.setRect( 0, sliderEnd, sliderW, addY - sliderEnd -
			  ( ( numButtons == 3 ) ? buttonDim : 0 ) );
        sliderR .setRect( 0, sliderStart, sliderW, sliderLength );
    }

    bool maxed = sb->maxValue() == sb->minValue();

    if ( controls & AddLine )
    {
	// Draw scroll down arrow
        drawSBButton(p, addB, g, activeControl == AddLine);
        drawArrow( p, horiz ? RightArrow : DownArrow,
                   false, addB.x()+4, addB.y()+4,
                   addB.width()-8, addB.height()-8, g, !maxed);
    }

    if ( controls & SubLine )
    {
	// Draw scroll up buttons
	bool isSubLine = activeControl == SubLine;
	Qt::ArrowType arrowType = horiz ? LeftArrow : UpArrow;

        drawSBButton(p, subB, g, isSubLine);
        drawArrow( p, arrowType,
                   false, subB.x() + 4, subB.y() + 4,
                   subB.width() - 8, subB.height() - 8, g, !maxed );

	if( numButtons == 3 )
	{
            drawSBButton( p, subHC, g, isSubLine );
	    drawArrow( p, arrowType,
	               false, subHC.x() + 4, subHC.y() + 4,
		       subHC.width() - 8, subHC.height() - 8, g, !maxed );
	}
    }

    GradientSet *scrollBarBg = NULL;
    if( highcolor && ( controls & AddPage || controls & SubPage ) )
    {
        scrollBarBg = gDict.find( g.background().rgb() );
        if( !scrollBarBg )
	{
            scrollBarBg = new GradientSet( g.background() );
            gDict.insert( g.background().rgb(), scrollBarBg );
        }
    }

    if(controls & AddPage){
        if(addPageR.width()){
            p->setPen(g.dark());
	    // Draw vertical scrollbar part below the slider
            if(horiz){
                if(highcolor){
                    KPixmap *vMed = scrollBarBg->gradient(VMed);
                    p->drawTiledPixmap(addPageR.x(), addPageR.y()+1,
                                       addPageR.width(), addPageR.height()-2,
                                       *vMed, 0, activeControl==AddPage ? 0 :
                                       vMed->height()-(addPageR.height()-2));
                    p->drawLine(addPageR.x(), addPageR.y(), addPageR.right(),
                                addPageR.y());
                    p->drawLine(addPageR.x(), addPageR.bottom(), addPageR.right(),
                                addPageR.bottom());
                }
                else{
                    p->fillRect(addPageR, activeControl == AddPage ?
                                g.brush(QColorGroup::Midlight) :
                                g.brush(QColorGroup::Mid));
                    p->drawLine(addPageR.x(), addPageR.y(), addPageR.right(),
                                addPageR.y());
                    p->drawLine(addPageR.x(), addPageR.bottom(), addPageR.right(),
                                addPageR.bottom());
                    p->setPen(activeControl==AddPage ? g.mid() : g.background());
                    p->drawLine(addPageR.x(), addPageR.y()+1, addPageR.right(),
                                addPageR.y()+1);
                }
            }
            else{
                if(highcolor){
		    KPixmap *hMed = scrollBarBg->gradient(HMed);
                    p->drawTiledPixmap(addPageR.x()+1, addPageR.y(),
                                       addPageR.width()-2, addPageR.height(),
                                       *hMed, activeControl==AddPage ? 0 :
                                       hMed->width()-(addPageR.width()-2),
                                       0);
                    p->drawLine(addPageR.x(), addPageR.y(), addPageR.x(),
                                addPageR.bottom());
                    p->drawLine(addPageR.right(), addPageR.y(), addPageR.right(),
                                addPageR.bottom());
                }
                else{
                    p->fillRect(addPageR, activeControl == AddPage ?
                                g.brush(QColorGroup::Midlight) :
                                g.brush(QColorGroup::Mid));
                    p->drawLine(addPageR.x(), addPageR.y(), addPageR.x(),
                                addPageR.bottom());
                    p->drawLine(addPageR.right(), addPageR.y(), addPageR.right(),
                                addPageR.bottom());
                    p->setPen(activeControl==AddPage ? g.mid() : g.background());
                    p->drawLine(addPageR.x()+1, addPageR.y(), addPageR.x()+1,
                                addPageR.bottom());
                }
            }
        }
    }
    if(controls & SubPage){
        if(subPageR.height()){
            p->setPen(g.dark());
	    // Draw vertical scrollbar part above the slider
            if(horiz){
                if(highcolor){
                    KPixmap *vMed = scrollBarBg->gradient(VMed);
                    p->drawTiledPixmap(subPageR.x(), subPageR.y()+1,
                                       subPageR.width(), subPageR.height()-2,
                                       *vMed, 0, activeControl==SubPage ? 0 :
                                       vMed->height()-(subPageR.height()-2));
                    p->drawLine(subPageR.x(), subPageR.y(), subPageR.right(),
                                subPageR.y());
                    p->drawLine(subPageR.x(), subPageR.bottom(), subPageR.right(),
                                subPageR.bottom());
                }
                else{
                    p->fillRect(subPageR, activeControl == SubPage ?
                                g.brush(QColorGroup::Midlight) :
                                g.brush(QColorGroup::Mid));
                    p->drawLine(subPageR.x(), subPageR.y(), subPageR.right(),
                                subPageR.y());
                    p->drawLine(subPageR.x(), subPageR.bottom(), subPageR.right(),
                                subPageR.bottom());
                    p->setPen(activeControl==SubPage ? g.mid() : g.background());
                    p->drawLine(subPageR.x(), subPageR.y()+1, subPageR.right(),
                                subPageR.y()+1);
                    p->drawLine(subPageR.x(), subPageR.y()+1, subPageR.x(),
                                subPageR.bottom()-1);
                }
            }
            else{
                if(highcolor){
                    KPixmap *hMed = scrollBarBg->gradient(HMed);
                    p->drawTiledPixmap(subPageR.x()+1, subPageR.y(),
                                       subPageR.width()-2, subPageR.height(),
                                       *hMed, activeControl==SubPage ? 0 :
                                       hMed->width()-(subPageR.width()-2),
                                       0);
                    p->drawLine(subPageR.x(), subPageR.y(), subPageR.x(),
                                subPageR.bottom());
                    p->drawLine(subPageR.right(), subPageR.y(), subPageR.right(),
                                subPageR.bottom());
                }
                else{
                    p->fillRect(subPageR, activeControl == SubPage ?
                                g.brush(QColorGroup::Midlight) :
                                g.brush(QColorGroup::Mid));
                    p->drawLine(subPageR.x(), subPageR.y(), subPageR.x(),
                                subPageR.bottom());
                    p->drawLine(subPageR.right(), subPageR.y(), subPageR.right(),
                                subPageR.bottom());
                    p->setPen(activeControl==SubPage ? g.mid() : g.background());
                    p->drawLine(subPageR.x()+1, subPageR.y(), subPageR.x()+1,
                                subPageR.bottom());
                    p->drawLine(subPageR.x()+1, subPageR.y(), subPageR.right()-1,
                                subPageR.y());
                }
            }
        }
    }
    if ( controls & Slider )
    {
	// Draw slider, but only if the slider is at least 4 pixels wide
	// and 4 pixels heigh!
	if( sliderR.width() > 4 && sliderR.height() > 4 )
	{
	    drawSBButton(p, sliderR, g, activeControl == Slider, false);
	    g.setColor(QColorGroup::Dark, g.mid());
	    drawSBDeco(p, sliderR, g, horiz);
	}
    }
}

void HCStyle::drawSBButton(QPainter *p, const QRect &r, const QColorGroup &g,
                           bool down, bool /*fast*/)
{
    p->setPen(g.dark());
    p->drawRect(r);
    if(highcolor){
        if(0 /*fast*/){
            //p->drawPixmap(r.x()+1, r.y()+1, *vSmall, 0, 0, r.width()-2,
            //              r.height()-2);
        }
        else{
            if(r.width() > r.height())
                drawVGradient(p, g, QColorGroup::Button, r.x()+1, r.y()+1,
                              r.width()-2, r.height()-2, 0, 0, r.width()-2, r.height()-2);
            else
                drawHGradient(p, g, QColorGroup::Button, r.x()+1, r.y()+1,
                              r.width()-2, r.height()-2, 0, 0, r.width()-2, r.height()-2);
        }

    }
    else
        p->fillRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2,
                    g.brush(down ? QColorGroup::Midlight : QColorGroup::Button));

    p->setPen(down ? g.light() : g.mid());
    p->drawLine(r.x()+1, r.bottom()-1, r.right()-1, r.bottom()-1);
    p->drawLine(r.right()-1, r.top()+1, r.right()-1, r.bottom()-1);
    p->setPen(down ? g.mid() : g.light());
    p->drawLine(r.x()+1, r.y()+1, r.right()-1, r.y()+1);
    p->drawLine(r.x()+1, r.y()+1, r.x()+1, r.bottom()-1);
}

void HCStyle::drawSBDeco(QPainter *p, const QRect &r, const QColorGroup &g,
                         bool horiz)
{
    if(horiz){
        if(r.width() >= 15){
            int y = r.y()+3;
            int x = r.x() + (r.width()-7)/2;
            int y2 = r.bottom()-3;
            p->setPen(g.light());
            p->drawLine(x, y, x, y2);
            p->drawLine(x+3, y, x+3, y2);
            p->drawLine(x+6, y, x+6, y2);

            p->setPen(g.mid());
            p->drawLine(x+1, y, x+1, y2);
            p->drawLine(x+4, y, x+4, y2);
            p->drawLine(x+7, y, x+7, y2);
        }
    }
    else{
        if(r.height() >= 15){
            int x = r.x()+3;
            int y = r.y() + (r.height()-7)/2;
            int x2 = r.right()-3;
            p->setPen(g.light());
            p->drawLine(x, y, x2, y);
            p->drawLine(x, y+3, x2, y+3);
            p->drawLine(x, y+6, x2, y+6);

            p->setPen(g.mid());
            p->drawLine(x, y+1, x2, y+1);
            p->drawLine(x, y+4, x2, y+4);
            p->drawLine(x, y+7, x2, y+7);
        }
    }

}


void HCStyle::scrollBarMetrics(const QScrollBar *sb, int &sliderMin,
                                  int &sliderMax, int &sliderLength,
                                  int &buttonDim)
{

    int maxLength;
    bool horiz = sb->orientation() == QScrollBar::Horizontal;
    int length = horiz ? sb->width()  : sb->height();
    int extent = horiz ? sb->height() : sb->width();

    if( length > ( extent - 1 ) * 2 )
	buttonDim = extent;
    else
	buttonDim = length / 2 - 1;

    sliderMin = buttonDim;

    uint numButtons = 3;   // Two scroll up/left buttons when possible...
    if( length < buttonDim * 4 )	// ... but otherwise only two!
	numButtons = 2;
    maxLength = length - buttonDim * numButtons;

    if ( sb->maxValue() == sb->minValue() ) {
	sliderLength = maxLength;
    } else {
	sliderLength = (sb->pageStep()*maxLength)/
			(sb->maxValue()-sb->minValue()+sb->pageStep());
	uint range = sb->maxValue()-sb->minValue();
	if ( sliderLength < 9 || range > INT_MAX/2 )
	    sliderLength = 9;
	if ( sliderLength > maxLength )
	    sliderLength = maxLength;
    }
    sliderMax = sliderMin + maxLength - sliderLength;
}

QStyle::ScrollControl HCStyle::scrollBarPointOver(const QScrollBar *sb,
                                                     int sliderStart,
                                                     const QPoint &p)
{
    if ( !sb->rect().contains( p ) )
        return NoScroll;
    int sliderMin, sliderMax, sliderLength, buttonDim, pos;
    scrollBarMetrics( sb, sliderMin, sliderMax, sliderLength, buttonDim );

    // Calc number of used buttons:
    uint len, numButtons;
    if( sb->orientation() == QScrollBar::Horizontal )
	len    = sb->width();
    else
	len    = sb->height();
    if( len < buttonDim * 4 )
	numButtons = 2;
    else
	numButtons = 3;

    pos = (sb->orientation() == QScrollBar::Horizontal)? p.x() : p.y();
    if ( pos < sliderMin )
	return SubLine;
    if ( pos < sliderStart )
	return SubPage;
    if ( pos < sliderStart + sliderLength )
        return Slider;
    if ( pos < sliderMax + sliderLength)
        return AddPage;
    if( pos > sliderMax + sliderLength + 16 )
        return AddLine;
    if( numButtons == 2 && pos > sliderMax + sliderLength )
        return AddLine;

    return SubLine;
}

#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)


QSize HCStyle::exclusiveIndicatorSize() const
{
    return(QSize(13,13));
}

void HCStyle::drawExclusiveIndicator(QPainter *p, int x, int y, int w,
                                     int h, const QColorGroup &g, bool on,
                                     bool down, bool)
{
    if (lightBmp.isNull())
    {
       lightBmp = QBitmap(13, 13, radiooff_light_bits, true);
       grayBmp = QBitmap(13, 13, radiooff_gray_bits, true);
       dgrayBmp = QBitmap(13, 13, radiooff_dgray_bits, true);
    }

    p->fillRect(x, y, w, h, g.brush(QColorGroup::Background));
    kColorBitmaps(p, g, x, y, &lightBmp, &grayBmp,
                  NULL, &dgrayBmp);

    if(on || down){
        p->setPen(Qt::black);
        p->drawLine(5, 4, 7, 4);
        p->drawLine(4, 5, 4, 7);
        p->drawLine(5, 8, 7, 8);
        p->drawLine(8, 5, 8, 7);
        p->fillRect(5, 5, 3, 3, Qt::black);
    }

}

void HCStyle::drawExclusiveIndicatorMask(QPainter *p, int x, int y, int w,
                                         int h, bool)
{
    if (maskBmp.isNull())
    {
        maskBmp = QBitmap(13, 13, radiomask_bits, true);
    }
    p->fillRect(x, y, w, h, Qt::color0);
    p->setPen(Qt::color1);
    p->drawPixmap(x, y, maskBmp);
}


QSize HCStyle::indicatorSize() const
{
    return(QSize(13, 13));
}

void HCStyle::drawIndicator(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, int state, bool down, bool)
{
    int x2 = x+w-1;
    int y2 = y+h-1;

    static const unsigned char x_bits[] = {0x63, 0x77, 0x3e, 0x1c, 0x3e, 0x77, 0x63};
    if (xBmp.isNull())
    {
        xBmp = QBitmap(7, 7, x_bits, true);
        xBmp.setMask(xBmp);
    }

    p->setPen(g.mid());
    p->drawLine(x, y, x2, y);
    p->drawLine(x, y, x, y2);

    p->setPen(g.light());
    p->drawLine(x2, y+1, x2, y2);
    p->drawLine(x+1, y2, x2, y2);

    p->setPen(g.dark());
    p->drawLine(x+1, y+1, x2-1, y+1);
    p->drawLine(x+1, y+1, x+1, y2-1);

    p->setPen(g.midlight());
    p->drawLine(x2-1, y+2, x2-1, y2-1);
    p->drawLine(x+2, y2-1, x2-1, y2-1);

    p->fillRect(x+2, y+2, w-4, h-4, down ? g.brush(QColorGroup::Background) :
                g.brush(QColorGroup::Light));

    if(state != QButton::Off){
        if(state == QButton::On){
            p->setPen(Qt::black);
            p->drawPixmap(3, 3, xBmp);
        }
        else{
            p->setPen(g.dark());
            p->drawRect(x+2, y+2, w-4, h-4);
            p->setPen(Qt::black);
            p->drawLine(x+3, (y+h)/2-2, x+w-4, (y+h)/2-2);
            p->drawLine(x+3, (y+h)/2, x+w-4, (y+h)/2);
            p->drawLine(x+3, (y+h)/2+2, x+w-4, (y+h)/2+2);
        }
    }

}

void HCStyle::drawIndicatorMask(QPainter *p, int x, int y, int w, int h, int)
{
    // needed for some reason by KHtml, even tho it's all filled ;P
    p->fillRect(x, y, w, h, Qt::color1);
}

void HCStyle::drawSlider(QPainter *p, int x, int y, int w, int h,
                         const QColorGroup &g, Orientation orient,
                         bool, bool)
{
    int x2 = x+w-1;
    int y2 = y+h-1;
    if(highcolor){
        if(orient == Horizontal)
            drawVGradient(p, g, QColorGroup::Background, x, y, w, h, 0, 0, w, h);
        else
            drawHGradient(p, g, QColorGroup::Background, x, y, w, h, 0, 0, w, h);
    }
    else
        p->fillRect(x, y, w, h, g.background());
    p->setPen(g.mid());
    p->drawLine(x, y, x2, y);
    p->drawLine(x, y, x, y2);
    p->setPen(Qt::black);
    p->drawLine(x2, y, x2, y2);
    p->drawLine(x, y2, x2, y2);

    p->setPen(g.dark());
    p->drawLine(x2-1, y+1, x2-1, y2-1);
    p->drawLine(x+1, y2-1, x2-1, y2-1);
    p->setPen(g.light());
    p->drawLine(x+1, y+1, x2-1, y+1);
    p->drawLine(x+1, y+1, x+1, y2-1);

    p->setPen(g.mid());
    p->drawLine(x2-2, y+2, x2-2, y2-2);
    p->drawLine(x+2, y2-2, x2-2, y2-2);
    p->setPen(g.midlight());
    p->drawLine(x+2, y+2, x2-2, y+2);
    p->drawLine(x+2, y+2, x+2, y2-2);

    if (orient == Horizontal){
        p->setPen(g.light());
        p->drawLine(x+5, y+4, x+5, y2-4);
        p->drawLine(x+8, y+4, x+8, y2-4);
        p->drawLine(x+11, y+4, x+11, y2-4);
        p->setPen(g.dark());
        p->drawLine(x+6, y+4, x+6, y2-4);
        p->drawLine(x+9, y+4, x+9, y2-4);
        p->drawLine(x+12, y+4, x+12, y2-4);
    }
    else{
        p->setPen(g.light());
        p->drawLine(x+4, y+5, x2-4, y+5);
        p->drawLine(x+4, y+8, x2-4, y+8);
        p->drawLine(x+4, y+11, x2-4, y+11);
        p->setPen(g.dark());
        p->drawLine(x+4, y+6, x2-4, y+6);
        p->drawLine(x+4, y+9, x2-4, y+9);
        p->drawLine(x+4, y+12, x2-4, y+12);
    }
    p->setPen(g.background());
    p->drawPoint(x, y);
    p->drawPoint(x2, y);
    p->drawPoint(x, y2);
    p->drawPoint(x2, y2);
}

void HCStyle::drawSliderMask(QPainter *p, int x, int y, int w, int h,
                             Orientation, bool, bool)
{
    int x2 = x+w-1;
    int y2 = y+h-1;
    p->fillRect(x, y, w, h, Qt::color1);
    p->setPen(Qt::color0);
    p->drawPoint(x, y);
    p->drawPoint(x2, y);
    p->drawPoint(x, y2);
    p->drawPoint(x2, y2);
}

int HCStyle::sliderLength() const
{
    return(18);
}

#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)

void HCStyle::drawArrow(QPainter *p, Qt::ArrowType type, bool on, int x,
                            int y, int w, int h, const QColorGroup &g,
                            bool enabled, const QBrush *)
{
    static const QCOORD u_arrow[]={3,1, 4,1, 2,2, 5,2, 1,3, 6,3, 0,4, 7,4, 0,5, 7,5};
    static const QCOORD d_arrow[]={0,2, 7,2, 0,3, 7,3, 1,4, 6,4, 2,5, 5,5, 3,6, 4,6};
    static const QCOORD l_arrow[]={1,3, 1,4, 2,2, 2,5, 3,1, 3,6, 4,0, 4,7, 5,0, 5,7};
    static const QCOORD r_arrow[]={2,0, 2,7, 3,0, 3,7, 4,1, 4,6, 5,2, 5,5, 6,3, 6,4};

    p->setPen(enabled ? on ? g.light() : g.buttonText() : g.mid());
    if(w > 8){
        x = x + (w-8)/2;
        y = y + (h-8)/2;
    }

    QPointArray a;
    switch(type){
    case Qt::UpArrow:
        a.setPoints(QCOORDARRLEN(u_arrow), u_arrow);
        break;
    case Qt::DownArrow:
        a.setPoints(QCOORDARRLEN(d_arrow), d_arrow);
        break;
    case Qt::LeftArrow:
        a.setPoints(QCOORDARRLEN(l_arrow), l_arrow);
        break;
    default:
        a.setPoints(QCOORDARRLEN(r_arrow), r_arrow);
        break;
    }

    a.translate(x, y);
    p->drawLineSegments(a);
}

void HCStyle::drawKBarHandle(QPainter *p, int x, int y, int w, int h,
                             const QColorGroup &g, KToolBarPos,
                             QBrush *)
{
    if(h > w){
        if(highcolor)
            drawVGradient(p, g, QColorGroup::Background, x, y, w, h, 0, 0, w, h);
        else
            p->fillRect(x, y, w, h, g.brush(QColorGroup::Background));
        int x2 = x+w-1;
        int y2 = y+h-1;

        p->setPen(g.light());
        p->drawLine(x, y, x2, y);
        p->drawLine(x, y, x, y2);

        p->drawLine(x+3, y+4, x+3, y2-4);
        p->drawLine(x+5, y+4, x+5, y2-4);
        p->drawLine(x+7, y+4, x+7, y2-4);

        p->setPen(g.dark());
        p->drawLine(x, y2, x2, y2);

        p->setPen(g.mid());
        p->drawLine(x+4, y+4, x+4, y2-4);
        p->drawLine(x+6, y+4, x+6, y2-4);
        p->drawLine(x+8, y+4, x+8, y2-4);

    }
    else{
        int x2 = x+w-1;
        int y2 = y+h-1;
        if(highcolor)
            drawHGradient(p, g, QColorGroup::Background, x, y, w, h, 0, 0, w, h);
        else
            p->fillRect(x, y, w, h, g.brush(QColorGroup::Background));

        p->setPen(g.light());
        p->drawLine(x, y, x2, y);
        p->drawLine(x, y, x, y2);
        p->drawLine(x2, y, x2, y2);

        p->drawLine(x+4, y+3, x2-4, y+3);
        p->drawLine(x+4, y+5, x2-4, y+5);
        p->drawLine(x+4, y+7, x2-4, y+7);

        p->setPen(g.dark());
        p->drawLine(x2, y, x2, y2);

        p->setPen(g.mid());
        p->drawLine(x+4, y+4, x2-4, y+4);
        p->drawLine(x+4, y+6, x2-4, y+6);
        p->drawLine(x+4, y+8, x2-4, y+8);

    }

}

void HCStyle::drawKMenuBar(QPainter *p, int x, int y, int w, int h,
                           const QColorGroup &g, bool mac, QBrush *)
{
    if(mac){
        p->setPen(g.dark());
        p->drawRect(x, y, w, h);
        ++x, ++y, w-=2, h-=2;
        int x2 = x+w-1;
        int y2 = y+h-1;
        p->setPen(g.light());
        p->drawLine(x, y, x2, y);
        p->drawLine(x, y, x, y2);
        p->setPen(g.mid());
        p->drawLine(x2, y, x2, y2);
        p->drawLine(x, y2, x2, y2);
        if(highcolor)
            drawVGradient(p, g, QColorGroup::Background, x+1, y+1, w-2, h-2, 0, 0, w-2, h-2);
        else
            p->fillRect(x+1, y+1, w-2, h-2, g.brush(QColorGroup::Midlight));
    }
    else{
        qDrawShadePanel(p, x, y, w, h, g, false, 1);
        if(highcolor)
            drawVGradient(p, g, QColorGroup::Background, x+1, y+1, w-2, h-2, 0, 0, w-2, h-2);
        else
            p->fillRect(x+1, y+1, w-2, h-2, g.brush(QColorGroup::Background));
    }

}

void HCStyle::drawKToolBar(QPainter *p, int x, int y, int w, int h,
                           const QColorGroup &g, KToolBarPos, QBrush *)
{
    if(highcolor){
        qDrawShadePanel(p, x, y, w, h, g, false, 1);
        if(w > h)
            drawVGradient(p, g, QColorGroup::Background, x+1, y+1, w-2, h-2, 0, 0, w-2, h-2);
        else
            drawHGradient(p, g, QColorGroup::Background, x+1, y+1, w-2, h-2, 0, 0, w-2, h-2);
    }
    else
        qDrawShadePanel(p, x, y, w, h, g, false, 1,
                        &g.brush(QColorGroup::Background));
}

void HCStyle::drawKToolBarButton(QPainter *p, int x, int y, int w, int h,
                                    const QColorGroup &g, bool sunken,
                                    bool raised, bool enabled, bool popup,
                                    KToolButtonType icontext,
                                    const QString& btext, const QPixmap *pixmap,
                                    QFont *font, QWidget *btn)
{
    int dx, dy;

    QFontMetrics fm(*font);

    QToolBar* toolbar = 0;
    if(btn->parent() && btn->parent()->isWidgetType() && btn->parent()->inherits("QToolBar"))
        toolbar = static_cast<QToolBar*>(btn->parent());

    if(!highcolor || !toolbar)
        p->fillRect(x, y, w, h, g.brush(QColorGroup::Background));
    else
    {
        // sync with drawKToolBar
        if(toolbar->orientation() == Qt::Horizontal)
            // should be background fill?
            drawVGradient(p, g, QColorGroup::Background, x, y, w, h, x, y, toolbar->width()-2, toolbar->height()-2);
        else
            drawHGradient(p, g, QColorGroup::Background, x, y, w, h, x, y, toolbar->width()-2, toolbar->height()-2);
    }

    if(raised || sunken) {
        int x2 = x+w;
        int y2 = y+h;
        p->setPen(g.dark());
        p->drawLine(x+1, y+1, x2-2, y+1);
        p->drawLine(x, y+2, x, y2-3);
        p->drawLine(x2-1, y+2, x2-1, y2-3);
        p->drawLine(x+1, y2-2, x2-2, y2-2);

        p->setPen(sunken ? g.mid() : g.light());
        p->drawLine(x+1, y+2, x2-2, y+2);
        p->drawLine(x+1, y+2, x+1, y2-3);
        p->setPen(sunken ? g.light() : g.mid());
        p->drawLine(x2-2, y+3, x2-2, y2-3);
        p->drawLine(x+2, y2-3, x2-2, y2-3);
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
            dx = 4;
            dy = ( h - pixmap->height() ) / 2;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            p->drawPixmap( x+dx, y+dy, *pixmap );
        }
        if (!btext.isNull()){
            int tf = AlignVCenter|AlignLeft;
            if (pixmap)
                dx= 4 + pixmap->width() + 2;
            else
                dx= 4;
            dy = 0;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            if (font)
                p->setFont(*font);
            if(raised)
                p->setPen(KGlobalSettings::toolBarHighlightColor());
            p->drawText(x+dx, y+dy, w-dx, h, tf, btext);
        }
    }
    else if (icontext == Text){ // only text, even if there is a icon
        if (!btext.isNull()){
            int tf = AlignVCenter|AlignLeft;
            if (!enabled)
                p->setPen(g.dark());
            dx= (w - fm.width(btext)) / 2;
            dy= (h - fm.lineSpacing()) / 2;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            if (font)
                p->setFont(*font);
            if(raised)
                p->setPen(KGlobalSettings::toolBarHighlightColor());
            p->drawText(x+dx, y+dy, fm.width(btext), fm.lineSpacing(), tf, btext);
        }
    }
    else if (icontext == IconTextBottom){
        if (pixmap){
            dx = (w - pixmap->width()) / 2;
            dy = (h - fm.lineSpacing() - pixmap->height()) / 2;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            p->drawPixmap( x+dx, y+dy, *pixmap );
        }
        if (!btext.isNull()){
            int tf = AlignBottom|AlignHCenter;
            dy= pixmap->height();
            dx = 2;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            if (font)
                p->setFont(*font);
            if(raised)
                p->setPen(KGlobalSettings::toolBarHighlightColor());
            p->drawText(x, y, w, h-3, tf, btext);
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


void HCStyle::drawKMenuItem(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, bool active, QMenuItem *mi,
                            QBrush *)
{
    if ( p->font() == KGlobalSettings::generalFont() )
      p->setFont( KGlobalSettings::menuFont() );

    if(active){
        /*
        if(highcolor){
            p->setPen(g.mid());
            p->drawLine(x, y, x2, y);
            p->drawLine(x, y, x, y2);
            p->setPen(g.dark());
            p->drawLine(x, y2, x2, y2);
            p->drawLine(x2, y, x2, y2);
            p->drawTiledPixmap(x+1, y+1, w-2, h-2, *vDark);
        }
        else*/
        qDrawShadePanel(p, x+1, y+1, w-1, h-1, g, true, 1,
                        &g.brush(QColorGroup::Midlight));
        QApplication::style().drawItem(p, x, y, w, h,
                                       AlignCenter|ShowPrefix|DontClip|SingleLine,
                                       g, mi->isEnabled(), mi->pixmap(), mi->text(),
                                       -1, &Qt::black);
    }
    else
        QApplication::style().drawItem(p, x, y, w, h,
                                       AlignCenter|ShowPrefix|DontClip|SingleLine,
                                       g, mi->isEnabled(), mi->pixmap(), mi->text(),
                                       -1, &g.text() );
}

void HCStyle::drawPopupMenuItem( QPainter* p, bool checkable, int maxpmw,
                                     int tab, QMenuItem* mi,
                                     const QPalette& pal, bool act,
                                     bool enabled, int x, int y, int w, int h)
{
static const int motifItemFrame         = 2;
static const int motifItemHMargin       = 3;
static const int motifItemVMargin       = 2;
static const int motifArrowHMargin      = 6;
static const int windowsRightBorder     = 12;

    if (checkable)
        maxpmw = QMAX( maxpmw, 20 );

    if ( p->font() == KGlobalSettings::generalFont() )
      p->setFont( KGlobalSettings::menuFont() );

    if(act){
        bool dis = !enabled;
        QColorGroup itemg = dis ? pal.disabled() : pal.active();

        int checkcol = maxpmw;

        /*if(vDark)
        {
            int x2 = x+w-1;
            int y2 = y+h-1;
            p->setPen(itemg.mid());
            p->drawLine(x, y, x2, y);
            p->drawLine(x, y, x, y2);
            p->setPen(itemg.dark());
            p->drawLine(x, y2, x2, y2);
            p->drawLine(x2, y, x2, y2);
            p->drawTiledPixmap(x+1, y+1, w-2, h-2, *vDark);
        }
        else*/
            qDrawShadePanel(p, x, y, w, h, itemg, true, 1,
                            &itemg.brush(QColorGroup::Midlight));

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
            p->setPen(itemg.highlightedText());
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
        p->setPen(Qt::black);
        QColor discol;
        if (dis) {
            discol = itemg.mid();
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

int HCStyle::popupMenuItemHeight(bool /*checkable*/, QMenuItem *mi,
                                 const QFontMetrics &fm)
{
    if (mi->isSeparator())
        return 2;

    int h = 0;
    if (mi->pixmap())
        h = mi->pixmap()->height();

    if (mi->iconSet())
        h = QMAX(mi->iconSet()->
                 pixmap(QIconSet::Small, QIconSet::Normal).height(), h);

    h = QMAX(fm.height() + 4, h);

    // we want a minimum size of 18
    h = QMAX(h, 18);

    return h;
}

void HCStyle::drawKProgressBlock(QPainter *p, int x, int y, int w, int h,
                                    const QColorGroup &g, QBrush *fill)
{
    p->fillRect(x, y, w, h, fill ? *fill : g.brush(QColorGroup::Text));
}

void HCStyle::drawFocusRect(QPainter *p, const QRect &r,
                            const QColorGroup &g, const QColor *c,
                            bool atBorder)
{
    KStyle::drawFocusRect(p, r, g, c, atBorder);
}

void HCStyle::polishPopupMenu(QPopupMenu *mnu)
{
    KConfig *config = KGlobal::config();
    QString oldGrp = config->group();
    config->setGroup("B2");

    KStyle::polishPopupMenu(mnu);
    if (config->readBoolEntry("AnimateMenus", false))
      (void)new HCAniMenu(mnu);
    config->setGroup(oldGrp);
}

void HCStyle::drawTab(QPainter *p, const QTabBar *tabBar, QTab *tab,
                      bool selected)
{
    if(tabBar->shape() != QTabBar::RoundedAbove){
        KStyle::drawTab(p, tabBar, tab, selected);
        return;
    }

    QRect r = tab->rect();
    QColorGroup g = tabBar->colorGroup();
    if(!selected){
        r.setY(r.y()+1);
        p->setPen(g.dark());
        p->drawLine(r.x(), r.y()+4, r.x(), r.bottom()-2);
        p->drawLine(r.x(), r.y()+4, r.x()+4, r.y());
        p->drawLine(r.x()+4, r.y(), r.right()-1, r.y());
        p->drawLine(r.right(), r.y()+1, r.right(), r.bottom()-2);

        p->setPen(g.midlight());
        p->drawLine(r.x()+1, r.y()+4, r.x()+1, r.bottom());
        p->drawLine(r.x()+1, r.y()+4, r.x()+4, r.y()+1);
        p->drawLine(r.x()+5, r.y()+1, r.right()-2, r.y()+1);
        p->drawLine(r.x(), r.bottom(), r.right(), r.bottom());

        p->setPen(g.mid());
        p->drawPoint(r.right()-1, r.y()+1);
        p->drawLine(r.x()+4, r.y()+2, r.right()-1, r.y()+2);
        p->drawLine(r.x()+3, r.y()+3, r.right()-1, r.y()+3);

        p->fillRect(r.x()+2, r.y()+4, r.width()-3, r.height()-6,
                    g.brush(QColorGroup::Mid));

        p->setPen(g.light());
        p->drawLine(r.x(), r.bottom()-1, r.right(), r.bottom()-1);

    }
    else{
        p->setPen(g.light());
        p->drawLine(r.x(), r.y()+4, r.x(), r.bottom());
        p->drawLine(r.x(), r.y()+4, r.x()+4, r.y());
        p->drawLine(r.x()+4, r.y(), r.right()-1, r.y());
        p->setPen(g.dark());
        p->drawLine(r.right(), r.y()+1, r.right(), r.bottom());

        p->setPen(g.midlight());
        p->drawLine(r.x()+1, r.y()+4, r.x()+1, r.bottom());
        p->drawLine(r.x()+1, r.y()+4, r.x()+4, r.y()+1);
        p->drawLine(r.x()+5, r.y()+1, r.right() - 2, r.y()+1);
        p->setPen(g.mid());
        p->drawLine(r.right()-1, r.y()+1, r.right()-1, r.bottom());
    }

}

void HCStyle::drawTabMask(QPainter *p, const QTabBar*, QTab *tab,
                          bool)


{
    p->fillRect(tab->rect(), Qt::color1);
}

void HCStyle::tabbarMetrics(const QTabBar *t, int &hFrame, int &vFrame,
                            int &overlap)
{
    if(t->shape() == QTabBar::RoundedAbove){
        overlap = 0;
        hFrame = 24;
        vFrame = 10;
    }
    else
        KStyle::tabbarMetrics(t, hFrame, vFrame, overlap);
}



void HCStyle::drawSplitter(QPainter *p, int x, int y, int w, int h,
                           const QColorGroup &g, Orientation)
{
    int x2 = x+w-1;
    int y2 = y+h-1;
    p->setPen(g.dark());
    p->drawRect(x, y, w, h);
    p->setPen(g.background());
    p->drawPoint(x, y);
    p->drawPoint(x2, y);
    p->drawPoint(x, y2);
    p->drawPoint(x2, y2);
    p->setPen(g.light());
    p->drawLine(x+1, y+1, x+1, y2-1);
    p->drawLine(x+1, y+1, x2-1, y+1);
    p->setPen(g.midlight());
    p->drawLine(x+2, y+2, x+2, y2-2);
    p->drawLine(x+2, y+2, x2-2, y+2);
    p->setPen(g.mid());
    p->drawLine(x2-1, y+1, x2-1, y2-1);
    p->drawLine(x+1, y2-1, x2-1, y2-1);
    p->fillRect(x+3, y+3, w-5, h-5, g.brush(QColorGroup::Background));
}

void HCStyle::drawPanel(QPainter *p, int x, int y, int w, int h,
                        const QColorGroup &g, bool sunken,
                        int lineWidth, const QBrush *fill)
{
    if(lineWidth != 2 || !sunken)
        KStyle::drawPanel(p, x, y, w, h, g, sunken, lineWidth, fill);
    else{
        QPen oldPen = p->pen();
        int x2 = x+w-1;
        int y2 = y+h-1;
        p->setPen(g.light());
        p->drawLine(x, y2, x2, y2);
        p->drawLine(x2, y, x2, y2);
        p->setPen(g.mid());
        p->drawLine(x, y, x2, y);
        p->drawLine(x, y, x, y2);

        p->setPen(g.midlight());
        p->drawLine(x+1, y2-1, x2-1, y2-1);
        p->drawLine(x2-1, y+1, x2-1, y2-1);
        p->setPen(g.dark());
        p->drawLine(x+1, y+1, x2-1, y+1);
        p->drawLine(x+1, y+1, x+1, y2-1);
        p->setPen(oldPen);
        if(fill){
            // I believe here we are only supposed to fill if there is a
            // specified fill brush...
            p->fillRect(x+2, y+2, w-4, h-4, *fill);
        }
    }
}

// no check here, make sure you're highcolor beforehand ;-)
void HCStyle::drawVGradient(QPainter *p, const QColorGroup &g,
                            QColorGroup::ColorRole type, int x, int y,
                            int w, int h, int /*sx*/, int sy, int /*tw*/, int th)
{
    GradientSet *grSet = gDict.find(g.color(type).rgb());
    if(!grSet){
        grSet = new GradientSet(g.color(type));
        gDict.insert(g.color(type).rgb(), grSet);
    }

    int grheight = th - sy;
    if(grheight <= 24){
        p->drawTiledPixmap(x, y, w, h, *grSet->gradient(VSmall));
    }
    else if(grheight <= 34){
        p->drawTiledPixmap(x, y, w, h, *grSet->gradient(VMed));
    }
    else if(grheight <= 64){
        p->drawTiledPixmap(x, y, w, h, *grSet->gradient(VLarge));
    }
    else{
        KPixmap *vLarge = grSet->gradient(VLarge);
        p->fillRect(x, y+vLarge->height(), w, h-vLarge->height(),
                    g.color(type).dark(110));
        p->drawTiledPixmap(x, y, w, vLarge->height(), *vLarge);
    }
}

void HCStyle::drawHGradient(QPainter *p, const QColorGroup &g,
                            QColorGroup::ColorRole type, int x, int y,
                            int w, int h, int sx, int /*sy*/, int tw, int /*th*/)
{
    GradientSet *grSet = gDict.find(g.color(type).rgb());
    if(!grSet){
        grSet = new GradientSet(g.color(type));
        gDict.insert(g.color(type).rgb(), grSet);
    }
    int grwidth = tw - sx;
    if(grwidth <= 34){
        p->drawTiledPixmap(x, y, w, h, *grSet->gradient(HMed));
    }
    else if(grwidth <= 52){
        p->drawTiledPixmap(x, y, w, h, *grSet->gradient(HLarge));
    }
    else{
        KPixmap *hLarge = grSet->gradient(HLarge);
        p->fillRect(x+hLarge->width(), y, w-hLarge->width(), h,
                    g.background().dark(110));
        p->drawTiledPixmap(x, y, hLarge->width(), h, *hLarge);
    }
}

void HCStyle::drawKickerAppletHandle(QPainter *p, int x, int y, int w, int h,
                                      const QColorGroup &g, QBrush *)
{
    if(h > w){
        int y2 = y+h-1;

        p->setPen(g.light());

        p->drawLine(x+1, y+2, x+1, y2-2);
        p->drawLine(x+4, y+2, x+4, y2-2);

        p->setPen(g.dark());
        p->drawLine(x+2, y+2, x+2, y2-2);
        p->drawLine(x+5, y+2, x+5, y2-2);

    }
    else{
        int x2 = x+w-1;

        p->setPen(g.light());

        p->drawLine(x+2, y+1, x2-2, y+1);
        p->drawLine(x+2, y+4, x2-2, y+4);

        p->setPen(g.dark());
        p->drawLine(x+2, y+2, x2-2, y+2);
        p->drawLine(x+2, y+5, x2-2, y+5);
    }

}

void HCStyle::drawKickerTaskButton(QPainter *p, int x, int y, int w, int h,
                                  const QColorGroup &g,
                                  const QString &text, bool sunken,
                                  QPixmap *pixmap, QBrush *)
{
    int x2 = x+w-1;
    int y2 = y+h-1;
    if( sunken || ( !highcolor ) )
        p->fillRect(x+1, y+1, w-2, h-2, g.brush(QColorGroup::Mid));
    else{
        GradientSet *grSet = gDict.find(g.background().rgb());
        if(!grSet){
            grSet = new GradientSet(g.background());
            gDict.insert(g.background().rgb(), grSet);
        }
        p->drawTiledPixmap(x+1, y+1, w-2, h-2, sunken ? *grSet->gradient(VLarge)
                           : *grSet->gradient(VSmall));
    }
    p->setPen(sunken ? Qt::black : g.light());
    p->drawLine(x, y, x2-1, y);
    p->drawLine(x, y, x, y2-1);
    p->setPen(sunken ? g.midlight() : g.mid());
    p->drawLine(x+1, y2-1, x2-1, y2-1);
    p->drawLine(x2-1, y+1, x2-1, y2-1);
    p->setPen(sunken ? g.light() : Qt::black);
    p->drawLine(x, y2, x2, y2);
    p->drawLine(x2, y, x2, y2);

    if(text.isNull() && !pixmap)
        return;

    const int pxWidth = 20;
    int textPos = pxWidth;
    QRect br(buttonRect(x, y, w, h));

    if (sunken)
        p->translate(1,1);

    if ( pixmap && !pixmap->isNull() ) {
        int dx = ( pxWidth - pixmap->width() ) / 2;
        int dy = ( h - pixmap->height() ) / 2;
        p->drawPixmap( br.x()+dx, dy, *pixmap );
    }

    QString s = text;
    static const QString &modStr = KGlobal::staticQString(
           QString::fromUtf8("[") + i18n("modified") + QString::fromUtf8("]"));

    int modStrPos = s.find(modStr);

    if (-1 != modStrPos) {

      // +1 because we include a space after the closing brace.
      s.remove(modStrPos, modStr.length()+1);

      QPixmap modPixmap = SmallIcon("modified");

      int dx = (pxWidth   - modPixmap.width())  / 2;
      int dy = (h  - modPixmap.height()) / 2;

      p->drawPixmap(br.x() + textPos + dx, dy, modPixmap);

      textPos += pxWidth;
    }

    if (!s.isEmpty()){
        if (p->fontMetrics().width(s) > br.width() - textPos) {

            int maxLen = br.width() - textPos - p->fontMetrics().width("...");

            while ((!s.isEmpty()) && (p->fontMetrics().width(s) > maxLen))
                s.truncate(s.length() - 1);

            s.append("...");
        }

        p->setPen(sunken ? g.light() : g.buttonText());

        p->drawText(br.x()+ textPos, -1, w-textPos, h,
                    AlignLeft|AlignVCenter, s);
    }

}


#include "hcstyle.moc"

/* vim: set noet sw=8 ts=8: */
