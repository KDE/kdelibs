/***************************************************************************
                          mainwidget.h  -  description
                             -------------------
    begin                : Mon Nov 8 1999
    copyright            : (C) 1999 by Falk Brettschneider
    email                : falkbr@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <qcheckbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qdom.h>
#include <k3mdimainfrm.h>

/**
  *@author Falk Brettschneider
  */
class MainWidget : public K3MdiMainFrm
{
    Q_OBJECT
public:
    MainWidget(QDomElement& dockConfig,K3Mdi::MdiMode mode);
    virtual ~MainWidget();
    void initMenu();

protected: // Protected methods
    virtual void resizeEvent( QResizeEvent *pRSE);
private:
    QDomElement m_dockConfig;
};



/**
  *@author Falk Brettschneider
  * This allows me to test K3MdiMainFrm::read/writeDockConfig by
  * closing and restarting the MainWidget via checkbox click.
  */
class RestartWidget : public KMainWindow
{
    Q_OBJECT
// methods
public:
    RestartWidget();
    void setWindow(MainWidget *w);

private Q_SLOTS:
    void onStateChanged(int on);

// attributes
public:
    QDomDocument domDoc;
    QDomElement dockConfig;
    K3Mdi::MdiMode mdimode;
private:
    MainWidget *m_w;
};

#endif
