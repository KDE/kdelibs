/* This file is part of the KDE project
 *
 * Copyright (C) 2000-2003 Simon Hausmann <hausmann@kde.org>
 *               2001-2003 George Staikos <staikos@kde.org>
 *               2001-2003 Laurent Montel <montel@kde.org>
 *               2001-2003 Dirk Mueller <mueller@kde.org>
 *               2001-2003 Waldo Bastian <bastian@kde.org>
 *               2001-2003 David Faure <faure@kde.org>
 *               2001-2003 Daniel Naber <dnaber@kde.org>
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
 * the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __khtml_ext_h__
#define __khtml_ext_h__

#include "khtml_part.h"

#include <qpointer.h>

#include <kaction.h>
#include <kio/global.h>

/**
 * This is the BrowserExtension for a KHTMLPart document. Please see the KParts documentation for
 * more information about the BrowserExtension.
 */
class KHTMLPartBrowserExtension : public KParts::BrowserExtension
{
  Q_OBJECT
  friend class KHTMLPart;
  friend class KHTMLView;
public:
  KHTMLPartBrowserExtension( KHTMLPart *parent );

  virtual int xOffset();
  virtual int yOffset();

  virtual void saveState( QDataStream &stream );
  virtual void restoreState( QDataStream &stream );

    // internal
    void editableWidgetFocused( QWidget *widget );
    void editableWidgetBlurred( QWidget *widget );

    void setExtensionProxy( KParts::BrowserExtension *proxyExtension );

public slots:
    void cut();
    void copy();
    void paste();
    void searchProvider();
    void openSelection();
    void reparseConfiguration();
    void print();
    void disableScrolling();

    // internal . updates the state of the cut/copt/paste action based
    // on whether data is available in the clipboard
    void updateEditActions();

private slots:
    // connected to a frame's browserextensions enableAction signal
    void extensionProxyActionEnabled( const char *action, bool enable );
    void extensionProxyEditableWidgetFocused();
    void extensionProxyEditableWidgetBlurred();

signals:
    void editableWidgetFocused();
    void editableWidgetBlurred();
private:
    void callExtensionProxyMethod( const char *method );

    KHTMLPart *m_part;
    QPointer<QWidget> m_editableFormWidget;
    QPointer<KParts::BrowserExtension> m_extensionProxy;
    bool m_connectedToClipboard;
};

class KHTMLPartBrowserHostExtension : public KParts::BrowserHostExtension
{
public:
  KHTMLPartBrowserHostExtension( KHTMLPart *part );
  virtual ~KHTMLPartBrowserHostExtension();

  virtual QStringList frameNames() const;

  virtual const Q3PtrList<KParts::ReadOnlyPart> frames() const;

  virtual bool openURLInFrame( const KURL &url, const KParts::URLArgs &urlArgs );

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KHTMLPart *m_part;
};

/**
 * @internal
 * INTERNAL class. *NOT* part of the public API.
 */
class KHTMLPopupGUIClient : public QObject, public KXMLGUIClient
{
  Q_OBJECT
public:
  KHTMLPopupGUIClient( KHTMLPart *khtml, const QString &doc, const KURL &url );
  virtual ~KHTMLPopupGUIClient();

  static void saveURL( QWidget *parent, const QString &caption, const KURL &url,
                       const QMap<QString, QString> &metaData = KIO::MetaData(),
                       const QString &filter = QString::null, long cacheId = 0,
                       const QString &suggestedFilename = QString::null );

  static void saveURL( const KURL &url, const KURL &destination,
                       const QMap<QString, QString> &metaData = KIO::MetaData(),
                       long cacheId = 0 );
private slots:
  void slotSaveLinkAs();
  void slotSaveImageAs();
  void slotCopyLinkLocation();
  void slotSendImage();
  void slotStopAnimations();
  void slotCopyImageLocation();
  void slotCopyImage();
  void slotViewImage();
  void slotReloadFrame();
  void slotFrameInWindow();
  void slotFrameInTop();
  void slotFrameInTab();
  void slotBlockImage();
  void slotBlockHost();
  void slotBlockIFrame();

private:
  class KHTMLPopupGUIClientPrivate;
  KHTMLPopupGUIClientPrivate *d;
};

class KHTMLZoomFactorAction : public KAction
{
    Q_OBJECT
public:
    //BCI: remove in KDE 4
    KHTMLZoomFactorAction( KHTMLPart *part, bool direction, const QString &text, const QString &icon, const QObject *receiver, const char *slot, QObject *parent, const char *name );
    KHTMLZoomFactorAction( KHTMLPart *part, bool direction, const QString &text,
            const QString &icon, const KShortcut& cut, const QObject *receiver,
            const char *slot, QObject *parent, const char *name );
    virtual ~KHTMLZoomFactorAction();

    virtual int plug( QWidget *w, int index );

private slots:
    void slotActivated( int );
protected slots:
    void slotActivated() { KAction::slotActivated(); }
private:
    void init(KHTMLPart *part, bool direction);
private:
    Q3PopupMenu *m_popup;
    bool m_direction;
    KHTMLPart *m_part;
};

#endif
