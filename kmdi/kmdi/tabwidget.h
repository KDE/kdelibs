/* This file is part of the KDE libraries
   Copyright (C) 2004 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002,2003 Joseph Wenninger <jowenn@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

   Based on:

   //----------------------------------------------------------------------------
   //    Project              : KDE MDI extension
   //
   //    begin                : 07/1999       by Szymon Stefanek as part of kvirc
   //                                         (an IRC application)
   //    changes              : 09/1999       by Falk Brettschneider to create an
   //                           - 06/2000     stand-alone Qt extension set of
   //                                         classes and a Qt-based library
   //                         : 02/2000       by Massimo Morin (mmorin@schedsys.com)
   //                           2000-2003     maintained by the KDevelop project
   //    patches              : -/2000        by Lars Beikirch (Lars.Beikirch@gmx.net)
   //                         : 01/2003       by Jens Zurheide (jens.zurheide@gmx.de)
   //
   //    copyright            : (C) 1999-2003 by Falk Brettschneider
   //                                         and
   //                                         Szymon Stefanek (stefanek@tin.it)
   //    email                :  falkbr@kdevelop.org (Falk Brettschneider)
   //----------------------------------------------------------------------------
*/

#ifndef _KMDI_TABWIDGET_H_
#define _KMDI_TABWIDGET_H_

#include <ktabwidget.h>
#include <kmdi/global.h>

namespace KMDIPrivate
{
  class TabWidgetPrivate;
}

namespace KMDI
{

class KMDI_EXPORT TabWidget : public KTabWidget
{
  Q_OBJECT

  public:
    TabWidget(QWidget* parent, const char* name=0);
    virtual ~TabWidget();

    virtual void addTab ( QWidget * child, const QString & label );

    virtual void addTab ( QWidget * child, const QIconSet & iconset, const QString & label );

    virtual void addTab ( QWidget * child, QTab * tab );

    virtual void insertTab ( QWidget * child, const QString & label, int index = -1 );

    virtual void insertTab ( QWidget * child, const QIconSet & iconset, const QString & label, int index = -1 );

    virtual void insertTab ( QWidget * child, QTab * tab, int index = -1 );

    virtual void removePage ( QWidget * w );

    KMDI::TabWidgetVisibility tabWidgetVisibility() const;

    void setTabWidgetVisibility( KMDI::TabWidgetVisibility );

    bool eventFilter(QObject *obj, QEvent *e );

  private slots:
    void closeTab(QWidget* w);

  public slots:
    void updateIconInView(QWidget*,QPixmap);
    void updateCaptionInView(QWidget*,const QString&);

  signals:
    void focusInEvent ();

  protected slots:
   void childDestroyed ();

  private:
    void maybeShow();
    void setCornerWidgetVisibility(bool visible);

  private:
    KMDI::TabWidgetVisibility m_visibility;

  private:
    /**
     * private d-pointer for BC
     */
    KMDIPrivate::TabWidgetPrivate *d;
};

}

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
