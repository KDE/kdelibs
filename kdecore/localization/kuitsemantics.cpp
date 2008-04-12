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

// Truncates string, for output of long messages.
static QString shorten (const QString &str)
{
    const int maxlen = 30;
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
            Ctx, Url, Address, Section, Label
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
            Window, Menu, Tab, Group, Column,
            Slider, Spinbox, Listbox, Textbox, Chooser,
            Check, Radio,
            Inlistbox, Intable, Inrange, Intext,
            Tooltip, Whatsthis, Status, Progress, Tipoftheday, Credit, Shell
        } Var;
    }

    namespace Fmt { // visual formats
        typedef enum {
            Plain, Rich, Term
        } Var;
    }

    namespace Numfmt { // number formats
        typedef enum {
            Posix, US, Euro, Euro2, Euro2ct
        } Var;
    }

    typedef Tag::Var TagVar;
    typedef Att::Var AttVar;
    typedef Rol::Var RolVar;
    typedef Cue::Var CueVar;
    typedef Fmt::Var FmtVar;
    typedef Numfmt::Var NumfmtVar;
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
    QHash<QString, Kuit::NumfmtVar> knownNumfmts;
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
    SETUP_TAG(Emphasis, "emphasis", None, None);
    SETUP_TAG(Placeholder, "placeholder", None, None);
    SETUP_TAG(Email, "email", Address, None);
    SETUP_TAG(Envar, "envar", None, None);
    SETUP_TAG(Message, "message", None, None);
    SETUP_TAG(Numid, "numid", None, None);
    SETUP_TAG(Nl, "nl", None, None);

    SETUP_TAG(NumIntg, KUIT_NUMINTG, None, None); // internal, not part of DTD
    SETUP_TAG(NumReal, KUIT_NUMREAL, None, None); // internal, not part of DTD

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
              Window << Menu << Tab << Group << Column);
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

    // Setup names of number formats.
    #undef SETUP_NUMFMT
    #define SETUP_NUMFMT(numfmt, name) do { \
        knownNumfmts[name] = Kuit::Numfmt::numfmt; \
    } while (0)
    SETUP_NUMFMT(Posix, "posix");
    SETUP_NUMFMT(US, "us");
    SETUP_NUMFMT(Euro, "euro");
    SETUP_NUMFMT(Euro2, "euro2");
    SETUP_NUMFMT(Euro2ct, "euro2ct");

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

K_GLOBAL_STATIC(KuitSemanticsStaticData, staticData)


// -----------------------------------------------------------------------------
// The KuitSemanticsPrivate methods, they do the work.

class KuitSemanticsPrivate
{
    public:

    KuitSemanticsPrivate (const QString &lang_);

    QString format (const QString &text, const QString &ctxt) const;

    // Set visual formatting patterns for text in semantic tags.
    void setFormattingPatterns (const KCatalog &cat);

    // Set data used in transformation of text within semantic tags.
    void setTextTransformData (const KCatalog &cat);

    // Compute integer hash key from the set of attributes.
    static int attSetKey (const QSet<Kuit::AttVar> &aset = QSet<Kuit::AttVar>());

    // Determine visual format by parsing the context marker.
    static Kuit::FmtVar formatFromContextMarker (const QString &ctxmark,
                                                 const QString &text);

    // Apply appropriate top tag is to the text.
    static QString equipTopTag (const QString &text, Kuit::TagVar &toptag,
                                Kuit::FmtVar &fmt);

    // Formats the semantic into visual text.
    QString semanticToVisualText (const QString &text,
                                  Kuit::FmtVar fmt) const;

    // Final touches to the formatted text.
    QString finalizeVisualText (const QString &final,
                                Kuit::FmtVar fmt,
                                bool hadQtTag = false) const;

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

    // Format text of the element.
    QString formatSubText (const QString &ptext, const OpenEl &oel,
                           Kuit::FmtVar fmt, int numctx) const;

    // Count number of newlines at start and at end of text.
    static void countWrappingNewlines (const QString &ptext,
                                       int &numle, int &numtr);

    // Modifies text for some tags.
    QString modifyTagText (Kuit::TagVar tag, const QString &text,
                           int numctx, Kuit::FmtVar fmt) const;

    private:

    QString m_lang;

