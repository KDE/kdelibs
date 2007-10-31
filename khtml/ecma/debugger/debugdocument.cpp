#include "debugdocument.h"

#include <QHash>
#include <QVector>
#include <QStringList>

#include "kjs_binding.h"
#include "khtml_part.h"

using namespace KJS;
class DebugDocument::Private : public QSharedData
{
public:
    QString url;
    QString name;
    QString source;
    Interpreter *interpreter;
    QHash<int, SourceFragment> codeFragments;
    QVector<int> breakpoints;
    QVector<CallStackEntry> callStack;
};

DebugDocument::DebugDocument(const QString &url, Interpreter *interpreter)
{
    d = new DebugDocument::Private;
    d->url = url;
    d->interpreter = interpreter;

    QStringList splitUrl = url.split('/');
    if (!splitUrl.isEmpty())
    {
        while (d->name.isEmpty() && !splitUrl.isEmpty())
            d->name = splitUrl.takeLast();

        if (d->name.isEmpty())
            d->name = "undefined";
    }
    else
        d->name = "undefined";
}

DebugDocument::DebugDocument(const DebugDocument &other)
{
    d = other.d;
}

DebugDocument::~DebugDocument()
{
}

QString DebugDocument::name() const
{
    return d->name;
}

QString DebugDocument::url() const
{
    return d->url;
}

Interpreter * DebugDocument::interpreter() const
{
    return d->interpreter;
}

QList<SourceFragment> DebugDocument::fragments()
{
    return d->codeFragments.values();
}

bool DebugDocument::deleteFragment(int sourceId)
{
    if (d->codeFragments.contains(sourceId))
    {
        d->codeFragments.remove(sourceId);
        return true;
    }
    return false;
}

SourceFragment DebugDocument::fragment(int sourceId)
{
    if (d->codeFragments.contains(sourceId))
        return d->codeFragments[sourceId];
    else
        return SourceFragment();
}

QString DebugDocument::source() const
{
    return d->source;
}

void DebugDocument::addCodeFragment(int sourceId, int baseLine, const QString &source)
{
    SourceFragment code;
    code.sourceId = sourceId;
    code.baseLine = baseLine;
    code.source = source;

    d->codeFragments[sourceId] = code;
}

void DebugDocument::setBreakpoint(int lineNumber)
{
    d->breakpoints.append(lineNumber);
}

void DebugDocument::removeBreakpoint(int lineNumber)
{
    int idx = d->breakpoints.indexOf(lineNumber);
    if (idx != -1)
        d->breakpoints.remove(idx);
}

bool DebugDocument::hasBreakpoint(int lineNumber)
{
    return d->breakpoints.contains(lineNumber);
}

QVector<int> DebugDocument::breakpoints()
{
    return d->breakpoints;
}

QVector<CallStackEntry> DebugDocument::callStack()
{
    return d->callStack;
}

void DebugDocument::addCall(const QString &function, int lineNumber)
{
    CallStackEntry entry;
    entry.name = function;
    entry.lineNumber = lineNumber;

    d->callStack.append(entry);
}

void DebugDocument::removeCall(const QString &function, int lineNumber)
{
    CallStackEntry entry;
    entry.name = function;
    entry.lineNumber = lineNumber;

    int idx = d->callStack.indexOf(entry);
    kDebug() << "DebugDocument::removeCall(..) idx = " << idx;
    if (idx != -1)
        d->callStack.remove(idx);
}

