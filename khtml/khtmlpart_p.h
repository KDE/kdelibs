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
 *                     2001-2003 George Stiakos <staikos@kde.org>
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
#include <kparts/statusbarextension.h>
#include <kparts/browserextension.h>
#include <kwallet.h>

#include <qguardedptr.h>
#include <qmap.h>
#include <qtimer.h>
#include <qvaluelist.h>

#include "html/html_formimpl.h"
#include "khtml_run.h"
#include "khtml_factory.h"
#include "khtml_events.h"
#include "khtml_ext.h"
#include "khtml_iface.h"
#include "khtml_settings.h"
#include "misc/decoder.h"
#include "ecma/kjs_proxy.h"

class KFind;
class KFindDialog;
class KPopupMenu;
class KSelectAction;
class KURLLabel;
class KJavaAppletContext;
class KJSErrorDlg;

namespace KIO
{
  class Job;
  class TransferJob;
}
namespace KParts
{
  class StatusBarExtension;
}

namespace khtml
{
  class ChildFrame : public QObject
  {
      Q_OBJECT
  public:
      enum Type { Frame, IFrame, Object };

      ChildFrame() : QObject (0L, "khtml_child_frame") {
          m_jscript = 0L;
          m_kjs_lib = 0;
          m_bCompleted = false; m_bPreloaded = false; m_type = Frame; m_bNotify = false;
          m_bPendingRedirection = false;
      }

      ~ChildFrame() {
          if (m_run) m_run->abort();
          delete m_jscript;
          if ( m_kjs_lib)
              m_kjs_lib->unload();
      }

    QGuardedPtr<khtml::RenderPart> m_frame;
    QGuardedPtr<KParts::ReadOnlyPart> m_part;
    QGuardedPtr<KParts::BrowserExtension> m_extension;
    QGuardedPtr<KParts::LiveConnectExtension> m_liveconnect;
    QString m_serviceName;
    QString m_serviceType;
    KJSProxy *m_jscript;
    KLibrary *m_kjs_lib;
    bool m_bCompleted;
    QString m_name;
    KParts::URLArgs m_args;
    QGuardedPtr<KHTMLRun> m_run;
    bool m_bPreloaded;
    KURL m_workingURL;
    Type m_type;
    QStringList m_params;
    bool m_bNotify;
    bool m_bPendingRedirection;
  protected slots:
    void liveConnectEvent(const unsigned long, const QString&, const KParts::LiveConnectExtension::ArgList&);
  };

}

struct KHTMLFrameList : public QValueList<khtml::ChildFrame*>
{
    Iterator find( const QString &name ) KDE_NO_EXPORT;
};

typedef KHTMLFrameList::ConstIterator ConstFrameIt;
typedef KHTMLFrameList::Iterator FrameIt;

static int khtml_part_dcop_counter = 0;


class KHTMLWalletQueue : public QObject
{
  Q_OBJECT
  public:
    KHTMLWalletQueue(QObject *parent) : QObject(parent) {
      wallet = 0L;
    }

    virtual ~KHTMLWalletQueue() {
      delete wallet;
      wallet = 0L;
    }

    KWallet::Wallet *wallet;
    typedef QPair<DOM::HTMLFormElementImpl*, QGuardedPtr<DOM::DocumentImpl> > Caller;
    typedef QValueList<Caller> CallerList;
    CallerList callers;
    QValueList<QPair<QString, QMap<QString, QString> > > savers;

  signals:
    void walletOpened(KWallet::Wallet*);

  public slots:
    void walletOpened(bool success) {
      if (!success) {
        delete wallet;
        wallet = 0L;
      }
      emit walletOpened(wallet);
      if (wallet) {
        if (!wallet->hasFolder(KWallet::Wallet::FormDataFolder())) {
          wallet->createFolder(KWallet::Wallet::FormDataFolder());
        }
        for (CallerList::Iterator i = callers.begin(); i != callers.end(); ++i) {
          if ((*i).first && (*i).second) {
            (*i).first->walletOpened(wallet);
          }
        }
        wallet->setFolder(KWallet::Wallet::FormDataFolder());
        for (QValueList<QPair<QString, QMap<QString, QString> > >::Iterator i = savers.begin(); i != savers.end(); ++i) {
          wallet->writeMap((*i).first, (*i).second);
        }
      }
      callers.clear();
      savers.clear();
      wallet = 0L; // gave it away
    }
};

