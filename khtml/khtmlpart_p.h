#ifndef khtmlpart_p_h
#define khtmlpart_p_h

/* This file is part of the KDE project
 *
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999-2001 Lars Knoll <knoll@kde.org>
 *                     1999-2001 Antti Koivisto <koivisto@kde.org>
 *                     2000-2001 Simon Hausmann <hausmann@kde.org>
 *                     2000-2001 Dirk Mueller <mueller@kde.org>
 *                     2000 Stefan Schimanski <1Stein@gmx.de>
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
#include <kcursor.h>
#include <klibloader.h>
#include <kxmlguifactory.h>
#include <kaction.h>
#include <kparts/partmanager.h>

#include "khtml_run.h"
#include "khtml_find.h"
#include "khtml_factory.h"
#include "khtml_events.h"
#include "khtml_ext.h"
#include "khtml_iface.h"
#include "khtml_settings.h"
#include "misc/decoder.h"
#include "java/kjavaappletcontext.h"
#include "ecma/kjs_proxy.h"

namespace KIO
{
  class Job;
  class TransferJob;
};

namespace khtml
{
  struct ChildFrame
  {
      enum Type { Frame, IFrame, Object };

      ChildFrame() { m_bCompleted = false; m_bPreloaded = false; m_type = Frame; m_bNotify = false; }

      ~ChildFrame() { if (m_run) m_run->abort(); }

    QGuardedPtr<khtml::RenderPart> m_frame;
    QGuardedPtr<KParts::ReadOnlyPart> m_part;
    QGuardedPtr<KParts::BrowserExtension> m_extension;
    QString m_serviceName;
    QString m_serviceType;
    QStringList m_services;
    bool m_bCompleted;
    QString m_name;
    KParts::URLArgs m_args;
    QGuardedPtr<KHTMLRun> m_run;
    bool m_bPreloaded;
    KURL m_workingURL;
    Type m_type;
    QStringList m_params;
    bool m_bNotify;
  };

};

class FrameList : public QValueList<khtml::ChildFrame>
{
public:
    Iterator find( const QString &name );
};

typedef FrameList::ConstIterator ConstFrameIt;
typedef FrameList::Iterator FrameIt;

static int khtml_part_dcop_counter = 0;

class KHTMLPartPrivate
{
public:
  KHTMLPartPrivate(QObject* parent)
  {
    m_doc = 0L;
    m_decoder = 0L;
    m_jscript = 0L;
    m_runningScripts = 0;
    m_kjs_lib = 0;
    m_job = 0L;
    m_bComplete = true;
    m_bLoadEventEmitted = true;
    m_cachePolicy = KIO::CC_Verify;
    m_manager = 0L;
    m_settings = new KHTMLSettings(*KHTMLFactory::defaultHTMLSettings());
    m_bClearing = false;
    m_bCleared = false;
    m_zoomFactor = 100;
    m_bDnd = true;
    m_startOffset = m_endOffset = 0;
    m_startBeforeEnd = true;
    m_linkCursor = KCursor::handCursor();
    m_loadedObjects = 0;
    m_totalObjectCount = 0;
    m_jobPercent = 0;
    m_haveEncoding = false;
    m_activeFrame = 0L;
    m_findDialog = 0;
    m_ssl_in_use = false;
#ifndef Q_WS_QWS
    m_javaContext = 0;
#endif
    m_cacheId = 0;
    m_frameNameId = 1;

    m_restored = false;

    m_focusNodeNumber = -1;
    m_focusNodeRestored = false;

    m_bJScriptForce = false;
    m_bJScriptOverride = false;
    m_bJavaForce = false;
    m_bJavaOverride = false;
    m_bPluginsForce = false;
    m_bPluginsOverride = false;
    m_onlyLocalReferences = false;

    m_metaRefreshEnabled = true;
    m_bHTTPRefresh = false;

    m_bFirstData = true;
    m_submitForm = 0;
    m_delayRedirect = 0;

    m_bPendingChildRedirection = false;

    // inherit settings from parent
    if(parent && parent->inherits("KHTMLPart"))
    {
        KHTMLPart* part = static_cast<KHTMLPart*>(parent);
        if(part->d)
        {
            m_bJScriptForce = part->d->m_bJScriptForce;
            m_bJScriptOverride = part->d->m_bJScriptOverride;
            m_bJavaForce = part->d->m_bJavaForce;
            m_bJavaOverride = part->d->m_bJavaOverride;
            m_bPluginsForce = part->d->m_bPluginsForce;
            m_bPluginsOverride = part->d->m_bPluginsOverride;
            // Same for SSL settings
            m_ssl_in_use = part->d->m_ssl_in_use;
            m_onlyLocalReferences = part->d->m_onlyLocalReferences;
            m_zoomFactor = part->d->m_zoomFactor;
        }
    }

    m_focusNodeNumber = -1;
    m_focusNodeRestored = false;
    m_opener = 0;
    m_openedByJS = false;
    m_newJSInterpreterExists = false;
    m_dcopobject = 0;
    m_dcop_counter = ++khtml_part_dcop_counter;
  }
  ~KHTMLPartPrivate()
  {
    delete m_dcopobject;
    delete m_extension;
    delete m_settings;
    delete m_jscript;
    if ( m_kjs_lib)
       m_kjs_lib->unload();
#ifndef Q_WS_QWS
    delete m_javaContext;
#endif
  }

  FrameList m_frames;
  QValueList<khtml::ChildFrame> m_objects;

  QGuardedPtr<KHTMLView> m_view;
  KHTMLPartBrowserExtension *m_extension;
  KHTMLPartBrowserHostExtension *m_hostExtension;
  DOM::DocumentImpl *m_doc;
  khtml::Decoder *m_decoder;
  QString m_encoding;
  QString m_sheetUsed;
  long m_cacheId;
  QString scheduledScript;
  DOM::Node scheduledScriptNode;

  KJSProxy *m_jscript;
  KLibrary *m_kjs_lib;
  int m_runningScripts;
  bool m_bJScriptEnabled :1;
  bool m_bJScriptDebugEnabled :1;
  bool m_bJavaEnabled :1;
  bool m_bPluginsEnabled :1;
  bool m_bJScriptForce :1;
  bool m_bJScriptOverride :1;
  bool m_bJavaForce :1;
  bool m_bJavaOverride :1;
  bool m_bPluginsForce :1;
  bool m_metaRefreshEnabled :1;
  bool m_bPluginsOverride :1;
  bool m_restored :1;
  int m_frameNameId;
  int m_dcop_counter;
  DCOPObject *m_dcopobject;

#ifndef Q_WS_QWS
  KJavaAppletContext *m_javaContext;
#endif

  KHTMLSettings *m_settings;

  KIO::TransferJob * m_job;

  QString m_kjsStatusBarText;
  QString m_kjsDefaultStatusBarText;
  QString m_lastModified;

  // QStrings for SSL metadata
  // Note: When adding new variables don't forget to update ::saveState()/::restoreState()!
  bool m_ssl_in_use;
  QString m_ssl_peer_certificate,
          m_ssl_peer_chain,
          m_ssl_peer_ip,
          m_ssl_cipher,
          m_ssl_cipher_desc,
          m_ssl_cipher_version,
          m_ssl_cipher_used_bits,
          m_ssl_cipher_bits,
          m_ssl_cert_state;

  bool m_bComplete:1;
  bool m_bLoadEventEmitted:1;
  bool m_haveEncoding:1;
  bool m_bHTTPRefresh:1;
  bool m_onlyLocalReferences :1;
  bool m_redirectLockHistory:1;

  KURL m_workingURL;

  KIO::CacheControl m_cachePolicy;
  QTimer m_redirectionTimer;
  QTime m_parsetime;
  int m_delayRedirect;
  QString m_redirectURL;

  KAction *m_paViewDocument;
  KAction *m_paViewFrame;
  KAction *m_paSaveBackground;
  KAction *m_paSaveDocument;
  KAction *m_paSaveFrame;
  KAction *m_paSecurity;
  KSelectAction *m_paSetEncoding;
  KSelectAction *m_paUseStylesheet;
  KHTMLZoomFactorAction *m_paIncZoomFactor;
  KHTMLZoomFactorAction *m_paDecZoomFactor;
  KAction *m_paLoadImages;
  KAction *m_paFind;
  KAction *m_paPrintFrame;
  KAction *m_paSelectAll;
  KAction *m_paDebugDOMTree;
  KAction *m_paDebugRenderTree;

  KParts::PartManager *m_manager;

  QString m_popupMenuXML;
  KHTMLPart::GUIProfile m_guiProfile;

  int m_zoomFactor;

  int m_findPos;
  DOM::NodeImpl *m_findNode;

  QString m_strSelectedURL;
  QString m_strSelectedURLTarget;
  QString m_referrer;

  struct SubmitForm
  {
    const char *submitAction;
    QString submitUrl;
    QByteArray submitFormData;
    QString target;
    QString submitContentType;
    QString submitBoundary;
  };

  SubmitForm *m_submitForm;

  bool m_bMousePressed;
  DOM::Node m_mousePressNode; //node under the mouse when the mouse was pressed (set in the mouse handler)

  DOM::Node m_selectionStart;
  long m_startOffset;
  DOM::Node m_selectionEnd;
  long m_endOffset;
  QString m_overURL;
  QString m_overURLTarget;

  bool m_startBeforeEnd:1;
  bool m_bDnd:1;
  bool m_bFirstData:1;
  bool m_bClearing:1;
  bool m_bCleared:1;
  bool m_bSecurityInQuestion:1;
  bool m_focusNodeRestored:1;

  int m_focusNodeNumber;

  QPoint m_dragStartPos;
#ifdef KHTML_NO_SELECTION
  QPoint m_dragLastPos;
#endif

  QCursor m_linkCursor;
  QTimer m_scrollTimer;

  unsigned long m_loadedObjects;
  unsigned long m_totalObjectCount;
  unsigned int m_jobPercent;

  KHTMLFind *m_findDialog;

  struct findState
  {
    findState()
    { caseSensitive = false; direction = false; }
    QString text;
    bool caseSensitive;
    bool direction;
  };

  findState m_lastFindState;

  //QGuardedPtr<KParts::Part> m_activeFrame;
  KParts::Part * m_activeFrame;
  QGuardedPtr<KHTMLPart> m_opener;
  bool m_openedByJS;
  bool m_newJSInterpreterExists; // set to 1 by setOpenedByJS, for window.open

  bool m_bPendingChildRedirection;
};

#endif
