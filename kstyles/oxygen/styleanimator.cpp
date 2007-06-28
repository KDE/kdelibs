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

#include <QEvent>
#include <QPainter>
#include <QApplication>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QProgressBar>

#include "styleanimator.h"
#ifndef QT_NO_XRENDER
#include "oxrender.h"
#endif
#include "eventkiller.h"

#define ANIMATIONS (activeTabs + progressbars.count() + \
hoverWidgets.count() + complexHoverWidgets.count() + indexedHoverWidgets.count())

#define startTimer if (!timer->isActive()) timer->start(50)

static QHash<QWidget*, uint> progressbars;
typedef QHash<QWidget*, HoverFadeInfo> HoverFades;
static HoverFades hoverWidgets;
typedef QHash<QWidget*, ComplexHoverFadeInfo> ComplexHoverFades;
static ComplexHoverFades complexHoverWidgets;
typedef QHash<QWidget*, IndexedFadeInfo> IndexedFades;
static IndexedFades indexedHoverWidgets;
static QHash<QTabWidget*, TabAnimInfo*> tabwidgets;
static int activeTabs = 0;

bool animationUpdate;

StyleAnimator::StyleAnimator(QObject *parent,
                             TabAnimInfo::TabTransition tabTrans) :
QObject(parent), tabTransition(tabTrans) {
   animationUpdate = false;
   timer = new QTimer( this );
   connect(timer, SIGNAL(timeout()), this, SLOT(updateProgressbars()));
   connect(timer, SIGNAL(timeout()), this, SLOT(updateTabAnimation()));
   connect(timer, SIGNAL(timeout()), this, SLOT(updateFades()));
   connect(timer, SIGNAL(timeout()), this, SLOT(updateComplexFades()));
   connect(timer, SIGNAL(timeout()), this, SLOT(updateIndexedFades()));
}

StyleAnimator::~StyleAnimator(){
   progressbars.clear();
}

void StyleAnimator::addTab(QTabWidget* tab, int currentIndex) {
   if (tabwidgets.contains(tab)) return; // accidental double add
   tabwidgets[tab] = new TabAnimInfo(tab, currentIndex);
   connect(tab, SIGNAL(currentChanged(int)),
           this, SLOT(tabChanged(int)));
   connect(tab, SIGNAL(destroyed(QObject*)),
           this, SLOT(tabDestroyed(QObject*)));
   startTimer;
}

void StyleAnimator::removeTab(QTabWidget* tab) {
   tabwidgets.remove(tab);
   disconnect(tab, SIGNAL(currentChanged(int)),
           this, SLOT(tabChanged(int)));
   disconnect(tab, SIGNAL(destroyed(QObject*)),
           this, SLOT(tabDestroyed(QObject*)));
   if (!ANIMATIONS) timer->stop();
}

void StyleAnimator::addProgressBar(QWidget* progress) {
   if (progressbars.contains(progress)) return; // accidental double add
   progressbars[progress] = 0;
   connect(progress, SIGNAL(destroyed(QObject*)),
           this, SLOT(progressbarDestroyed(QObject*)));
   startTimer;
}

void StyleAnimator::removeProgressBar(QWidget* progress) {
   progressbars.remove(progress);
   disconnect(progress, SIGNAL(destroyed(QObject*)),
              this, SLOT(progressbarDestroyed(QObject*)));
   if (!ANIMATIONS) timer->stop();
}

bool TabAnimInfo::eventFilter( QObject* object, QEvent* event ) {
   if (event->type() != QEvent::Paint || !animStep)
      return false;
   QPainter p((QWidget*)object);
   p.drawPixmap(0,0, tabPix[2]);
   p.end();
   return true;
}