class KHTMLPartPrivate
{
  KHTMLPartPrivate(const KHTMLPartPrivate & other);
public:
  KHTMLPartPrivate(QObject* parent)
  {
    m_doc = 0L;
    m_decoder = 0L;
    m_wallet = 0L;
    m_bWalletOpened = false;
    m_runningScripts = 0;
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
    m_extendAtEnd = true;
    m_linkCursor = KCursor::handCursor();
    m_loadedObjects = 0;
    m_totalObjectCount = 0;
    m_jobPercent = 0;
    m_haveEncoding = false;
    m_activeFrame = 0L;
    m_find = 0;
    m_findDialog = 0;
    m_ssl_in_use = false;
    m_jsedlg = 0;
    m_formNotification = KHTMLPart::NoNotification;

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

    m_caretMode = false;
    m_designMode = false;

    m_metaRefreshEnabled = true;
    m_statusMessagesEnabled = true;

    m_bFirstData = true;
    m_submitForm = 0;
    m_delayRedirect = 0;
    m_autoDetectLanguage = khtml::Decoder::SemiautomaticDetection;

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
            m_caretMode = part->d->m_caretMode;
            m_designMode = part->d->m_designMode;
            m_zoomFactor = part->d->m_zoomFactor;
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
    m_dcopobject = 0;
    m_jobspeed = 0;
    m_dcop_counter = ++khtml_part_dcop_counter;
    m_statusBarWalletLabel = 0L;
    m_statusBarUALabel = 0L;
    m_statusBarJSErrorLabel = 0L;
    m_userStyleSheetLastModified = 0;
    m_wq = 0;
  }
  ~KHTMLPartPrivate()
  {
    delete m_dcopobject;
    delete m_statusBarExtension;
    delete m_extension;
    delete m_settings;
    delete m_wallet;
#ifndef Q_WS_QWS
    //delete m_javaContext;
#endif
  }

  QGuardedPtr<khtml::ChildFrame> m_frame;
  KHTMLFrameList m_frames;
  KHTMLFrameList m_objects;

  QGuardedPtr<KHTMLView> m_view;
  KHTMLPartBrowserExtension *m_extension;
  KParts::StatusBarExtension *m_statusBarExtension;
  KHTMLPartBrowserHostExtension *m_hostExtension;
  KURLLabel* m_statusBarIconLabel;
  KURLLabel* m_statusBarWalletLabel;
  KURLLabel* m_statusBarUALabel;
  KURLLabel* m_statusBarJSErrorLabel;
  DOM::DocumentImpl *m_doc;
  khtml::Decoder *m_decoder;
  QString m_encoding;
  QString m_sheetUsed;
  long m_cacheId;
  QString scheduledScript;
  DOM::Node scheduledScriptNode;

  KWallet::Wallet* m_wallet;
  int m_runningScripts;
  bool m_bOpenMiddleClick :1;
  bool m_bBackRightClick :1;
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
  bool m_statusMessagesEnabled :1;
  bool m_bWalletOpened :1;
  int m_frameNameId;
  int m_dcop_counter;
  DCOPObject *m_dcopobject;

#ifndef Q_WS_QWS
  KJavaAppletContext *m_javaContext;
#endif

  KHTMLSettings *m_settings;

  KIO::TransferJob * m_job;

  QString m_statusBarText[3];
  unsigned long m_jobspeed;
  QString m_lastModified;
  QString m_httpHeaders;
  QString m_pageServices;

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
          m_ssl_cert_state,
          m_ssl_parent_ip,
          m_ssl_parent_cert;

