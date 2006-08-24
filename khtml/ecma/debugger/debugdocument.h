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
    void readSource();

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


/*
class DebugDocumentModel : public QAbstractListModel
{
public:
    enum Type
    {
        Breakpoint,
        Callstack
    };

    DebugDocumentModel(Type type, QObject *parent = 0);
    ~DebugDocumentModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    Type type() const;
    void setType(Type);

public slots:
    void addDocument(DebugDocument*);
    void removeDocument(DebugDocument*);

private:
    Type m_type;
    QList<DebugDocument*> m_documents;
};
*/

};
Q_DECLARE_METATYPE(KJS::DebugDocument*)

#endif
