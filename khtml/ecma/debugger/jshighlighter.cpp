#include <QtGui>

#include "jshighlighter.h"
#include "jshighlighter.moc"

JSHighlighter::JSHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // Keywords
    keywordFormat.setForeground(Qt::black);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bif\\b"       << "\\belse\\b"     << "\\bfor\\b"
                    << "\\bin\\b"       << "\\bwhile\\b"    << "\\bdo\\b"
                    << "\\bcontinue\\b" << "\\bbreak\\b"    << "\\bwith\\b"
                    << "\\btry\\b"      << "\\bcatch\\b"    << "\\bfinally\\b"
                    << "\\bswitch\\b"   << "\\bcase\\b"     << "\\bnew\\b"
                    << "\\bvar\\b"      << "\\bfunction\\b" << "\\breturn\\b"
                    << "\\bdelete\\b"   << "\\btrue\\b"     << "\\bfalse\\b"
                    << "\\bvoid\\b"     << "\\bthrow\\b"    << "\\btypeof\\b"
                    << "\\bconst\\b"    << "\\bdefault\\b";

    foreach (QString pattern, keywordPatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }


    // Functions
    builtinFunctionFormat.setForeground(Qt::black);
    builtinFunctionFormat.setFontWeight(QFont::Bold);
    QStringList builtinFunctionPatterns;
    builtinFunctionPatterns << "\\bescape\\b"  << "\\bisFinite\\b"   << "\\bisNaN\\b"
                            << "\\bNumber\\b"  << "\\bparseFloat\\b" << "\\bparseInt\\b"
                            << "\\breload\\b"  << "\\btaint\\b"      << "\\bunescape\\b"
                            << "\\buntaint\\b" << "\\bwrite\\b";

    foreach (QString pattern, builtinFunctionPatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = builtinFunctionFormat;
        highlightingRules.append(rule);
    }

    // Objects
    objectFormat.setForeground(Qt::black);
    objectFormat.setFontWeight(QFont::Bold);
    QStringList objectPatterns;
    objectPatterns << "\\bAnchor\\b"   << "\\bApplet\\b"    << "\\bArea\\b"
                   << "\\bArray\\b"    << "\\bBoolean\\b"   << "\\bButton\\b"
                   << "\\bCheckbox\\b" << "\\bDate\\b"      << "\\bdocument\\b"
                   << "\\bwindow\\b"   << "\\bImage\\b"     << "\\bFileUpload\\b"
                   << "\\bForm\\b"     << "\\bFrame\\b"     << "\\bFunction\\b"
                   << "\\bHidden\\b"   << "\\bLink\\b"      << "\\bMimeType\\b"
                   << "\\bMath\\b"     << "\\bMax\\b"       << "\\bMin\\b"
                   << "\\bLayer\\b"    << "\\bnavigator\\b" << "\\bObject\\b"
                   << "\\bPassword\\b" << "\\bPlugin\\b"    << "\\bRadio\\b"
                   << "\\bRegExp\\b"   << "\\bReset\\b"     << "\\bScreen\\b"
                   << "\\bSelect\\b"   << "\\bString\\b"    << "\\bText\\b"
                   << "\\bTextarea\\b" << "\\bthis\\b"      << "\\bWindow\\b";

    foreach (QString pattern, objectPatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = objectFormat;
        highlightingRules.append(rule);
    }


    // Math
    mathFormat.setForeground(Qt::black);
    mathFormat.setFontWeight(QFont::Bold);
    QStringList mathPatterns;
    objectPatterns << "\\babs\\b"     << "\\bacos\\b"  << "\\basin\\b"
                   << "\\batan\\b"    << "\\batan2\\b" << "\\bceil\\b"
                   << "\\bcos\\b"     << "\\bctg\\b"   << "\\bE\\b"
                   << "\\bexp\\b"     << "\\bfloor\\b" << "\\bLN2\\b"
                   << "\\bLN10\\b"    << "\\blog\\b"   << "\\bLOG2E\\b"
                   << "\\bLOG10E\\b"  << "\\bPI\\b"    << "\\bpow\\b"
                   << "\\bround\\b"   << "\\bsin\\b"   << "\\bsqrt\\b"
                   << "\\bSQRT1_2\\b" << "\\bSQRT2\\b" << "\\btan\\b";

    foreach (QString pattern, mathPatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = mathFormat;
        highlightingRules.append(rule);
    }

    // Events
    eventsFormat.setForeground(Qt::black);
    eventsFormat.setFontWeight(QFont::Bold);
    QStringList eventsPatterns;
    eventsPatterns  << "\\bonAbort\\b"     << "\\bonBlur\\b"       << "\\bonChange\\b"
                    << "\\bonClick\\b"     << "\\bonError\\b"      << "\\bonFocus\\b"
                    << "\\bonLoad\\b"      << "\\bonMouseOut\\b"   << "\\bonMouseOver\\b"
                    << "\\bonReset\\b"     << "\\bonSelect\\b"     << "\\bonSubmit\\b"
                    << "\\bonUnload\\b";

    foreach (QString pattern, eventsPatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = eventsFormat;
        highlightingRules.append(rule);
    }