void TabAnimInfo::updatePixmaps(TabTransition tabTransition) {
   switch (tabTransition) {
#ifndef QT_NO_XRENDER
   case CrossFade: // the quotient says "accelerate animation"!
      OXRender::blend(tabPix[1], tabPix[2], 1.1666-0.1666*animStep);
      break;
#endif
   case ScanlineBlend:
   default: {
      QPainter p(&tabPix[2]);
      for (int i = animStep; i < tabPix[2].height(); i+=6)
         p.drawPixmap(0, i, tabPix[1], 0, i, tabPix[1].width(), 1);
      break;
   }
   case SlideIn: {
      //TODO handle different bar positions (currently assumes top)
      QPainter p(&tabPix[2]);
      p.drawPixmap(0, 0, tabPix[0], 0, animStep*tabPix[1].height()/7,
                   tabPix[0].width(), (7-animStep)*tabPix[1].height()/7);
      break;
   }
   case SlideOut: {
      tabPix[2] = tabPix[1];
      //TODO handle different bar positions (currently assumes top)
      QPainter p(&tabPix[2]);
      p.drawPixmap(0, 0, tabPix[0], 0, (7 - animStep) * tabPix[0].height()/7,
                   tabPix[0].width(), animStep*tabPix[0].height()/7);
      break;
   }
   case RollIn: {
      QPainter p(&tabPix[2]);
      int h = (7-animStep)*tabPix[1].height()/14;
      p.drawPixmap(0, 0, tabPix[1], 0, 0, tabPix[1].width(), h);
      p.drawPixmap(0, tabPix[1].height()-h, tabPix[1],
                   0, tabPix[1].height()-h, tabPix[1].width(), h);
      break;
   }
   case RollOut: {
      QPainter p(&tabPix[2]);
      int h = (7-animStep)*tabPix[1].height()/7;
      int y = (tabPix[1].height()-h)/2;
      p.drawPixmap(0, y, tabPix[1], 0, y, tabPix[1].width(), h);
      break;
   }
   case OpenVertically: {
      tabPix[2] = tabPix[1];
      QPainter p(&tabPix[2]);
      int h = animStep*tabPix[0].height()/14;
      p.drawPixmap(0,0,tabPix[0],0,tabPix[0].height()/2-h,
                  tabPix[0].width(),h);
      p.drawPixmap(0,tabPix[0].height()-h,tabPix[0],
                  0,tabPix[0].height()/2,tabPix[0].width(),h);
      break;
   }
   case CloseVertically: {
      QPainter p(&tabPix[2]);
      int h = (7-animStep)*tabPix[1].height()/14;
      p.drawPixmap(0, 0, tabPix[1],
                   0, tabPix[1].height()/2-h, tabPix[1].width(), h);
      p.drawPixmap(0, tabPix[1].height()-h, tabPix[1],
                   0, tabPix[1].height()/2, tabPix[1].width(), h);
      break;
   }
   case OpenHorizontally: {
      tabPix[2] = tabPix[1];
      QPainter p(&tabPix[2]);
      int w = animStep*tabPix[0].width()/14;
      p.drawPixmap(0,0,tabPix[0],tabPix[0].width()/2-w,0,
                  w,tabPix[0].height());
      p.drawPixmap(tabPix[0].width()-w,0,tabPix[0],
                  tabPix[0].width()/2,0,w,tabPix[0].height());
      break;
   }
   case CloseHorizontally: {
      QPainter p(&tabPix[2]);
      int w = (7-animStep)*tabPix[1].width()/14;
      p.drawPixmap(0, 0, tabPix[1],
                   tabPix[1].width()/2-w, 0, w, tabPix[1].height());
      p.drawPixmap(tabPix[1].width()-w, 0, tabPix[1],
                   tabPix[1].width()/2, 0, w, tabPix[1].height());
      break;
   }
   }
}

#include <QStyleOption>

