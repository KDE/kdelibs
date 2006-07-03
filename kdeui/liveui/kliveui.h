#ifndef GUIEDITOR_H
#define GUIEDITOR_H

#include <QString>
#include <QObject>
#include <QKeySequence>
#include <QPointer>
#include <QMainWindow>

class GuiEditorPrivate;
class QAction;
class QString;
class QWidget;
class QMenu;
class QToolBar;
class QIcon;
class GuiEditor;

class GuiEditorComponentInterface
{
    friend class GuiEditor;
public:
    inline virtual ~GuiEditorComponentInterface() {}
    
    QObject *qObject() { return static_cast<QObject *>(qt_metacast("QObject")); }

    void activateComponentGui(QMainWindow *mw);
    void deactivateComponentGui(QMainWindow *mw);

protected:
    virtual void buildGui() = 0;
private:
    QPointer<QMainWindow> currentGuiEditorMainWindow;
    virtual void *qt_metacast(const char *) = 0;
};

Q_DECLARE_INTERFACE(GuiEditorComponentInterface, "org.kde.framework.GuiEditorComponentInterfaces/1.0")

class GuiEditor
{
public:
    explicit GuiEditor(QMainWindow *mw);
    explicit GuiEditor(QObject *plugin);
    GuiEditor();
    ~GuiEditor();

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
    GuiEditorPrivate *d;
    Q_DISABLE_COPY(GuiEditor)
};

#endif // GUIEDITOR_H