/*
    <list name="methods">
        <item>above</item>
        <item>action</item>
        <item>alinkColor</item>
        <item>alert</item>
        <item>anchor</item>
        <item>anchors</item>
        <item>appCodeName</item>
        <item>applets</item>
        <item>apply</item>
        <item>appName</item>
        <item>appVersion</item>
        <item>argument</item>
        <item>arguments</item>
        <item>arity</item>
        <item>availHeight</item>
        <item>availWidth</item>
        <item>back</item>
        <item>background</item>
        <item>below</item>
        <item>bgColor</item>
        <item>border</item>
        <item>big</item>
        <item>blink</item>
        <item>blur</item>
        <item>bold</item>
        <item>border</item>
        <item>call</item>
        <item>caller</item>
        <item>charAt</item>
        <item>charCodeAt</item>
        <item>checked</item>
        <item>clearInterval</item>
        <item>clearTimeout</item>
        <item>click</item>
        <item>clip</item>
        <item>close</item>
        <item>closed</item>
        <item>colorDepth</item>
        <item>complete</item>
        <item>compile</item>
        <item>constructor</item>
        <item>confirm</item>
        <item>cookie</item>
        <item>current</item>
        <item>cursor</item>
        <item>data</item>
        <item>defaultChecked</item>
        <item>defaultSelected</item>
        <item>defaultStatus</item>
        <item>defaultValue</item>
        <item>description</item>
        <item>disableExternalCapture</item>
        <item>domain</item>
        <item>elements</item>
        <item>embeds</item>
        <item>enabledPlugin</item>
        <item>enableExternalCapture</item>
        <item>encoding</item>
        <item>eval</item>
        <item>exec</item>
        <item>fgColor</item>
        <item>filename</item>
        <item>find</item>
        <item>fixed</item>
        <item>focus</item>
        <item>fontcolor</item>
        <item>fontsize</item>
        <item>form</item>
        <item>forms</item>
        <item>formName</item>
        <item>forward</item>
        <item>frames</item>
        <item>fromCharCode</item>
        <item>getDate</item>
        <item>getDay</item>
        <item>getHours</item>
        <item>getMiliseconds</item>
        <item>getMinutes</item>
        <item>getMonth</item>
        <item>getSeconds</item>
        <item>getSelection</item>
        <item>getTime</item>
        <item>getTimezoneOffset</item>
        <item>getUTCDate</item>
        <item>getUTCDay</item>
        <item>getUTCFullYear</item>
        <item>getUTCHours</item>
        <item>getUTCMilliseconds</item>
        <item>getUTCMinutes</item>
        <item>getUTCMonth</item>
        <item>getUTCSeconds</item>
        <item>getYear</item>
        <item>global</item>
        <item>go</item>
        <item>hash</item>
        <item>height</item>
        <item>history</item>
        <item>home</item>
        <item>host</item>
        <item>hostname</item>
        <item>href</item>
        <item>hspace</item>
        <item>ignoreCase</item>
        <item>images</item>
        <item>index</item>
        <item>indexOf</item>
        <item>innerHeight</item>
        <item>innerWidth</item>
        <item>input</item>
        <item>italics</item>
        <item>javaEnabled</item>
        <item>join</item>
        <item>language</item>
        <item>lastIndex</item>
        <item>lastIndexOf</item>
        <item>lastModified</item>
        <item>lastParen</item>
        <item>layers</item>
        <item>layerX</item>
        <item>layerY</item>
        <item>left</item>
        <item>leftContext</item>
        <item>length</item>
        <item>link</item>
        <item>linkColor</item>
        <item>links</item>
        <item>location</item>
        <item>locationbar</item>
        <item>load</item>
        <item>lowsrc</item>
        <item>match</item>
        <item>MAX_VALUE</item>
        <item>menubar</item>
        <item>method</item>
        <item>mimeTypes</item>
        <item>MIN_VALUE</item>
        <item>modifiers</item>
        <item>moveAbove</item>
        <item>moveBelow</item>
        <item>moveBy</item>
        <item>moveTo</item>
        <item>moveToAbsolute</item>
        <item>multiline</item>
        <item>name</item>
        <item>NaN</item>
        <item>NEGATIVE_INFINITY</item>
        <item>negative_infinity</item>
        <item>next</item>
        <item>open</item>
        <item>opener</item>
        <item>options</item>
        <item>outerHeight</item>
        <item>outerWidth</item>
        <item>pageX</item>
        <item>pageY</item>
        <item>pageXoffset</item>
        <item>pageYoffset</item>
        <item>parent</item>
        <item>parse</item>
        <item>pathname</item>
        <item>personalbar</item>
        <item>pixelDepth</item>
        <item>platform</item>
        <item>plugins</item>
        <item>pop</item>
        <item>port</item>
        <item>POSITIVE_INFINITY</item>
        <item>positive_infinity</item>
        <item>preference</item>
        <item>previous</item>
        <item>print</item>
        <item>prompt</item>
        <item>protocol</item>
        <item>prototype</item>
        <item>push</item>
        <item>referrer</item>
        <item>refresh</item>
        <item>releaseEvents</item>
        <item>reload</item>
        <item>replace</item>
        <item>reset</item>
        <item>resizeBy</item>
        <item>resizeTo</item>
        <item>reverse</item>
        <item>rightContext</item>
        <item>screenX</item>
        <item>screenY</item>
        <item>scroll</item>
        <item>scrollbar</item>
        <item>scrollBy</item>
        <item>scrollTo</item>
        <item>search</item>
        <item>select</item>
        <item>selected</item>
        <item>selectedIndex</item>
        <item>self</item>
        <item>setDate</item>
        <item>setHours</item>
        <item>setMinutes</item>
        <item>setMonth</item>
        <item>setSeconds</item>
        <item>setTime</item>
        <item>setTimeout</item>
        <item>setUTCDate</item>
        <item>setUTCDay</item>
        <item>setUTCFullYear</item>
        <item>setUTCHours</item>
        <item>setUTCMilliseconds</item>
        <item>setUTCMinutes</item>
        <item>setUTCMonth</item>
        <item>setUTCSeconds</item>
        <item>setYear</item>
        <item>shift</item>
        <item>siblingAbove</item>
        <item>siblingBelow</item>
        <item>small</item>
        <item>sort</item>
        <item>source</item>
        <item>splice</item>
        <item>split</item>
        <item>src</item>
        <item>status</item>
        <item>statusbar</item>
        <item>strike</item>
        <item>sub</item>
        <item>submit</item>
        <item>substr</item>
        <item>substring</item>
        <item>suffixes</item>
        <item>sup</item>
        <item>taintEnabled</item>
        <item>target</item>
        <item>test</item>
        <item>text</item>
        <item>title</item>
        <item>toGMTString</item>
        <item>toLocaleString</item>
        <item>toLowerCase</item>
        <item>toolbar</item>
        <item>toSource</item>
        <item>toString</item>
        <item>top</item>
        <item>toUpperCase</item>
        <item>toUTCString</item>
        <item>type</item>
        <item>URL</item>
        <item>unshift</item>
        <item>unwatch</item>
        <item>userAgent</item>
        <item>UTC</item>
        <item>value</item>
        <item>valueOf</item>
        <item>visibility</item>
        <item>vlinkColor</item>
        <item>vspace</item>
        <item>width</item>
        <item>watch</item>
        <item>which</item>
        <item>width</item>
        <item>write</item>
        <item>writeln</item>
        <item>x</item>
        <item>y</item>
        <item>zIndex</item>
    </list>
*/

    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

    singleLineCommentFormat.setForeground(Qt::red);
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(Qt::red);

    quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");
}

void JSHighlighter::highlightBlock(const QString &text)
{
    foreach (HighlightingRule rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = text.indexOf(expression);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = text.indexOf(expression, index + length);
        }
    }
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

    while (startIndex >= 0) {
       int endIndex = text.indexOf(commentEndExpression, startIndex);
       int commentLength;
       if (endIndex == -1) {
           setCurrentBlockState(1);
           commentLength = text.length() - startIndex;
       } else {
           commentLength = endIndex - startIndex
                           + commentEndExpression.matchedLength();
       }
       setFormat(startIndex, commentLength, multiLineCommentFormat);
       startIndex = text.indexOf(commentStartExpression,
                                               startIndex + commentLength);
    }
}
