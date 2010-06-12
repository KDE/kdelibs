/*  This file is part of the KDE libraries
    Copyright (C) 2007 Chusslove Illich <caslav.ilic@gmx.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kuitsemantics_p.h>

#include <config.h>

#include <QHash>
#include <QSet>
#include <QRegExp>
#include <QStack>
#include <QXmlStreamReader>
#include <QStringList>
#include <QPair>
#include <QDir>

#include <kdebug.h>
#include <kglobal.h>
#include <kcatalog_p.h>
#include <kuitformats_p.h>
#include <klocale.h>
#include <kconfiggroup.h>

// Truncates string, for output of long messages.
// (But don't truncate too much otherwise it's impossible to determine
// which message is faulty if many messages start with the same beginning).
static QString shorten (const QString &str)
{
    const int maxlen = 80;
    if (str.length() <= maxlen)
        return str;
    else
        return str.left(maxlen).append("...");
}

// -----------------------------------------------------------------------------
// All the tag, attribute, and context marker element enums.
namespace Kuit {

    namespace Tag { // tag names
        typedef enum {
            None,
            TopLong, TopShort,
            Title, Subtitle, Para, List, Item, Note, Warning, Link,
            Filename, Application, Command, Resource, Icode, Bcode, Shortcut,
            Interface, Emphasis, Placeholder, Email, Numid, Envar, Message, Nl,
            NumIntg, NumReal // internal helpers for numbers, not part of DTD
        } Var;
    }

    namespace Att { // tag attribute names
        typedef enum {
            None,
            Ctx, Url, Address, Section, Label, Strong,
            Width, Fill // internal helpers for numbers, not part of DTD
        } Var;
    }

    namespace Rol { // semantic roles
        typedef enum {
            None,
            Action, Title, Option, Label, Item, Info
        } Var;
    }

    namespace Cue { // interface subcues
        typedef enum {
            None,
            Button, Inmenu, Intoolbar,
            Window, Menu, Tab, Group, Column, Row,
            Slider, Spinbox, Listbox, Textbox, Chooser,
            Check, Radio,
            Inlistbox, Intable, Inrange, Intext,
            Tooltip, Whatsthis, Status, Progress, Tipoftheday, Credit, Shell
        } Var;
    }

    namespace Fmt { // visual formats
        typedef enum {
            None, Plain, Rich, Term
        } Var;
    }

    typedef Tag::Var TagVar;
    typedef Att::Var AttVar;
    typedef Rol::Var RolVar;
    typedef Cue::Var CueVar;
    typedef Fmt::Var FmtVar;
}

// -----------------------------------------------------------------------------
// All the global data.

class KuitSemanticsStaticData
{
    public:

    QHash<QString, Kuit::TagVar> knownTags;
    QHash<QString, Kuit::AttVar> knownAtts;
    QHash<QString, Kuit::FmtVar> knownFmts;
    QHash<QString, Kuit::RolVar> knownRols;
    QHash<QString, Kuit::CueVar> knownCues;

    QHash<Kuit::TagVar, QSet<Kuit::TagVar> > tagSubs;
    QHash<Kuit::TagVar, QSet<Kuit::AttVar> > tagAtts;
    QHash<Kuit::RolVar, QSet<Kuit::CueVar> > rolCues;

    QHash<Kuit::RolVar, QHash<Kuit::CueVar, Kuit::FmtVar> > defFmts;

    QHash<Kuit::TagVar, QString> tagNames;

    QSet<QString> qtHtmlTagNames;

    QHash<Kuit::TagVar, int> leadingNewlines;

    QHash<QString, QString> xmlEntities;
    QHash<QString, QString> xmlEntitiesInverse;

    KuitSemanticsStaticData ();
};

KuitSemanticsStaticData::KuitSemanticsStaticData ()
{
    // Setup known tag names, attributes, and subtags.
    // A "lax" version of the DTD.
    #undef SETUP_TAG
    #define SETUP_TAG(tag, name, atts, subs) do { \
        knownTags[name] = Kuit::Tag::tag; \
        tagNames[Kuit::Tag::tag] = name; \
        { \
            using namespace Kuit::Att; \
            tagAtts[Kuit::Tag::tag] << atts; \
        } \
        { \
            using namespace Kuit::Tag; \
            tagSubs[Kuit::Tag::tag] << subs << NumIntg << NumReal; \
        } \
    } while (0)

    #undef INLINES
    #define INLINES \
        Filename << Link << Application << Command << Resource << Icode << \
        Shortcut << Interface << Emphasis << Placeholder << Email << \
        Numid << Envar << Nl

    SETUP_TAG(TopLong, "kuit", Ctx, Title << Subtitle << Para);
    SETUP_TAG(TopShort, "kuil", Ctx, INLINES << Note << Warning << Message);

    SETUP_TAG(Title, "title", None, INLINES);
    SETUP_TAG(Subtitle, "subtitle", None, INLINES);
    SETUP_TAG(Para, "para", None,
              INLINES << Note << Warning << Message << List);
    SETUP_TAG(List, "list", None, Item);
    SETUP_TAG(Item, "item", None, INLINES << Note << Warning << Message);

    SETUP_TAG(Note, "note", Label, INLINES);
    SETUP_TAG(Warning, "warning", Label, INLINES);
    SETUP_TAG(Filename, "filename", None, Envar << Placeholder);
    SETUP_TAG(Link, "link", Url, None);
    SETUP_TAG(Application, "application", None, None);
    SETUP_TAG(Command, "command", Section, None);
    SETUP_TAG(Resource, "resource", None, None);
    SETUP_TAG(Icode, "icode", None, Envar << Placeholder);
    SETUP_TAG(Bcode, "bcode", None, None);
    SETUP_TAG(Shortcut, "shortcut", None, None);
    SETUP_TAG(Interface, "interface", None, None);
    SETUP_TAG(Emphasis, "emphasis", Strong, None);
    SETUP_TAG(Placeholder, "placeholder", None, None);
    SETUP_TAG(Email, "email", Address, None);
    SETUP_TAG(Envar, "envar", None, None);
    SETUP_TAG(Message, "message", None, None);
    SETUP_TAG(Numid, "numid", None, None);
    SETUP_TAG(Nl, "nl", None, None);
    // Internal, not part of DTD.
    SETUP_TAG(NumIntg, KUIT_NUMINTG, Width << Fill, None);
    SETUP_TAG(NumReal, KUIT_NUMREAL, Width << Fill, None);

    // Setup known attribute names.
    #undef SETUP_ATT
    #define SETUP_ATT(att, name) do { \
        knownAtts[name] = Kuit::Att::att; \
    } while (0)
    SETUP_ATT(Ctx, "ctx");
    SETUP_ATT(Url, "url");
    SETUP_ATT(Address, "address");
    SETUP_ATT(Section, "section");
    SETUP_ATT(Label, "label");
    SETUP_ATT(Strong, "strong");
    // Internal, not part of DTD.
    SETUP_ATT(Width, "width");
    SETUP_ATT(Fill, "fill");

    // Setup known format names.
    #undef SETUP_FMT
    #define SETUP_FMT(fmt, name) do { \
        knownFmts[name] = Kuit::Fmt::fmt; \
    } while (0)
    SETUP_FMT(Plain, "plain");
    SETUP_FMT(Rich, "rich");
    SETUP_FMT(Term, "term");

    // Setup known role names, their default format and subcues.
    #undef SETUP_ROL
    #define SETUP_ROL(rol, name, fmt, cues) do { \
        knownRols[name] = Kuit::Rol::rol; \
        defFmts[Kuit::Rol::rol][Kuit::Cue::None] = Kuit::Fmt::fmt; \
        { \
            using namespace Kuit::Cue; \
            rolCues[Kuit::Rol::rol] << cues; \
        } \
    } while (0)
    SETUP_ROL(Action, "action", Plain,
              Button << Inmenu << Intoolbar);
    SETUP_ROL(Title, "title", Plain,
              Window << Menu << Tab << Group << Column << Row);
    SETUP_ROL(Label, "label", Plain,
              Slider << Spinbox << Listbox << Textbox << Chooser);
    SETUP_ROL(Option, "option", Plain,
              Check << Radio);
    SETUP_ROL(Item, "item", Plain,
              Inmenu << Inlistbox << Intable << Inrange << Intext);
    SETUP_ROL(Info, "info", Rich,
                 Tooltip << Whatsthis << Kuit::Cue::Status << Progress
              << Tipoftheday << Credit << Shell);

    // Setup override formats by subcue.
    #undef SETUP_ROLCUEFMT
    #define SETUP_ROLCUEFMT(rol, cue, fmt) do { \
        defFmts[Kuit::Rol::rol][Kuit::Cue::cue] = Kuit::Fmt::fmt; \
    } while (0)
    SETUP_ROLCUEFMT(Info, Status, Plain);
    SETUP_ROLCUEFMT(Info, Progress, Plain);
    SETUP_ROLCUEFMT(Info, Credit, Plain);
    SETUP_ROLCUEFMT(Info, Shell, Term);

    // Setup known subcue names.
    #undef SETUP_CUE
    #define SETUP_CUE(cue, name) do { \
        knownCues[name] = Kuit::Cue::cue; \
    } while (0)
    SETUP_CUE(Button, "button");
    SETUP_CUE(Inmenu, "inmenu");
    SETUP_CUE(Intoolbar, "intoolbar");
    SETUP_CUE(Window, "window");
    SETUP_CUE(Menu, "menu");
    SETUP_CUE(Tab, "tab");
    SETUP_CUE(Group, "group");
    SETUP_CUE(Column, "column");
    SETUP_CUE(Row, "row");
    SETUP_CUE(Slider, "slider");
    SETUP_CUE(Spinbox, "spinbox");
    SETUP_CUE(Listbox, "listbox");
    SETUP_CUE(Textbox, "textbox");
    SETUP_CUE(Chooser, "chooser");
    SETUP_CUE(Check, "check");
    SETUP_CUE(Radio, "radio");
    SETUP_CUE(Inlistbox, "inlistbox");
    SETUP_CUE(Intable, "intable");
    SETUP_CUE(Inrange, "inrange");
    SETUP_CUE(Intext, "intext");
    SETUP_CUE(Tooltip, "tooltip");
    SETUP_CUE(Whatsthis, "whatsthis");
    SETUP_CUE(Status, "status");
    SETUP_CUE(Progress, "progress");
    SETUP_CUE(Tipoftheday, "tipoftheday");
    SETUP_CUE(Credit, "credit");
    SETUP_CUE(Shell, "shell");

    // Collect all Qt's rich text engine HTML tags, for some checks later.
    qtHtmlTagNames << "a" << "address" << "b" << "big" << "blockquote"
                   << "body" << "br" << "center" << "cita" << "code"
                   << "dd" << "dfn" << "div" << "dl" << "dt" << "em"
                   << "font" << "h1" << "h2" << "h3" << "h4" << "h5"
                   << "h6" << "head" << "hr" << "html" << "i" << "img"
                   << "kbd" << "meta" << "li" << "nobr" << "ol" << "p"
                   << "pre" << "qt" << "s" << "samp" << "small" << "span"
                   << "strong" << "sup" << "sub" << "table" << "tbody"
                   << "td" << "tfoot" << "th" << "thead" << "title"
                   << "tr" << "tt" << "u" << "ul" << "var";

    // Tags that format with number of leading newlines.
    #undef SETUP_TAG_NL
    #define SETUP_TAG_NL(tag, nlead) do { \
        leadingNewlines[Kuit::Tag::tag] = nlead; \
    } while (0)
    SETUP_TAG_NL(Title, 2);
    SETUP_TAG_NL(Subtitle, 2);
    SETUP_TAG_NL(Para, 2);
    SETUP_TAG_NL(List, 1);
    SETUP_TAG_NL(Bcode, 1);
    SETUP_TAG_NL(Item, 1);

    // Known XML entities, direct/inverse mapping.
    xmlEntities["lt"] = '<';
    xmlEntities["gt"] = '>';
    xmlEntities["amp"] = '&';
    xmlEntities["apos"] = '\'';
    xmlEntities["quot"] = '"';
    xmlEntitiesInverse[QString('<')] = "lt";
    xmlEntitiesInverse[QString('>')] = "gt";
    xmlEntitiesInverse[QString('&')] = "amp";
    xmlEntitiesInverse[QString('\'')] = "apos";
    xmlEntitiesInverse[QString('"')] = "quot";
}

K_GLOBAL_STATIC(KuitSemanticsStaticData, semanticsStaticData)


// -----------------------------------------------------------------------------
// The KuitSemanticsPrivate methods, they do the work.

class KuitSemanticsPrivate
{
    public:

    KuitSemanticsPrivate (const QString &lang_);

    QString format (const QString &text, const QString &ctxt) const;

    // Get metatranslation (formatting patterns, etc.)
    QString metaTr (const char *ctxt, const char *id) const;

    // Set visual formatting patterns for text in semantic tags.
    void setFormattingPatterns ();

    // Set data used in transformation of text within semantic tags.
    void setTextTransformData ();

    // Compute integer hash key from the set of attributes.
    static int attSetKey (const QSet<Kuit::AttVar> &aset = QSet<Kuit::AttVar>());

    // Determine visual format by parsing the context marker.
    static Kuit::FmtVar formatFromContextMarker (const QString &ctxmark,
                                                 const QString &text);
    // Determine visual format by parsing tags.
    static Kuit::FmtVar formatFromTags (const QString &text);

    // Apply appropriate top tag is to the text.
    static QString equipTopTag (const QString &text, Kuit::TagVar &toptag);

    // Formats the semantic into visual text.
    QString semanticToVisualText (const QString &text,
                                  Kuit::FmtVar fmtExp,
                                  Kuit::FmtVar fmtImp) const;

    // Final touches to the formatted text.
    QString finalizeVisualText (const QString &final,
                                Kuit::FmtVar fmt,
                                bool hadQtTag = false,
                                bool hadAnyHtmlTag = false) const;

    // In case of markup errors, try to make result not look too bad.
    QString salvageMarkup (const QString &text, Kuit::FmtVar fmt) const;

    // Data for XML parsing state.
    class OpenEl
    {
        public:

        typedef enum { Proper, Ignored, Dropout } Handling;

        Kuit::TagVar tag;
        QString name;
        QHash<Kuit::AttVar, QString> avals;
        int akey;
        QString astr;
        Handling handling;
        QString formattedText;
    };

    // Gather data about current element for the parse state.
    KuitSemanticsPrivate::OpenEl parseOpenEl (const QXmlStreamReader &xml,
                                              Kuit::TagVar etag,
                                              const QString &text) const;

    // Select visual pattern for given tag+attributes+format combination.
    QString visualPattern (Kuit::TagVar tag, int akey, Kuit::FmtVar fmt) const;

    // Format text of the element.
    QString formatSubText (const QString &ptext, const OpenEl &oel,
                           Kuit::FmtVar fmt, int numctx) const;

    // Count number of newlines at start and at end of text.
    static void countWrappingNewlines (const QString &ptext,
                                       int &numle, int &numtr);

    // Modifies text for some tags.
    QString modifyTagText (const QString &text, Kuit::TagVar tag,
                           const QHash<Kuit::AttVar, QString> &avals,
                           int numctx, Kuit::FmtVar fmt) const;

    private:

    QString m_lang;

    QHash<Kuit::TagVar,
          QHash<int, // attribute set key
                QHash<Kuit::FmtVar, QString> > > m_patterns;

    QHash<Kuit::FmtVar, QString> m_comboKeyDelim;
    QHash<Kuit::FmtVar, QString> m_guiPathDelim;

    QHash<QString, QString> m_keyNames;

    // For fetching metatranslations.
    KCatalog *m_metaCat;
};

KuitSemanticsPrivate::KuitSemanticsPrivate (const QString &lang)
: m_metaCat(NULL)
{
    m_lang = lang;

    // NOTE: This function draws translation from raw message catalogs
    // because full i18n system is not available at this point (this
    // function is called within the initialization of the i18n system),
    // Also, pattern/transformation strings are "metastrings", not
    // fully proper i18n strings on their own.

    m_metaCat = new KCatalog("kdelibs4", lang);

    // Get formatting patterns for all tag/att/fmt combinations.
    setFormattingPatterns();

    // Get data for tag text transformations.
    setTextTransformData();

    // Catalog not needed any more.
    delete m_metaCat;
    m_metaCat = NULL;
}

QString KuitSemanticsPrivate::metaTr (const char *ctxt, const char *id) const
{
    if (m_metaCat == NULL) {
        return QString(id);
    }
    return m_metaCat->translate(ctxt, id);
}

void KuitSemanticsPrivate::setFormattingPatterns ()
{
    using namespace Kuit;

    // Macro to expedite setting the patterns.
    #undef SET_PATTERN
    #define SET_PATTERN(tag, atts, fmt, ctxt_ptrn) do { \
        QSet<AttVar> aset; \
        aset << atts; \
        int akey = attSetKey(aset); \
        QString pattern = metaTr(ctxt_ptrn); \
        m_patterns[tag][akey][fmt] = pattern; \
        /* Make Term pattern same as Plain, unless explicitly given. */ \
        if (fmt == Fmt::Plain && !m_patterns[tag][akey].contains(Fmt::Term)) { \
            m_patterns[tag][akey][Fmt::Term] = pattern; \
        } \
    } while (0)

    // Normal I18N_NOOP2 removes context, but below we need both.
    #undef I18N_NOOP2
    #define I18N_NOOP2(ctxt, msg) ctxt, msg

    // Some of the formatting patterns are intentionally not exposed for
    // localization.
    #undef XXXX_NOOP2
    #define XXXX_NOOP2(ctxt, msg) ctxt, msg

    // NOTE: The following "i18n:" comments are oddly placed in order that
    // xgettext extracts them properly.

    // -------> Title
    SET_PATTERN(Tag::Title, Att::None, Fmt::Plain,
                I18N_NOOP2("@title/plain",
    // i18n: The following messages, with msgctxt "@tag/modifier",
    // are KUIT patterns for formatting the text found inside semantic tags.
    // For review of the KUIT semantic markup, see the article on Techbase:
    // http://techbase.kde.org/Development/Tutorials/Localization/i18n_Semantics
    // The "/modifier" tells if the pattern is used for plain text, or rich text
    // which can use HTML tags.
    // You may be in general satisfied with the patterns as they are in the
    // original. Some things you may think about changing:
    // - the proper quotes, those used in msgid are English-standard
    // - the <i> and <b> tags, does your language script work well with them?
                           "== %1 =="));
    SET_PATTERN(Tag::Title, Att::None, Fmt::Rich,
                I18N_NOOP2("@title/rich",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "<h2>%1</h2>"));

    // -------> Subtitle
    SET_PATTERN(Tag::Subtitle, Att::None, Fmt::Plain,
                I18N_NOOP2("@subtitle/plain",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "~ %1 ~"));
    SET_PATTERN(Tag::Subtitle, Att::None, Fmt::Rich,
                I18N_NOOP2("@subtitle/rich",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "<h3>%1</h3>"));

    // -------> Para
    SET_PATTERN(Tag::Para, Att::None, Fmt::Plain,
                XXXX_NOOP2("@para/plain",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "%1"));
    SET_PATTERN(Tag::Para, Att::None, Fmt::Rich,
                XXXX_NOOP2("@para/rich",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "<p>%1</p>"));

    // -------> List
    SET_PATTERN(Tag::List, Att::None, Fmt::Plain,
                XXXX_NOOP2("@list/plain",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "%1"));
    SET_PATTERN(Tag::List, Att::None, Fmt::Rich,
                XXXX_NOOP2("@list/rich",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "<ul>%1</ul>"));

    // -------> Item
    SET_PATTERN(Tag::Item, Att::None, Fmt::Plain,
                I18N_NOOP2("@item/plain",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "  * %1"));
    SET_PATTERN(Tag::Item, Att::None, Fmt::Rich,
                I18N_NOOP2("@item/rich",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "<li>%1</li>"));

    // -------> Note
    SET_PATTERN(Tag::Note, Att::None, Fmt::Plain,
                I18N_NOOP2("@note/plain",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "Note: %1"));
    SET_PATTERN(Tag::Note, Att::None, Fmt::Rich,
                I18N_NOOP2("@note/rich",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "<i>Note</i>: %1"));
    SET_PATTERN(Tag::Note, Att::Label, Fmt::Plain,
                I18N_NOOP2("@note-with-label/plain\n"
                           "%1 is the note label, %2 is the text",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "%1: %2"));
    SET_PATTERN(Tag::Note, Att::Label, Fmt::Rich,
                I18N_NOOP2("@note-with-label/rich\n"
                           "%1 is the note label, %2 is the text",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "<i>%1</i>: %2"));

    // -------> Warning
    SET_PATTERN(Tag::Warning, Att::None, Fmt::Plain,
                I18N_NOOP2("@warning/plain",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "WARNING: %1"));
    SET_PATTERN(Tag::Warning, Att::None, Fmt::Rich,
                I18N_NOOP2("@warning/rich",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "<b>Warning</b>: %1"));
    SET_PATTERN(Tag::Warning, Att::Label, Fmt::Plain,
                I18N_NOOP2("@warning-with-label/plain\n"
                           "%1 is the warning label, %2 is the text",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "%1: %2"));
    SET_PATTERN(Tag::Warning, Att::Label, Fmt::Rich,
                I18N_NOOP2("@warning-with-label/rich\n"
                           "%1 is the warning label, %2 is the text",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "<b>%1</b>: %2"));

    // -------> Link
    SET_PATTERN(Tag::Link, Att::None, Fmt::Plain,
                XXXX_NOOP2("@link/plain",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "%1"));
    SET_PATTERN(Tag::Link, Att::None, Fmt::Rich,
                XXXX_NOOP2("@link/rich",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "<a href=\"%1\">%1</a>"));
    SET_PATTERN(Tag::Link, Att::Url, Fmt::Plain,
                I18N_NOOP2("@link-with-description/plain\n"
                           "%1 is the URL, %2 is the descriptive text",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "%2 (%1)"));
    SET_PATTERN(Tag::Link, Att::Url, Fmt::Rich,
                I18N_NOOP2("@link-with-description/rich\n"
                           "%1 is the URL, %2 is the descriptive text",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "<a href=\"%1\">%2</a>"));

    // -------> Filename
    SET_PATTERN(Tag::Filename, Att::None, Fmt::Plain,
                I18N_NOOP2("@filename/plain",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "‘%1’"));
    SET_PATTERN(Tag::Filename, Att::None, Fmt::Rich,
                I18N_NOOP2("@filename/rich",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "<tt>%1</tt>"));

    // -------> Application
    SET_PATTERN(Tag::Application, Att::None, Fmt::Plain,
                I18N_NOOP2("@application/plain",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "%1"));
    SET_PATTERN(Tag::Application, Att::None, Fmt::Rich,
                I18N_NOOP2("@application/rich",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "%1"));

    // -------> Command
    SET_PATTERN(Tag::Command, Att::None, Fmt::Plain,
                I18N_NOOP2("@command/plain",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "%1"));
    SET_PATTERN(Tag::Command, Att::None, Fmt::Rich,
                I18N_NOOP2("@command/rich",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "<tt>%1</tt>"));
    SET_PATTERN(Tag::Command, Att::Section, Fmt::Plain,
                I18N_NOOP2("@command-with-section/plain\n"
                           "%1 is the command name, %2 is its man section",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "%1(%2)"));
    SET_PATTERN(Tag::Command, Att::Section, Fmt::Rich,
                I18N_NOOP2("@command-with-section/rich\n"
                           "%1 is the command name, %2 is its man section",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "<tt>%1(%2)</tt>"));

    // -------> Resource
    SET_PATTERN(Tag::Resource, Att::None, Fmt::Plain,
                I18N_NOOP2("@resource/plain",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "“%1”"));
    SET_PATTERN(Tag::Resource, Att::None, Fmt::Rich,
                I18N_NOOP2("@resource/rich",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "“%1”"));

    // -------> Icode
    SET_PATTERN(Tag::Icode, Att::None, Fmt::Plain,
                I18N_NOOP2("@icode/plain",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "“%1”"));
    SET_PATTERN(Tag::Icode, Att::None, Fmt::Rich,
                I18N_NOOP2("@icode/rich",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "<tt>%1</tt>"));

    // -------> Bcode
    SET_PATTERN(Tag::Bcode, Att::None, Fmt::Plain,
                XXXX_NOOP2("@bcode/plain",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "\n%1\n"));
    SET_PATTERN(Tag::Bcode, Att::None, Fmt::Rich,
                XXXX_NOOP2("@bcode/rich",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "<pre>%1</pre>"));

    // -------> Shortcut
    SET_PATTERN(Tag::Shortcut, Att::None, Fmt::Plain,
                I18N_NOOP2("@shortcut/plain",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "%1"));
    SET_PATTERN(Tag::Shortcut, Att::None, Fmt::Rich,
                I18N_NOOP2("@shortcut/rich",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "<b>%1</b>"));

    // -------> Interface
    SET_PATTERN(Tag::Interface, Att::None, Fmt::Plain,
                I18N_NOOP2("@interface/plain",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "|%1|"));
    SET_PATTERN(Tag::Interface, Att::None, Fmt::Rich,
                I18N_NOOP2("@interface/rich",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "<i>%1</i>"));

    // -------> Emphasis
    SET_PATTERN(Tag::Emphasis, Att::None, Fmt::Plain,
                I18N_NOOP2("@emphasis/plain",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "*%1*"));
    SET_PATTERN(Tag::Emphasis, Att::None, Fmt::Rich,
                I18N_NOOP2("@emphasis/rich",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "<i>%1</i>"));
    SET_PATTERN(Tag::Emphasis, Att::Strong, Fmt::Plain,
                I18N_NOOP2("@emphasis-strong/plain",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "**%1**"));
    SET_PATTERN(Tag::Emphasis, Att::Strong, Fmt::Rich,
                I18N_NOOP2("@emphasis-strong/rich",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "<b>%1</b>"));

    // -------> Placeholder
    SET_PATTERN(Tag::Placeholder, Att::None, Fmt::Plain,
                I18N_NOOP2("@placeholder/plain",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "&lt;%1&gt;"));
    SET_PATTERN(Tag::Placeholder, Att::None, Fmt::Rich,
                I18N_NOOP2("@placeholder/rich",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "&lt;<i>%1</i>&gt;"));

    // -------> Email
    SET_PATTERN(Tag::Email, Att::None, Fmt::Plain,
                I18N_NOOP2("@email/plain",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "&lt;%1&gt;"));
    SET_PATTERN(Tag::Email, Att::None, Fmt::Rich,
                I18N_NOOP2("@email/rich",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "&lt;<a href=\"mailto:%1\">%1</a>&gt;"));
    SET_PATTERN(Tag::Email, Att::Address, Fmt::Plain,
                I18N_NOOP2("@email-with-name/plain\n"
                           "%1 is name, %2 is address",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "%1 &lt;%2&gt;"));
    SET_PATTERN(Tag::Email, Att::Address, Fmt::Rich,
                I18N_NOOP2("@email-with-name/rich\n"
                           "%1 is name, %2 is address",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "<a href=\"mailto:%2\">%1</a>"));

    // -------> Envar
    SET_PATTERN(Tag::Envar, Att::None, Fmt::Plain,
                I18N_NOOP2("@envar/plain",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "$%1"));
    SET_PATTERN(Tag::Envar, Att::None, Fmt::Rich,
                I18N_NOOP2("@envar/rich",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "<tt>$%1</tt>"));

    // -------> Message
    SET_PATTERN(Tag::Message, Att::None, Fmt::Plain,
                I18N_NOOP2("@message/plain",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "/%1/"));
    SET_PATTERN(Tag::Message, Att::None, Fmt::Rich,
                I18N_NOOP2("@message/rich",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "<i>%1</i>"));

    // -------> Nl
    SET_PATTERN(Tag::Nl, Att::None, Fmt::Plain,
                XXXX_NOOP2("@nl/plain",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "%1\n"));
    SET_PATTERN(Tag::Nl, Att::None, Fmt::Rich,
                XXXX_NOOP2("@nl/rich",
    // i18n: KUIT pattern, see the comment to the first of these entries above.
                           "%1<br/>"));
}

