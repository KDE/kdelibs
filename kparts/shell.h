#ifndef OFFICE_SHELL_H
#define OFFICE_SHELL_H

#include <qmainwindow.h>
#include <qlist.h>
#include <qtoolbar.h>
#include <qaction.h>

class QPopupMenu;
class QString;
class QStatusBar;

class Part;
class View;

class Shell : public QMainWindow
{
    Q_OBJECT
public:
    enum SelectionPolicy { Direct, TriState };

    Shell( QWidget* parent = 0, const char* name = 0 );
    ~Shell();

    virtual void initShell();

    QActionCollection* actionCollection();

    void setRootPart( Part* );
    Part* rootPart();
    View* rootView();

    /**
     * Setting the active view to 0 shows only the menus of
     * the shell, otherwise the menus and toolbars of the
     * view will be shown, too.
     *
     * @ref #createToolBars
     * @ref #createMenuBar
     */
    void setActiveView( View* view, Part* part = 0 );
    View* activeView();
    Part* activePart();

    void setSelectedView( View* view, Part* part = 0 );
    View* selectedView();
    Part* selectedPart();

    void setSelectionPolicy( SelectionPolicy );
    SelectionPolicy selectionPolicy();

    QStatusBar *createStatusBar();

protected:
    void createToolBars( const QDomElement& element );
    void createMenuBar( const QDomElement& shell, const QDomElement& part );
    QPopupMenu* createMenu( const QDomElement& shell, const QDomElement& part );

    /**
     * Creates a toolbar for a view. The bar is appended to @ref #m_toolbars
     * and will be destructed if the currently active view becomes deactivated.
     *
     * @ref #createToolBars
     */
    QToolBar* createToolBar( const char* name = 0 );

    /**
     * This event filter detected focus changes. This is used
     * to detect when the user wants to activate another view.
     */
    bool eventFilter( QObject*, QEvent* );

    QString readConfigFile( const QString& filename ) const;
    virtual QString configFile() const = 0;

    QAction* action( const char* name, bool shell = FALSE );

private:
    View* m_activeView;
    Part* m_activePart;

    View* m_selectedView;
    Part* m_selectedPart;

    Part* m_rootPart;

    QList<QToolBar> m_toolbars;

    QActionCollection m_collection;

    SelectionPolicy m_policy;

    QStatusBar *m_statusBar;
};

#endif
