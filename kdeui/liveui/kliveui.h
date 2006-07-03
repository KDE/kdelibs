#ifndef KLIVEUI_H
#define KLIVEUI_H

#include <QString>
#include <QObject>
#include <QKeySequence>
#include <QPointer>
#include <QMainWindow>

#include <kdelibs_export.h>

class KLiveUiBuilderPrivate;
class QAction;
class QString;
class QWidget;
class QMenu;
class QToolBar;
class QIcon;
class KLiveUiBuilder;

class KDEUI_EXPORT KLiveUiComponent
{
    friend class KLiveUiBuilder;
public:
    inline virtual ~KLiveUiComponent() {}
    
    QObject *qObject() { return static_cast<QObject *>(qt_metacast("QObject")); }

    void activateComponentGui(QMainWindow *mw);
    void deactivateComponentGui(QMainWindow *mw);

protected:
    virtual void buildGui() = 0;
private:
    QPointer<QMainWindow> currentBuilderMainWindow;
    virtual void *qt_metacast(const char *) = 0;
};

Q_DECLARE_INTERFACE(KLiveUiComponent, "org.kde.framework.KLiveUiComponent/1.0")

class KDEUI_EXPORT KLiveUiBuilder
{
public:
    explicit KLiveUiBuilder(QMainWindow *mw);
    explicit KLiveUiBuilder(QObject *plugin);
    KLiveUiBuilder();
    ~KLiveUiBuilder();

    void begin(QMainWindow *mw);
    void begin(QObject *plugin);
    void end();

    void beginMenuBar();

    QMenu *beginMenu(const QString &name, const QString &title);
    void endMenu();

    QToolBar *beginToolBar(const QString &title = QString());
    void endToolBar();

    void addAction(QAction *action);
    void addActions(const QList<QAction *> actions);
    
    // convenience
    QAction *addAction(const QString &text);
    QAction *addAction(const QIcon &icon, const QString &text);
    QAction *addAction(const QString &text, const QObject *receiver, const char *member,
                       const QKeySequence &shortcut = QKeySequence());
    QAction *addAction(const QIcon &icon, const QString &text,
                       const QObject *receiver, const char *member,
                       const QKeySequence &shortcut = QKeySequence());
    
    QAction *addSeparator();
    QAction *addWidget(QWidget *widget);

    void addActionGroup(const QString &name);

    void beginActionGroup(const QString &name);
    void endActionGroup();
    
    void populateFromXmlGui(const QString &fileName);

private:
    KLiveUiBuilderPrivate *d;
    Q_DISABLE_COPY(KLiveUiBuilder)
};

#endif // KLIVEUI_H