void KuitSemanticsPrivate::setTextTransformData ()
{
    // Mask metaTr with I18N_NOOP2 to have stuff extracted.
    #undef I18N_NOOP2
    #define I18N_NOOP2(ctxt, msg) metaTr(ctxt, msg)

    // i18n: Decide which string is used to delimit keys in a keyboard
    // shortcut (e.g. + in Ctrl+Alt+Tab) in plain text.
    m_comboKeyDelim[Kuit::Fmt::Plain] = I18N_NOOP2("shortcut-key-delimiter/plain", "+");
    m_comboKeyDelim[Kuit::Fmt::Term] = m_comboKeyDelim[Kuit::Fmt::Plain];
    // i18n: Decide which string is used to delimit keys in a keyboard
    // shortcut (e.g. + in Ctrl+Alt+Tab) in rich text.
    m_comboKeyDelim[Kuit::Fmt::Rich] = I18N_NOOP2("shortcut-key-delimiter/rich", "+");

    // i18n: Decide which string is used to delimit elements in a GUI path
    // (e.g. -> in "Go to Settings->Advanced->Core tab.") in plain text.
    m_guiPathDelim[Kuit::Fmt::Plain] = I18N_NOOP2("gui-path-delimiter/plain", "→");
    m_guiPathDelim[Kuit::Fmt::Term] = m_guiPathDelim[Kuit::Fmt::Plain];
    // i18n: Decide which string is used to delimit elements in a GUI path
    // (e.g. -> in "Go to Settings->Advanced->Core tab.") in rich text.
    m_guiPathDelim[Kuit::Fmt::Rich] = I18N_NOOP2("gui-path-delimiter/rich", "→");
    // NOTE: The '→' glyph seems to be available in all widespread fonts.

    // Collect keyboard key names.
    #undef SET_KEYNAME
    #define SET_KEYNAME(rawname) do { \
        /* Normalize key, trim and all lower-case. */ \
        QString normname = QString(rawname).trimmed().toLower(); \
        m_keyNames[normname] = metaTr("keyboard-key-name", rawname); \
    } while (0)

    // Now we need I18N_NOOP2 that does remove context.
    #undef I18N_NOOP2
    #define I18N_NOOP2(ctxt, msg) msg

    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Alt"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "AltGr"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Backspace"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "CapsLock"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Control"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Ctrl"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Del"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Delete"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Down"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "End"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Enter"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Esc"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Escape"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Home"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Hyper"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Ins"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Insert"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Left"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Menu"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Meta"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "NumLock"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "PageDown"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "PageUp"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "PgDown"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "PgUp"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "PauseBreak"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "PrintScreen"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "PrtScr"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Return"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Right"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "ScrollLock"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Shift"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Space"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Super"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "SysReq"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Tab"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Up"));
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "Win"));
    // TODO: Add rest of the key names?

    // i18n: Pattern for the function keys.
    SET_KEYNAME(I18N_NOOP2("keyboard-key-name", "F%1"));
}

