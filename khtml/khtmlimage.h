#ifndef __khtmlimage_h__
#define __khtmlimage_h__

#include <kparts/part.h>
#include <kparts/factory.h>
#include <kparts/browserextension.h>

class KHTMLPart;
class KInstance;

class KHTMLImageFactory : public KParts::Factory
{
    Q_OBJECT
public:
    KHTMLImageFactory();
    virtual ~KHTMLImageFactory();

    virtual KParts::Part *createPartObject( QWidget *parentWidget, const char *widgetName,
                                            QObject *parent, const char *name,
                                            const char *className, const QStringList &args );

    static KInstance *instance() { return s_instance; }

private:
    static KInstance *s_instance;
};

class KHTMLImage : public KParts::ReadOnlyPart
{
    Q_OBJECT
public:
    KHTMLImage( QWidget *parentWidget, const char *widgetName,
                QObject *parent, const char *name );
    virtual ~KHTMLImage();

    virtual bool openFile() { return true; } // grmbl, should be non-pure in part.h, IMHO

    virtual bool openURL( const KURL &url );

    virtual bool closeURL();

protected:
    virtual void guiActivateEvent( KParts::GUIActivateEvent *e );

private slots:
    void slotPopupMenu( KXMLGUIClient *cl, const QPoint &pos, const KURL &u, const QString &mime, mode_t mode );
    void slotImageJobFinished( KIO::Job *job );

private:
    KHTMLPart *m_khtml;
    KParts::BrowserExtension *m_ext;
    QString m_mimeType;
};

#endif
