#ifndef DEBUGDOCUMENT_H
#define DEBUGDOCUMENT_H

#include <QSharedDataPointer>
#include <QMetaType>

namespace KJS {
    class Interpreter;
}

namespace KJSDebugger {

struct CallStackEntry;
struct BreakPoint;
struct SourceFragment;

class DebugDocument
{
public:
    DebugDocument(const QString &url, KJS::Interpreter *interpreter);
    DebugDocument(const DebugDocument &other);
    ~DebugDocument();

    QString name() const;
    QString url() const;
    KJS::Interpreter *interpreter() const;

    QList<SourceFragment> fragments();
    SourceFragment fragment(int sourceId);
    void addCodeFragment(int sourceId, int baseLine, const QString &source);
    bool deleteFragment(int sourceId);

    QString source() const;

    QVector<int> breakpoints();
    void setBreakpoint(int lineNumber);
    void removeBreakpoint(int lineNumber);
    bool hasBreakpoint(int lineNumber);

private:
    class Private;
    QSharedDataPointer<Private> d;

};

struct BreakPoint
{
    int lineNumber;
    int column;
    DebugDocument *document;
};

struct SourceFragment
{
    int sourceId;
    int baseLine;
    QString source;
    bool operator < (const SourceFragment& other) const
    {
        return baseLine < other.baseLine;
    }
};

}
Q_DECLARE_METATYPE(KJSDebugger::DebugDocument*)

#endif