QString KuitSemanticsPrivate::format (const QString &text,
                                      const QString &ctxt) const
{
    // Parse context marker to determine format.
    Kuit::FmtVar fmtExplicit = formatFromContextMarker(ctxt, text);

    // Quick check: are there any tags at all?
    if (text.indexOf('<') < 0) {
        return finalizeVisualText(text, fmtExplicit);
    }

    // If format not explicitly given, heuristically determine
    // implicit format based on presence or lack of HTML tags.
    Kuit::FmtVar fmtImplicit = fmtExplicit;
    if (fmtExplicit == Kuit::Fmt::None) {
        fmtImplicit = formatFromTags(text);
    }

    // Decide on the top tag, either TopLong or TopShort,
    // and wrap the text with it.
    Kuit::TagVar toptag;
    QString wtext = equipTopTag(text, toptag);

    // Format the text.
    QString ftext = semanticToVisualText(wtext, fmtExplicit, fmtImplicit);
    if (ftext.isEmpty()) { // error while processing markup
        return salvageMarkup(text, fmtImplicit);
    }

    return ftext;
}

int KuitSemanticsPrivate::attSetKey (const QSet<Kuit::AttVar> &aset)
{
    QList<Kuit::AttVar> alist = aset.toList();
    qSort(alist);
    int key = 0;
    int tenp = 1;
    foreach (const Kuit::AttVar &att, alist) {
        key += att * tenp;
        tenp *= 10;
    }
    return key;
}

