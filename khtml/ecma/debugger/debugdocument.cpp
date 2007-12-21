#include "debugdocument.h"

#include <QHash>
#include <QVector>
#include <QStringList>

#include "kjs_binding.h"
#include "khtml_part.h"

using namespace KJS;
using namespace KJSDebugger;

class DebugDocument::Private : public QSharedData
{
public:
    QString url;
    QString name;
    QString source;
    Interpreter *interpreter;
    QHash<int, SourceFragment> codeFragments;
    QVector<int> breakpoints;
};

DebugDocument::DebugDocument(const QString &url, Interpreter *interpreter)
{
    d = new DebugDocument::Private;
    d->url = url;
    d->interpreter = interpreter;

    KUrl kurl(url);
    d->name = kurl.fileName();

    // Might have to fall back in case of query-like things;
    // ad scripts tend to do that
    while (d->name.contains("=") || d->name.contains("&")) {
        kurl = kurl.upUrl();
        d->name = kurl.fileName();
    }

    if (d->name.isEmpty())
        d->name = kurl.host();

    if (d->name.isEmpty())
        d->name = "????"; //Probably better than un-i18n'd 'undefined'...
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
