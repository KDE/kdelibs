#ifndef DEBUGDOCUMENT_H
#define DEBUGDOCUMENT_H

#include <QSharedDataPointer>
#include <QMetaType>

namespace KJS
{

struct CallStackEntry;
struct BreakPoint;
struct SourceFragment;
class Interpreter;

class DebugDocument
{
public:
    DebugDocument(const QString &url, Interpreter *interpreter);
    DebugDocument(const DebugDocument &other);
    ~DebugDocument();

    QString name() const;
    QString url() const;
    Interpreter *interpreter() const;

    QList<SourceFragment> fragments();
    SourceFragment fragment(int sourceId);
    void addCodeFragment(int sourceId, int baseLine, const QString &source);
    bool deleteFragment(int sourceId);

    QString source() const;

    QVector<int> breakpoints();
    void setBreakpoint(int lineNumber);
    void removeBreakpoint(int lineNumber);
    bool hasBreakpoint(int lineNumber);

    QVector<CallStackEntry> callStack();
    void addCall(const QString&, int);
    void removeCall(const QString&, int);

private:
    class Private;
    QSharedDataPointer<Private> d;

};


struct CallStackEntry
{
    QString name;
    int lineNumber;

    bool operator==(const CallStackEntry& other) const        // you're being lazy..
    {
        return ((other.name == name) && (other.lineNumber == lineNumber));
    }
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
};

}
Q_DECLARE_METATYPE(KJS::DebugDocument*)

#endif