Kuit::FmtVar KuitSemanticsPrivate::formatFromContextMarker (
    const QString &ctxmark_, const QString &text)
{
    #ifdef NDEBUG
    Q_UNUSED(text);
    #endif

    KuitSemanticsStaticData *s = semanticsStaticData;

    // Semantic context marker is in the form @rolname:cuename/fmtname,
    // and must start just after any leading whitespace in the context string.
    QString rolname;
    QString fmtname;
    QString cuename;
    QString ctxmark = ctxmark_.trimmed();
    if (ctxmark.startsWith('@')) { // found context marker
        static QRegExp wsRx("\\s");
        ctxmark = ctxmark.mid(1, wsRx.indexIn(ctxmark) - 1);

        // Possible visual format.
        int pfmt = ctxmark.indexOf('/');
        if (pfmt >= 0) {
            fmtname = ctxmark.mid(pfmt + 1);
            ctxmark = ctxmark.left(pfmt);
        }

        // Possible interface subcue.
        int pcue = ctxmark.indexOf(':');
        if (pcue >= 0) {
            cuename = ctxmark.mid(pcue + 1);
            ctxmark = ctxmark.left(pcue);
        }

        // Semantic role.
        rolname = ctxmark;
    }
    // Names remain empty if marker was not found, which is ok.

    // Normalize names.
    rolname = rolname.trimmed().toLower();
    cuename = cuename.trimmed().toLower();
    fmtname = fmtname.trimmed().toLower();

    // Set role from name.
    Kuit::RolVar rol;
    if (s->knownRols.contains(rolname)) { // known role
        rol = s->knownRols[rolname];
    }
    else { // unknown role
        rol = Kuit::Rol::None;
        if (!rolname.isEmpty()) {
            kDebug(173) << QString("Unknown semantic role '@%1' in "
                                   "context marker for message {%2}.")
                                  .arg(rolname, shorten(text));
        }
    }

    // Set subcue from name.
    Kuit::CueVar cue;
    if (s->knownCues.contains(cuename)) { // known subcue
        cue = s->knownCues[cuename];
    }
    else { // unknown or not given subcue
        cue = Kuit::Cue::None;
        if (!cuename.isEmpty()) {
            kDebug(173) << QString("Unknown interface subcue ':%1' in "
                                   "context marker for message {%2}.")
                                  .arg(cuename, shorten(text));
        }
    }

    // Set format from name, or by derivation from contex/subcue.
    Kuit::FmtVar fmt;
    if (s->knownFmts.contains(fmtname)) { // known format
        fmt = s->knownFmts[fmtname];
    }
    else { // unknown or not given format

        // Check first if there is a format defined for role/subcue
        // combination, than for role only, then default to none.
        if (s->defFmts.contains(rol)) {
            if (s->defFmts[rol].contains(cue)) {
                fmt = s->defFmts[rol][cue];
            }
            else {
                fmt = s->defFmts[rol][Kuit::Cue::None];
            }
        }
        else {
            fmt = Kuit::Fmt::None;
        }

        if (!fmtname.isEmpty()) {
            kDebug(173) << QString("Unknown visual format '/%1' in "
                                   "context marker for message {%2}.")
                                  .arg(fmtname, shorten(text));
        }
    }

    return fmt;
}

