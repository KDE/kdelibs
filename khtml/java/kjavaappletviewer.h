// -*- c++ -*-

/* This file is part of the KDE project
 *
 * Copyright (C) 2003 Koos Vriezen <koos.vriezen@xs4all.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef KJAVAAPPLETVIEWER_H
#define KJAVAAPPLETVIEWER_H

#include <kparts/part.h>
#include <kparts/browserextension.h>
#include <kparts/factory.h>
#include <kdialogbase.h>
#include <kurl.h>
#include <qobject.h>
#include <qstringlist.h>

#include "kjavaappletwidget.h"

class QTable;
class KJavaProcess;
class KJavaAppletWidget;
class KJavaAppletViewer;
class KAboutData;
class KInstance;
class KConfig;

/* FIXME: merge with KJavaAppletWidget */
class KJavaAppletViewerWidget : public KJavaAppletWidget {
public:
    KJavaAppletViewerWidget (KJavaAppletContext* context,
                            QWidget* parent=0, const char* name=0);
protected:
    void showEvent (QShowEvent *);
};

class KJavaAppletViewerBrowserExtension : public KParts::BrowserExtension {
    Q_OBJECT
public:
    KJavaAppletViewerBrowserExtension (KJavaAppletViewer *parent);
    void urlChanged (const QString & url);
    void setLoadingProgress (int percentage);

    void setURLArgs (const KParts::URLArgs & args);
    void saveState (QDataStream & stream);
    void restoreState (QDataStream & stream);
public slots:
    void showDocument (const QString & doc, const QString & frame);
};

class KJavaAppletViewer : public KParts::ReadOnlyPart {
    Q_OBJECT
public: 
    KJavaAppletViewer (QWidget * wparent, const char * wname,
              QObject * parent, const char * name, const QStringList &args);
    ~KJavaAppletViewer ();
    KJavaAppletWidget * view () const { return m_view; }
    static KAboutData* createAboutData ();

    KJavaAppletViewerBrowserExtension * browserextension() const
        { return m_browserextension; }
public slots:
    virtual bool openURL (const KURL & url);
    void appletLoaded ();
    void infoMessage (const QString &);
protected:
    bool openFile();
private:
    KJavaAppletWidget * m_view;
    KConfig * m_config;
    KJavaProcess * process;
    KJavaAppletViewerBrowserExtension * m_browserextension;
};

class KJavaAppletViewerFactory : public KParts::Factory {
    Q_OBJECT
public:
    KJavaAppletViewerFactory ();
    virtual ~KJavaAppletViewerFactory ();
    virtual KParts::Part *createPartObject 
        (QWidget *wparent, const char *wname,
         QObject *parent, const char *name,
         const char *className, const QStringList &args);
    static KInstance * instance () { return s_instance; }
private:
    static KInstance * s_instance;
};

class AppletParameterDialog : public KDialogBase {
    Q_OBJECT
public:
    AppletParameterDialog (KJavaAppletWidget * parent);
protected slots:
    void slotClose ();
private:
    KJavaAppletWidget * m_appletWidget;
    QTable * table;
};

#endif
