/* This file is part of the KDE libraries
   Copyright (C) 2000 Max Judin <novaprint@mtu-net.ru>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

/*
   IMPORTANT Note: This file compiles also in Qt-only mode by using the NO_KDE2 precompiler definition
*/

#ifndef KDOCKWIDGET_PRIVATE_H
#define KDOCKWIDGET_PRIVATE_H

#include <qwidget.h>
#include <qpushbutton.h>

#ifndef NO_KDE2
#include <netwm_def.h>
#endif

class QFrame;

/**
 * Like QSplitter but specially designed for dockwidgets stuff.
 * @internal
 *
 * @author Max Judin.
*/
class KDockSplitter : public QWidget
{
  Q_OBJECT
public:
  KDockSplitter(QWidget *parent= 0, const char *name= 0, Orientation orient= Vertical, int pos= 50, bool highResolution=false);  
  virtual ~KDockSplitter(){};

  void activate(QWidget *c0, QWidget *c1 = 0L);
  void deactivate();

  int separatorPos() const;
  void setSeparatorPos(int pos, bool do_resize = true);

  virtual bool eventFilter(QObject *, QEvent *);
  virtual bool event( QEvent * );

  QWidget* getFirst() const { return child0; }
  QWidget* getLast() const { return child1; }
  QWidget* getAnother( QWidget* ) const;
  void updateName();

  void setOpaqueResize(bool b=true);
  bool opaqueResize() const;

  void setKeepSize(bool b=true);
  bool keepSize() const;

  void setHighResolution(bool b=true);
  bool highResolution() const;

protected:
  int checkValue( int ) const;
  virtual void resizeEvent(QResizeEvent *);

private:
  void setupMinMaxSize();

  QWidget *child0, *child1;
  Orientation orientation;
  bool initialised;
  QFrame* divider;
  int xpos;
  bool mOpaqueResize, mKeepSize, mHighResolution;
};

/**
 * A mini-button usually placed in the dockpanel.
 * @internal
 *
 * @author Max Judin.
*/
class KDockButton_Private : public QPushButton
{
  Q_OBJECT
public:
  KDockButton_Private( QWidget *parent=0, const char *name=0 );
  ~KDockButton_Private();

protected:
  virtual void drawButton( QPainter * );
  virtual void enterEvent( QEvent * );
  virtual void leaveEvent( QEvent * );

private:
  bool moveMouse;
};

/**
 * additional KDockWidget stuff (private)
*/
class KDockWidgetPrivate : public QObject
{
  Q_OBJECT
public:
  KDockWidgetPrivate();
  ~KDockWidgetPrivate();

public slots:
  /**
   * Especially used for Tab page docking. Switching the pages requires additional setFocus() for the embedded widget.
   */
  void slotFocusEmbeddedWidget(QWidget* w = 0L);

public:
  int index;
  int splitPosInPercent;
  bool pendingFocusInEvent;
  bool blockHasUndockedSignal;
  bool pendingDtor;

#ifndef NO_KDE2
  NET::WindowType windowType;
#endif

  QWidget *_parent;
  bool transient;
};

#endif
