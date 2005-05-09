/*

  Copyright (c) 2000 KDE Project

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.

 */

#ifndef   __KLegacyStyle_hh
#define   __KLegacyStyle_hh

#include <kstyle.h>

// forward declaration
class KLegacyStylePrivate;


class Q_EXPORT KLegacyStyle : public KStyle {
    Q_OBJECT
public:
    KLegacyStyle(void);
    virtual ~KLegacyStyle(void);

    virtual int defaultFrameWidth() const;

    virtual void polish(QApplication *);
    virtual void polish(QWidget *);
    virtual void polishPopupMenu(Q3PopupMenu *);
    virtual void unPolish(QWidget *);
    virtual void unPolish(QApplication *);

    // combo box
    virtual void drawComboButton(QPainter *, int, int, int, int, const QColorGroup &,
				 bool = false, bool = false, bool = true,
				 const QBrush * = 0);
    virtual QRect comboButtonRect(int, int, int, int);
    virtual QRect comboButtonFocusRect(int, int, int, int);

    // menubar items
    virtual void drawMenuBarItem(QPainter *, int, int, int, int, QMenuItem *,
				 QColorGroup &, bool, bool);
    virtual void drawKMenuItem(QPainter *, int, int, int, int, const QColorGroup &, bool,
                               QMenuItem *, QBrush * = 0);
    
    // toolbar stuffs
    virtual void drawKBarHandle(QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g, KToolBarPos type, QBrush *fill = 0);
    virtual void drawKickerHandle(QPainter *p, int x, int y, int w, int h,
                                  const QColorGroup &g, QBrush *fill = 0);
    virtual void drawKickerAppletHandle(QPainter *p, int x, int y, int w, int h,
                                        const QColorGroup &g, QBrush *fill = 0);
    virtual void drawKickerTaskButton(QPainter *p, int x, int y, int w, int h,
                                      const QColorGroup &g, const QString &title, bool active,
                                      QPixmap *icon = 0, QBrush *fill = 0);

    // arrows
    virtual void drawArrow(QPainter *, Qt::ArrowType, bool, int, int, int, int,
			   const QColorGroup &, bool, const QBrush * = 0);

    // button stuffs
    virtual void drawButton(QPainter *, int, int, int, int, const QColorGroup &g,
    			    bool = false, const QBrush * = 0);
    virtual void drawPushButton(QPushButton *, QPainter *);
    virtual void drawBevelButton(QPainter *, int, int, int, int,
				 const QColorGroup &, bool = false,
				 const QBrush * = 0);

    // indicators (QCheckBox)
    virtual void drawCheckMark(QPainter *, int, int, int, int, const QColorGroup &,
			       bool = false, bool = true);
    virtual void drawIndicator(QPainter *, int, int, int, int, const QColorGroup &,
			       int, bool = false, bool = true);
    virtual void drawIndicatorMask(QPainter *, int, int, int, int, int);
    virtual QSize indicatorSize(void) const;

    // exclusive indicators (QRadioButton)
    virtual void drawExclusiveIndicator(QPainter *, int, int, int, int,
					const QColorGroup &, bool, bool = false,
					bool = true);
    virtual void drawExclusiveIndicatorMask(QPainter *, int, int, int, int, bool);
    virtual QSize exclusiveIndicatorSize(void) const;

    // popup menus
    virtual void drawPopupPanel(QPainter *, int, int, int, int, const QColorGroup &,
				int = 2, const QBrush * = 0);
    virtual void drawPopupMenuItem(QPainter *, bool, int, int, QMenuItem *,
				   const QPalette &, bool, bool, int, int, int, int);


    // scrollbars
    virtual ScrollControl scrollBarPointOver(const QScrollBar *, int, const QPoint &);
    virtual void scrollBarMetrics(const QScrollBar *, int &, int &, int &, int &);
    virtual void drawScrollBarControls(QPainter *, const QScrollBar *,
				       int, uint, uint);

    // sliders
    virtual void drawSlider(QPainter *, int , int , int , int ,
			    const QColorGroup &, Qt::Orientation, bool, bool);
    virtual void drawSliderGroove(QPainter *, int, int, int, int, const QColorGroup &,
				  QCOORD, Qt::Orientation);

    // panel
    virtual void drawPanel(QPainter *, int, int, int, int, const QColorGroup &,
			   bool = false, int = 1, const QBrush * = 0);

    // splitters
    virtual void drawSplitter(QPainter *, int, int, int, int,
			      const QColorGroup &, Qt::Orientation);

    // tabs
    virtual void drawTab(QPainter *, const QTabBar *, QTab *, bool);


protected:
    bool eventFilter(QObject *, QEvent *);

    void drawMenuArrow(QPainter *, Qt::ArrowType, bool, int, int, int, int,
		       const QColorGroup &, bool, const QBrush * = 0);


private:
    KLegacyStylePrivate *priv;

#if defined(Q_DISABLE_COPY)
    KLegacyStyle( const KLegacyStyle & );
    KLegacyStyle& operator=( const KLegacyStyle & );
#endif

};


#endif // __KLegacyStyle_hh
