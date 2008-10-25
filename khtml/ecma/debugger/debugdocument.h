#ifndef DEBUGDOCUMENT_H
#define DEBUGDOCUMENT_H

#include <QHash>
#include <QObject>
#include <QVector>
#include <QStringList>

#include "misc/shared.h"

namespace KJS {
    class Interpreter;
}

namespace KTextEditor {
    class Document;
    class View;
    class Editor;
}

namespace KJSDebugger {

struct SourceFragment
{
    int sourceId;
    int baseLine; // Note: this is stored 0-based
    QStringList sourceLines;

    int lastLine() const
    {
        return baseLine + sourceLines.size() - 1;
    }

    bool inRange(int otherFirst, int otherLast) const
    {
        if (lastLine() < otherFirst)
            return false;
        if (baseLine > otherLast)
            return false;
        return true;
    }
};

class DebugDocument : public QObject, public khtml::Shared<DebugDocument>
{
    Q_OBJECT
public:
    typedef SharedPtr<DebugDocument> Ptr;

    DebugDocument(KJS::Interpreter* interp, const QString& url, const QString& iuKey);
    ~DebugDocument();

    QString name() const;
    QString url() const;
    QString iuKey() const;

    KTextEditor::Document* viewerDocument();
    KTextEditor::View*     viewerView();

    SourceFragment fragment(int sourceId);
    void addCodeFragment(int sourceId, int baseLine, const QString &source);
    QList<int> fragments() const;

    // Tells the document that any new updates will come on a fresh
    // load, so the fragments have to be discarded on a next addCodeFragment
    void requestDeferredClear();

    void setBreakpoint(int lineNumber);
    void removeBreakpoint(int lineNumber);
    bool hasBreakpoint(int lineNumber);
    
    // We keep track of whether documents have functions, since we can't discard
    // eval contexts that do
    bool hasFunctions();
    void setHasFunctions();
    
    KJS::Interpreter* interpreter(); 

signals:
    void documentDestroyed(KJSDebugger::DebugDocument*);
private:
    QString m_url;
    QString m_iuKey;
    QString m_name;
    KJS::Interpreter* m_interpreter;
    
    bool m_hasFunctions;

    // This is set to true when we are rebuilding the document.
    // in that case, the UI might get undesired mark add/remove events,
    // and update the breakpoint set accordingly --- such as removing all of them
    // on clear. 
    bool m_rebuilding;

    // see requestDeferredClear
    bool m_deferredClear;

    void rebuildViewerDocument(int firstLine = 0, int lastLine = -1);
    void setupViewerDocument();

    QHash<int, SourceFragment> m_codeFragments;

    // We store breakpoints differently for scopes with URL
    // and without it. Those that have it are stored globally,
    // so that breakpoints persist across multiple visits of the
    // document. Those for eval, etc., are only associated with the
    // sid (and are likely useless :( )
    static QHash<QString, QVector<int> >* s_perUrlBreakPoints;
    QVector<int> m_localBreakpoints;

    QVector<int>& breakpoints();

    KTextEditor::Document* m_kteDoc;
    KTextEditor::View*     m_kteView;
    static KTextEditor::Editor* s_kate;
    static KTextEditor::Editor* kate();
};

}


#endif
