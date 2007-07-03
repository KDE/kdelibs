/***************************************************************************
 *   Copyright (C) 2006-2007 by Thomas Lübking                             *
 *   thomas.luebking@web.de                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QHeaderView>
#include <QMenuBar>
#include <QStyleOptionTab>
#include <QStyleOptionHeader>
#include <QStyleOptionSlider>
#include <QStyleOptionProgressBarV2>
#include <QStyleOptionToolBox>
#include <QPainter>
#include "oxygen.h"

#include <QtDebug>

using namespace Oxygen;

extern Config config;
extern Dpi dpi;
// extern HoverFades hoverWidgets;q
extern bool animationUpdate;

#include "inlinehelp.cpp"
#include "makros.h"

static const int windowsItemFrame	= 1; // menu item frame width
static const int windowsItemHMargin	= 3; // menu item hor text margin
static const int windowsItemVMargin	= 1; // menu item ver text margin
static const int windowsRightBorder	= 12; // right border on windows

static void drawArrow(const QStyle *style, const QStyleOptionToolButton *toolbutton,
                      const QRect &rect, QPainter *painter, const QWidget *widget = 0)
{
   QStyle::PrimitiveElement pe;
   switch (toolbutton->arrowType) {
   case Qt::LeftArrow:
      pe = QStyle::PE_IndicatorArrowLeft;
      break;
   case Qt::RightArrow:
      pe = QStyle::PE_IndicatorArrowRight;
      break;
   case Qt::UpArrow:
      pe = QStyle::PE_IndicatorArrowUp;
      break;
   case Qt::DownArrow:
      pe = QStyle::PE_IndicatorArrowDown;
      break;
   default:
      return;
   }
   QStyleOption arrowOpt;
   arrowOpt.rect = rect;
   arrowOpt.palette = toolbutton->palette;
   arrowOpt.state = toolbutton->state;
   style->drawPrimitive(pe, &arrowOpt, painter, widget);
}

void OxygenStyle::drawControl ( ControlElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget) const
{
   Q_ASSERT(option);
   Q_ASSERT(painter);
   
   bool sunken = option->state & State_Sunken;
   bool isEnabled = option->state & State_Enabled;
   bool hover = isEnabled && (option->state & State_MouseOver);
   bool hasFocus = option->state & State_HasFocus;
   
   switch ( element ) {
   case CE_PushButton: // A QPushButton, draws case CE_PushButtonBevel, case CE_PushButtonLabel and PE_FrameFocusRect
      if (const QStyleOptionButton *btn =
          qstyleoption_cast<const QStyleOptionButton *>(option)) {
         QStyleOptionButton tmpBtn = *btn;
         if (btn->features & QStyleOptionButton::Flat) { // more like a toolbtn
            //TODO: handle focus indication here (or in the primitive...)!
            drawPrimitive(PE_PanelButtonTool, option, painter, widget);
         }
         else
            drawControl(CE_PushButtonBevel, &tmpBtn, painter, widget);
//          tmpBtn.rect = subElementRect(SE_PushButtonContents, btn, widget);
         tmpBtn.rect = btn->rect.adjusted(dpi._4,dpi._4,-dpi._4,-dpi._4);
         drawControl(CE_PushButtonLabel, &tmpBtn, painter, widget);
      }
      break;
   case CE_PushButtonBevel: // The bevel and default indicator of a QPushButton.
      if (const QStyleOptionButton *btn =
          qstyleoption_cast<const QStyleOptionButton *>(option)) {
         drawPrimitive(PE_PanelButtonCommand, option, painter, widget);
         if (btn->features & QStyleOptionButton::HasMenu) {
//             int mbi = pixelMetric(PM_MenuButtonIndicator, btn, widget);
            QStyleOptionButton newBtn = *btn;
            int sz = (RECT.height()-dpi._6)/2;
            newBtn.rect = RECT;
            newBtn.rect.setLeft(RECT.right() - (dpi._10+sz));
            shadows.line[1][Sunken].render(newBtn.rect, painter);
            newBtn.rect.setLeft(newBtn.rect.left() + dpi._4);
            newBtn.rect.setTop((RECT.height()-sz)/2 + dpi._2);
            newBtn.rect.setHeight(sz); newBtn.rect.setWidth(sz);
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(midColor(COLOR(Window),COLOR(WindowText)));
            drawPrimitive(PE_IndicatorArrowDown, &newBtn, painter, widget);
            painter->restore();
         }
      }
      break;
   case CE_PushButtonLabel: // The label (icon with text or pixmap) of a QPushButton
      if (const QStyleOptionButton *btn =
          qstyleoption_cast<const QStyleOptionButton *>(option)) {
         QRect ir = btn->rect;
         uint tf = Qt::AlignVCenter | Qt::TextShowMnemonic;
         if (!styleHint(SH_UnderlineShortcut, btn, widget))
            tf |= Qt::TextHideMnemonic;
         
         if (!btn->icon.isNull()) {
            QIcon::Mode mode = isEnabled ? QIcon::Normal
               : QIcon::Disabled;
            if (mode == QIcon::Normal && hasFocus)
               mode = QIcon::Active;
            QIcon::State state = QIcon::Off;
            if (btn->state & State_On)
               state = QIcon::On;
            QPixmap pixmap = btn->icon.pixmap(btn->iconSize, mode, state);
            int pixw = pixmap.width();
            int pixh = pixmap.height();
            
            //Center the icon if there is no text
            QPoint point;
            if (btn->text.isEmpty())
               point = QPoint(ir.x() + ir.width() / 2 - pixw / 2, ir.y() + ir.height() / 2 - pixh / 2);
            else
               point = QPoint(ir.x() + 2, ir.y() + ir.height() / 2 - pixh / 2);
            
            if (btn->direction == Qt::RightToLeft)
               point.rx() += pixw;
            
            painter->drawPixmap(visualPos(btn->direction, btn->rect, point), pixmap);
            
            if (btn->direction == Qt::RightToLeft)
               ir.translate(-4, 0);
            else
               ir.translate(pixw + 4, 0);
            ir.setWidth(ir.width() - (pixw + 4));
                // left-align text if there is
            if (!btn->text.isEmpty())
               tf |= Qt::AlignLeft;
         }
         else
            tf |= Qt::AlignHCenter;
             
         if (btn->features & QStyleOptionButton::HasMenu) {
            ir.setRight(ir.right() - ir.height()/2 - dpi._10);
         }
         else if (widget)
         if (const QAbstractButton* btn =
             qobject_cast<const QAbstractButton*>(widget))
         if (btn->isCheckable())
            ir.setRight(ir.right() - ir.height()/2 - dpi._10);
         
         if (btn->features & QStyleOptionButton::Flat) {
            drawItemText(painter, ir, tf, PAL, isEnabled, btn->text,
                         QPalette::WindowText);
            break;
         }
         painter->save();
         QColor fg = btnFgColor(PAL, isEnabled, hover);
         if (btn->features & QStyleOptionButton::DefaultButton) {
            painter->setPen(midColor(btnBgColor(PAL, isEnabled, hover), fg, 3,1));
            ir.translate(0,1);
            drawItemText(painter, ir, tf, PAL, isEnabled, btn->text);
//             ir.translate(2,2);
//             drawItemText(painter, ir, tf, PAL, isEnabled, btn->text);
            ir.translate(0,-1);
         }
         painter->setPen(fg);
         drawItemText(painter, ir, tf, PAL, isEnabled, btn->text);
         painter->restore();
      }
      break;
   case CE_DockWidgetTitle: // Dock window title.
      if (const QStyleOptionDockWidget *dwOpt =
          qstyleoption_cast<const QStyleOptionDockWidget *>(option)) {
         QRect textRect;
         int x3 = RECT.right()-7;
         if (dwOpt->floatable)
            x3 -= 18;
         if (dwOpt->closable)
            x3 -= 18;
         int x2 = x3;
         if (!dwOpt->title.isEmpty()) {
            int itemtextopts = Qt::AlignCenter | Qt::TextShowMnemonic;
            drawItemText(painter, RECT, itemtextopts, PAL, isEnabled, dwOpt->title, QPalette::WindowText);
            textRect = painter->boundingRect ( RECT, itemtextopts, dwOpt->title );
            x2 = textRect.x()-8;
         }

         const Tile::Line &line = shadows.line[0][Sunken];
         textRect.setTop(textRect.top()+(textRect.height()-line.thickness())/2);
         int x = textRect.right()+dpi._4;
         textRect.setRight(textRect.left()-dpi._4);
         textRect.setLeft(qMin(RECT.x()+RECT.width()/4,textRect.x()-(textRect.x()-RECT.x())/2));
         line.render(textRect, painter, Tile::Left|Tile::Center);
         textRect.setLeft(x);
         textRect.setRight(qMax(RECT.right()-RECT.width()/4,x+(RECT.right()-x)/2));
         line.render(textRect, painter, Tile::Right|Tile::Center);
         //TODO: hover?
      }
      break;
   case CE_Splitter: // Splitter handle; see also QSplitter.
      drawPrimitive(PE_IndicatorDockWidgetResizeHandle,option,painter,widget);
      break;
   case CE_RadioButton: // A QRadioButton, draws a PE_ExclusiveRadioButton, a case CE_RadioButtonLabel
   case CE_CheckBox: // A QCheckBox, draws a PE_IndicatorCheckBox, a case CE_CheckBoxLabel
      if (const QStyleOptionButton *btn =
          qstyleoption_cast<const QStyleOptionButton *>(option)) {
         QStyleOptionButton subopt = *btn;
         if (element == CE_RadioButton) {
            subopt.rect = subElementRect(SE_RadioButtonIndicator, btn, widget);
            drawPrimitive(PE_IndicatorRadioButton, &subopt, painter, widget);
            subopt.rect = subElementRect(SE_RadioButtonContents, btn, widget);
            drawControl(CE_RadioButtonLabel, &subopt, painter, widget);
         }
         else {
            subopt.rect = subElementRect(SE_CheckBoxIndicator, btn, widget);
            drawPrimitive(PE_IndicatorCheckBox, &subopt, painter, widget);
            subopt.rect = subElementRect(SE_CheckBoxContents, btn, widget);
            drawControl(CE_CheckBoxLabel, &subopt, painter, widget);
         }
      }
      break;
//    case CE_CheckBoxLabel: // The label (text or pixmap) of a QCheckBox
//    case CE_RadioButtonLabel: // The label (text or pixmap) of a QRadioButton
   case CE_TabBarTab: // The tab and label within a QTabBar
      if (const QStyleOptionTab *tab =
          qstyleoption_cast<const QStyleOptionTab *>(option)) {
         // do we have to exclude the scrollers?
         bool needRestore = false;
         if (widget && (RECT.right() > widget->width())) {
            painter->save();
            needRestore = true;
            QRect r = RECT;
            r.setRight(widget->width() -
                  2*pixelMetric(PM_TabBarScrollButtonWidth,option,widget));
            painter->setClipRect(r);
         }
         // paint shape and label
         drawControl(CE_TabBarTabShape, tab, painter, widget);
         drawControl(CE_TabBarTabLabel, tab, painter, widget);
         if (needRestore)
            painter->restore();
      }
      break;
   case CE_TabBarTabShape: // The tab shape within a tab bar
      if (const QStyleOptionTab *tab =
          qstyleoption_cast<const QStyleOptionTab *>(option)) {
//          IndexedFadeInfo *info = 0;
//          int index = -1, hoveredIndex = -1, step = 0;

         // fade animation stuff
//          if (widget)
//          if (const QTabBar* tbar = qobject_cast<const QTabBar*>(widget)) {
//             // NOTICE: the index increment is IMPORTANT to make sure it's no "0"
//             index = tbar->tabAt(RECT.topLeft()) + 1; // is the action for this item!
//             hoveredIndex = hover ? index :
//                   tbar->tabAt(tbar->mapFromGlobal(QCursor::pos())) + 1;
//             info = const_cast<IndexedFadeInfo *>
//                (animator->indexedFadeInfo(widget, hoveredIndex));
//          }
//          if (info)
//             step = info->step(index);
         
         // maybe we're done here?!
//          if (!(step || hover || selected || sunken))
         if (!(option->state & State_Selected))
            break;
         
         const int _2 = dpi._2, _4 = dpi._4;
         Tile::PosFlags pf = Tile::Ring;
         QRect rect = RECT;
         int size = RECT.height();
         Qt::Orientation o = Qt::Vertical;
         
//          if (selected) {
            
            // invert the shape alignment if we're not on a tabwidget
            // (== safari style tabs)
            int shape = tab->shape;
            if (widget && !(widget->parentWidget() &&
                  qobject_cast<QTabWidget*>(widget->parentWidget()))) {
               //NOTICE: this is elegant but NOT invariant against enum changes!!!
               shape%2 ? --shape : ++shape;
            }
            
            QRect fillRect = RECT;
            switch ((QTabBar::Shape)shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth: {
               pf &= ~Tile::Bottom;
               const QPixmap &shadow = Gradients::shadow(fillRect.height()-_4);
               rect.adjust(0, _2, 0, 0);
               fillRect = rect.adjusted(_2, dpi._1, -_2, 0);
               painter->drawPixmap(fillRect.topRight(), shadow);
               size = fillRect.height();
               break;
            }
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth: {
               pf &= ~Tile::Top;
               const QPixmap &shadow = Gradients::shadow(fillRect.height()-_4, true);
               rect.adjust(0, 0, 0, -_2);
               fillRect = rect.adjusted(_2, 0, -_2, -_4);
               size = fillRect.height();
               painter->drawPixmap(fillRect.topRight(), shadow);
               break;
            }
            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
               pf &= ~Tile::Left;
               fillRect.adjust(0, _2, -dpi._6, -_4);
               rect.adjust(0, 0, -_4, 0);
               o = Qt::Horizontal;
               size = fillRect.width();
               break;
            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
               pf &= ~Tile::Right;
               fillRect.adjust(dpi._6, _2, 0, -_4);
               rect.adjust(_4, 0, 0, 0);
               o = Qt::Horizontal;
               size = fillRect.width();
               break;
            }
            masks.tab.render(fillRect, painter, Gradients::brush(COLOR(Window),
               size, o, config.gradChoose), pf | Tile::Center);
            shadows.tab[1][0].render(rect, painter, pf);
//          }
#if 0
         else
         {
            switch (tab->shape) {
            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
               o = Qt::Horizontal;
               size = RECT.width();
            default:
               break;
            }
            
            if (!step) step = 6;
            const QColor c =
               midColor(COLOR(Window), COLOR(WindowText), step, 6-step);
            QPoint off;
            if (sunken) {
               rect.adjust(_2, _2, -_2, -_2);
               off = QPoint(_2, _2);
            }
            else {
               rect.adjust(_4, _4, -_4, -_4);
               off = QPoint(_4, _4);
            }
            masks.tab.render(rect, painter, Gradients::pix(c, size, o, Gradients::Glass),
                             Tile::Full, false, off);
            
         }
#endif
      }
      break;
   case CE_TabBarTabLabel: // The label within a tab
      if (const QStyleOptionTab *tab =
          qstyleoption_cast<const QStyleOptionTab *>(option)) {
         painter->save();
         QStyleOptionTabV2 tabV2(*tab);
         QRect tr = tabV2.rect;
         bool verticalTabs = false;
         bool east = false;
         bool selected = tabV2.state & State_Selected;
         if (selected) hover = false;
         int alignment = Qt::AlignCenter | Qt::TextShowMnemonic;
         bool bottom = false;
         
         int shape = tab->shape;
         if (widget && !(widget->parentWidget() &&
             qobject_cast<QTabWidget*>(widget->parentWidget()))) {
            //NOTICE: this is elegant but NOT invariant against enum changes!!!
            shape%2 ? --shape : ++shape;
         }
         
         switch((QTabBar::Shape)shape) {
         case QTabBar::RoundedNorth:
         case QTabBar::TriangularNorth:
            if (selected)
               tr.setTop(tr.top()+dpi._2);
            break;
         case QTabBar::RoundedSouth:
         case QTabBar::TriangularSouth:
            bottom = true;
            if (selected)
               tr.setBottom(tr.bottom()-2);
            break;
         case QTabBar::RoundedEast:
         case QTabBar::TriangularEast:
            if (selected) tr.setRight(tr.right()-2);
            verticalTabs = true; east = true;
            break;
         case QTabBar::RoundedWest:
         case QTabBar::TriangularWest:
            if (selected) tr.setLeft(tr.left()+2);
            verticalTabs = true;
            break;
         }
         
         if (verticalTabs) {
            int newX, newY, newRot;
            if (east) {
               newX = tr.width(); newY = tr.y(); newRot = 90;
            }
            else {
               newX = 0; newY = tr.y() + tr.height(); newRot = -90;
            }
            tr.setRect(0, 0, tr.height(), tr.width());
            QMatrix m;
            m.translate(newX, newY); m.rotate(newRot);
            painter->setMatrix(m, true);
         }
         
         if (!tabV2.icon.isNull()) {
            QSize iconSize = tabV2.iconSize;
            if (!iconSize.isValid()) {
               int iconExtent = pixelMetric(PM_SmallIconSize);
               iconSize = QSize(iconExtent, iconExtent);
            }
            QPixmap tabIcon = tabV2.icon.pixmap(iconSize, (isEnabled) ?
                  QIcon::Normal : QIcon::Disabled);
            painter->drawPixmap(tr.left() + 6,
                                tr.center().y() - tabIcon.height() / 2, tabIcon);
            tr.setLeft(tr.left() + iconSize.width() + 4);
         }
         
         // color adjustment
         QColor cF, cB;
         if (selected) {
            cF = COLOR(WindowText);
            cB = COLOR(Window);
         }
         else {
            cB = PAL.color(config.role_tab[0]);
            cF = hover ? PAL.color(config.role_tab[1]) :
               midColor(cB, PAL.color(config.role_tab[1]), 1,2);
         }
         // dark background, let's paint an emboss
         if (qGray(cB.rgb()) < 148) {
            painter->setPen(cB.dark(120));
            tr.moveTop(tr.top()-1);
            drawItemText(painter, tr, alignment, PAL, isEnabled, tab->text);
            tr.moveTop(tr.top()+1);
         }
         painter->setPen(cF);
         drawItemText(painter, tr, alignment, PAL, isEnabled, tab->text);
         
         painter->restore();
      }
      break;
      case CE_ProgressBar: // CE_ProgressBarGroove, CE_ProgressBarContents, CE_ProgressBarLabel
      if (const QStyleOptionProgressBar *pb
          = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
//          if (animationUpdate) {
//             painter->save();
//             painter->setClipRect(RECT.adjusted(dpi._3,dpi._4,-dpi._3,-dpi._5),
//                                  Qt::IntersectClip);
//          }
         QStyleOptionProgressBarV2 subopt = *pb;
         // groove
//          subopt.rect = subElementRect(SE_ProgressBarGroove, pb, widget);
         drawControl(CE_ProgressBarGroove, pb, painter, widget);
         // contents
         subopt.rect = subElementRect(SE_ProgressBarContents, pb, widget);
         drawControl(CE_ProgressBarContents, &subopt, painter, widget);
#if 1
         // label?
         if (pb->textVisible) {
            subopt.rect = subElementRect(SE_ProgressBarLabel, pb, widget);
            drawControl(CE_ProgressBarLabel, &subopt, painter, widget);
         }
#endif
//          if (animationUpdate)
//             painter->restore();
      }
      break;
   case CE_ProgressBarGroove: // The groove where the progress indicator is drawn in a QProgressBar
      if (const QStyleOptionProgressBarV2 *pb =
            qstyleoption_cast<const QStyleOptionProgressBarV2*>(option)) {
         int size = RECT.height();
         Qt::Orientation o = Qt::Vertical;
         if (pb->orientation == Qt::Vertical) {
            size = RECT.width();
            o = Qt::Horizontal;
         }
         const QPixmap &groove = Gradients::pix(PAL.color(config.role_progress[0]), size,
                                          o, config.glassProgress ? Gradients::Glass : Gradients::Sunken);
         fillWithMask(painter, RECT.adjusted(0,0,0,-dpi._2), groove, &masks.button, Tile::Full);
//          if (!animationUpdate)
//             shadows.lineEdit[isEnabled].render(RECT, painter);
      }
      break;
   case CE_ProgressBarContents: // The progress indicator of a QProgressBar
      if (const QStyleOptionProgressBarV2 *pb =
            qstyleoption_cast<const QStyleOptionProgressBarV2*>(option)) {
         double val = pb->progress; val /= (pb->maximum - pb->minimum);
         if (val == 0.0)
            break;
         QRect r = RECT; int size = r.height();
         Qt::Orientation o = Qt::Vertical;
         bool reverse = option->direction == Qt::RightToLeft;
         if (pb->invertedAppearance)
            reverse = !reverse;
         int step = animator->progressStep(widget);
         if (pb->orientation == Qt::Vertical) {
            size = r.width();
            o = Qt::Horizontal;
            r.setTop(r.bottom() -
                  (int)(val*RECT.height())+1);
         }
         else if (reverse) {
            r.setLeft(r.right() -
                  (int)(val*RECT.width())+1);
         }
         else {
            r.setRight(r.left() +
                     (int)(val*RECT.width())-1);
         }
         const bool unicolor = config.role_progress[0] == config.role_progress[1];
         const QColor c1 = (pb->progress == pb->maximum || unicolor ) ?
               PAL.color(config.role_progress[0]) :
               midColor( PAL.color(config.role_progress[0]),
                         PAL.color(config.role_progress[1]), 10, 10+step/2);
         const QColor c2 = unicolor ?
               PAL.color(config.role_progress[0]).light(85+step) :
               midColor( PAL.color(config.role_progress[0]),
                                     PAL.color(config.role_progress[1]), 1, 2);
         const QBrush chunk1 = Gradients::brush(c1, size, o, config.gradProgress);
         const QBrush chunk2 = Gradients::brush(c2, size, o, config.gradProgress);
         
         QPixmap pix; QPainter p;
         if (pb->orientation == Qt::Horizontal) {
            pix = QPixmap(2*size, size);
            p.begin(&pix);
            p.fillRect(0,0,size,size, chunk1);
            p.fillRect(size,0,size,size, chunk2);
         }
         else {
            pix = QPixmap(size, 2*size);
            p.begin(&pix);
            p.fillRect(0,0,size,size, chunk1);
            p.fillRect(0,size,size,size, chunk2);
         }
         p.end();
         
         fillWithMask(painter, r, pix, &masks.button/*, Tile::Full, false, off*/);
      }
      break;
   case CE_ProgressBarLabel: // The text label of a QProgressBar
