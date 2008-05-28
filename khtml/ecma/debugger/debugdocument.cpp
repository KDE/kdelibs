#include "debugdocument.h"
#include "debugwindow.h"

#include <QHash>
#include <QVector>
#include <QStringList>
#include <QApplication>

#include "kjs_binding.h"
#include "khtml_part.h"

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/configinterface.h>
#include <ktexteditor/markinterface.h>
#include <kiconloader.h>
#include <kmessagebox.h>

using namespace KJS;
using namespace KJSDebugger;

DebugDocument::DebugDocument(KJS::Interpreter* intp, const QString& url, const QString& iuKey)
{
    m_interpreter = intp;
    m_url   = url;
    m_iuKey = iuKey;

    KUrl kurl(url);
    m_name = kurl.fileName();

    // Might have to fall back in case of query-like things;
    // ad scripts tend to do that
    while (m_name.contains("=") || m_name.contains("&")) {
        kurl = kurl.upUrl();
        m_name = kurl.fileName();
    }

    if (m_name.isEmpty())
        m_name = kurl.host();

    if (m_name.isEmpty())
        m_name = "????"; //Probably better than un-i18n'd 'undefined'...

    m_kteDoc  = 0;
    m_kteView = 0;
    m_rebuilding    = false;
    m_deferredClear = false;
    m_hasFunctions  = false;
}

DebugDocument::~DebugDocument()
{
    emit documentDestroyed(this);

    // View has an another parent for UI purposes, so we have to clean it up
    delete m_kteView;
}

KJS::Interpreter* DebugDocument::interpreter()
{
    return m_interpreter;
}

bool DebugDocument::hasFunctions()
{
    return m_hasFunctions;
}

void DebugDocument::setHasFunctions()
{
    m_hasFunctions = true;
}

QString DebugDocument::name() const
{
    return m_name;
}

QString DebugDocument::url() const
{
    return m_url;
}

QString DebugDocument::iuKey() const
{
    return m_iuKey;
}

QList<int> DebugDocument::fragments() const
{
    return m_codeFragments.keys();
}

SourceFragment DebugDocument::fragment(int sourceId)
{
    if (m_codeFragments.contains(sourceId))
        return m_codeFragments[sourceId];
    else
        return SourceFragment();
}

void DebugDocument::addCodeFragment(int sourceId, int baseLine, const QString &source)
{
    if (m_deferredClear)
    {
        m_codeFragments.clear();
        m_deferredClear = false;
    }

    SourceFragment code;
    code.sourceId = sourceId;
    code.baseLine = baseLine - 1;
    if (code.baseLine < 0) //(messed up debug info)
        code.baseLine = 0;

    code.sourceLines = source.split("\n"); //### is \n enough?

    m_codeFragments[sourceId] = code;

    if (m_kteDoc) // Update docu if needed
        rebuildViewerDocument(code.baseLine, code.lastLine());
}

void DebugDocument::requestDeferredClear()
{
    m_deferredClear = true;
}

void DebugDocument::setBreakpoint(int lineNumber)
{
    if (m_rebuilding) return;

    breakpoints().append(lineNumber);
}

void DebugDocument::removeBreakpoint(int lineNumber)
{
    if (m_rebuilding) return;

    QVector<int>& br = breakpoints();
    int idx = breakpoints().indexOf(lineNumber);
    if (idx != -1)
    {
        br.remove(idx);
        if (br.isEmpty() && !m_url.isEmpty())
        {
            // We just removed the last breakpoint per URL,
            // so we can kill the entire list
            s_perUrlBreakPoints->remove(url());
        }
    }
}

bool DebugDocument::hasBreakpoint(int lineNumber)
{
    return breakpoints().contains(lineNumber);
}

QHash<QString, QVector<int> >* DebugDocument::s_perUrlBreakPoints = 0;

QVector<int>& DebugDocument::breakpoints()
{
    if (m_url.isEmpty())
        return m_localBreakpoints;
    else
    {
        if (!s_perUrlBreakPoints)
            s_perUrlBreakPoints = new QHash<QString, QVector<int> >;

        return (*s_perUrlBreakPoints)[m_url];
    }
}

KTextEditor::Document* DebugDocument::viewerDocument()
{
    if (!m_kteDoc)
        rebuildViewerDocument();
    return m_kteDoc;
}

// A couple helpers to transparently grow an array
static void ensureIndexSafe(QStringList& lines, int line)
{
    while (line >= lines.size())
        lines.append(QString());
}

static void setLine(QStringList& lines, int line, const QString& lineText)
{
    ensureIndexSafe(lines, line);
    lines[line] = lineText;
}

static QString getLine(QStringList& lines, int line)
{
    ensureIndexSafe(lines, line);
    return lines[line];
}

KTextEditor::Editor* DebugDocument::s_kate = 0;

KTextEditor::Editor* DebugDocument::kate()
{
    if (!s_kate)
        s_kate = KTextEditor::editor("katepart");

    if (!s_kate)
    {
        KMessageBox::error(DebugWindow::window(), i18n("Unable to find the Kate editor component;\n"
                                      "please check your KDE installation."));
        qApp->exit(1);
    }

    return s_kate;
}


