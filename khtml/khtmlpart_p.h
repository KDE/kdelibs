/* This file is part of the KDE project
 *
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999-2001 Lars Knoll <knoll@kde.org>
 *                     1999-2001 Antti Koivisto <koivisto@kde.org>
 *                     2000-2001 Simon Hausmann <hausmann@kde.org>
 *                     2000-2001 Dirk Mueller <mueller@kde.org>
 *                     2000 Stefan Schimanski <1Stein@gmx.de>
 *                     2001-2005 George Staikos <staikos@kde.org>
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef khtmlpart_p_h
#define khtmlpart_p_h

#include <kcursor.h>
#include <klibloader.h>
#include <kxmlguifactory.h>
#include <kaction.h>
#include <kparts/partmanager.h>
#include <kparts/statusbarextension.h>
#include <kparts/browserextension.h>
#include <kparts/scriptableextension.h>

#include <QtCore/QDate>
#include <QtCore/QPointer>
#include <QtCore/QMap>
#include <QtCore/QTimer>
#include <QtCore/QList>
#include <QtCore/QQueue>

#include "html/html_formimpl.h"
#include "html/html_objectimpl.h"
#include "khtml_run.h"
#include "khtml_global.h"
#include "khtml_events.h"
#include "khtml_ext.h"
#include "khtml_settings.h"
#include <kencodingdetector.h>
#include "ecma/kjs_proxy.h"
#include "xml/dom_nodeimpl.h"
#include "editing/editing_p.h"
#include "ui/findbar/khtmlfind_p.h"
#include "ui/passwordbar/storepassbar.h"
#include "ecma/kjs_scriptable.h"

class KFind;
class KFindDialog;
class KCodecAction;
class KUrlLabel;
class KJavaAppletContext;
class KJSErrorDlg;
class KToggleAction;
class KHTMLViewBar;

namespace KIO
{
  class Job;
  class TransferJob;
}
namespace KParts
{
  class StatusBarExtension;
}

#include "khtml_wallet_p.h"

namespace khtml
{
  class KHTML_EXPORT ChildFrame : public QObject
  {
      Q_OBJECT
  public:
      enum Type { Frame, IFrame, Object };

      ChildFrame() : QObject (0) {
          setObjectName( "khtml_child_frame" );
          m_jscript = 0L;
#ifndef DIRECT_LINKAGE_TO_ECMA
          m_kjs_lib = 0;
#endif
          m_bCompleted = false; m_bPreloaded = false; m_type = Frame; m_bNotify = false;
          m_bPendingRedirection = false;
      }

      ~ChildFrame() {
          if (m_run) m_run->abort();
          delete m_jscript;
#ifndef DIRECT_LINKAGE_TO_ECMA
          if ( m_kjs_lib)
              m_kjs_lib->unload();
#endif
      }

    QPointer<DOM::HTMLPartContainerElementImpl> m_partContainerElement;
    QPointer<KParts::ReadOnlyPart> m_part;
    QPointer<KParts::BrowserExtension> m_extension;
    QWeakPointer<KParts::ScriptableExtension> m_scriptable;
    QString m_serviceName;
    QString m_serviceType;
    KJSProxy *m_jscript;
#ifndef DIRECT_LINKAGE_TO_ECMA
    KLibrary *m_kjs_lib;
#endif
    bool m_bCompleted;
    QString m_name;
    KParts::OpenUrlArguments m_args;
    KParts::BrowserArguments m_browserArgs;
    QPointer<KHTMLRun> m_run;
    KUrl m_workingURL;
    Type m_type;
    QStringList m_params;
    bool m_bPreloaded;
    bool m_bNotify;
    bool m_bPendingRedirection;
  };
}

struct KHTMLFrameList : public QList<khtml::ChildFrame*>
{
    Iterator find( const QString &name ) KDE_NO_EXPORT;
};

typedef KHTMLFrameList::ConstIterator ConstFrameIt;
typedef KHTMLFrameList::Iterator FrameIt;

enum MimeType {
    MimeHTML,
    MimeSVG,
    MimeXHTML,
    MimeXML, // XML but not SVG or XHTML
    MimeImage,
    MimeText,
    MimeOther
};


class KHTMLPartPrivate
{
  KHTMLPartPrivate(const KHTMLPartPrivate & other);
  KHTMLPartPrivate& operator=(const KHTMLPartPrivate&);
public:
  KHTMLPartPrivate(KHTMLPart* part, QObject* parent) :
    m_find( part, (part->parentPart() ? &part->parentPart()->d->m_find : 0) ), m_storePass( part )
  {
    q     = part;
    m_doc = 0L;
    m_decoder = 0L;
#ifndef KHTML_NO_WALLET
    m_wallet = 0L;
#endif
    m_bWalletOpened = false;
    m_runningScripts = 0;
    m_job = 0L;
    m_bComplete = true;
    m_bLoadEventEmitted = true;
    m_cachePolicy = KIO::CC_Verify;
    m_manager = 0L;
    m_settings = new KHTMLSettings(*KHTMLGlobal::defaultHTMLSettings());
    m_bClearing = false;
    m_bCleared = false;
    m_zoomFactor = 100;
    m_fontScaleFactor = 100;
    m_bDnd = true;
    m_linkCursor = QCursor(Qt::PointingHandCursor);
    m_loadedObjects = 0;
    m_totalObjectCount = 0;
    m_jobPercent = 0;
    m_haveEncoding = false;
    m_activeFrame = 0L;
    m_ssl_in_use = false;
    m_jsedlg = 0;
    m_formNotification = KHTMLPart::NoNotification;

    m_cacheId = 0;
    m_frameNameId = 1;

    m_restored = false;
    m_restoreScrollPosition = false;

    m_focusNodeNumber = -1;
    m_focusNodeRestored = false;

    m_bJScriptForce = false;
    m_bJScriptOverride = false;
    m_bJavaForce = false;
    m_bJavaOverride = false;
    m_bPluginsForce = false;
    m_bPluginsOverride = false;
    m_onlyLocalReferences = false;
    m_bDNSPrefetch = KHTMLPart::DNSPrefetchDisabled;
    m_bDNSPrefetchIsDefault = true;
    m_DNSPrefetchTimer = -1;
    m_DNSTTLTimer = -1;
    m_numDNSPrefetchedNames = 0;

    m_caretMode = false;
    m_designMode = false;

    m_metaRefreshEnabled = true;
    m_statusMessagesEnabled = true;

    m_bFirstData = true;
    m_bStrictModeQuirk = true;
    m_submitForm = 0;
    m_delayRedirect = 0;
    m_autoDetectLanguage = KEncodingDetector::SemiautomaticDetection;

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
            m_bDNSPrefetch = part->d->m_bDNSPrefetch;
            m_bDNSPrefetchIsDefault = part->d->m_bDNSPrefetchIsDefault;
            // Same for SSL settings
            m_ssl_in_use = part->d->m_ssl_in_use;
            m_onlyLocalReferences = part->d->m_onlyLocalReferences;
            m_caretMode = part->d->m_caretMode;
            m_designMode = part->d->m_designMode;
            m_zoomFactor = part->d->m_zoomFactor;
            m_fontScaleFactor = part->d->m_fontScaleFactor;
            m_autoDetectLanguage = part->d->m_autoDetectLanguage;
            m_encoding = part->d->m_encoding;
            m_haveEncoding = part->d->m_haveEncoding;
        }
    }

    m_focusNodeNumber = -1;
    m_focusNodeRestored = false;
    m_opener = 0;
    m_openedByJS = false;
    m_newJSInterpreterExists = false;
    m_jobspeed = 0;
    m_statusBarWalletLabel = 0L;
    m_statusBarUALabel = 0L;
    m_statusBarJSErrorLabel = 0L;
    m_userStyleSheetLastModified = 0;
#ifndef KHTML_NO_WALLET
    m_wq = 0;
#endif
  }
  ~KHTMLPartPrivate()
  {
    delete m_statusBarExtension;
    delete m_scriptableExtension;
    delete m_extension;
    delete m_settings;
#ifndef KHTML_NO_WALLET
    delete m_wallet;
#endif
#ifndef Q_WS_QWS
    //delete m_javaContext;
#endif
  }

  KHTMLPart* q;

  QPointer<khtml::ChildFrame> m_frame;
  KHTMLFrameList m_frames;
  KHTMLFrameList m_objects;

  QPointer<KHTMLView> m_view;
  QPointer<KHTMLViewBar> m_topViewBar;
  QPointer<KHTMLViewBar> m_bottomViewBar;
  KHTMLPartBrowserExtension *m_extension;
  KParts::StatusBarExtension *m_statusBarExtension;
  KHTMLPartBrowserHostExtension *m_hostExtension;
  KJS::KHTMLPartScriptable *m_scriptableExtension;
  KUrlLabel* m_statusBarIconLabel;
  KUrlLabel* m_statusBarWalletLabel;
  KUrlLabel* m_statusBarUALabel;
  KUrlLabel* m_statusBarJSErrorLabel;
  KUrlLabel* m_statusBarPopupLabel;
  QList<QPointer<KHTMLPart> > m_suppressedPopupOriginParts; // We need to guard these in case the origin
                                                            // is a child part.
  int m_openableSuppressedPopups;
  DOM::DocumentImpl *m_doc;
  KEncodingDetector::AutoDetectScript m_autoDetectLanguage;
  KEncodingDetector *m_decoder;
  QString m_encoding;
  QString m_sheetUsed;
  qlonglong m_cacheId;

#ifndef KHTML_NO_WALLET
  KWallet::Wallet* m_wallet;
  QStringList m_walletForms;
#endif
  int m_runningScripts;
  bool m_bOpenMiddleClick;
  bool m_bJScriptEnabled;
  bool m_bJScriptDebugEnabled;
  bool m_bJavaEnabled;
  bool m_bPluginsEnabled;
  bool m_bJScriptForce;
  bool m_bJScriptOverride;
  bool m_bJavaForce;
  bool m_bJavaOverride;
  bool m_bPluginsForce;
  bool m_metaRefreshEnabled;
  bool m_bPluginsOverride;
  bool m_restored;
  bool m_restoreScrollPosition;
  bool m_statusMessagesEnabled;
  bool m_bWalletOpened;
  bool m_urlSelectedOpenedURL; // KDE4: remove
  bool m_bDNSPrefetchIsDefault;
  int m_DNSPrefetchTimer;
  int m_DNSTTLTimer;
  int m_numDNSPrefetchedNames;
  QQueue<QString> m_DNSPrefetchQueue;
  KHTMLPart::DNSPrefetch m_bDNSPrefetch;
  int m_frameNameId;

  KHTMLSettings *m_settings;

  KIO::TransferJob * m_job;

  QString m_statusBarText[3];
  unsigned long m_jobspeed;
  QString m_lastModified;
  QString m_httpHeaders;
  QString m_pageServices;

  // QStrings for SSL metadata
  // Note: When adding new variables don't forget to update ::saveState()/::restoreState()!
  QString m_ssl_peer_chain,
          m_ssl_peer_ip,
          m_ssl_cipher,
          m_ssl_protocol_version,
          m_ssl_cipher_used_bits,
          m_ssl_cipher_bits,
          m_ssl_cert_errors,
          m_ssl_parent_ip,
          m_ssl_parent_cert;
  bool m_ssl_in_use;

  bool m_bComplete;
  bool m_bLoadEventEmitted;
  bool m_haveEncoding;
  bool m_onlyLocalReferences;
  bool m_redirectLockHistory;

  KUrl m_workingURL;

  KIO::CacheControl m_cachePolicy;
  QTimer m_redirectionTimer;
  QTime m_parsetime;
  int m_delayRedirect;
  QString m_redirectURL;

  KAction *m_paViewDocument;
  KAction *m_paViewFrame;
  KAction *m_paViewInfo;
  KAction *m_paSaveBackground;
  KAction *m_paSaveDocument;
  KAction *m_paSaveFrame;
  KAction *m_paSecurity;
  KCodecAction *m_paSetEncoding;
  KSelectAction *m_paUseStylesheet;
  KSelectAction *m_paIncZoomFactor;
  KSelectAction *m_paDecZoomFactor;
  KAction *m_paLoadImages;
  KAction *m_paFind;
  KAction *m_paFindNext;
  KAction *m_paFindPrev;
  KAction *m_paFindAheadText;
  KAction *m_paFindAheadLinks;
  KAction *m_paPrintFrame;
  KAction *m_paSelectAll;
  KAction *m_paDebugScript;
  KAction *m_paDebugDOMTree;
  KAction *m_paDebugRenderTree;
  KAction *m_paStopAnimations;
  KToggleAction *m_paToggleCaretMode;
  QMap<QAction*, int> m_paLanguageMap;

  KParts::PartManager *m_manager;

  KHTMLPart::GUIProfile m_guiProfile;

  int m_zoomFactor;
  int m_fontScaleFactor;

  QString m_strSelectedURL;
  QString m_strSelectedURLTarget;
  QString m_referrer;
  QString m_pageReferrer;

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
  bool m_bRightMousePressed;
  DOM::Node m_mousePressNode; //node under the mouse when the mouse was pressed (set in the mouse handler)

  khtml::EditorContext editor_context;

  QString m_overURL;
  QString m_overURLTarget;

  bool m_bDnd;
  bool m_bFirstData;
  bool m_bStrictModeQuirk;
  bool m_bClearing;
  bool m_bCleared;
  bool m_focusNodeRestored;

  int m_focusNodeNumber;

  QPoint m_dragStartPos;
#ifdef KHTML_NO_SELECTION
  QPoint m_dragLastPos;
#endif

  bool m_designMode;
  bool m_caretMode;

  QCursor m_linkCursor;
  QTimer m_scrollTimer;

  unsigned long m_loadedObjects;
  unsigned long m_totalObjectCount;
  unsigned int m_jobPercent;

  KHTMLPart::FormNotification m_formNotification;
  QTimer m_progressUpdateTimer;

  QStringList m_pluginPageQuestionAsked;

  KHTMLFind m_find;
  StorePass m_storePass;

  KJSErrorDlg *m_jsedlg;

  //QGuardedPtr<KParts::Part> m_activeFrame;
  KParts::Part * m_activeFrame;
  QPointer<KHTMLPart> m_opener;
  bool m_openedByJS;
  bool m_newJSInterpreterExists; // set to 1 by setOpenedByJS, for window.open

  void setFlagRecursively(bool KHTMLPartPrivate::*flag, bool value);

  time_t m_userStyleSheetLastModified;
  
  QSet<QString> m_lookedupHosts;
  static bool s_dnsInitialised;

#ifndef KHTML_NO_WALLET
  KHTMLWalletQueue *m_wq;
#endif

  // Does determination of how we should handle the given type, as per HTML5 rules
  MimeType classifyMimeType(const QString& mime);

  void clearRedirection();

  bool isLocalAnchorJump(const KUrl& url);
  void executeAnchorJump(const KUrl& url, bool lockHistory);

  static bool isJavaScriptURL(const QString& url);
  static QString codeForJavaScriptURL(const QString& url);
  void executeJavascriptURL(const QString &u);

  bool isInPageURL(const QString& url) {
    return isLocalAnchorJump(KUrl(url)) || isJavaScriptURL(url);
  }

  void executeInPageURL(const QString& url, bool lockHistory) {
    KUrl kurl(url);
    if (isLocalAnchorJump(kurl))
      executeAnchorJump(kurl, lockHistory);
    else
      executeJavascriptURL(url);
  }

  void propagateInitialDomainTo(KHTMLPart* kid);

  void renameFrameForContainer(DOM::HTMLPartContainerElementImpl* cont,
                               const QString& newName);

  // Check whether the frame is fully loaded.
  // The return value doesn't consider any pending redirections.
  // If the return value is true, however, pendingRedirections will
  // report if there are any
  bool isFullyLoaded(bool* pendingRedirections) const;
};

#endif