//       break;
      if (const QStyleOptionProgressBarV2 *progress =
          qstyleoption_cast<const QStyleOptionProgressBarV2*>(option)) {
//          if (!animationUpdate)
         painter->save();
         QFont fnt = painter->font();
         fnt.setBold(true);
         painter->setFont(fnt);
         QRect rect = RECT;
         double val = progress->progress;
         bool reverse = option->direction == Qt::RightToLeft;
         if (progress->invertedAppearance) reverse = !reverse;
         val = val / (progress->maximum - progress->minimum);
         QMatrix m;
         if (progress->orientation == Qt::Vertical) {
            rect.setRect(RECT.x(), RECT.y(), RECT.height(), RECT.width());
            if (progress->bottomToTop) {
               m.translate(0.0, RECT.height()); m.rotate(-90);
            }
            else {
               m.translate(RECT.width(), 0.0); m.rotate(90);
            }
         }
         if ( val > 0.0 ) {
            int s;
            QRect cr;
            if (progress->orientation == Qt::Vertical) {
               s = qMin( RECT.height(), ( int ) (val * RECT.height() ) );
               if ( s > 1 )
                  cr = QRect(RECT.x(), RECT.bottom()-s+1, RECT.width(), s);
            }
            else {
               s = qMin( RECT.width(), ( int ) (val * RECT.width() ) );
               if ( s > 1 ) {
//                   QRect progressRect = RECT;
                  cr = RECT;
                  if (reverse) {
                     cr.setLeft(RECT.right()-s+1);
                     cr.setWidth(s);
//                      int left = progressRect.x()-progressRect.height();
//                      for (int i = 0; i < progressRect.height(); i++)
//                         cr += QRect(left+i,progressRect.y()+i,progressRect.height(),1);
                  }
                  else
//                   {
                     cr.setWidth(s);
/*                     int right = progressRect.x()+progressRect.width();
                     for (int i = 0; i < progressRect.height(); i++)
                        cr += QRect(right-i-1, progressRect.y()+i, progressRect.height(),1);
                  }
                  cr = QRegion(progressRect) - cr;*/
               }
            }
//             painter->setClipRegion(cr);
            painter->setClipRect(cr);
            painter->setMatrix(m);
            drawItemText(painter, rect, Qt::AlignCenter | Qt::TextSingleLine, PAL, isEnabled,
                         progress->text, QPalette::WindowText); // config.role_progress[4]
            painter->resetMatrix();
            painter->setClipRegion(QRegion(RECT).subtract(cr));
         }
         painter->setMatrix(m);
         drawItemText(painter, rect, Qt::AlignCenter | Qt::TextSingleLine, PAL, isEnabled,
                      progress->text, QPalette::WindowText); // config.role_progress[3]
         painter->restore();
      }
      break;
   case CE_ToolButtonLabel: // A tool button's label
      if (const QStyleOptionToolButton *toolbutton
          = qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
         // Arrow type always overrules and is always shown
         bool hasArrow = toolbutton->features & QStyleOptionToolButton::Arrow;
         if ((!hasArrow && toolbutton->icon.isNull()) && !toolbutton->text.isEmpty() ||
             toolbutton->toolButtonStyle == Qt::ToolButtonTextOnly) {
            drawItemText(painter, RECT,
                         Qt::AlignCenter | Qt::TextShowMnemonic, PAL,
                         isEnabled, toolbutton->text, QPalette::WindowText);
         }
         else {
            QPixmap pm;
            QSize pmSize = toolbutton->iconSize;
            if (!toolbutton->icon.isNull()) {
               QIcon::State state = toolbutton->state & State_On ? QIcon::On : QIcon::Off;
               QIcon::Mode mode;
               if (!isEnabled)
                  mode = QIcon::Disabled;
               else if (hover && (option->state & State_AutoRaise))
                  mode = QIcon::Active;
               else
                  mode = QIcon::Normal;
               pm = toolbutton->icon.pixmap(RECT.size().boundedTo(toolbutton->iconSize), mode, state);
               pmSize = pm.size();
            }
                    
            if (toolbutton->toolButtonStyle != Qt::ToolButtonIconOnly) {
               painter->setFont(toolbutton->font);
               QRect pr = RECT, tr = RECT;
               int alignment = Qt::TextShowMnemonic;
               
               if (toolbutton->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
                  int fh = painter->fontMetrics().height();
                  pr.adjust(0, dpi._3, 0, -fh - dpi._5);
                  tr.adjust(0, pr.bottom(), 0, -dpi._3);
                  if (!hasArrow)
                     drawItemPixmap(painter, pr, Qt::AlignCenter, pm);
                  else
                     drawArrow(this, toolbutton, pr, painter, widget);
                  alignment |= Qt::AlignCenter;
               }
               else {
                  pr.setWidth(pmSize.width() + dpi._8);
                  tr.adjust(pr.right(), 0, 0, 0);
                  if (!hasArrow)
                     drawItemPixmap(painter, pr, Qt::AlignCenter, pm);
                  else
                     drawArrow(this, toolbutton, pr, painter, widget);
                  alignment |= Qt::AlignLeft | Qt::AlignVCenter;
               }
               drawItemText(painter, tr, alignment, PAL, isEnabled, toolbutton->text, QPalette::WindowText);
            }
            else {
               if (hasArrow)
                  drawArrow(this, toolbutton, RECT.adjusted(dpi._5,dpi._5,-dpi._5,-dpi._5), painter, widget);
               else
                  drawItemPixmap(painter, RECT, Qt::AlignCenter, pm);
            }
         }
      }
      break;
   case CE_MenuBarItem: // A menu item in a QMenuBar
      if (const QStyleOptionMenuItem *mbi =
          qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
         QPalette::ColorRole cr = QPalette::WindowText;
         IndexedFadeInfo *info = 0;
         QAction *action = 0, *activeAction = 0;
         int step = 0;
         if (widget)
         if (const QMenuBar* mbar = qobject_cast<const QMenuBar*>(widget)) {
            action = mbar->actionAt(RECT.topLeft()); // is the action for this item!
            activeAction = mbar->activeAction();
            info = const_cast<IndexedFadeInfo *>
               (animator->indexedFadeInfo(widget, (long int)activeAction));
         }
         if (info && (!activeAction || !activeAction->menu() ||
             activeAction->menu()->isHidden()))
            step = info->step((long int)action);
         if (step || option->state & State_Selected) {
            QRect r = RECT.adjusted(0, dpi._2, 0, -dpi._4);
            if (step) {
               step = 6-step;
               int dx = step*r.width()/18/*, dy = step*r.height()/18*/;
               r.adjust(dx, 0, -dx, -0);
            }
            
            const QBrush fill =
               Gradients::brush(sunken ?
                              midColor(COLOR(Highlight), COLOR(Window), 1, 3) :
                              COLOR(Window), r.height(), Qt::Vertical,
                              config.gradChoose);
            fillWithMask(painter, r, fill, &masks.tab);
            r.setHeight(r.height()+dpi._2);
            shadows.tabSunken.render(r, painter);
            cr = config.role_popup[1];
         }
         QPixmap pix =
                mbi->icon.pixmap(pixelMetric(PM_SmallIconSize), isEnabled ?
                                 QIcon::Normal : QIcon::Disabled);
         const uint alignment =
                Qt::AlignCenter | Qt::TextShowMnemonic |
                Qt::TextDontClip | Qt::TextSingleLine;
         if (!pix.isNull())
            drawItemPixmap(painter,mbi->rect, alignment, pix);
         else
            drawItemText(painter, mbi->rect, alignment, mbi->palette, isEnabled, mbi->text, cr);
      }
      break;
   case CE_MenuBarEmptyArea: // The empty area of a QMenuBar
      break;
   case CE_MenuItem: // A menu item in a QMenu
        // Draws one item in a popup menu.
      if (const QStyleOptionMenuItem *menuItem =
          qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
             
         QPalette::ColorRole fgr = QPalette::WindowText;
//          QPalette::ColorRole bgr = QPalette::Window;
         // separator
         if (menuItem->menuItemType == QStyleOptionMenuItem::Separator) {
            int dx = RECT.width()/6,
            dy = (RECT.height()-shadows.line[0][Sunken].thickness())/2;
            shadows.line[0][Sunken].render(RECT.adjusted(dx,dy,-dx,-dy), painter);
            if (!menuItem->text.isEmpty()) {
               painter->setFont(menuItem->font);
               drawItemText(painter, RECT, Qt::AlignCenter, PAL, isEnabled,
                            menuItem->text, fgr);
            }
            break;
         }
         
         bool selected = menuItem->state & State_Selected;
         
         QColor bg = midColor(COLOR(Highlight), COLOR(Window), 1,
                              sunken ? 1 : 3);
         QColor fg = isEnabled ? COLOR(WindowText) :
                midColor(COLOR(Window), COLOR(WindowText), 2,1);

         painter->save();
         bool checkable =
               (menuItem->checkType != QStyleOptionMenuItem::NotCheckable);
         bool checked = checkable && menuItem->checked;
         
         if (selected && isEnabled) {
            QRect r = RECT.adjusted(0,0,0,-dpi._2);
            fillWithMask(painter, r,
                         Gradients::brush(bg, r.height(), Qt::Vertical,
                                        config.gradChoose), &masks.tab, Tile::Full);
            shadows.tabSunken.render(RECT, painter);
         }

         // Text and icon, ripped from windows style
         const QStyleOptionMenuItem *menuitem = menuItem;
         int iconCol = config.showMenuIcons*menuitem->maxIconWidth;
         
         if (config.showMenuIcons && !menuItem->icon.isNull()) {
            QRect vCheckRect = visualRect(option->direction, RECT,
                                       QRect(RECT.x(), RECT.y(), iconCol, RECT.height()));
            QIcon::Mode mode = isEnabled ? (selected ? QIcon::Active :
                  QIcon::Normal) : QIcon::Disabled;
            QPixmap pixmap = menuItem->icon.pixmap(pixelMetric(PM_SmallIconSize),
                  mode, checked ? QIcon::On : QIcon::Off);
            
            QRect pmr(QPoint(0, 0), pixmap.size());
            pmr.moveCenter(vCheckRect.center());
            
            painter->drawPixmap(pmr.topLeft(), pixmap);
         }
             
         painter->setPen ( fg );
         painter->setBrush ( Qt::NoBrush );
         
         int x, y, w, h;
         RECT.getRect(&x, &y, &w, &h);
         int tab = menuitem->tabWidth;
         int cDim = 2*(RECT.height() - dpi._4)/3;
         int xm = windowsItemFrame + iconCol + windowsItemHMargin;
         int xpos = RECT.x() + xm;
         QRect textRect(xpos, y + windowsItemVMargin,
                        w - xm - menuItem->menuHasCheckableItems*(cDim+dpi._7) -
                              windowsRightBorder - tab + 1,
                        h - 2 * windowsItemVMargin);
         QRect vTextRect = visualRect(option->direction, RECT, textRect);
         QString s = menuitem->text;
         if (!s.isEmpty()) {
            // draw text
            int t = s.indexOf('\t');
            const int text_flags = Qt::AlignVCenter | Qt::TextShowMnemonic |
                  Qt::TextDontClip | Qt::TextSingleLine;
            if (t >= 0) {
               QRect vShortcutRect = visualRect(option->direction, RECT,
                     QRect(textRect.topRight(),
                           QPoint(textRect.right()+tab, textRect.bottom())));
               painter->drawText(vShortcutRect, text_flags | Qt::AlignLeft, s.mid(t + 1));
               s = s.left(t);
            }
            if (menuitem->menuItemType == QStyleOptionMenuItem::DefaultItem) {
               QFont font = menuitem->font;
               font.setBold(true);
               painter->setFont(font);
            }
            painter->drawText(vTextRect, text_flags | Qt::AlignLeft, s.left(t));
         }
         // Arrow
         if (menuItem->menuItemType == QStyleOptionMenuItem::SubMenu) {
            // draw sub menu arrow
            PrimitiveElement arrow = (option->direction == Qt::RightToLeft) ?
                  PE_IndicatorArrowLeft : PE_IndicatorArrowRight;
            
            int dim = RECT.height()/3;
            xpos = RECT.x() + RECT.width() - dpi._7 - dim;
            
            QStyleOptionMenuItem tmpOpt = *menuItem;
            tmpOpt.rect = visualRect(option->direction, RECT,
                                     QRect(xpos, RECT.y() +
                                           (RECT.height() - dim)/2, dim, dim));
            painter->setPen(midColor(bg, fg, 1, 3));
            drawPrimitive(arrow, &tmpOpt, painter, widget);
         }
         else if (checkable) { // Checkmark
            xpos = RECT.right() - dpi._7 - cDim;
            QRect checkRect(xpos, RECT.y() + (RECT.height() - cDim)/2, cDim, cDim);
            checkRect = visualRect(menuItem->direction, menuItem->rect, checkRect);
            if (menuItem->checkType & QStyleOptionMenuItem::Exclusive) {
               // Radio button
               painter->setRenderHint ( QPainter::Antialiasing );
               painter->drawEllipse ( checkRect );
               if (checked || sunken) {
                  painter->setBrush ( fg );
                  painter->drawEllipse ( checkRect.adjusted(checkRect.width()/4,
                                         checkRect.height()/4, -checkRect.width()/4,
                                         -checkRect.height()/4) );
//                   painter->setBrush ( Qt::NoBrush );
               }
            }
            else {
               // Check box
//                painter->setBrush ( Qt::NoBrush );
               QStyleOptionMenuItem tmpOpt = *menuItem;
               tmpOpt.rect = checkRect;
               tmpOpt.state &= ~State_Selected; // cause of color, not about checkmark!
               if (checked) {
                  tmpOpt.state |= State_On;
                  tmpOpt.state &= ~State_Off;
               }
               else {
                  tmpOpt.state |= State_Off;
                  tmpOpt.state &= ~State_On;
               }
               drawPrimitive(PE_IndicatorMenuCheckMark, &tmpOpt, painter, widget);
            }
         }
         painter->restore();
      }
      break;
   case CE_MenuScroller: { // Scrolling areas in a QMenu when the style supports scrolling
      QPalette::ColorRole bg = widget ? widget->backgroundRole() : QPalette::Window;
      if (option->state & State_DownArrow) {
         painter->drawTiledPixmap(RECT, Gradients::pix(PAL.color(QPalette::Active, bg),
            RECT.height()*2, Qt::Vertical, sunken ? Gradients::Sunken : Gradients::Button), QPoint(0,RECT.height()));
         drawPrimitive(PE_IndicatorArrowDown, option, painter, widget);
      }
      else {
         painter->drawTiledPixmap(RECT, Gradients::pix(PAL.color(QPalette::Active, bg),
            RECT.height()*2, Qt::Vertical, sunken ? Gradients::Sunken : Gradients::Button));
         drawPrimitive(PE_IndicatorArrowUp, option, painter, widget);
      }
      break;
   }
