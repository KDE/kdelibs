/* Oxygen widget style for KDE 4
   Copyright (C) 2006-2007 Thomas Luebking <thomas.luebking@web.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#ifndef STYLEANIMATOR_H
#define STYLEANIMATOR_H

#include <QStyle>
#include <QHash>
#include <QList>
#include <QPixmap>
#include <QTimer>
#include <QTabWidget>

class HoverFadeInfo {
public:
   HoverFadeInfo(int s = 0, bool fI = true) {step = s; fadeIn = fI; }
   int step;
   bool fadeIn;
};

class ComplexHoverFadeInfo {
public:
   ComplexHoverFadeInfo() {
      activeSubControls = fadingInControls = fadingOutControls = QStyle::SC_None;
   }
   QStyle::SubControls activeSubControls, fadingInControls, fadingOutControls;
   QHash<QStyle::SubControl, int> steps;
};

class IndexedFadeInfo {
public:
   IndexedFadeInfo(long int index) { this->index = index; }
   long int index;
   QHash<long int, int> fadingInIndices, fadingOutIndices;
   int step(long int index);
};

class TabAnimInfo : public QObject {
   Q_OBJECT
public:
   enum TabTransition {Jump = 0, ScanlineBlend, SlideIn, SlideOut,
   RollIn, RollOut, OpenVertically, CloseVertically, OpenHorizontally,
   CloseHorizontally
#ifndef QT_NO_XRENDER
         , CrossFade
#endif
   };
   TabAnimInfo(QObject *parent = 0, int currentTab = -1) :
      QObject(parent), lastTab(currentTab), animStep(0){}
   void updatePixmaps(TabTransition transition);
protected:
   bool eventFilter( QObject* object, QEvent* event );
public:
   QList < QWidget* > autofillingWidgets;
   int lastTab, animStep;
   QPixmap tabPix[3];
};

class StyleAnimator : public QObject {
   Q_OBJECT
public:
   StyleAnimator(QObject *parent, TabAnimInfo::TabTransition tabTransition =
                 TabAnimInfo::CrossFade);
   ~StyleAnimator();
   void fadeIn(QWidget *widget);
   void fadeOut(QWidget *widget );
   void addTab(QTabWidget* tab, int currentTab = -1);
   void removeTab(QTabWidget* tab);
   void addProgressBar(QWidget *progressBar);
   void removeProgressBar(QWidget *progressBar);
   int hoverStep(const QWidget *widget) const;
   const ComplexHoverFadeInfo *complexHoverFadeInfo(const QWidget *widget,
      QStyle::SubControls activeSubControls) const;
   const IndexedFadeInfo *indexedFadeInfo(const QWidget *widget, long int index) const;
   int progressStep(const QWidget *w) const;
public slots:
   void tabChanged(int index);
private slots:
   void progressbarDestroyed(QObject*);
   void updateProgressbars();
   
   void updateTabAnimation();
   void tabDestroyed(QObject* obj);
   
   void updateFades();
   void fadeDestroyed(QObject* obj);
   
   void updateComplexFades();
   void complexFadeDestroyed(QObject* obj);
   
   void updateIndexedFades();
   void indexedFadeDestroyed(QObject* obj);
private:
   QTimer* timer;
   TabAnimInfo::TabTransition tabTransition;
};

#endif // STYLEANIMATOR_H
