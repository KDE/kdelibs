#ifndef SGISTYLE_H
#define SGISTYLE_H

#include <kstyle.h>
#include <qbitmap.h>
#include <qpalette.h>

class QPushButton;

class KSgiStyle : public KStyle
{

  public:
	
    KSgiStyle();
    ~KSgiStyle();
	
    virtual void polish(QWidget *w);
    virtual void unPolish(QWidget *w);
    virtual void polish(QApplication *a) { KStyle::polish(a); }
    virtual void unPolish(QApplication *a) { KStyle::unPolish(a); }
    virtual void polish(QPalette &a) { KStyle::polish(a); }

    bool eventFilter(QObject *obj, QEvent *ev);
		
    void 	drawButton(QPainter *p, int x, int y, int w, int h,
    const       QColorGroup &g, bool sunken = FALSE,
    const       QBrush *fill = 0 );
    void 	drawPushButton(QPushButton *btn, QPainter *p);
		QRect buttonRect (int x, int y, int w, int h);

    void 	drawBevelButton(QPainter *p, int x, int y, int w, int h,
                         const QColorGroup &g, bool sunken = FALSE,
                         const QBrush *fill = 0 );

    QSize       indicatorSize() const;
    void 	drawIndicator(QPainter *p, int x, int y, int w, int h,
			const QColorGroup &g, int state, bool down=FALSE, 
			bool enabled = TRUE);
    void	drawIndicatorMask (QPainter *p, int x, int y, int w, int h,
															int state);
		
    QSize       exclusiveIndicatorSize() const;
    void 	drawExclusiveIndicator(QPainter *p, int x, int y, int w, int h,
					const QColorGroup &g, bool on, bool down=FALSE, 
					bool enabled = TRUE);

    void 	drawComboButton(QPainter *p, int x, int y, int w, int h,
                         const QColorGroup &g, bool sunken = FALSE,
                         bool editable = FALSE, bool enabled = TRUE,
                         const QBrush *fill = 0 );
    QRect   comboButtonRect (int x, int y, int w, int h);
    QRect   comboButtonFocusRect (int x, int y, int w, int h);
		
    int     sliderLength() const;
    void    drawSliderGroove(QPainter *p, int x, int y, int w, int h,
                          const QColorGroup &g, QCOORD, Orientation);
    void    drawSlider(QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &g, Orientation orient, bool, bool);

    void    drawCheckMark( QPainter *p, int x, int y, int w, int h,
				const QColorGroup &g, bool act = FALSE, bool dis = FALSE);
									
    void    drawScrollBarControls(QPainter *p, const QScrollBar *sb,
                                       int sliderStart, uint controls,
                                       uint activeControl);
    void    scrollBarMetrics(const QScrollBar *sb, int &sliderMin,
                                  int &sliderMax, int &sliderLength,
                                  int &buttonDim);			
    QStyle::ScrollControl scrollBarPointOver(const QScrollBar *sb, 
																							int sliderStart, const QPoint &p);
    void    drawScrollBarArrow(QPainter *p, Qt::ArrowType type, int x,
                                  int y, const QColorGroup &g);
																	
    int     splitterWidth() const;
    void    drawSplitter (QPainter *p, int x, int y, int w, int h,
												const QColorGroup &g, Orientation);
    void    drawPopupPanel(QPainter *p, int x, int y, int w, int h,
			            	const QColorGroup &g,  int , const QBrush *fill );																	
    void    drawPanel(QPainter *p, int x, int y, int w, int h,
			const QColorGroup &g, bool sunken, int , 
			const QBrush* fill);
                        
    void    drawKToolBar(QPainter *p, int x, int y, int w, int h,
                          const QColorGroup &g, KToolBarPos pos,
                          QBrush *);
    void    drawKToolBarButton(QPainter *p, int x, int y, int w, int h,
                               const QColorGroup &g, bool sunken,
                               bool raised, bool enabled, bool popup,
                               KToolButtonType icontext,
                               const QString& btext, const QPixmap *pixmap,
                               QFont *font, QWidget *);

    void    drawKMenuBar(QPainter *p, int x, int y, int w, int h,
                          const QColorGroup &g, bool, QBrush *fill=NULL);
    void    drawKMenuItem(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, bool active, QMenuItem *mi,
                            QBrush *fill=NULL);
    void    drawPopupMenuItem(QPainter *p, bool checkable, int maxpmw,
                           int tab, QMenuItem *mi, const QPalette &pal,
                           bool act, bool enabled, int x, int y, int w,
                           int h);
    int     popupMenuItemHeight(bool c, QMenuItem *mi, const QFontMetrics &fm);
      									 																
  protected:
    void    drawFullShadeButton (QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &g, bool sunken = FALSE,
                    const QBrush *fill = 0);
    void    drawPartShadeButton (QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &g, bool sunken = FALSE,
                    const QBrush *fill = 0);								
	
  private:
    QPalette palStore;
    QPixmap checkPix;
    QBitmap checkMask;
    QBitmap outlineBitmap;
    QBitmap fillBitmap;
    QBitmap inactfillBitmap;
    QBitmap hiliteBitmap;
    QBitmap arrowBitmap;
    QBitmap shadowBitmap;
    QBitmap upArrow;
    QBitmap downArrow;
    QBitmap leftArrow;
    QBitmap rightArrow;
};

#endif