Kuit::FmtVar KuitSemanticsPrivate::formatFromTags (const QString &text)
{
    KuitSemanticsStaticData *s = semanticsStaticData;
    static QRegExp staticTagRx("<\\s*(\\w+)[^>]*>");

    QRegExp tagRx = staticTagRx; // for thread-safety
    int p = tagRx.indexIn(text);
    while (p >= 0) {
        QString tagname = tagRx.capturedTexts().at(1).toLower();
        if (s->qtHtmlTagNames.contains(tagname)) {
            return Kuit::Fmt::Rich;
        }
        p = tagRx.indexIn(text, p + tagRx.matchedLength());
    }
    return Kuit::Fmt::Plain;
}

QString KuitSemanticsPrivate::equipTopTag (const QString &text_,
                                           Kuit::TagVar &toptag)
{
    KuitSemanticsStaticData *s = semanticsStaticData;

    // Unless the text opens either with TopLong or TopShort tags,
    // make a guess: if it opens with one of Title, Subtitle, Para,
    // consider it TopLong, otherwise TopShort.
    static QRegExp opensWithTagRx("^\\s*<\\s*(\\w+)[^>]*>");
    bool explicitTopTag = false;

    QString text = text_;
    int p = opensWithTagRx.indexIn(text);

    // <qt> or <html> tag are to be ignored for deciding the top tag.
    if (p >= 0) {
        QString fullmatch = opensWithTagRx.capturedTexts().at(0);
        QString tagname = opensWithTagRx.capturedTexts().at(1).toLower();
        if (tagname == "qt" || tagname == "html") {
            // Kill the tag and see if there is another one following,
            // for primary check below.
            text = text.mid(fullmatch.length());
            p = opensWithTagRx.indexIn(text);
        }
    }

    // Check the first non-<qt>/<html> tag.
    if (p >= 0) { // opens with a tag
        QString tagname = opensWithTagRx.capturedTexts().at(1).toLower();
        if (s->knownTags.contains(tagname)) { // a known tag
            Kuit::TagVar tag = s->knownTags[tagname];
            if (   tag == Kuit::Tag::TopLong
                || tag == Kuit::Tag::TopShort) { // explicitly given top tag
                toptag = tag;
                explicitTopTag = true;
            }
            else if (   tag == Kuit::Tag::Para
                     || tag == Kuit::Tag::Title
                     || tag == Kuit::Tag::Subtitle) { // one of long text tags
                toptag = Kuit::Tag::TopLong;
            }
            else { // not one of long text tags
                toptag = Kuit::Tag::TopShort;
            }
        }
        else { // not a KUIT tag
            toptag = Kuit::Tag::TopShort;
        }
    }
    else { // doesn't open with a tag
        toptag = Kuit::Tag::TopShort;
    }

    // Wrap text with top tag if not explicitly given.
    if (!explicitTopTag) {
        return   '<' + s->tagNames[toptag] + '>'
               + text_ // original text, not the one possibly stripped above
               + "</" + s->tagNames[toptag] + '>';
    }
    else {
        return text;
    }
}

