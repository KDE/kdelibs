#include "misc/htmlnames.h"
#include "dom/dom_string.h"

using namespace DOM;

namespace khtml {

IDTable<NamespaceFactory>* NamespaceFactory::s_idTable;
IDTable<NamespaceFactory>* NamespaceFactory::initIdTable()
{
    s_idTable = new IDTable<NamespaceFactory>();
    s_idTable->addStaticMapping(DOM::xhtmlNamespace, XHTML_NAMESPACE);
    s_idTable->addStaticMapping(DOM::emptyNamespace, DOMString());
    s_idTable->addStaticMapping(DOM::svgNamespace, SVG_NAMESPACE);
    return s_idTable;
}

IDTable<LocalNameFactory>* LocalNameFactory::s_idTable;
IDTable<LocalNameFactory>* LocalNameFactory::initIdTable()
{
    s_idTable = new IDTable<LocalNameFactory>();
    s_idTable->addStaticMapping(0, DOMString());
    s_idTable->addStaticMapping(localNamePart(ID_A), "a");
    s_idTable->addStaticMapping(localNamePart(ID_ABBR), "abbr");
    s_idTable->addStaticMapping(localNamePart(ID_ACRONYM), "acronym");
    s_idTable->addStaticMapping(localNamePart(ID_ADDRESS), "address");
    s_idTable->addStaticMapping(localNamePart(ID_APPLET), "applet");
    s_idTable->addStaticMapping(localNamePart(ID_AREA), "area");
    s_idTable->addStaticMapping(localNamePart(ID_AUDIO), "audio");
    s_idTable->addStaticMapping(localNamePart(ID_B), "b");
    s_idTable->addStaticMapping(localNamePart(ID_BASE), "base");
    s_idTable->addStaticMapping(localNamePart(ID_BASEFONT), "basefont");
    s_idTable->addStaticMapping(localNamePart(ID_BDO), "bdo");
    s_idTable->addStaticMapping(localNamePart(ID_BIG), "big");
    s_idTable->addStaticMapping(localNamePart(ID_BLOCKQUOTE), "blockquote");
    s_idTable->addStaticMapping(localNamePart(ID_BODY), "body");
    s_idTable->addStaticMapping(localNamePart(ID_BR), "br");
    s_idTable->addStaticMapping(localNamePart(ID_BUTTON), "button");
    s_idTable->addStaticMapping(localNamePart(ID_CANVAS), "canvas");
    s_idTable->addStaticMapping(localNamePart(ID_CAPTION), "caption");
    s_idTable->addStaticMapping(localNamePart(ID_CENTER), "center");
    s_idTable->addStaticMapping(localNamePart(ID_CITE), "cite");
    s_idTable->addStaticMapping(localNamePart(ID_CODE), "code");
    s_idTable->addStaticMapping(localNamePart(ID_COL), "col");
    s_idTable->addStaticMapping(localNamePart(ID_COLGROUP), "colgroup");
    s_idTable->addStaticMapping(localNamePart(ID_DD), "dd");
    s_idTable->addStaticMapping(localNamePart(ID_DEL), "del");
    s_idTable->addStaticMapping(localNamePart(ID_DFN), "dfn");
    s_idTable->addStaticMapping(localNamePart(ID_DIR), "dir");
    s_idTable->addStaticMapping(localNamePart(ID_DIV), "div");
    s_idTable->addStaticMapping(localNamePart(ID_DL), "dl");
    s_idTable->addStaticMapping(localNamePart(ID_DT), "dt");
    s_idTable->addStaticMapping(localNamePart(ID_EM), "em");
    s_idTable->addStaticMapping(localNamePart(ID_EMBED), "embed");
    s_idTable->addStaticMapping(localNamePart(ID_FIELDSET), "fieldset");
    s_idTable->addStaticMapping(localNamePart(ID_FONT), "font");
    s_idTable->addStaticMapping(localNamePart(ID_FORM), "form");
    s_idTable->addStaticMapping(localNamePart(ID_FRAME), "frame");
    s_idTable->addStaticMapping(localNamePart(ID_FRAMESET), "frameset");
    s_idTable->addStaticMapping(localNamePart(ID_H1), "h1");
    s_idTable->addStaticMapping(localNamePart(ID_H2), "h2");
    s_idTable->addStaticMapping(localNamePart(ID_H3), "h3");
    s_idTable->addStaticMapping(localNamePart(ID_H4), "h4");
    s_idTable->addStaticMapping(localNamePart(ID_H5), "h5");
    s_idTable->addStaticMapping(localNamePart(ID_H6), "h6");
    s_idTable->addStaticMapping(localNamePart(ID_HEAD), "head");
    s_idTable->addStaticMapping(localNamePart(ID_HR), "hr");
    s_idTable->addStaticMapping(localNamePart(ID_HTML), "html");
    s_idTable->addStaticMapping(localNamePart(ID_I), "i");
    s_idTable->addStaticMapping(localNamePart(ID_IFRAME), "iframe");
    s_idTable->addStaticMapping(localNamePart(ID_ILAYER), "ilayer");
    s_idTable->addStaticMapping(localNamePart(ID_IMG), "img");
    s_idTable->addStaticMapping(localNamePart(ID_INPUT), "input");
    s_idTable->addStaticMapping(localNamePart(ID_INS), "ins");
    s_idTable->addStaticMapping(localNamePart(ID_ISINDEX), "isindex");
    s_idTable->addStaticMapping(localNamePart(ID_KBD), "kbd");
    s_idTable->addStaticMapping(localNamePart(ID_KEYGEN), "keygen");
    s_idTable->addStaticMapping(localNamePart(ID_LABEL), "label");
    s_idTable->addStaticMapping(localNamePart(ID_LAYER), "layer");
    s_idTable->addStaticMapping(localNamePart(ID_LEGEND), "legend");
    s_idTable->addStaticMapping(localNamePart(ID_LI), "li");
    s_idTable->addStaticMapping(localNamePart(ID_LINK), "link");
    s_idTable->addStaticMapping(localNamePart(ID_MAP), "map");
    s_idTable->addStaticMapping(localNamePart(ID_MARQUEE), "marquee");
    s_idTable->addStaticMapping(localNamePart(ID_MENU), "menu");
    s_idTable->addStaticMapping(localNamePart(ID_META), "meta");
    s_idTable->addStaticMapping(localNamePart(ID_NOBR), "nobr");
    s_idTable->addStaticMapping(localNamePart(ID_NOEMBED), "noembed");
    s_idTable->addStaticMapping(localNamePart(ID_NOFRAMES), "noframes");
    s_idTable->addStaticMapping(localNamePart(ID_NOSCRIPT), "noscript");
    s_idTable->addStaticMapping(localNamePart(ID_NOLAYER), "nolayer");
    s_idTable->addStaticMapping(localNamePart(ID_OBJECT), "object");
    s_idTable->addStaticMapping(localNamePart(ID_OL), "ol");
    s_idTable->addStaticMapping(localNamePart(ID_OPTGROUP), "optgroup");
    s_idTable->addStaticMapping(localNamePart(ID_OPTION), "option");
    s_idTable->addStaticMapping(localNamePart(ID_P), "p");
    s_idTable->addStaticMapping(localNamePart(ID_PARAM), "param");
    s_idTable->addStaticMapping(localNamePart(ID_PLAINTEXT), "plaintext");
    s_idTable->addStaticMapping(localNamePart(ID_PRE), "pre");
    s_idTable->addStaticMapping(localNamePart(ID_Q), "q");
    s_idTable->addStaticMapping(localNamePart(ID_S), "s");
    s_idTable->addStaticMapping(localNamePart(ID_SAMP), "samp");
    s_idTable->addStaticMapping(localNamePart(ID_SCRIPT), "script");
    s_idTable->addStaticMapping(localNamePart(ID_SELECT), "select");
    s_idTable->addStaticMapping(localNamePart(ID_SMALL), "small");
    s_idTable->addStaticMapping(localNamePart(ID_SOURCE), "source");
    s_idTable->addStaticMapping(localNamePart(ID_SPAN), "span");
    s_idTable->addStaticMapping(localNamePart(ID_STRIKE), "strike");
    s_idTable->addStaticMapping(localNamePart(ID_STRONG), "strong");
    s_idTable->addStaticMapping(localNamePart(ID_STYLE), "style");
    s_idTable->addStaticMapping(localNamePart(ID_SUB), "sub");
    s_idTable->addStaticMapping(localNamePart(ID_SUP), "sup");
    s_idTable->addStaticMapping(localNamePart(ID_TABLE), "table");
    s_idTable->addStaticMapping(localNamePart(ID_TBODY), "tbody");
    s_idTable->addStaticMapping(localNamePart(ID_TD), "td");
    s_idTable->addStaticMapping(localNamePart(ID_TEXTAREA), "textarea");
    s_idTable->addStaticMapping(localNamePart(ID_TFOOT), "tfoot");
    s_idTable->addStaticMapping(localNamePart(ID_TH), "th");
    s_idTable->addStaticMapping(localNamePart(ID_THEAD), "thead");
    s_idTable->addStaticMapping(localNamePart(ID_TITLE), "title");
    s_idTable->addStaticMapping(localNamePart(ID_TR), "tr");
    s_idTable->addStaticMapping(localNamePart(ID_TT), "tt");
    s_idTable->addStaticMapping(localNamePart(ID_U), "u");
    s_idTable->addStaticMapping(localNamePart(ID_UL), "ul");
    s_idTable->addStaticMapping(localNamePart(ID_VAR), "var");
    s_idTable->addStaticMapping(localNamePart(ID_VIDEO), "video");
    s_idTable->addStaticMapping(localNamePart(ID_WBR), "wbr");
    s_idTable->addStaticMapping(localNamePart(ID_XMP), "xmp");
    s_idTable->addHiddenMapping(ID_TEXT, "text");
    s_idTable->addHiddenMapping(ID_COMMENT, "comment");
    s_idTable->addStaticMapping(localNamePart(ATTR_ACCEPT_CHARSET), "accept-charset");
    s_idTable->addStaticMapping(localNamePart(ATTR_ACCEPT), "accept");
    s_idTable->addStaticMapping(localNamePart(ATTR_ACCESSKEY), "accesskey");
    s_idTable->addStaticMapping(localNamePart(ATTR_ALIGN), "align");
    s_idTable->addStaticMapping(localNamePart(ATTR_ALINK), "alink");
    s_idTable->addStaticMapping(localNamePart(ATTR_AUTOCOMPLETE), "autocomplete");
    s_idTable->addStaticMapping(localNamePart(ATTR_AUTOPLAY), "autoplay");
    s_idTable->addStaticMapping(localNamePart(ATTR_AXIS), "axis");
    s_idTable->addStaticMapping(localNamePart(ATTR_BEHAVIOR), "behavior");
    s_idTable->addStaticMapping(localNamePart(ATTR_BGCOLOR), "bgcolor");
    s_idTable->addStaticMapping(localNamePart(ATTR_BGPROPERTIES), "bgproperties");
    s_idTable->addStaticMapping(localNamePart(ATTR_BORDER), "border");
    s_idTable->addStaticMapping(localNamePart(ATTR_BORDERCOLOR), "bordercolor");
    s_idTable->addStaticMapping(localNamePart(ATTR_CELLPADDING), "cellpadding");
    s_idTable->addStaticMapping(localNamePart(ATTR_CELLSPACING), "cellspacing");
    s_idTable->addStaticMapping(localNamePart(ATTR_CHAR), "char");
    s_idTable->addStaticMapping(localNamePart(ATTR_CHALLENGE), "challenge");
    s_idTable->addStaticMapping(localNamePart(ATTR_CHAROFF), "charoff");
    s_idTable->addStaticMapping(localNamePart(ATTR_CHARSET), "charset");
    s_idTable->addStaticMapping(localNamePart(ATTR_CHECKED), "checked");
    s_idTable->addStaticMapping(localNamePart(ATTR_CLEAR), "clear");
    s_idTable->addStaticMapping(localNamePart(ATTR_CODETYPE), "codetype");
    s_idTable->addStaticMapping(localNamePart(ATTR_COLOR), "color");
    s_idTable->addStaticMapping(localNamePart(ATTR_COLS), "cols");
    s_idTable->addStaticMapping(localNamePart(ATTR_COLSPAN), "colspan");
    s_idTable->addStaticMapping(localNamePart(ATTR_COMPACT), "compact");
    s_idTable->addStaticMapping(localNamePart(ATTR_CONTENTEDITABLE), "contenteditable");
    s_idTable->addStaticMapping(localNamePart(ATTR_CONTROLS), "controls");
    s_idTable->addStaticMapping(localNamePart(ATTR_COORDS), "coords");
    s_idTable->addStaticMapping(localNamePart(ATTR_DECLARE), "declare");
    s_idTable->addStaticMapping(localNamePart(ATTR_DEFER), "defer");
    s_idTable->addStaticMapping(localNamePart(ATTR_DIRECTION), "direction");
    s_idTable->addStaticMapping(localNamePart(ATTR_DISABLED), "disabled");
    s_idTable->addStaticMapping(localNamePart(ATTR_ENCTYPE), "enctype");
    s_idTable->addStaticMapping(localNamePart(ATTR_END), "end");
    s_idTable->addStaticMapping(localNamePart(ATTR_FACE), "face");
    s_idTable->addStaticMapping(localNamePart(ATTR_FRAMEBORDER), "frameborder");
    s_idTable->addStaticMapping(localNamePart(ATTR_HEIGHT), "height");
    s_idTable->addStaticMapping(localNamePart(ATTR_HIDDEN), "hidden");
    s_idTable->addStaticMapping(localNamePart(ATTR_HREFLANG), "hreflang");
    s_idTable->addStaticMapping(localNamePart(ATTR_HSPACE), "hspace");
    s_idTable->addStaticMapping(localNamePart(ATTR_HTTP_EQUIV), "http-equiv");
    s_idTable->addStaticMapping(localNamePart(ATTR_ISMAP), "ismap");
    s_idTable->addStaticMapping(localNamePart(ATTR_LANG), "lang");
    s_idTable->addStaticMapping(localNamePart(ATTR_LANGUAGE), "language");
    s_idTable->addStaticMapping(localNamePart(ATTR_LEFT), "left");
    s_idTable->addStaticMapping(localNamePart(ATTR_LEFTMARGIN), "leftmargin");
    s_idTable->addStaticMapping(localNamePart(ATTR_LOOP), "loop");
    s_idTable->addStaticMapping(localNamePart(ATTR_LOOPEND), "loopend");
    s_idTable->addStaticMapping(localNamePart(ATTR_LOOPSTART), "loopstart");
    s_idTable->addStaticMapping(localNamePart(ATTR_MARGINHEIGHT), "marginheight");
    s_idTable->addStaticMapping(localNamePart(ATTR_MARGINWIDTH), "marginwidth");
    s_idTable->addStaticMapping(localNamePart(ATTR_MAXLENGTH), "maxlength");
    s_idTable->addStaticMapping(localNamePart(ATTR_MEDIA), "media");
    s_idTable->addStaticMapping(localNamePart(ATTR_METHOD), "method");
    s_idTable->addStaticMapping(localNamePart(ATTR_MULTIPLE), "multiple");
    s_idTable->addStaticMapping(localNamePart(ATTR_NOHREF), "nohref");
    s_idTable->addStaticMapping(localNamePart(ATTR_NORESIZE), "noresize");
    s_idTable->addStaticMapping(localNamePart(ATTR_NOSAVE), "nosave");
    s_idTable->addStaticMapping(localNamePart(ATTR_NOSHADE), "noshade");
    s_idTable->addStaticMapping(localNamePart(ATTR_NOWRAP), "nowrap");
    s_idTable->addStaticMapping(localNamePart(ATTR_ONABORT), "onabort");
    s_idTable->addStaticMapping(localNamePart(ATTR_ONERROR), "onerror");
    s_idTable->addStaticMapping(localNamePart(ATTR_ONRESIZE), "onresize");
    s_idTable->addStaticMapping(localNamePart(ATTR_OVERSRC), "oversrc");
    s_idTable->addStaticMapping(localNamePart(ATTR_PAGEX), "pagex");
    s_idTable->addStaticMapping(localNamePart(ATTR_PAGEY), "pagey");
    s_idTable->addStaticMapping(localNamePart(ATTR_PLAIN), "plain");
    s_idTable->addStaticMapping(localNamePart(ATTR_PLAYCOUNT), "playcount");
    s_idTable->addStaticMapping(localNamePart(ATTR_PLUGINPAGE), "pluginpage");
    s_idTable->addStaticMapping(localNamePart(ATTR_PLUGINSPAGE), "pluginspage");
    s_idTable->addStaticMapping(localNamePart(ATTR_PLUGINURL), "pluginurl");
    s_idTable->addStaticMapping(localNamePart(ATTR_POSTER), "poster");
    s_idTable->addStaticMapping(localNamePart(ATTR_READONLY), "readonly");
    s_idTable->addStaticMapping(localNamePart(ATTR_REL), "rel");
    s_idTable->addStaticMapping(localNamePart(ATTR_REV), "rev");
    s_idTable->addStaticMapping(localNamePart(ATTR_ROWS), "rows");
    s_idTable->addStaticMapping(localNamePart(ATTR_ROWSPAN), "rowspan");
    s_idTable->addStaticMapping(localNamePart(ATTR_RULES), "rules");
    s_idTable->addStaticMapping(localNamePart(ATTR_SCOPE), "scope");
    s_idTable->addStaticMapping(localNamePart(ATTR_SCROLLAMOUNT), "scrollamount");
    s_idTable->addStaticMapping(localNamePart(ATTR_SCROLLDELAY), "scrolldelay");
    s_idTable->addStaticMapping(localNamePart(ATTR_SCROLLING), "scrolling");
    s_idTable->addStaticMapping(localNamePart(ATTR_SELECTED), "selected");
    s_idTable->addStaticMapping(localNamePart(ATTR_SHAPE), "shape");
    s_idTable->addStaticMapping(localNamePart(ATTR_SIZE), "size");
    s_idTable->addStaticMapping(localNamePart(ATTR_START), "start");
    s_idTable->addStaticMapping(localNamePart(ATTR_TABINDEX), "tabindex");
    s_idTable->addStaticMapping(localNamePart(ATTR_TARGET), "target");
    s_idTable->addStaticMapping(localNamePart(ATTR_TEXT), "text");
    s_idTable->addStaticMapping(localNamePart(ATTR_TOP), "top");
    s_idTable->addStaticMapping(localNamePart(ATTR_TOPMARGIN), "topmargin");
    s_idTable->addStaticMapping(localNamePart(ATTR_TRUESPEED), "truespeed");
    s_idTable->addStaticMapping(localNamePart(ATTR_TYPE), "type");
    s_idTable->addStaticMapping(localNamePart(ATTR_UNKNOWN), "unknown");
    s_idTable->addStaticMapping(localNamePart(ATTR_VALIGN), "valign");
    s_idTable->addStaticMapping(localNamePart(ATTR_VALUETYPE), "valuetype");
    s_idTable->addStaticMapping(localNamePart(ATTR_VERSION), "version");
    s_idTable->addStaticMapping(localNamePart(ATTR_VISIBILITY), "visibility");
    s_idTable->addStaticMapping(localNamePart(ATTR_VLINK), "vlink");
    s_idTable->addStaticMapping(localNamePart(ATTR_VSPACE), "vspace");
    s_idTable->addStaticMapping(localNamePart(ATTR_WIDTH), "width");
    s_idTable->addStaticMapping(localNamePart(ATTR_WRAP), "wrap");
    s_idTable->addStaticMapping(localNamePart(ATTR_Z_INDEX), "z-index");
    s_idTable->addStaticMapping(localNamePart(ATTR_ACTION), "action");
    s_idTable->addStaticMapping(localNamePart(ATTR_ALT), "alt");
    s_idTable->addStaticMapping(localNamePart(ATTR_ARCHIVE), "archive");
    s_idTable->addStaticMapping(localNamePart(ATTR_BACKGROUND), "background");
    s_idTable->addStaticMapping(localNamePart(ATTR_CLASS), "class");
    s_idTable->addStaticMapping(localNamePart(ATTR_CLASSID), "classid");
    s_idTable->addStaticMapping(localNamePart(ATTR_CODEBASE), "codebase");
    s_idTable->addStaticMapping(localNamePart(ATTR_CONTENT), "content");
    s_idTable->addStaticMapping(localNamePart(ATTR_DATA), "data");
    s_idTable->addStaticMapping(localNamePart(ATTR_DATETIME), "datetime");
    s_idTable->addStaticMapping(localNamePart(ATTR_FOR), "for");
    s_idTable->addStaticMapping(localNamePart(ATTR_HEADERS), "headers");
    s_idTable->addStaticMapping(localNamePart(ATTR_HREF), "href");
    s_idTable->addStaticMapping(localNamePart(ATTR_ID), "id");
    s_idTable->addStaticMapping(localNamePart(ATTR_LONGDESC), "longdesc");
    s_idTable->addStaticMapping(localNamePart(ATTR_NAME), "name");
    s_idTable->addStaticMapping(localNamePart(ATTR_ONBLUR), "onblur");
    s_idTable->addStaticMapping(localNamePart(ATTR_ONCHANGE), "onchange");
    s_idTable->addStaticMapping(localNamePart(ATTR_ONCLICK), "onclick");
    s_idTable->addStaticMapping(localNamePart(ATTR_ONDBLCLICK), "ondblclick");
    s_idTable->addStaticMapping(localNamePart(ATTR_ONFOCUS), "onfocus");
    s_idTable->addStaticMapping(localNamePart(ATTR_ONKEYDOWN), "onkeydown");
    s_idTable->addStaticMapping(localNamePart(ATTR_ONKEYPRESS), "onkeypress");
    s_idTable->addStaticMapping(localNamePart(ATTR_ONKEYUP), "onkeyup");
    s_idTable->addStaticMapping(localNamePart(ATTR_ONLOAD), "onload");
    s_idTable->addStaticMapping(localNamePart(ATTR_ONMOUSEDOWN), "onmousedown");
    s_idTable->addStaticMapping(localNamePart(ATTR_ONMOUSEMOVE), "onmousemove");
    s_idTable->addStaticMapping(localNamePart(ATTR_ONMOUSEOUT), "onmouseout");
    s_idTable->addStaticMapping(localNamePart(ATTR_ONMOUSEOVER), "onmouseover");
    s_idTable->addStaticMapping(localNamePart(ATTR_ONMOUSEUP), "onmouseup");
    s_idTable->addStaticMapping(localNamePart(ATTR_ONRESET), "onreset");
    s_idTable->addStaticMapping(localNamePart(ATTR_ONSELECT), "onselect");
    s_idTable->addStaticMapping(localNamePart(ATTR_ONSCROLL), "onscroll");
    s_idTable->addStaticMapping(localNamePart(ATTR_ONSUBMIT), "onsubmit");
    s_idTable->addStaticMapping(localNamePart(ATTR_ONUNLOAD), "onunload");
    s_idTable->addStaticMapping(localNamePart(ATTR_PROFILE), "profile");
    s_idTable->addStaticMapping(localNamePart(ATTR_PROMPT), "prompt");
    s_idTable->addStaticMapping(localNamePart(ATTR_SCHEME), "scheme");
    s_idTable->addStaticMapping(localNamePart(ATTR_SRC), "src");
    s_idTable->addStaticMapping(localNamePart(ATTR_STANDBY), "standby");
    s_idTable->addStaticMapping(localNamePart(ATTR_SUMMARY), "summary");
    s_idTable->addStaticMapping(localNamePart(ATTR_USEMAP), "usemap");
    s_idTable->addStaticMapping(localNamePart(ATTR_VALUE), "value");
    return s_idTable;
}

IDTable<PrefixFactory>* PrefixFactory::s_idTable;
IDTable<PrefixFactory>* PrefixFactory::initIdTable()
{
    s_idTable = new IDTable<PrefixFactory>();
    s_idTable->addStaticMapping(DOM::emptyPrefix, DOMString());
    return s_idTable;
}

}

namespace DOM {

LocalName emptyLocalName = LocalName::fromId(0);
PrefixName emptyPrefixName = PrefixName::fromId(0);
NamespaceName emptyNamespaceName = NamespaceName::fromId(0);

QString getPrintableName(int id) {
    QString local = QString("null");
    QString namespacename = QString("null");

    if (localNamePart(id) != anyLocalName) {
        DOMString localName = LocalName::fromId(localNamePart(id)).toString();
        if (localName.implementation())
            local = localName.string();
    } else {
        local = "*";
    }

    if (namespacePart(id) != anyNamespace) {
        DOMString namespaceName = NamespaceName::fromId(namespacePart(id)).toString();
        if (namespaceName.implementation())
            namespacename = namespaceName.string();
    } else {
        namespacename = "*";
    }
    return "{ns:" + QString::number(namespacePart(id)) + ",[" + namespacename + "] local:" + QString::number(localNamePart(id)) + ",[" + local + "]}";
}

}