void DebugDocument::rebuildViewerDocument(int firstLine, int lastLine)
{
    // We collect the lines ourselves, not to bother katepart so much
    bool fullDoc = (firstLine == 0) && (lastLine == -1);

    // Figure out our last line if not specified..
    if (lastLine == -1)
    {
        lastLine = 0;
        foreach(const SourceFragment &fragment, m_codeFragments)
            lastLine = qMax(fragment.lastLine(), lastLine);
    }

    QStringList lines;
    foreach (const SourceFragment &fragment, m_codeFragments)
    {
        // See if this fragment is in the [firstLine, lastLine] range.
        if (!fragment.inRange(firstLine, lastLine))
            continue;

        const QStringList& sourceLines = fragment.sourceLines;

        // Now scan through the fragment, and incorporate all the lines that
        // are in range.
        for (int scanLine = 0; scanLine < sourceLines.size(); ++scanLine)
        {
            int absLine = fragment.baseLine + scanLine;
            int outLine = absLine - firstLine;

            if (absLine  < firstLine || absLine > lastLine)
                continue;

            // We have to be a bit careful here, since
            // in an ultra-stupid HTML documents, there may be more than
            // one script tag on a line. So we try to append things.
            if (scanLine == 0 && !getLine(lines, outLine).isEmpty())
                setLine(lines, outLine, getLine(lines, outLine) + "  " + sourceLines[0]);
            else if (scanLine == sourceLines.size() - 1 && !getLine(lines, outLine).isEmpty())
                setLine(lines, outLine, sourceLines[scanLine] + " " + getLine(lines, outLine));
            else
                setLine(lines, outLine, sourceLines[scanLine]);
        }
    }

    m_rebuilding = true;

    if (!m_kteDoc)
    {
        m_kteDoc = kate()->createDocument(this);
        setupViewerDocument();
    }

    KTextEditor::Cursor oldPos;
    if (m_kteView)
        oldPos = m_kteView->cursorPosition();

    m_kteDoc->setReadWrite(true);

    if (fullDoc)
        m_kteDoc->setText(lines.join("\n"));
    else
    {
        // Expand the document if needed..
        while (lastLine + 1 > m_kteDoc->lines())
            m_kteDoc->insertLine(m_kteDoc->lines(), " ");

        // Update text
        m_kteDoc->replaceText(KTextEditor::Range(
                                KTextEditor::Cursor(firstLine, 0),
                                KTextEditor::Cursor(lastLine, m_kteDoc->line(lastLine).length())),
                              lines.join("\n"));
    }

    // Restore cursor pos, if there is a view
    if (m_kteView)
        m_kteView->setCursorPosition(oldPos);

    // Check off the pending/URL-based breakpoints. We have to do even
    // when the document is being updated as they may be on later lines
    KTextEditor::MarkInterface* imark = qobject_cast<KTextEditor::MarkInterface*>(m_kteDoc);
    if (imark)
    {
        QVector<int>& bps = breakpoints();
        foreach (int bpLine, bps)
            imark->addMark(bpLine - 1, KTextEditor::MarkInterface::BreakpointActive);
    }

    m_kteDoc->setReadWrite(false);
    m_rebuilding = false;
}

void DebugDocument::setupViewerDocument()
{
    // Highlight as JS..
    m_kteDoc->setMode("JavaScript");

    // Configure all the breakpoint/execution point marker stuff.
    // ### there is an odd split of mark use between here and DebugWindow.
    // Perhaps we should just emit a single and let it do it, and
    // limit ourselves to ownership?
    KTextEditor::MarkInterface* imark = qobject_cast<KTextEditor::MarkInterface*>(m_kteDoc);
    assert(imark);

    imark->setEditableMarks(KTextEditor::MarkInterface::BreakpointActive);
    connect(m_kteDoc, SIGNAL(markChanged(KTextEditor::Document*, KTextEditor::Mark, KTextEditor::MarkInterface::MarkChangeAction)),
                DebugWindow::window(), SLOT(markSet(KTextEditor::Document*, KTextEditor::Mark, KTextEditor::MarkInterface::MarkChangeAction)));

    imark->setMarkDescription(KTextEditor::MarkInterface::BreakpointActive,
                                          i18n("Breakpoint"));
    imark->setMarkPixmap(KTextEditor::MarkInterface::BreakpointActive,
                                     SmallIcon("flag-red"));
    imark->setMarkPixmap(KTextEditor::MarkInterface::Execution,
                                     SmallIcon("arrow-right"));
}

KTextEditor::View* DebugDocument::viewerView()
{
    if (m_kteView)
        return m_kteView;

    // Ensure document is created
    viewerDocument();

    m_kteView = m_kteDoc->createView(DebugWindow::window());
    KTextEditor::ConfigInterface* iconf = qobject_cast<KTextEditor::ConfigInterface*>(m_kteView);
    assert(iconf);
    if (iconf->configKeys().contains("line-numbers"))
        iconf->setConfigValue("line-numbers", true);
    if (iconf->configKeys().contains("icon-bar"))
        iconf->setConfigValue("icon-bar", true);
    if (iconf->configKeys().contains("dynamic-word-wrap"))
        iconf->setConfigValue("dynamic-word-wrap", true);

    return m_kteView;
}