#define ENTITY_SUBRX "[a-z]+|#[0-9]+|#x[0-9a-fA-F]+"

QString KuitSemanticsPrivate::semanticToVisualText (const QString &text_,
                                                    Kuit::FmtVar fmtExp_,
                                                    Kuit::FmtVar fmtImp_) const
{
    KuitSemanticsStaticData *s = semanticsStaticData;

    // Replace &-shortcut marker with "&amp;", not to confuse the parser;
    // but do not touch & which forms an XML entity as it is.
    QString original = text_;
    QString text;
    int p = original.indexOf('&');
    while (p >= 0) {
        text.append(original.mid(0, p + 1));
        original.remove(0, p + 1);
        static QRegExp restRx("^("ENTITY_SUBRX");");
        if (original.indexOf(restRx) != 0) { // not an entity
            text.append("amp;");
        }
        p = original.indexOf('&');
    }
    text.append(original);

    Kuit::FmtVar fmtExp = fmtExp_;
    Kuit::FmtVar fmtImp = fmtImp_;
    int numCtx = 0;
    bool hadQtTag = false;
    bool hadAnyHtmlTag = false;
    QStack<OpenEl> openEls;
    QXmlStreamReader xml(text);
    QStringRef lastElementName;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            lastElementName = xml.name();

            // Find first proper enclosing element tag.
            Kuit::TagVar etag = Kuit::Tag::None;
            for (int i = openEls.size() - 1; i >= 0; --i) {
                if (openEls[i].handling == OpenEl::Proper) {
                    etag = openEls[i].tag;
                    break;
                }
            }

            // Collect data about this element.
            OpenEl oel = parseOpenEl(xml, etag, text);
            if (oel.name == "qt" || oel.name == "html") {
                hadQtTag = true;
            }
            if (s->qtHtmlTagNames.contains(oel.name)) {
                hadAnyHtmlTag = true;
            }

            // If this is top tag, check if it overrides the context marker
            // by its ctx attribute.
            if (openEls.isEmpty() && oel.avals.contains(Kuit::Att::Ctx)) {
                // Resolve format override.
                fmtExp = formatFromContextMarker(oel.avals[Kuit::Att::Ctx], text);
                fmtImp = fmtExp;
            }

            // Record the new element on the parse stack.
            openEls.push(oel);

            // Update numeric context.
            if (oel.tag == Kuit::Tag::Numid) {
                ++numCtx;
            }
        }
        else if (xml.isEndElement()) {
            // Get closed element data.
            OpenEl oel = openEls.pop();

            // If this was closing of the top element, we're done.
            if (openEls.isEmpty()) {
                // Return with final touches applied.
                return finalizeVisualText(oel.formattedText, fmtExp,
                                          hadQtTag, hadAnyHtmlTag);
            }

            // Append formatted text segment.
            QString pt = openEls.top().formattedText; // preceding text
            openEls.top().formattedText += formatSubText(pt, oel, fmtImp, numCtx);

            // Update numeric context.
            if (oel.tag == Kuit::Tag::Numid) {
                --numCtx;
            }
        }
        else if (xml.isCharacters()) {
            // Stream reader will automatically resolve default XML entities,
            // which is not desired in this case, as the final text may
            // be rich. Convert them back into entities.
            QString text = xml.text().toString();
            QString ntext;
            foreach (const QChar &c, text) {
                if (s->xmlEntitiesInverse.contains(c)) {
                    QString entname = s->xmlEntitiesInverse[c];
                    ntext += '&' + entname + ';';
                } else {
                    ntext += c;
                }
            }
            openEls.top().formattedText += ntext;
        }
    }

    if (xml.hasError()) {
        kDebug(173) << QString("Markup error in message {%1}: %2. Last tag parsed: %3")
                              .arg(shorten(text), xml.errorString(), lastElementName.toString());
        return QString();
    }

    // Cannot reach here.
    return text;
}

