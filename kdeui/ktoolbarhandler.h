#ifndef KBARHANDLER_H
#define KBARHANDLER_H

#include <qobject.h>
#include <qguardedptr.h>
#include <kxmlguiclient.h>

class KMainWindow;
class KToolBar;

namespace KDEPrivate
{

class ToolBarHandler : public QObject,
                       public KXMLGUIClient
{
    Q_OBJECT
public:
    ToolBarHandler( KMainWindow *mainWindow, const char *name = 0 );
    ToolBarHandler( KMainWindow *mainWindow, QObject *parent, const char *name = 0 );
    virtual ~ToolBarHandler();

    KAction *toolBarMenuAction();

public slots:
    void setupActions();

private slots:
    void clientAdded( KXMLGUIClient *client );

private:
    void init( KMainWindow *mainWindow );
    void connectToActionContainers();
    void connectToActionContainer( KAction *action );
    void connectToActionContainer( QWidget *container );

    struct Data;
    Data *d;

    QGuardedPtr<KMainWindow> m_mainWindow;
    QPtrList<KAction> m_actions;
    QPtrList<KToolBar> m_toolBars;
};

} // namespace KDEPrivate

#endif // KBARHANDLER_H

/* vim: et sw=4 ts=4
 */