// to get an idea about what the bg of out tabs looks like - seems as if we
// need to paint it
static QPixmap dumpBackground(QWidget *target, const QRect &r, const QStyle *style) {
   if (!target) return QPixmap();
   QPoint zero(0,0);
   QPixmap pix(r.size());
   QWidgetList widgets; widgets << target;
   QWidget *w = target->parentWidget();
   while (w) {
      if (!w->isVisible()) { w = w->parentWidget(); continue; }
      widgets << w;
      if (w->isTopLevel() || w->autoFillBackground()) break;
      w = w->parentWidget();
   }

   QPainter p(&pix);
   const QBrush bg = w->palette().brush(w->backgroundRole());
   if (bg.style() == Qt::TexturePattern)
      p.drawTiledPixmap(pix.rect(), bg.texture(), target->mapTo(w, r.topLeft()));
   else
      p.fillRect(pix.rect(), bg);
   
   if (w->isTopLevel() && w->testAttribute(Qt::WA_StyledBackground)) {
      QStyleOption opt; opt.initFrom(w);// opt.rect = r;
      opt.rect.translate(-target->mapTo(w, r.topLeft()));
      style->drawPrimitive ( QStyle::PE_Widget, &opt, &p, w);
   }
   p.end();

   QPaintEvent e(r); int i = widgets.size();
   while (i) {
      w = widgets.at(--i);
      QPainter::setRedirected( w, &pix, target->mapTo(w, r.topLeft()) );
      e = QPaintEvent(QRect(zero, r.size()));
      QCoreApplication::sendEvent(w, &e);
      QPainter::restoreRedirected(w);
   }
   return pix;
}

// QPixmap::grabWidget(.) currently fails on the background offset,
// so we use our own implementation
//TODO: fix scrollareas (the scrollbars aren't painted, so check for availability and usage...)
static void grabWidget(QWidget * root, QPixmap *pix) {
    if (!root)
        return;

   QPoint zero(0,0);
//    QPainter p(pix);
//    const QBrush bg = root->palette().brush(root->backgroundRole());
//    if (bg.style() == Qt::TexturePattern)
//       p.drawTiledPixmap(root->rect(), bg.texture(),
//                         root->mapTo(root->topLevelWidget(), zero));
//    else
//       p.fillRect(root->rect(), bg);
//    p.end();
   
   QList <QWidget*> widgets = root->findChildren<QWidget*>();
   
   // resizing (in case) -- NOTICE may be dropped for performance...?!
//    if (root->testAttribute(Qt::WA_PendingResizeEvent) ||
//        !root->testAttribute(Qt::WA_WState_Created)) {
//       QResizeEvent e(root->size(), QSize());
//       QApplication::sendEvent(root, &e);
//    }
//    foreach (QWidget *w, widgets) {
//       if (root->testAttribute(Qt::WA_PendingResizeEvent) ||
//          !root->testAttribute(Qt::WA_WState_Created)) {
//          QResizeEvent e(w->size(), QSize());
//          QApplication::sendEvent(w, &e);
//       }
//    }
   // painting
   QPainter::setRedirected( root, pix );
   QPaintEvent e(QRect(zero, root->size()));
   QCoreApplication::sendEvent(root, &e);
   QPainter::restoreRedirected(root);
   
   QPainter p;
   foreach (QWidget *w, widgets) {
      if (w->isVisibleTo(root)) {
         if (w->autoFillBackground()) {
            const QBrush bg = w->palette().brush(w->backgroundRole());
            p.begin(pix);
            QRect wrect = QRect(zero, w->size()).translated(w->mapTo(root, zero));
            if (bg.style() == Qt::TexturePattern)
               p.drawTiledPixmap(wrect, bg.texture(),
                                 w->mapTo(root->topLevelWidget(), zero));
            else
               p.fillRect(wrect, bg);
            p.end();
         }
         QPainter::setRedirected( w, pix, -w->mapTo(root, zero) );
         e = QPaintEvent(QRect(zero, w->size()));
         QCoreApplication::sendEvent(w, &e);
         QPainter::restoreRedirected(w);
      }
   }
}

// --- ProgressBars --------------------
void StyleAnimator::updateProgressbars() {
   if (progressbars.isEmpty())
      return;
   //Update the registered progressbars.
   QHash<QWidget*, uint>::iterator iter;
   QProgressBar *pb;
   animationUpdate = true;
   for (iter = progressbars.begin(); iter != progressbars.end(); iter++) {
      if ( !qobject_cast<QProgressBar*>(iter.key()) )
         continue;
      pb = (QProgressBar*)(iter.key());
      if (pb->paintingActive() || !pb->isVisible() ||
          !(pb->value() > pb->minimum()) || !(pb->value() < pb->maximum()))
         continue;
//       int mod = (pb->orientation() == Qt::Horizontal) ?
//          (pb->height()-dpi.$8)*2 :
//          (pb->width()-dpi.$6)*2;
//       if (!mod)
//          continue;
//       ++iter.value();
//       if (mod)
//          iter.value() %= mod;
      if (iter.value() % 2) { // odd - fade out
         iter.value() -= 2;
         if (iter.value() < 4) // == 3
            ++iter.value(); // 4
         if ((iter.value()+1) % 4) // save some cycles...
            continue;
      }
      else { //fade in
         iter.value() += 2;
         if (iter.value() > 39) // == 40
            ++iter.value(); // 41
         if (iter.value() % 4) // save some cycles...
            continue;
      }
      pb->repaint();
   }
   animationUpdate = false;
}