  bool m_bComplete:1;
  bool m_bLoadEventEmitted:1;
  bool m_haveEncoding:1;
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
  KAction *m_paViewInfo;
  KAction *m_paSaveBackground;
  KAction *m_paSaveDocument;
  KAction *m_paSaveFrame;
  KAction *m_paSecurity;
  KActionMenu *m_paSetEncoding;
  KSelectAction *m_paUseStylesheet;
  KHTMLZoomFactorAction *m_paIncZoomFactor;
  KHTMLZoomFactorAction *m_paDecZoomFactor;
  KAction *m_paLoadImages;
  KAction *m_paFind;
  KAction *m_paFindNext;
  KAction *m_paPrintFrame;
  KAction *m_paSelectAll;
  KAction *m_paDebugScript;
  KAction *m_paDebugDOMTree;
  KAction *m_paDebugRenderTree;
  KAction *m_paStopAnimations;
  KToggleAction *m_paToggleCaretMode;

  KParts::PartManager *m_manager;

  QString m_popupMenuXML;
  KHTMLPart::GUIProfile m_guiProfile;

  int m_zoomFactor;

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

  // simply using the selection limits for the caret position does not suffice
  // as we need to know on which side to extend the selection
//  DOM::Node m_caretNode;	// node containing the caret
//  long m_caretOffset;		// offset within this node (0-based)

  // the caret uses the selection variables for its position. If m_extendAtEnd
  // is true, m_selectionEnd and m_endOffset contain the mandatory caret
  // position, otherwise it's m_selectionStart and m_startOffset.
  DOM::Node m_selectionStart;
  long m_startOffset;
  DOM::Node m_selectionEnd;
  long m_endOffset;
  DOM::Node m_initialNode;	// (Node, Offset) pair on which the
  long m_initialOffset;		// selection has been initiated
  QString m_overURL;
  QString m_overURLTarget;

  bool m_startBeforeEnd:1;
  bool m_extendAtEnd:1;		// true if selection is to be extended at its end
  enum { ExtendByChar, ExtendByWord, ExtendByLine } m_extendMode:2;
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

  /////////// 'Find' feature
  struct StringPortion
  {
      // Just basic ref/deref on our node to make sure it doesn't get deleted
      StringPortion( int i, DOM::NodeImpl* n ) : index(i), node(n) { if (node) node->ref(); }
      StringPortion() : index(0), node(0) {} // for QValueList
      StringPortion( const StringPortion& other ) : node(0) { operator=(other); }
      StringPortion& operator=( const StringPortion& other ) {
          index=other.index;
          if (other.node) other.node->ref();
          if (node) node->deref();
          node=other.node;
          return *this;
      }
      ~StringPortion() { if (node) node->deref(); }

      int index;
      DOM::NodeImpl *node;
  };
  QValueList<StringPortion> m_stringPortions;

  KFind *m_find;
  KFindDialog *m_findDialog;

  struct findState
  {
    findState()
    { options = 0; }
    QStringList history;
    QString text;
    int options;
  };

  findState m_lastFindState;

  KJSErrorDlg *m_jsedlg;

  DOM::NodeImpl *m_findNode; // current node
  DOM::NodeImpl *m_findNodeEnd; // end node
  int m_findPos; // current pos in current node
  int m_findPosEnd; // pos in end node
  /////////

  //QGuardedPtr<KParts::Part> m_activeFrame;
  KParts::Part * m_activeFrame;
  QGuardedPtr<KHTMLPart> m_opener;
  bool m_openedByJS;
  bool m_newJSInterpreterExists; // set to 1 by setOpenedByJS, for window.open

  khtml::Decoder::AutoDetectLanguage m_autoDetectLanguage;
  KPopupMenu *m_automaticDetection;
  KSelectAction *m_manualDetection;

  void setFlagRecursively(bool KHTMLPartPrivate::*flag, bool value);
  /** returns the caret node */
  DOM::Node &caretNode() {
    return m_extendAtEnd ? m_selectionEnd : m_selectionStart;
  }
  /** returns the caret offset */
  long &caretOffset() {
    return m_extendAtEnd ? m_endOffset : m_startOffset;
  }

  time_t m_userStyleSheetLastModified;

  KHTMLWalletQueue *m_wq;
};

#endif
