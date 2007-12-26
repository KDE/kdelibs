#ifndef DEBUGDOCUMENT_H
#define DEBUGDOCUMENT_H

#include <QHash>
#include <QObject>
#include <QVector>

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
    int baseLine;
    QString source;
};

class DebugDocument : public QObject, public khtml::Shared<DebugDocument>
{
    Q_OBJECT
public:
    typedef SharedPtr<DebugDocument> Ptr;

    DebugDocument(const QString& url, const QString& iuKey);
    ~DebugDocument();

    QString name() const;
    QString url() const;
    QString iuKey() const;

    KTextEditor::Document* viewerDocument();
    KTextEditor::View*     viewerView();

    SourceFragment fragment(int sourceId);

    void addCodeFragment(int sourceId, int baseLine, const QString &source);

    enum FragmentInfo
    {
        LastFragment,
        NonLastFragment
    };

    /**
     Tells the document that the fragment with this ID no longer exists.
     Returns whether it was the last fragment or not.
    */
    FragmentInfo deleteFragment(int sourceId);

    void setBreakpoint(int lineNumber);
    void removeBreakpoint(int lineNumber);
    bool hasBreakpoint(int lineNumber);
signals:
    void documentDestroyed(KJSDebugger::DebugDocument*);
private:
    QString m_url;
    QString m_iuKey;
    QString m_name;

    void buildViewerDocument();
    void setupViewerDocument();

    QHash<int, SourceFragment> m_codeFragments;

    // We store breakpoints differently for scopes with URL
    // and without it. Those that have it are stored globally,
    // so that breakpoints persist accross multiple visits of the
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
