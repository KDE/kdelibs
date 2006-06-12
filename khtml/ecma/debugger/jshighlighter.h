#ifndef JSHIGHLIGHTER_H
#define JSHIGHLIGHTER_H

#include <QSyntaxHighlighter>

#include <QHash>
#include <QTextCharFormat>

class QTextDocument;
class QTextCharFormat;

class JSHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    JSHighlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat builtinFunctionFormat;
    QTextCharFormat objectFormat;
    QTextCharFormat mathFormat;
    QTextCharFormat eventsFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
};

#endif
