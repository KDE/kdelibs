#ifndef __KSTYLE_H
#define __KSTYLE_H

#include <qplatinumstyle.h>
#include <qfont.h>
#include <qpalette.h>
#include <qpixmap.h>

class QMenuItem;
class QPixmap;

/**
 * Extends the QStyle class with virtual methods to draw KDE widgets.
 *
 * @author Daniel M. Duley <mosfet@kde.org>
 */
class KStyle : public QPlatinumStyle
{
    Q_OBJECT
public:
    enum KToolButtonType{Icon=0, IconTextRight, Text, IconTextBottom};
    KStyle() : QPlatinumStyle(){;}
    virtual void drawKToolBar(QPainter *p, int x, int y, int w, int h,
                              const QColorGroup &g, bool floating = false);
    virtual void drawKBarHandle(QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g, bool horizontal=false,
                                QBrush *fill=NULL);
    virtual void drawKToolBarButton(QPainter *p, int x, int y, int w, int h,
                                    const QColorGroup &g, bool sunken=false,
                                    bool raised = true, bool enabled = true,
                                    bool popup = false,
                                    KToolButtonType icontext = Icon,
                                    const QString& btext=QString::null,
                                    const QPixmap *icon=NULL,
                                    QFont *font=NULL);
    virtual void drawKMenuBar(QPainter *p, int x, int y, int w, int h,
                              const QColorGroup &g, QBrush *fill=NULL);
    virtual void drawKMenuItem(QPainter *p, int x, int y, int w, int h,
                               const QColorGroup &g, bool active,
                               QMenuItem *item, QBrush *fill=NULL);
    virtual void drawKProgressBlock(QPainter *p, int x, int y, int w, int h,
                                    const QColorGroup &g, QBrush *fill);
    // this isn't a drawXXX method due to KProgress implementation
    virtual void getKProgressBackground(const QColorGroup &g, QBrush &bg);
};

#endif
    
    