//    case CE_MenuTearoff: // A menu item representing the tear off section of a QMenu
   case CE_MenuEmptyArea: // The area in a menu without menu items
   case CE_MenuHMargin: // The horizontal extra space on the left/right of a menu
   case CE_MenuVMargin: { // The vertical extra space on the top/bottom of a menu
//       QPalette::ColorRole role = QPalette::Window;
//       if (widget)
//          role = qobject_cast<const QComboBox*>(widget) ?
//          QPalette::WindowText : widget->backgroundRole();
//       painter->fillRect(RECT, PAL.brush(role));
      break;
   }
   case CE_Q3DockWindowEmptyArea: // The empty area of a QDockWidget
      break;
   case CE_ToolBoxTab: // The toolbox's tab area
      if (const QStyleOptionToolBox* tbt =
          qstyleoption_cast<const QStyleOptionToolBox*>(option)) {
         
         // Qt does some funky color updates on the toolboxbutton palette, leading to button color on init display, so i just skip it and use daddies palette...
         const QPalette &pal = (widget && widget->parentWidget()) ?
               widget->parentWidget()->palette() : PAL;
         
         QPalette::ColorRole role = QPalette::WindowText, bgRole = QPalette::Window;
         if (isEnabled) {
            if (option->state & State_Selected) {
               const QBrush fill = Gradients::brush(pal.color(QPalette::WindowText),
                                              RECT.height(), Qt::Vertical,
                                              config.gradChoose);
               painter->fillRect(RECT, fill);
               QFont f(painter->font());
               f.setBold(true);
               painter->setFont(f);
               bgRole = QPalette::WindowText;
               role = QPalette::Window;
            }
            else {
               painter->fillRect(RECT, Gradients::brush(pal.color(QPalette::Window),
                  RECT.height(), Qt::Vertical, sunken ?
                  Gradients::Sunken : config.gradChoose));
            }
         }
/*========================================================================
         i WANT (read this TrottelTech: WANT!) this to be color swapped on
         select (and centered as sugar on top)
         now as the toolboxbutton is a private class and it's selected member is
         as well, i cannot overwrite the paint event
         so instead i respond a null rect for selected tabs contents
         (from subElementRect query), what makes the widget abort the content
         painting - then i paint it instead... works ;)
========================================================================*/
         const QPixmap &pm =
               tbt->icon.pixmap(pixelMetric(QStyle::PM_SmallIconSize),
                                isEnabled ? QIcon::Normal : QIcon::Disabled);
         
         QRect cr = option->rect.adjusted(0, 0, -30, 0);
         QRect tr, ir;
         int ih = 0;
         if (pm.isNull()) {
            tr = cr;
            tr.adjust(4, 0, -8, 0);
         }
         else {
            int iw = pm.width() + 4;
            ih = pm.height();
            ir = QRect(cr.left() + 4, cr.top(), iw + 2, ih);
            tr = QRect(ir.right(), cr.top(), cr.width() - ir.right() - 4, cr.height());
         }
         
         QString txt = tbt->text;
         if (painter->fontMetrics().width(txt) > tr.width()) {
            txt = txt.left(1);
            int ew = painter->fontMetrics().width("...");
            int i = 1;
            while (painter->fontMetrics().width(txt) + ew + painter->fontMetrics().width(tbt->text[i])  < tr.width())
               txt += tbt->text[i++];
            txt += "...";
         }
         
         if (ih)
            painter->drawPixmap(ir.left(), (RECT.height() - ih) / 2, pm);

         // dark background, let's paint an emboss
         if (qGray(pal.color(bgRole).rgb()) < 128) {
            painter->save();
            painter->setPen(pal.color(bgRole).dark(120));
            tr.moveTop(tr.top()-1);
            drawItemText(painter, tr, Qt::AlignCenter | Qt::TextShowMnemonic,
                         pal, isEnabled, txt);
            tr.moveTop(tr.top()+1);
            painter->restore();
         }

         drawItemText(painter, tr, Qt::AlignCenter | Qt::TextShowMnemonic,
                      pal, isEnabled, txt, role);
      }
      break;
   case CE_SizeGrip: {
      painter->save();
      int x1, y1, x2, y2;
      RECT.getRect(&x1, &y1, &x2, &y2);
      int dx = x2/4, dy = y2/4;

      Qt::Corner corner;
      if (const QStyleOptionSizeGrip *sgOpt =
         qstyleoption_cast<const QStyleOptionSizeGrip *>(option))
         corner = sgOpt->corner;
      else if (option->direction == Qt::RightToLeft)
         corner = Qt::BottomLeftCorner;
      else
         corner = Qt::BottomRightCorner;
      
      x2 += x1; y2 += y1; // turn w/h into coords
      switch (corner) {
      default:
      case Qt::BottomRightCorner:
         break;
      case Qt::BottomLeftCorner: {
         int help = y2; y2 = y1; y1 = help; // swap y1/y2
         dx = -dx;
         break;
      }
      case Qt::TopRightCorner: {
         int help = y2; y2 = y1; y1 = help; // swap y1/y2
         dy = -dy;
         break;
      }
      case Qt::TopLeftCorner: {
         int help = x2; x2 = x1; x1 = help; // swap x1/x2
         help = y2; y2 = y1; y1 = help; // swap y1/y2
         dx = -dx; dy = -dy; // invert both directions
      }
      }
      
      for (int i = 1; i < 5; ++i) {
         painter->setPen(QPen(midColor(COLOR(Window), COLOR(WindowText), 14, 11-2*i), dpi._1));
         painter->drawLine(x1, y2, x2, y1);
         x1 += dx; y1 += dy;
      }
      
      painter->restore();
      break;
   }
   case CE_Header: // A header
   if (const QStyleOptionHeader *header =
       qstyleoption_cast<const QStyleOptionHeader *>(option)) {
      // init
      const QRegion clipRegion = painter->clipRegion();
      painter->setClipRect(option->rect, Qt::IntersectClip);
      QStyleOptionHeader subopt = *header;
      const QHeaderView* hdv = qobject_cast<const QHeaderView*>(widget);
      // extend the sunken state on sorting headers
      sunken = sunken ||
             (hdv && hdv->isClickable () && hdv->sortIndicatorSection() == header->section);
      if (sunken)
         subopt.state |= State_Sunken;
      
      // base
      drawControl(CE_HeaderSection, &subopt, painter, widget);
          
      // label
      subopt.rect = subElementRect(SE_HeaderLabel, header, widget);
      if (subopt.rect.isValid())
         drawControl(CE_HeaderLabel, &subopt, painter, widget);
          
      // sort Indicator on sorting or (inverted) on hovered headers
      if (hover && hdv && hdv->isSortIndicatorShown()) {
         if (subopt.sortIndicator == QStyleOptionHeader::SortDown)
            subopt.sortIndicator = QStyleOptionHeader::SortUp;
          else
            subopt.sortIndicator = QStyleOptionHeader::SortDown;
      }
      if (subopt.sortIndicator != QStyleOptionHeader::None) {
         subopt.rect = subElementRect(SE_HeaderArrow, option, widget);
         painter->save();
         if (hover)
            painter->setPen(COLOR(Text));
         else
            painter->setPen(Qt::NoPen);
         const QPixmap &fill = Gradients::pix(hover?COLOR(Base):midColor(COLOR(Text),COLOR(Base)), RECT.height(), Qt::Vertical, sunken?Gradients::Sunken:Gradients::Button);
         painter->setBrush(fill);
         drawPrimitive(PE_IndicatorHeaderArrow, &subopt, painter, widget);
         painter->restore();
      }
      painter->setClipRegion(clipRegion);
      break;
   }
   case CE_HeaderSection: { // A header section
      if (sunken)
         painter->drawTiledPixmap(RECT, Gradients::pix(COLOR(Text), RECT.height(), Qt::Vertical, Gradients::Sunken));
      else {
         QRect r = RECT; r.setWidth(RECT.width()-1);
         painter->drawTiledPixmap(r, Gradients::pix(COLOR(Text), RECT.height(), Qt::Vertical, Gradients::Button));
         r = RECT; r.setLeft(r.right()-dpi._1);
         painter->drawTiledPixmap(r, Gradients::pix(COLOR(Text), RECT.height(), Qt::Vertical, Gradients::Sunken));
      }
      break;
   }
   case CE_HeaderLabel: { // The header's label
      const QStyleOptionHeader* hopt = qstyleoption_cast<const QStyleOptionHeader*>(option);
      QRect rect = option->rect;
      if ( !hopt->icon.isNull() ) {
         QPixmap pixmap = hopt->icon.pixmap( 22,22, isEnabled ? QIcon::Normal : QIcon::Disabled );
         int pixw = pixmap.width();
         int pixh = pixmap.height();
         
         QRect pixRect = option->rect;
         pixRect.setY( option->rect.center().y() - (pixh - 1) / 2 );
         // "pixh - 1" because of tricky integer division
         drawItemPixmap ( painter, pixRect, Qt::AlignCenter, pixmap );
         rect.setLeft( rect.left() + pixw + 2 );
      }
      if (hopt->text.isEmpty())
         break;
      painter->save();
      QColor bg = COLOR(Text), fg = COLOR(Base);
      if (qGray(bg.rgb()) < 148) { // dark background, let's paint an emboss
         rect.moveTop(rect.top()-1);
         painter->setPen(bg.dark(120));
         drawItemText ( painter, rect, Qt::AlignCenter, PAL, isEnabled, hopt->text);
         rect.moveTop(rect.top()+1);
      }
      painter->setPen(fg);
      drawItemText ( painter, rect, Qt::AlignCenter, PAL, isEnabled, hopt->text);
      painter->restore();
      break;
   }
   case CE_ScrollBarAddLine: // ======= scroll down
      if (option->state & State_Item) { // combobox scroller
         painter->save();
         painter->setPen(hover?COLOR(Text):midColor(COLOR(Base),COLOR(Text)));
         QStyleOption opt = *option;
         opt.rect = RECT.adjusted(RECT.width()/4, RECT.height()/4,
                                  -RECT.width()/4, -RECT.height()/4);
         if (option->state & QStyle::State_Horizontal)
            drawPrimitive (PE_IndicatorArrowRight, &opt, painter, widget);
         else
            drawPrimitive (PE_IndicatorArrowDown, &opt, painter, widget);
         painter->restore();
         break;
      }
   case CE_ScrollBarSubLine: // ======= scroll up
      if (option->state & State_Item) { // combobox scroller
         painter->save();
         painter->setPen(hover?COLOR(Text):midColor(COLOR(Base),COLOR(Text)));
         QStyleOption opt = *option;
         opt.rect = RECT.adjusted(RECT.width()/4, RECT.height()/4,
                                  -RECT.width()/4, -RECT.height()/4);
         if (option->state & QStyle::State_Horizontal)
            drawPrimitive (PE_IndicatorArrowLeft, &opt, painter, widget);
         else
            drawPrimitive (PE_IndicatorArrowUp, &opt, painter, widget);
         painter->restore();
         break;
      }
      if (const QStyleOptionSlider *opt =
            qstyleoption_cast<const QStyleOptionSlider *>(option)) {
         bool alive = isEnabled && ((element == CE_ScrollBarAddLine &&
                                     opt->sliderValue < opt->maximum) ||
                                    (element == CE_ScrollBarSubLine &&
                                     opt->sliderValue > opt->minimum));
         hover = hover && alive;
         QPoint xy = RECT.topLeft();
         if (sunken || !alive)
            painter->drawPixmap(xy+QPoint(dpi._1,dpi._1), shadows.radio[1][hover]);
         else
            painter->drawPixmap(xy, shadows.radio[0][hover]);
         xy += QPoint(dpi._2,dpi._1);
         int sz = dpi.ExclusiveIndicator - dpi._4;
         fillWithMask(painter, xy,
                      Gradients::brush(btnBgColor(PAL, alive, hover, complexStep),
                                     sz, Qt::Vertical, alive ? config.gradButton :
                                     Gradients::Sunken), masks.radio);
         break;
      }
   case CE_ScrollBarSubPage: // Scroll bar page decrease indicator (i.e., page up).
   case CE_ScrollBarAddPage: {// Scolllbar page increase indicator (i.e., page down).
      if (option->state & State_Item) // combobox scroller
         break;
      
      SAVE_PEN;
      int step = widgetStep;
      if (!step && (hover || scrollAreaHovered_))
         step = 6;
      const QColor c =
         midColor(COLOR(Highlight), COLOR(WindowText), step, 8-step);
      painter->setPen(QPen(midColor(COLOR(Window), c, 4, 1), dpi._3));
      if (option->state & QStyle::State_Horizontal) {
         const int y = RECT.center().y();
         painter->drawLine(RECT.x(), y, RECT.right(), y);
      }
      else {
         const int x = RECT.center().x();
         painter->drawLine(x, RECT.y(), x, RECT.bottom());
      }
      RESTORE_PEN;
      break;
   }
   case CE_ScrollBarSlider: // Scroll bar slider.
      if (option->state & State_Item) {
         painter->fillRect(RECT.adjusted(dpi._2, 0, -dpi._2, 0),
                           (hover || sunken) ? COLOR(Text) :
                                 midColor(COLOR(Base), COLOR(Text), 8, 1));
         break;
      }
      if (/*const QStyleOptionSlider *opt =*/
          qstyleoption_cast<const QStyleOptionSlider *>(option)) {
  
         // the groove TODO: might be midColor(bg, fg) is better?!
         // (if min == max, i.e. no slide usefull)
         if (!isEnabled) {
            SAVE_PEN;
            int step = widgetStep;
            if (!step && (hover || scrollAreaHovered_))
               step = 6;
            const QColor c =
               midColor(COLOR(Highlight), COLOR(WindowText), step, 8-step);
            painter->setPen(QPen(midColor(COLOR(Window), c, 4, 1), dpi._3));
            if (option->state & QStyle::State_Horizontal) {
               const int y = RECT.center().y();
               painter->drawLine(RECT.x(), y, RECT.right(), y);
            }
            else {
               const int x = RECT.center().x();
               painter->drawLine(x, RECT.y(), x, RECT.bottom());
            }
            RESTORE_PEN;
            break;
         }
         
             
         // we need to paint a slider
         const int _1 = dpi._1, _2 = dpi._2;
         int step = sunken ? 0 : complexStep;
         QRect r = RECT;
      
         // shadow
         if (sunken) {
            r.adjust(_1, _1, -_1, -_2);
            shadows.tab[true][true].render(r, painter);
            r.adjust(_1, _1, -_1, -_1);
         }
         else {
            shadows.tab[true][false].render(r, painter);
            r.adjust(_2, _2, -_2, -dpi._3);
         }
      
         // backlight & glas
         QColor c;
         if (sunken || hover || step) {
            if (!step) step = 6;
//             c = midColor(COLOR(Highlight), Qt::white, step, 5);
//             masks.tab.outline(r.adjusted(-_1,-_1,_1,_1), painter, c);
            c = midColor(COLOR(Highlight), COLOR(Window), step, 160);
         }
//          else if (sunken)
//             c = midColor(COLOR(Highlight), COLOR(Window), 1, 30);
         else
            c = COLOR(Window);
         masks.tab.render(r, painter, Gradients::brush(c, r.height(),
            Qt::Vertical, config.gradButton));
         masks.tab.outline(r, painter, Qt::white, true);
      }
      break;