int StyleAnimator::progressStep(const QWidget *w) const {
   return progressbars.value(const_cast<QWidget*>(w),0);
}

void StyleAnimator::progressbarDestroyed(QObject* obj) {
   progressbars.remove(static_cast<QWidget*>(obj));
   if (!ANIMATIONS) timer->stop();
}

// --- TabWidgets --------------------
void StyleAnimator::tabChanged(int index) {
   if (tabTransition == TabAnimInfo::Jump) return; // ugly nothing ;)
   QTabWidget* tw = (QTabWidget*)sender();
   if (!tw->currentWidget()) return;
   QHash<QTabWidget*, TabAnimInfo*>::iterator i = tabwidgets.find(tw);
   if (i == tabwidgets.end()) // this tab isn't handled for some reason?
      return;
   
   TabAnimInfo* tai = i.value();
   
   QWidget *ctw = tw->widget(tai->lastTab);
   tai->lastTab = index;
   if (!ctw) return;
   tai->tabPix[0] = tai->tabPix[1] =
      dumpBackground(tw, QRect(ctw->mapTo(tw, QPoint(0,0)), ctw->size()),
                     qApp->style());
   grabWidget(ctw, &tai->tabPix[0]);
   tai->tabPix[2] = tai->tabPix[0];
   ctw = tw->currentWidget();
   grabWidget(ctw, &tai->tabPix[1]);
   
   tai->animStep = 6;
   tai->updatePixmaps(tabTransition);
   ctw->parentWidget()->installEventFilter(tai);
   _BLOCKEVENTS_(ctw);
   QList<QWidget*> widgets = ctw->findChildren<QWidget*>();
   foreach(QWidget *widget, widgets) {
      _BLOCKEVENTS_(widget);
      if (widget->autoFillBackground()) {
         tai->autofillingWidgets.append(widget);
         widget->setAutoFillBackground(false);
      }
   }
   ctw->repaint();
   startTimer;
}

void StyleAnimator::updateTabAnimation() {
   if (tabwidgets.isEmpty())
      return;
   QHash<QTabWidget*, TabAnimInfo*>::iterator i;
   activeTabs = 0;
   TabAnimInfo* tai;
   QWidget *ctw = 0, *widget = 0; QList<QWidget*> widgets;
   int index;
   for (i = tabwidgets.begin(); i != tabwidgets.end(); i++) {
      tai = i.value();
      if (!tai->animStep)
         continue;
      ctw = i.key()->currentWidget();
      if (! --(tai->animStep)) { // zero, stop animation
         tai->tabPix[2] =
            tai->tabPix[1] =
            tai->tabPix[0] = QPixmap();
         ctw->parentWidget()->removeEventFilter(tai);
         _UNBLOCKEVENTS_(ctw);
         widgets = ctw->findChildren<QWidget*>();
//          ctw->repaint();
         foreach(widget, widgets) {
            index = tai->autofillingWidgets.indexOf(widget);
            if (index != -1) {
               tai->autofillingWidgets.removeAt(index);
               widget->setAutoFillBackground(true);
            }
            _UNBLOCKEVENTS_(widget);
            widget->update(); //if necessary
         }
         ctw->repaint(); //asap
         tai->autofillingWidgets.clear();
         continue;
      }
      ++activeTabs;
      tai->updatePixmaps(tabTransition);
      ctw->parentWidget()->repaint();
   }
   if (!ANIMATIONS) timer->stop();
}