    QHash<Kuit::TagVar,
          QHash<int, // attribute set key
                QHash<Kuit::FmtVar, QString> > > m_patterns;

    Kuit::NumfmtVar m_numfmtInt;
    Kuit::NumfmtVar m_numfmtReal;

    QHash<Kuit::FmtVar, QString> m_comboKeyDelim;
    QHash<Kuit::FmtVar, QString> m_guiPathDelim;

    QHash<QString, QString> m_keyNames;
};

KuitSemanticsPrivate::KuitSemanticsPrivate (const QString &lang)
{
    m_lang = lang;

    // NOTE: This function draws translation from raw message catalogs
    // because full i18n system is not available at this point (this
    // function is called within the initialization of the i18n system).
    //
    // A possibility would be to delay the initializations until the first
    // call of format(); however, even so it is preferable to do raw reads
    // from the catalog, as we must assure that the exact supplied language
    // is being used for translation (normal i18n calls do fallbacks).
    KCatalog cat("kdelibs4", m_lang);

    // Get formatting patterns for all tag/att/fmt combinations.
    setFormattingPatterns(cat);

    // Get data for tag text transformations.
    setTextTransformData(cat);
}

void KuitSemanticsPrivate::setFormattingPatterns (const KCatalog &cat)
{
    using namespace Kuit;

    // Macro to expedite setting the patterns.
    #undef SET_PATTERN
    #define SET_PATTERN(tag, atts, fmt, ctxt_ptrn) do { \
        QSet<AttVar> aset; \
        aset << atts; \
        int akey = attSetKey(aset); \
        QString pattern = cat.translate(ctxt_ptrn); \
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

void KuitSemanticsPrivate::setTextTransformData (const KCatalog &cat)
{
    KuitSemanticsStaticData *s = staticData;

    m_numfmtInt = Kuit::Numfmt::Posix;
    // i18n: Decide how integer-valued amounts will be formatted in your
    // language. Currently available number formats are:
    //   posix   - decimal point
    //   us      - thousands separation by comma, decimal point
    //   euro    - thousands separation by point, decimal comma
    //   euro2   - thousands separation by space, decimal comma
    //   euro2ct - as euro2, except thousand not separated when <10000
    // If none of the existing formats is appropriate for your language,
    // write to kde-i18n-doc@kde.org to arrange for a new format.
    QString fmtnameInt = cat.translate("number-format:integer", "us").toLower();
    if (s->knownNumfmts.contains(fmtnameInt)) {
        m_numfmtInt = s->knownNumfmts[fmtnameInt];
    }
    else {
        kDebug(173) << QString("Format of integer numbers '%1', selected in "
                               "kdelibs4.po, is not valid; using POSIX format.")
                              .arg(fmtnameInt);
    }

    m_numfmtReal = Kuit::Numfmt::Posix;
    // i18n: Decide how real-valued amounts will be formatted in your
    // language. See the comment to previous entry.
    QString fmtnameReal = cat.translate("number-format:real", "us").toLower();
    if (s->knownNumfmts.contains(fmtnameReal)) {
        m_numfmtReal = s->knownNumfmts[fmtnameReal];
    }
    else {
        kDebug(173) << QString("Format of real numbers '%1', selected in "
                               "kdelibs4.po, is not valid; using POSIX format.")
                              .arg(fmtnameReal);
    }

    // i18n: Decide which string is used to delimit keys in a keyboard
    // shortcut (e.g. + in Ctrl+Alt+Tab) in plain text.
    m_comboKeyDelim[Kuit::Fmt::Plain] = cat.translate("shortcut-key-delimiter/plain", "+");
    m_comboKeyDelim[Kuit::Fmt::Term] = m_comboKeyDelim[Kuit::Fmt::Plain];
    // i18n: Decide which string is used to delimit keys in a keyboard
    // shortcut (e.g. + in Ctrl+Alt+Tab) in rich text.
    m_comboKeyDelim[Kuit::Fmt::Rich] = cat.translate("shortcut-key-delimiter/rich", "+");

    // i18n: Decide which string is used to delimit elements in a GUI path
    // (e.g. -> in "Go to Settings->Advanced->Core tab.") in plain text.
    m_guiPathDelim[Kuit::Fmt::Plain] = cat.translate("gui-path-delimiter/plain", "→");
    m_guiPathDelim[Kuit::Fmt::Term] = m_guiPathDelim[Kuit::Fmt::Plain];
    // i18n: Decide which string is used to delimit elements in a GUI path
    // (e.g. -> in "Go to Settings->Advanced->Core tab.") in rich text.
    m_guiPathDelim[Kuit::Fmt::Rich] = cat.translate("gui-path-delimiter/rich", "→");
    // NOTE: The '→' glyph seems to be available in all widespread fonts.

    // Collect keyboard key names.
    #undef SET_KEYNAME
    #define SET_KEYNAME(rawname) do { \
        /* Normalize key, trim and all lower-case. */ \
        QString normname = QString(rawname).trimmed().toLower(); \
        m_keyNames[normname] = cat.translate("keyboard-key-name", rawname); \
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
    Kuit::FmtVar fmt = formatFromContextMarker(ctxt, text);

    // Quick check: are there any tags at all?
    if (text.indexOf('<') < 0) {
        return finalizeVisualText(text, fmt);
    }

    // Decide on the top tag, either TopLong or TopShort,
    // and wrap the text with it. Possibly also override format.
    Kuit::TagVar toptag;
    QString wtext = equipTopTag(text, toptag, fmt);

    // Format the text.
    QString ftext = semanticToVisualText(wtext, fmt);
    if (ftext.isEmpty()) { // error while processing markup
        return text;
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

    KuitSemanticsStaticData *s = staticData;

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
        // combination, than for role only, then default to Plain.
        if (s->defFmts.contains(rol)) {
            if (s->defFmts[rol].contains(cue)) {
                fmt = s->defFmts[rol][cue];
            }
            else {
                fmt = s->defFmts[rol][Kuit::Cue::None];
            }
        }
        else {
            fmt = Kuit::Fmt::Plain;
        }

        if (!fmtname.isEmpty()) {
            kDebug(173) << QString("Unknown visual format '/%1' in "
                                   "context marker for message {%2}.")
                                  .arg(fmtname, shorten(text));
        }
    }

    return fmt;
}

QString KuitSemanticsPrivate::equipTopTag (const QString &text_,
                                           Kuit::TagVar &toptag,
                                           Kuit::FmtVar &fmt)
{
    KuitSemanticsStaticData *s = staticData;

    // Unless the text opens either with TopLong or TopShort tags,
    // make a guess: if it opens with one of Title, Subtitle, Para,
    // consider it TopLong, otherwise TopShort.
    static QRegExp opensWithTagRx("^\\s*<\\s*(\\w+)[^>]*>");
    bool explicitTopTag = false;

    QString text = text_;
    int p = opensWithTagRx.indexIn(text);

    // First check for <qt> or <html> tag, which are to be ignored in the
    // context of deciding upon the top tag, but do override the visual format.
    if (p >= 0) {
        QString fullmatch = opensWithTagRx.capturedTexts().at(0);
        QString tagname = opensWithTagRx.capturedTexts().at(1).toLower();
        if (tagname == "qt" || tagname == "html") {
            // Override format.
            fmt = Kuit::Fmt::Rich;
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

QString KuitSemanticsPrivate::semanticToVisualText (const QString &text_,
                                                    Kuit::FmtVar fmt_) const
{
    KuitSemanticsStaticData *s = staticData;

    // Replace &-shortcut marker with "&amp;", not to confuse the parser;
    // but do not touch & in "&[a-z]+;", which is an XML entity as it is.
    QString original = text_;
    QString text;
    int p = original.indexOf('&');
    while (p >= 0) {
        text.append(original.mid(0, p + 1));
        original.remove(0, p + 1);
        static QRegExp restRx("^(?:[a-z]+|#[0-9]+);");
        if (original.indexOf(restRx) != 0) { // not an entity
            text.append("amp;");
        }
        p = original.indexOf('&');
    }
    text.append(original);

    Kuit::FmtVar fmt = fmt_;
    int numCtx = 0;
    bool hadQtTag = false;
    QStack<OpenEl> openEls;
    QXmlStreamReader xml(text);

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
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

            // If this is top tag, check if it overrides the context marker
            // by its ctx attribute.
            if (openEls.isEmpty() && oel.avals.contains(Kuit::Att::Ctx)) {
                // Resolve format override.
                fmt = formatFromContextMarker(oel.avals[Kuit::Att::Ctx], text);
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
                return finalizeVisualText(oel.formattedText, fmt, hadQtTag);
            }

            // Append formatted text segment.
            QString pt = openEls.top().formattedText; // preceding text
            openEls.top().formattedText += formatSubText(pt, oel, fmt, numCtx);

            // Update numeric context.
            if (oel.tag == Kuit::Tag::Numid) {
                --numCtx;
            }
        }
        else if (xml.isCharacters()) {
            // Stream reader will automatically reslove entities, which is
            // not desired in this case, as the final text may be rich.
            // The text element will be broken at the resolved entity, so
            // the first character may be one of those resolved.
            // Convert it back into an entity.
            QString text = xml.text().toString();
            QString firstChar = text.left(1);
            if (s->xmlEntitiesInverse.contains(firstChar)) {
                QString entname = s->xmlEntitiesInverse[firstChar];
                text = '&' + entname + ';' + text.mid(1);
            }
            openEls.top().formattedText += text;
        }
    }

    if (xml.hasError()) {
        kDebug(173) << QString("Markup error in message {%1}: %2")
                              .arg(shorten(text), xml.errorString());
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

    KuitSemanticsStaticData *s = staticData;

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

QString KuitSemanticsPrivate::formatSubText (const QString &ptext,
                                             const OpenEl &oel,
                                             Kuit::FmtVar fmt,
                                             int numctx) const
{
    KuitSemanticsStaticData *s = staticData;

    if (oel.handling == OpenEl::Proper) {
        // Default pattern: simple substitution.
        QString pattern("%1");

        // See if there is a special pattern for this element.
        if (   m_patterns.contains(oel.tag)
            && m_patterns[oel.tag].contains(oel.akey)
            && m_patterns[oel.tag][oel.akey].contains(fmt)) {
            pattern = m_patterns[oel.tag][oel.akey][fmt];
        }

        // Some tags modify their text.
        QString mtext = modifyTagText(oel.tag, oel.formattedText, numctx, fmt);

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

QString KuitSemanticsPrivate::modifyTagText (Kuit::TagVar tag,
                                             const QString &text,
                                             int numctx,
                                             Kuit::FmtVar fmt) const
{
    // numctx < 1 means that the number is not in numeric-id context.
    if (   (tag == Kuit::Tag::NumIntg || tag == Kuit::Tag::NumReal) \
        && numctx < 1)
    {
        int numfmt = (tag == Kuit::Tag::NumIntg ? m_numfmtInt : m_numfmtReal);
        switch (numfmt) {
        case Kuit::Numfmt::US:
            return KuitFormats::toNumberUS(text);
        case Kuit::Numfmt::Euro:
            return KuitFormats::toNumberEuro(text);
        case Kuit::Numfmt::Euro2:
            return KuitFormats::toNumberEuro2(text);
        case Kuit::Numfmt::Euro2ct:
            return KuitFormats::toNumberEuro2ct(text);
        default:
            return text;
        }
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
                                                  bool hadQtTag) const
{
    KuitSemanticsStaticData *s = staticData;

    QString text = final;

    // Wrap with <html> tag if rich text.
    // *Do not* go with <qt> instead: if the message is later concatenated
    // to more rich text, <qt> would introduce line break, whereas a
    // non-top <html> will be just ignored by Qt rich text engine.
    if (fmt == Kuit::Fmt::Rich) {
        QString rich = "<html>" + text + "</html>";
        return rich;
    }
    // Replace XML entities if not rich text.
    else {
        static QRegExp entRx("&([a-z]+);");
        QString plain;
        int p = entRx.indexIn(text);
        while (p >= 0) {
            QString ent = entRx.capturedTexts().at(1);
            plain.append(text.mid(0, p));
            text.remove(0, p + ent.length() + 2);
            if (s->xmlEntities.contains(ent)) { // known entity
                plain.append(s->xmlEntities[ent]);
            }
            else { // unknown entity, just leave as is
                plain.append('&' + ent + ';');
            }
            p = entRx.indexIn(text);
        }
        plain.append(text);

        // If there was a qt/html tag, it means the text was intended as rich
        // but it was not equipped with KUIT. Since other HTML tags have
        // remained preserved in that case, also put back the <html> tag
        // which was removed while formatting.
        if (hadQtTag)
            plain = "<html>" + text + "</html>";

        return plain;
    }
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