KuitSemanticsPrivate::OpenEl
KuitSemanticsPrivate::parseOpenEl (const QXmlStreamReader &xml,
                                   Kuit::TagVar etag,
                                   const QString &text) const
{
    #ifdef NDEBUG
    Q_UNUSED(text);
    #endif

    KuitSemanticsStaticData *s = semanticsStaticData;

    OpenEl oel;
    oel.name = xml.name().toString().toLower();

    // Collect attribute names and values, and format attribute string.
    QStringList attnams, attvals;
    foreach (const QXmlStreamAttribute &xatt, xml.attributes()) {
        attnams += xatt.name().toString().toLower();
        attvals += xatt.value().toString();
        QChar qc = attvals.last().indexOf('\'') < 0 ? '\'' : '"';
        oel.astr += ' ' + attnams.last() + '=' + qc + attvals.last() + qc;
    }

    if (s->knownTags.contains(oel.name)) { // known KUIT element
        oel.tag = s->knownTags[oel.name];

        // If this element can be contained within enclosing element,
        // mark it proper, otherwise mark it for removal.
        if (etag == Kuit::Tag::None || s->tagSubs[etag].contains(oel.tag)) {
            oel.handling = OpenEl::Proper;
        }
        else {
            oel.handling = OpenEl::Dropout;
            kDebug(173) << QString("Tag '%1' cannot be subtag of '%2' "
                                   "in message {%3}.")
                                  .arg(s->tagNames[oel.tag], s->tagNames[etag],
                                       shorten(text));
        }

        // Resolve attributes and compute attribute set key.
        QSet<Kuit::AttVar> attset;
        for (int i = 0; i < attnams.size(); ++i) {
            if (s->knownAtts.contains(attnams[i])) {
                Kuit::AttVar att = s->knownAtts[attnams[i]];
                if (s->tagAtts[oel.tag].contains(att)) {
                    attset << att;
                    oel.avals[att] = attvals[i];
                }
                else {
                    kDebug(173) << QString("Attribute '%1' cannot be used in "
                                           "tag '%2' in message {%3}.")
                                          .arg(attnams[i], oel.name,
                                               shorten(text));
                }
            }
            else {
                kDebug(173) << QString("Unknown semantic tag attribute '%1' "
                                       "in message {%2}.")
                                      .arg(attnams[i], shorten(text));
            }
        }
        oel.akey = attSetKey(attset);
    }
    else if (oel.name == "qt" || oel.name == "html") {
        // Drop qt/html tags (gets added in the end).
        oel.handling = OpenEl::Dropout;
    }
    else { // other element, leave it in verbatim
        oel.handling = OpenEl::Ignored;
        if (!s->qtHtmlTagNames.contains(oel.name)) {
            kDebug(173) << QString("Tag '%1' is neither semantic nor HTML in "
                                   "message {%3}.")
                                  .arg(oel.name, shorten(text));
        }
    }

    return oel;
}

QString KuitSemanticsPrivate::visualPattern (Kuit::TagVar tag, int akey,
                                             Kuit::FmtVar fmt) const
{
    // Default pattern: simple substitution.
    QString pattern("%1");

    // See if there is a pattern specifically for this element.
    if (   m_patterns.contains(tag)
        && m_patterns[tag].contains(akey)
        && m_patterns[tag][akey].contains(fmt))
    {
        pattern = m_patterns[tag][akey][fmt];
    }

    return pattern;
}

QString KuitSemanticsPrivate::formatSubText (const QString &ptext,
                                             const OpenEl &oel,
                                             Kuit::FmtVar fmt,
                                             int numctx) const
{
    KuitSemanticsStaticData *s = semanticsStaticData;

    if (oel.handling == OpenEl::Proper) {
        // Select formatting pattern.
        QString pattern = visualPattern(oel.tag, oel.akey, fmt);

        // Some tags modify their text.
        QString mtext = modifyTagText(oel.formattedText, oel.tag, oel.avals,
                                      numctx, fmt);

        using namespace Kuit;

        // Format text according to pattern.
        QString ftext;
        /**/ if (oel.tag == Tag::Link && oel.avals.contains(Att::Url)) {
            ftext = pattern.arg(oel.avals[Att::Url], mtext);
        }
        else if (oel.tag == Tag::Command && oel.avals.contains(Att::Section)) {
            ftext = pattern.arg(mtext, oel.avals[Att::Section]);
        }
        else if (oel.tag == Tag::Email && oel.avals.contains(Att::Address)) {
            ftext = pattern.arg(mtext, oel.avals[Att::Address]);
        }
        else if (oel.tag == Tag::Note && oel.avals.contains(Att::Label)) {
            ftext = pattern.arg(oel.avals[Att::Label], mtext);
        }
        else if (oel.tag == Tag::Warning && oel.avals.contains(Att::Label)) {
            ftext = pattern.arg(oel.avals[Att::Label], mtext);
        }
        else {
            ftext = pattern.arg(mtext);
        }

        // Handle leading newlines, if this is not start of the text
        // (ptext is the preceding text).
        if (!ptext.isEmpty() && s->leadingNewlines.contains(oel.tag)) {
            // Count number of present newlines.
            int pnumle, pnumtr, fnumle, fnumtr;
            countWrappingNewlines(ptext, pnumle, pnumtr);
            countWrappingNewlines(ftext, fnumle, fnumtr);
            // Number of leading newlines already present.
            int numle = pnumtr + fnumle;
            // The required extra newlines.
            QString strle;
            if (numle < s->leadingNewlines[oel.tag]) {
                strle = QString(s->leadingNewlines[oel.tag] - numle, '\n');
            }
            ftext = strle + ftext;
        }

        return ftext;
    }
    else if (oel.handling == OpenEl::Ignored) {
        if (oel.name == "br" || oel.name == "hr") {
            // Close these tags in-place (just for looks).
            return '<' + oel.name + "/>";
        }
        else {
            return   '<' + oel.name + oel.astr + '>'
                   + oel.formattedText
                   + "</" + oel.name + '>';
        }
    }
    else { // oel.handling == OpenEl::Dropout
        return oel.formattedText;
    }
}

void KuitSemanticsPrivate::countWrappingNewlines (const QString &text,
                                                  int &numle, int &numtr)
{
    int len = text.length();
    // Number of newlines at start of text.
    numle = 0;
    while (numle < len && text[numle] == '\n') {
        ++numle;
    }
    // Number of newlines at end of text.
    numtr = 0;
    while (numtr < len && text[len - numtr - 1] == '\n') {
        ++numtr;
    }
}

QString KuitSemanticsPrivate::modifyTagText (const QString &text,
                                             Kuit::TagVar tag,
                                             const QHash<Kuit::AttVar, QString> &avals,
                                             int numctx,
                                             Kuit::FmtVar fmt) const
{
    // numctx < 1 means that the number is not in numeric-id context.
    if (   (tag == Kuit::Tag::NumIntg || tag == Kuit::Tag::NumReal) \
        && numctx < 1)
    {
        int fieldWidth = avals.value(Kuit::Att::Width, QString('0')).toInt();
        QString fillStr = avals.value(Kuit::Att::Fill, QString(' '));
        QChar fillChar = !fillStr.isEmpty() ? fillStr[0] : QChar(' ');
        return QString("%1").arg(KGlobal::locale()->formatNumber(text, false),
                                 fieldWidth, fillChar);
    }
    else if (tag == Kuit::Tag::Filename) {
        return QDir::toNativeSeparators(text);
    }
    else if (tag == Kuit::Tag::Shortcut) {
        return KuitFormats::toKeyCombo(text, m_comboKeyDelim[fmt], m_keyNames);
    }
    else if (tag == Kuit::Tag::Interface) {
        return KuitFormats::toInterfacePath(text, m_guiPathDelim[fmt]);
    }

    // Fell through, no modification.
    return text;
}