void StyleAnimator::tabDestroyed(QObject* obj) {
   tabwidgets.remove(static_cast<QTabWidget*>(obj));
//    delete tai;
   if (!ANIMATIONS) timer->stop();
}


// -- Buttons etc. -------------------------------
void StyleAnimator::updateFades() {
   if (hoverWidgets.isEmpty())
      return;
   HoverFades::iterator it = hoverWidgets.begin();
   while (it != hoverWidgets.end()) {
      if (it.value().fadeIn) {
         it.value().step += 2;
         it.key()->update();
         if (it.value().step > 4)
            it = hoverWidgets.erase(it);
         else
            ++it;
      }
      else { // fade out
         --it.value().step;
         it.key()->update();
         if (it.value().step < 1)
            it = hoverWidgets.erase(it);
         else
            ++it;
      }
   }
   if (!ANIMATIONS) timer->stop();
}

void StyleAnimator::fadeIn(QWidget *widget) {
   HoverFades::iterator it = hoverWidgets.find(widget);
   if (it == hoverWidgets.end()) {
      it = hoverWidgets.insert(widget, HoverFadeInfo(1, true));
   }
   it.value().fadeIn = true;
   connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(fadeDestroyed(QObject*)));
   startTimer;
}

void StyleAnimator::fadeOut(QWidget *widget) {
   HoverFades::iterator it = hoverWidgets.find(widget);
   if (it == hoverWidgets.end()) {
      it = hoverWidgets.insert(widget, HoverFadeInfo(6, false));
   }
   it.value().fadeIn = false;
   connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(fadeDestroyed(QObject*)));
   startTimer;
}

void StyleAnimator::fadeDestroyed(QObject* obj) {
   hoverWidgets.remove(static_cast<QWidget*>(obj));
   if (!ANIMATIONS) timer->stop();
}

int StyleAnimator::hoverStep(const QWidget *widget) const {
   if (!widget)
      return 0;
   HoverFades::iterator it = hoverWidgets.find(const_cast<QWidget*>(widget));
   if (it != hoverWidgets.end())
      return it.value().step;
   return 0;
}

// -- Complex controls ----------------------

void StyleAnimator::updateComplexFades() {
   if (complexHoverWidgets.isEmpty())
      return;
   bool update;
   ComplexHoverFades::iterator it = complexHoverWidgets.begin();
   while (it != complexHoverWidgets.end()) {
      ComplexHoverFadeInfo &info = it.value();
      update = false;
      for (QStyle::SubControl control = (QStyle::SubControl)0x01;
           control <= (QStyle::SubControl)0x80;
           control = (QStyle::SubControl)(control<<1)) {
         if (info.fadingInControls & control) {
            update = true;
            info.steps[control] += 2;
            if (info.steps.value(control) > 4)
               info.fadingInControls &= ~control;
         }
         else if (info.fadingOutControls & control) {
            update = true;
            --info.steps[control];
            if (info.steps.value(control) < 1)
               info.fadingOutControls &= ~control;
         }
      }
      if (update)
         it.key()->update();
      if (info.activeSubControls == QStyle::SC_None && // needed to detect changes!
          info.fadingOutControls == QStyle::SC_None &&
          info.fadingInControls == QStyle::SC_None)
         it = complexHoverWidgets.erase(it);
      else
         ++it;
   }
   if (!ANIMATIONS) timer->stop();
}

const ComplexHoverFadeInfo *StyleAnimator::complexHoverFadeInfo(const QWidget *widget,
   QStyle::SubControls activeSubControls) const {
      QWidget *w = const_cast<QWidget*>(widget);
   ComplexHoverFades::iterator it = complexHoverWidgets.find(w);
   if (it == complexHoverWidgets.end()) {
      // we have no entry yet
      if (activeSubControls == QStyle::SC_None)
         return 0; // no need here
      // ...but we'll need one
      it = complexHoverWidgets.insert(w, ComplexHoverFadeInfo());
      connect(widget, SIGNAL(destroyed(QObject*)),
               this, SLOT(complexFadeDestroyed(QObject*)));
      startTimer;
   }
   // we now have an entry - check for validity and update in case
   ComplexHoverFadeInfo *info = &it.value();
   if (info->activeSubControls != activeSubControls) { // sth. changed
      QStyle::SubControls diff = info->activeSubControls ^ activeSubControls;
      QStyle::SubControls newActive = diff & activeSubControls;
      QStyle::SubControls newDead = diff & info->activeSubControls;
      info->fadingInControls &= ~newDead;
      info->fadingInControls |= newActive;
      info->fadingOutControls &= ~newActive;
      info->fadingOutControls |= newDead;
      info->activeSubControls = activeSubControls;
      for (QStyle::SubControl control = (QStyle::SubControl)0x01;
      control <= (QStyle::SubControl)0x80;
      control = (QStyle::SubControl)(control<<1)) {
         if (newActive & control)
            info->steps[control] = 1;
         else if (newDead & control) {
            info->steps[control] = 6;
         }
      }
   }
   return info;
}

