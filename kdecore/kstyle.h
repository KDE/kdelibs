#ifndef __KSTYLE_H
#define __KSTYLE_H

#include <qplatinumstyle.h>
#include <qfont.h>
#include <qpalette.h>
#include <qpixmap.h>

class KStyle : public QPlatinumStyle
{
    Q_OBJECT
public:
    KStyle() : QPlatinumStyle(){;}
    virtual void drawKToolBar(QPainter *p, int x, int y, int w, int h,
                              const QColorGroup &g, bool floating = false);
    virtual void drawKBarHandle(QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g, bool horizontal=false,
                                QBrush *fill=NULL);
    virtual void drawKToolBarButton(QPainter *p, int x, int y, int w, int h,
                                    const QColorGroup &g, bool sunken=false,
                                    bool raised = true, bool enabled = true,
                                    bool popup = false, int icontext = 0,
                                    const QString btext=QString::null,
                                    const QPixmap *icon=NULL,
                                    QFont *font=NULL);
};

#endif
    
    