QString KuitSemanticsPrivate::finalizeVisualText (const QString &final,
                                                  Kuit::FmtVar fmt,
                                                  bool hadQtTag,
                                                  bool hadAnyHtmlTag) const
{
    KuitSemanticsStaticData *s = semanticsStaticData;

    QString text = final;

    // Resolve XML entities if format explicitly not rich
    // and no HTML tag encountered.
    if (fmt != Kuit::Fmt::Rich && !hadAnyHtmlTag)
    {
        static QRegExp staticEntRx("&("ENTITY_SUBRX");");
        // We have to have a local copy here, otherwise this function
        // will not be thread safe because QRegExp is not thread safe.
        QRegExp entRx = staticEntRx;
        int p = entRx.indexIn(text);
        QString plain;
        while (p >= 0) {
            QString ent = entRx.capturedTexts().at(1);
            plain.append(text.mid(0, p));
            text.remove(0, p + ent.length() + 2);
            if (ent.startsWith('#')) { // numeric character entity
                QChar c;
                bool ok;
                if (ent[1] == 'x') {
                    c = QChar(ent.mid(2).toInt(&ok, 16));
                } else {
                    c = QChar(ent.mid(1).toInt(&ok, 10));
                }
                if (ok) {
                    plain.append(c);
                } else { // unknown Unicode point, leave as is
                    plain.append('&' + ent + ';');
                }
            }
            else if (s->xmlEntities.contains(ent)) { // known entity
                plain.append(s->xmlEntities[ent]);
            } else { // unknown entity, just leave as is
                plain.append('&' + ent + ';');
            }
            p = entRx.indexIn(text);
        }
        plain.append(text);
        text = plain;
    }

    // Add top rich tag if format explicitly rich or such tag encountered.
    if (fmt == Kuit::Fmt::Rich || hadQtTag) {
        text = "<html>" + text + "</html>";
    }

    return text;
}

QString KuitSemanticsPrivate::salvageMarkup (const QString &text_,
                                             Kuit::FmtVar fmt) const
{
    KuitSemanticsStaticData *s = semanticsStaticData;
    QString text = text_;
    QString ntext;
    int pos;

    // Resolve KUIT tags simple-mindedly.

    // - tags with content
    static QRegExp staticWrapRx("(<\\s*(\\w+)\\b([^>]*)>)(.*)(<\\s*/\\s*\\2\\s*>)");
    QRegExp wrapRx = staticWrapRx; // for thread-safety
    wrapRx.setMinimal(true);
    pos = 0;
    ntext.clear();
    while (true) {
        int previousPos = pos;
        pos = wrapRx.indexIn(text, previousPos);
        if (pos < 0) {
            ntext += text.mid(previousPos);
            break;
        }
        ntext += text.mid(previousPos, pos - previousPos);
        const QStringList capts = wrapRx.capturedTexts();
        QString tagname = capts[2].toLower();
        QString content = salvageMarkup(capts[4], fmt);
        if (s->knownTags.contains(tagname)) {
            // Select formatting pattern.
            // TODO: Do not ignore attributes (in capts[3]).
            QString pattern = visualPattern(s->knownTags[tagname], 0, fmt);
            ntext += pattern.arg(content);
        } else {
            ntext += capts[1] + content + capts[5];
        }
        pos += wrapRx.matchedLength();
    }
    text = ntext;

    // - content-less tags
    static QRegExp staticNowrRx("<\\s*(\\w+)\\b([^>]*)/\\s*>");
    QRegExp nowrRx = staticNowrRx; // for thread-safety
    nowrRx.setMinimal(true);
    pos = 0;
    ntext.clear();
    while (true) {
        int previousPos = pos;
        pos = nowrRx.indexIn(text, previousPos);
        if (pos < 0) {
            ntext += text.mid(previousPos);
            break;
        }
        ntext += text.mid(previousPos, pos - previousPos);
        const QStringList capts = nowrRx.capturedTexts();
        QString tagname = capts[1].toLower();
        if (s->knownTags.contains(tagname)) {
            QString pattern = visualPattern(s->knownTags[tagname], 0, fmt);
            ntext += pattern.arg(QString());
        } else {
            ntext += capts[0];
        }
        pos += nowrRx.matchedLength();
    }
    text = ntext;

    return text;
}

// -----------------------------------------------------------------------------
// The KuitSemantics methods, only delegate to KuitSemanticsPrivate.

KuitSemantics::KuitSemantics (const QString &lang)
: d(new KuitSemanticsPrivate(lang))
{
}

KuitSemantics::~KuitSemantics ()
{
    delete d;
}

QString KuitSemantics::format (const QString &text, const QString &ctxt) const
{
    return d->format(text, ctxt);
}

bool KuitSemantics::mightBeRichText (const QString &text)
{
    KuitSemanticsStaticData *s = semanticsStaticData;

    // Check by appearance of a valid XML entity at first ampersand.
    int p1 = text.indexOf('&');
    if (p1 >= 0) {
        p1 += 1;
        int p2 = text.indexOf(';', p1);
        return (p2 > p1 && s->xmlEntities.contains(text.mid(p1, p2 - p1)));
    }

    // Check by appearance of a valid Qt rich-text tag at first less-than.
    int tlen = text.length();
    p1 = text.indexOf('<');
    if (p1 >= 0) {
        p1 += 1;
        // Also allow first tag to be closing tag,
        // e.g. in case the text is pieced up with list.join("</foo><foo>")
        bool closing = false;
        while (p1 < tlen && (text[p1].isSpace() || text[p1] == '/')) {
            if (text[p1] == '/') {
                if (!closing) {
                    closing = true;
                } else {
                    return false;
                }
            }
            ++p1;
        }
        for (int p2 = p1; p2 < tlen; ++p2) {
            QChar c = text[p2];
            if (c == '>' || (!closing && c == '/') || c.isSpace()) {
                return s->qtHtmlTagNames.contains(text.mid(p1, p2 - p1));
            } else if (!c.isLetter()) {
                return false;
            }
        }
        return false;
    }

    return false;
}

QString KuitSemantics::escape (const QString &text)
{
    int tlen = text.length();
    QString ntext;
    ntext.reserve(tlen);
    for (int i = 0; i < tlen; ++i) {
        QChar c = text[i];
        if (c == '&') {
            ntext += "&amp;";
        } else if (c == '<') {
            ntext += "&lt;";
        } else if (c == '>') {
            ntext += "&gt;";
        } else if (c == '\'') {
            ntext += "&apos;";
        } else if (c == '"') {
            ntext += "&quot;";
        } else {
            ntext += c;
        }
    }

    return ntext;
}