void StyleAnimator::complexFadeDestroyed(QObject* obj) {
   complexHoverWidgets.remove(static_cast<QWidget*>(obj));
   if (!ANIMATIONS) timer->stop();
}

// -- Indexed items like menus, tabs ---------------------
void StyleAnimator::updateIndexedFades() {
   if (indexedHoverWidgets.isEmpty())
      return;
   IndexedFades::iterator it;
   QHash<long int, int>::iterator stepIt;
   it = indexedHoverWidgets.begin();
   while (it != indexedHoverWidgets.end()) {
      IndexedFadeInfo &info = it.value();
      if (info.fadingInIndices.isEmpty() && info.fadingOutIndices.isEmpty()) {
         ++it;
         continue;
      }
      
      stepIt = info.fadingInIndices.begin();
      while (stepIt != info.fadingInIndices.end()) {
         stepIt.value() += 2;
         if (stepIt.value() > 4)
            stepIt = info.fadingInIndices.erase(stepIt);
         else
            ++stepIt;
      }
      
      stepIt = info.fadingOutIndices.begin();
      while (stepIt != info.fadingOutIndices.end()) {
         --stepIt.value();
         if (stepIt.value() < 1)
            stepIt = info.fadingOutIndices.erase(stepIt);
         else
            ++stepIt;
      }
      
      it.key()->update();
      
      if (info.index == 0L && // nothing actually hovered
          info.fadingInIndices.isEmpty() && // no fade ins
          info.fadingOutIndices.isEmpty()) // no fade outs
         it = indexedHoverWidgets.erase(it);
      else
         ++it;
   }
   if (!ANIMATIONS) timer->stop();
}

const IndexedFadeInfo *StyleAnimator::indexedFadeInfo(const QWidget *widget,
   long int index) const {
   QWidget *w = const_cast<QWidget*>(widget);
   IndexedFades::iterator it = indexedHoverWidgets.find(w);
   if (it == indexedHoverWidgets.end()) {
      // we have no entry yet
      if (index == 0L)
         return 0L;
      // ... but we'll need one
      it = indexedHoverWidgets.insert(w, IndexedFadeInfo(0L));
      connect(widget, SIGNAL(destroyed(QObject*)),
               this, SLOT(indexedFadeDestroyed(QObject*)));
      startTimer;
   }
   // we now have an entry - check for validity and update in case
   IndexedFadeInfo *info = &it.value();
   if (info->index != index) { // sth. changed
      info->fadingInIndices[index] = 1;
      if (info->index)
         info->fadingOutIndices[info->index] = 6;
      info->index = index;
   }
   return info;
}

void StyleAnimator::indexedFadeDestroyed(QObject* obj) {
   indexedHoverWidgets.remove(static_cast<QWidget*>(obj));
   if (!ANIMATIONS) timer->stop();
}

int IndexedFadeInfo::step(long int index) {
   typedef QHash<long int, int> Index2Step;
   Index2Step::iterator stepIt;
   for (stepIt = fadingInIndices.begin(); stepIt != fadingInIndices.end(); stepIt++)
      if (stepIt.key() == index)
         return stepIt.value();
   for (stepIt = fadingOutIndices.begin(); stepIt != fadingOutIndices.end(); stepIt++)
      if (stepIt.key() == index)
         return stepIt.value();
   return 0;
}
