#ifndef __khtml_ext_h__
#define __khtml_ext_h__

#include "khtml_part.h"
#include <kaction.h>

/**
 * This is the BrowserExtension for a @ref KHTMLPart document. Please see the KParts documentation for
 * more information about the BrowserExtension.
 */
class KHTMLPartBrowserExtension : public KParts::BrowserExtension
{
  Q_OBJECT
  friend class KHTMLPart;
  friend class KHTMLView;
public:
  KHTMLPartBrowserExtension( KHTMLPart *parent, const char *name = 0L );

  virtual int xOffset();
  virtual int yOffset();

  virtual void saveState( QDataStream &stream );
  virtual void restoreState( QDataStream &stream );

public slots:
  void copy();
  void reparseConfiguration();
  void print();

private:
  KHTMLPart *m_part;
};

class KHTMLPartBrowserHostExtension : public KParts::BrowserHostExtension
{
public:
  KHTMLPartBrowserHostExtension( KHTMLPart *part );
  virtual ~KHTMLPartBrowserHostExtension();

  virtual QStringList frameNames() const;

  virtual const QList<KParts::ReadOnlyPart> frames() const;

  virtual bool openURLInFrame( const KURL &url, const KParts::URLArgs &urlArgs );
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

  static void saveURL( QWidget *parent, const QString &caption, const KURL &url );

private slots:
  void slotSaveLinkAs();
  void slotSaveImageAs();
  void slotCopyLinkLocation();
  void slotCopyImageLocation();
  void slotReloadFrame();
private:
  class KHTMLPopupGUIClientPrivate;
  KHTMLPopupGUIClientPrivate *d;
};

class KHTMLFontSizeAction : public KAction
{
    Q_OBJECT
public:
    KHTMLFontSizeAction( KHTMLPart *part, bool direction, const QString &text, const QString &icon, const QObject *receiver, const char *slot, QObject *parent, const char *name );
    ~KHTMLFontSizeAction();
    
    virtual int plug( QWidget *w, int index );
    
private slots:
    void slotActivated( int );
private:
    QPopupMenu *m_popup;
    bool m_direction;
    KHTMLPart *m_part;
};

#endif