//    case CE_ScrollBarFirst: // Scroll bar first line indicator (i.e., home).
//    case CE_ScrollBarLast: // Scroll bar last line indicator (i.e., end).
   case CE_RubberBand: {// Rubber band used in such things as iconview.
      painter->save();
      QColor c = COLOR(Highlight);
      painter->setPen(c);
      c.setAlpha(100);
      painter->setBrush(c);
      painter->drawRect(RECT.adjusted(0,0,-1,-1));
      painter->restore();
      break;
   }
   case CE_FocusFrame: // Focus Frame that can is style controled.
      break;
   case CE_ComboBoxLabel: // The label of a non-editable QComboBox
      if (const QStyleOptionComboBox *cb =
          qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
         QRect editRect = subControlRect(CC_ComboBox, cb, SC_ComboBoxEditField, widget);
         painter->save();
         painter->setClipRect(editRect);
         // icon
         if (!cb->currentIcon.isNull()) {
            QIcon::Mode mode = isEnabled ? QIcon::Normal
               : QIcon::Disabled;
            QPixmap pixmap = cb->currentIcon.pixmap(cb->iconSize, mode);
            QRect iconRect(editRect);
            iconRect.setWidth(cb->iconSize.width() + 4);
            iconRect = alignedRect(QApplication::layoutDirection(), Qt::AlignLeft | Qt::AlignVCenter, iconRect.size(), editRect);
/*            if (cb->editable)
               painter->fillRect(iconRect, opt->palette.brush(QPalette::Base));*/
            drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
            
            if (cb->direction == Qt::RightToLeft)
               editRect.translate(-4 - cb->iconSize.width(), 0);
            else
               editRect.translate(cb->iconSize.width() + 4, 0);
         }
         // text
         if (!cb->currentText.isEmpty() && !cb->editable) {
            int _3 = dpi._3;
            editRect.adjust(_3,0, -_3, 0);
            painter->setPen(COLOR(Text));
            painter->drawText(editRect, Qt::AlignCenter, cb->currentText);
         }
         painter->restore();
      }
      break;
   case CE_ToolBar:
      break;
   default:
         QCommonStyle::drawControl( element, option, painter, widget );
   } // switch
}
