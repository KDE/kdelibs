#ifndef KFILEBOOKMARKHANDLER_H
#define KFILEBOOKMARKHANDLER_H

#include <kbookmarkmanager.h>
#include <kbookmarkmenu.h>

class QTextStream;
class KPopupMenu;

class KFileBookmarkHandler : public QObject, public KBookmarkOwner
{
    Q_OBJECT

public:
    KFileBookmarkHandler( KFileDialog *dialog );
    ~KFileBookmarkHandler();

    QPopupMenu * popupMenu();

    // KBookmarkOwner interface:
    virtual void openBookmarkURL( const QString& url ) { emit openURL( url ); }
    virtual QString currentURL() const;

    KPopupMenu *menu() const { return m_menu; }

signals:
    void openURL( const QString& url );

private slots:
    // for importing
    void slotNewBookmark( const QString& text, const QCString& url,
                          const QString& additionalInfo );
    void slotNewFolder( const QString& text, bool open,
                        const QString& additionalInfo );
    void newSeparator();
    void endFolder();

private:
    void importOldBookmarks( const QString& path, const QString& destinationPath );

    KFileDialog *m_dialog;
    KPopupMenu *m_menu;
    KBookmarkMenu *m_bookmarkMenu;

    QTextStream *m_importStream;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KFileBookmarkHandlerPrivate;
    KFileBookmarkHandlerPrivate *d;
};


#endif // KFILEBOOKMARKHANDLER_H
