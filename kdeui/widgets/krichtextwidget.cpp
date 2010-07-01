/* This file is part of the KDE libraries

   Copyright 2008 Stephen Kelly <steveire@gmail.com>
   Copyright 2008 Thomas McGuire <thomas.mcguire@gmx.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "krichtextwidget.h"

// KDE includes
#include <kactioncollection.h>
#include <kcolordialog.h>
#include <kcolorscheme.h>
#include <kfontaction.h>
#include <kfontsizeaction.h>
#include <klocale.h>
#include <ktoggleaction.h>
#include <kdebug.h>

// Qt includes
#include <QtGui/QTextList>

#include "klinkdialog.h"

// TODO: Add i18n context

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KRichTextWidget::Private
{
public:
    Private(KRichTextWidget *parent)
            :   q(parent),
            painterActive(false),
            richTextEnabled(false), // It's only enabled when an action makes text rich.
            enableRichText(0),
            action_text_foreground_color(0),
            action_text_background_color(0),
            action_text_bold(0),
            action_text_italic(0),
            action_text_underline(0),
            action_text_strikeout(0),
            action_font_family(0),
            action_font_size(0),
            action_list_style(0),
            action_list_indent(0),
            action_list_dedent(0),
            action_manage_link(0),
            action_insert_horizontal_rule(0),
            action_format_painter(0),
            action_to_plain_text(0),
            action_align_left(0),
            action_align_right(0),
            action_align_center(0),
            action_align_justify(0),
            action_direction_ltr(0),
            action_direction_rtl(0),
            action_text_superscript(0),
            action_text_subscript(0)
    {
    }

    KRichTextWidget *q;

    RichTextSupport richTextSupport;

    QTextCharFormat painterFormat;
    bool painterActive;

    QList<KAction*> richTextActionList;

    bool richTextEnabled;
    KToggleAction *enableRichText;

    KAction *action_text_foreground_color;
    KAction *action_text_background_color;

    KToggleAction *action_text_bold;
    KToggleAction *action_text_italic;
    KToggleAction *action_text_underline;
    KToggleAction *action_text_strikeout;

    KFontAction *action_font_family;
    KFontSizeAction *action_font_size;

    KSelectAction *action_list_style;
    KAction *action_list_indent;
    KAction *action_list_dedent;

    KAction *action_manage_link;
    KAction *action_insert_horizontal_rule;
    KAction *action_format_painter;
    KAction *action_to_plain_text;

    KToggleAction *action_align_left;
    KToggleAction *action_align_right;
    KToggleAction *action_align_center;
    KToggleAction *action_align_justify;

    KToggleAction *action_direction_ltr;
    KToggleAction *action_direction_rtl;

    KToggleAction *action_text_superscript;
    KToggleAction *action_text_subscript;

    //
    // Normal functions
    //
    void init();

    //
    // Slots
    //

    /**
     * @brief Opens a dialog to allow the user to select a foreground color.
     */
    void _k_setTextForegroundColor();

    /**
     * @brief Opens a dialog to allow the user to select a background color.
     */
    void _k_setTextBackgroundColor();

    /**
     * Opens a dialog which lets the user turn the currently selected text into
     * a link.
     * If no text is selected, the word under the cursor will be taken.
     * If the cursor is already over a link, the user can edit that link.
     *
     */
    void _k_manageLink();

    /**
     * Activates a format painter to allow the user to copy font/text formatting
     * to different parts of the document.
     *
     */
    void _k_formatPainter(bool active);

    /**
     * @brief Update actions relating to text format (bold, size etc.).
     */
    void _k_updateCharFormatActions(const QTextCharFormat &format);

    /**
     * Update actions not covered by text formatting, such as alignment,
     * list style and level.
     */
    void _k_updateMiscActions();

    /**
     * Change the style of the current list or create a new list with the style given by @a index.
     */
    void _k_setListStyle(int index);

};
//@endcond

void KRichTextWidget::Private::init()
{
    q->setRichTextSupport(KRichTextWidget::FullSupport);
}

KRichTextWidget::KRichTextWidget(QWidget* parent)
        : KRichTextEdit(parent),
        d(new Private(this))
{
    d->init();
}

KRichTextWidget::KRichTextWidget(const QString& text, QWidget *parent)
        : KRichTextEdit(text,parent),
        d(new Private(this))
{
    d->init();
}

KRichTextWidget::~KRichTextWidget()
{
    delete d;
}

KRichTextWidget::RichTextSupport KRichTextWidget::richTextSupport() const
{
    return d->richTextSupport;
}

void KRichTextWidget::setRichTextSupport(const KRichTextWidget::RichTextSupport &support)
{
    d->richTextSupport = support;
}

void KRichTextWidget::createActions(KActionCollection *actionCollection)
{
    Q_ASSERT(actionCollection);

    // Note to maintainers: If adding new functionality here, make sure to disconnect
    // and delete actions which should not be supported.
    //
    // New Actions need to be added to the following places:
    // - possibly the RichTextSupportValues enum
    // - the API documentation for createActions()
    // - this function
    // - the action needs to be added to the private class as a member
    // - the constructor of the private class
    // - depending on the action, some slot that changes the toggle state when
    //   appropriate, such as _k_updateCharFormatActions or _k_updateMiscActions.

    // The list of actions currently supported is also stored internally.
    // This is used to disable all actions at once in setActionsEnabled.
    d->richTextActionList.clear();

    if (d->richTextSupport & SupportTextForegroundColor) {
        //Foreground Color
        d->action_text_foreground_color = new KAction(KIcon("format-stroke-color"), i18nc("@action", "Text &Color..."), actionCollection);
        d->action_text_foreground_color->setIconText(i18nc("@label stroke color", "Color"));
        d->richTextActionList.append((d->action_text_foreground_color));
        actionCollection->addAction("format_text_foreground_color", d->action_text_foreground_color);
        connect(d->action_text_foreground_color, SIGNAL(triggered()), this, SLOT(_k_setTextForegroundColor()));
    } else {
        actionCollection->removeAction(d->action_text_foreground_color);
        d->action_text_foreground_color = 0;
    }

    if (d->richTextSupport & SupportTextBackgroundColor) {
        //Background Color
        d->action_text_background_color = new KAction(KIcon("format-fill-color"), i18nc("@action", "Text &Highlight..."), actionCollection);
        d->richTextActionList.append((d->action_text_background_color));
        actionCollection->addAction("format_text_background_color", d->action_text_background_color);
        connect(d->action_text_background_color, SIGNAL(triggered()), this, SLOT(_k_setTextBackgroundColor()));
    } else {
        actionCollection->removeAction(d->action_text_background_color);
        d->action_text_background_color = 0;
    }

    if (d->richTextSupport & SupportFontFamily) {
        //Font Family
        d->action_font_family = new KFontAction(i18nc("@action", "&Font"), actionCollection);
        d->richTextActionList.append((d->action_font_family));
        actionCollection->addAction("format_font_family", d->action_font_family);
        connect(d->action_font_family, SIGNAL(triggered(QString)), this, SLOT(setFontFamily(QString)));
    } else {
        actionCollection->removeAction(d->action_font_family);
        d->action_font_family = 0;
    }

    if (d->richTextSupport & SupportFontSize) {
        //Font Size
        d->action_font_size = new KFontSizeAction(i18nc("@action", "Font &Size"), actionCollection);
        d->richTextActionList.append((d->action_font_size));
        actionCollection->addAction("format_font_size", d->action_font_size);
        connect(d->action_font_size, SIGNAL(fontSizeChanged(int)), this, SLOT(setFontSize(int)));
    } else {
        actionCollection->removeAction(d->action_font_size);
        d->action_font_size = 0;
    }

    if (d->richTextSupport & SupportBold) {
        d->action_text_bold = new KToggleAction(KIcon("format-text-bold"), i18nc("@action boldify selected text", "&Bold"), actionCollection);
        QFont bold;
        bold.setBold(true);
        d->action_text_bold->setFont(bold);
        d->richTextActionList.append((d->action_text_bold));
        actionCollection->addAction("format_text_bold", d->action_text_bold);
        d->action_text_bold->setShortcut(KShortcut(Qt::CTRL + Qt::Key_B));
        connect(d->action_text_bold, SIGNAL(triggered(bool)), this, SLOT(setTextBold(bool)));
    } else {
        actionCollection->removeAction(d->action_text_bold);
        d->action_text_bold = 0;
    }

    if (d->richTextSupport & SupportItalic) {
        d->action_text_italic = new KToggleAction(KIcon("format-text-italic"), i18nc("@action italicize selected text", "&Italic"), actionCollection);
        QFont italic;
        italic.setItalic(true);
        d->action_text_italic->setFont(italic);
        d->richTextActionList.append((d->action_text_italic));
        actionCollection->addAction("format_text_italic", d->action_text_italic);
        d->action_text_italic->setShortcut(KShortcut(Qt::CTRL + Qt::Key_I));
        connect(d->action_text_italic, SIGNAL(triggered(bool)),
                this, SLOT(setTextItalic(bool)));
    } else {
        actionCollection->removeAction(d->action_text_italic);
        d->action_text_italic = 0;
    }

    if (d->richTextSupport & SupportUnderline) {
        d->action_text_underline = new KToggleAction(KIcon("format-text-underline"), i18nc("@action underline selected text", "&Underline"), actionCollection);
        QFont underline;
        underline.setUnderline(true);
        d->action_text_underline->setFont(underline);
        d->richTextActionList.append((d->action_text_underline));
        actionCollection->addAction("format_text_underline", d->action_text_underline);
        d->action_text_underline->setShortcut(KShortcut(Qt::CTRL + Qt::Key_U));
        connect(d->action_text_underline, SIGNAL(triggered(bool)),
                this, SLOT(setTextUnderline(bool)));
    } else {
        actionCollection->removeAction(d->action_text_underline);
        d->action_text_underline = 0;
    }

    if (d->richTextSupport & SupportStrikeOut) {
        d->action_text_strikeout = new KToggleAction(KIcon("format-text-strikethrough"), i18nc("@action", "&Strike Out"), actionCollection);
        d->richTextActionList.append((d->action_text_strikeout));
        actionCollection->addAction("format_text_strikeout", d->action_text_strikeout);
        d->action_text_strikeout->setShortcut(KShortcut(Qt::CTRL + Qt::Key_L));
        connect(d->action_text_strikeout, SIGNAL(triggered(bool)),
                this, SLOT(setTextStrikeOut(bool)));
    } else {
        actionCollection->removeAction(d->action_text_strikeout);
        d->action_text_strikeout = 0;
    }

    if (d->richTextSupport & SupportAlignment) {
        //Alignment
        d->action_align_left = new KToggleAction(KIcon("format-justify-left"), i18nc("@action", "Align &Left"), actionCollection);
        d->action_align_left->setIconText(i18nc("@label left justify", "Left"));
        d->richTextActionList.append((d->action_align_left));
        actionCollection->addAction("format_align_left", d->action_align_left);
        connect(d->action_align_left, SIGNAL(triggered()),
                this, SLOT(alignLeft()));

        d->action_align_center = new KToggleAction(KIcon("format-justify-center"), i18nc("@action", "Align &Center"), actionCollection);
        d->action_align_center->setIconText(i18nc("@label center justify", "Center"));
        d->richTextActionList.append((d->action_align_center));
        actionCollection->addAction("format_align_center", d->action_align_center);
        connect(d->action_align_center, SIGNAL(triggered()),
                this, SLOT(alignCenter()));

        d->action_align_right = new KToggleAction(KIcon("format-justify-right"), i18nc("@action", "Align &Right"), actionCollection);
        d->action_align_right->setIconText(i18nc("@label right justify", "Right"));
        d->richTextActionList.append((d->action_align_right));
        actionCollection->addAction("format_align_right", d->action_align_right);
        connect(d->action_align_right, SIGNAL(triggered()),
                this, SLOT(alignRight()));

        d->action_align_justify = new KToggleAction(KIcon("format-justify-fill"), i18nc("@action", "&Justify"), actionCollection);
        d->action_align_justify->setIconText(i18nc("@label justify fill", "Justify"));
        d->richTextActionList.append((d->action_align_justify));
        actionCollection->addAction("format_align_justify", d->action_align_justify);
        connect(d->action_align_justify, SIGNAL(triggered()),
                this, SLOT(alignJustify()));

        QActionGroup *alignmentGroup = new QActionGroup(this);
        alignmentGroup->addAction(d->action_align_left);
        alignmentGroup->addAction(d->action_align_center);
        alignmentGroup->addAction(d->action_align_right);
        alignmentGroup->addAction(d->action_align_justify);
    } else {

        actionCollection->removeAction(d->action_align_left);
        actionCollection->removeAction(d->action_align_center);
        actionCollection->removeAction(d->action_align_right);
        actionCollection->removeAction(d->action_align_justify);

        d->action_align_left = 0;
        d->action_align_center = 0;
        d->action_align_right = 0;
        d->action_align_justify = 0;
    }

    if (d->richTextSupport & SupportDirection) {
        d->action_direction_ltr = new KToggleAction(KIcon("format-text-direction-ltr"), i18nc("@action", "Left-to-Right"), actionCollection);
        d->action_direction_ltr->setIconText(i18nc("@label left-to-right", "Left-to-Right"));
        d->richTextActionList.append(d->action_direction_ltr);
        actionCollection->addAction("direction_ltr", d->action_direction_ltr);
        connect(d->action_direction_ltr, SIGNAL(triggered()),
                this, SLOT(makeLeftToRight()));

        d->action_direction_rtl = new KToggleAction(KIcon("format-text-direction-rtl"), i18nc("@action", "Right-to-Left"), actionCollection);
        d->action_direction_rtl->setIconText(i18nc("@label right-to-left", "Right-to-Left"));
        d->richTextActionList.append(d->action_direction_rtl);
        actionCollection->addAction("direction_rtl", d->action_direction_rtl);
        connect(d->action_direction_rtl, SIGNAL(triggered()),
                this, SLOT(makeRightToLeft()));

        QActionGroup *directionGroup = new QActionGroup(this);
        directionGroup->addAction(d->action_direction_ltr);
        directionGroup->addAction(d->action_direction_rtl);
    } else {
        actionCollection->removeAction(d->action_direction_ltr);
        actionCollection->removeAction(d->action_direction_rtl);

        d->action_direction_ltr = 0;
        d->action_direction_rtl = 0;
    }

    if (d->richTextSupport & SupportChangeListStyle) {
        d->action_list_style = new KSelectAction(KIcon("format-list-unordered"), i18nc("@title:menu", "List Style"), actionCollection);
        QStringList listStyles;
        listStyles      << i18nc("@item:inmenu no list style", "None")
        << i18nc("@item:inmenu disc list style", "Disc")
        << i18nc("@item:inmenu circle list style", "Circle")
        << i18nc("@item:inmenu square list style", "Square")
        << i18nc("@item:inmenu numbered lists", "123")
        << i18nc("@item:inmenu lowercase abc lists", "abc")
        << i18nc("@item:inmenu uppercase abc lists", "ABC");
        d->action_list_style->setItems(listStyles);
        d->action_list_style->setCurrentItem(0);
        d->richTextActionList.append((d->action_list_style));
        actionCollection->addAction("format_list_style", d->action_list_style);
        connect(d->action_list_style, SIGNAL(triggered(int)),
                this, SLOT(_k_setListStyle(int)));
        connect(d->action_list_style, SIGNAL(triggered()),
                this, SLOT(_k_updateMiscActions()));

    } else {
        actionCollection->removeAction(d->action_list_style);
        d->action_list_style = 0;
    }

    if (d->richTextSupport & SupportIndentLists) {
        d->action_list_indent = new KAction(KIcon("format-indent-more"), i18nc("@action", "Increase Indent"), actionCollection);
        d->richTextActionList.append((d->action_list_indent));
        actionCollection->addAction("format_list_indent_more", d->action_list_indent);
        connect(d->action_list_indent, SIGNAL(triggered()),
                this, SLOT(indentListMore()));
        connect(d->action_list_indent, SIGNAL(triggered()),
                this, SLOT(_k_updateMiscActions()));
    } else {
        actionCollection->removeAction(d->action_list_indent);
        d->action_list_indent = 0;
    }

    if (d->richTextSupport & SupportDedentLists) {
        d->action_list_dedent = new KAction(KIcon("format-indent-less"), i18nc("@action", "Decrease Indent"), actionCollection);
        d->richTextActionList.append((d->action_list_dedent));
        actionCollection->addAction("format_list_indent_less", d->action_list_dedent);
        connect(d->action_list_dedent, SIGNAL(triggered()),
                this, SLOT(indentListLess()));
        connect(d->action_list_dedent, SIGNAL(triggered()),
                this, SLOT(_k_updateMiscActions()));
    } else {
        actionCollection->removeAction(d->action_list_dedent);
        d->action_list_dedent = 0;
    }

    if (d->richTextSupport & SupportRuleLine) {
        d->action_insert_horizontal_rule = new KAction(KIcon("insert-horizontal-rule"), i18nc("@action", "Insert Rule Line"), actionCollection);
        d->richTextActionList.append((d->action_insert_horizontal_rule));
        actionCollection->addAction("insert_horizontal_rule", d->action_insert_horizontal_rule);
        connect(d->action_insert_horizontal_rule, SIGNAL(triggered()),
                this, SLOT(insertHorizontalRule()));
    } else {
        actionCollection->removeAction(d->action_insert_horizontal_rule);
        d->action_insert_horizontal_rule = 0;
    }

    if (d->richTextSupport & SupportHyperlinks) {
        d->action_manage_link = new KAction(KIcon("insert-link"), i18nc("@action", "Link"), actionCollection);
        d->richTextActionList.append((d->action_manage_link));
        actionCollection->addAction("manage_link", d->action_manage_link);
        connect(d->action_manage_link, SIGNAL(triggered()),
                this, SLOT(_k_manageLink()));
    } else {
        actionCollection->removeAction(d->action_manage_link);
        d->action_manage_link = 0;
    }

    if (d->richTextSupport & SupportFormatPainting) {
        d->action_format_painter = new KToggleAction(KIcon("draw-brush"), i18nc("@action", "Format Painter"), actionCollection);
        d->richTextActionList.append((d->action_format_painter));
        actionCollection->addAction("format_painter", d->action_format_painter);
        connect(d->action_format_painter, SIGNAL(toggled(bool)),
                this, SLOT(_k_formatPainter(bool)));
    } else {
        actionCollection->removeAction(d->action_format_painter);
        d->action_format_painter = 0;
    }

    if (d->richTextSupport & SupportToPlainText) {
        d->action_to_plain_text = new KToggleAction(i18nc("@action", "To Plain Text"), actionCollection);
        d->richTextActionList.append((d->action_to_plain_text));
        actionCollection->addAction("action_to_plain_text", d->action_to_plain_text);
        connect(d->action_to_plain_text, SIGNAL(triggered()),
                this, SLOT(switchToPlainText()));
    } else {
        actionCollection->removeAction(d->action_to_plain_text);
        d->action_to_plain_text = 0;
    }

    if (d->richTextSupport & SupportSuperScriptAndSubScript) {
        d->action_text_subscript = new KToggleAction(KIcon("format-text-subscript"), i18nc("@action", "Subscript"), actionCollection);
        d->richTextActionList.append((d->action_text_subscript));
        actionCollection->addAction("format_text_subscript", d->action_text_subscript);

        connect(d->action_text_subscript, SIGNAL(triggered(bool)),
                this, SLOT(setTextSubScript(bool)));

        d->action_text_superscript = new KToggleAction(KIcon("format-text-superscript"), i18nc("@action", "Superscript"), actionCollection);
        d->richTextActionList.append((d->action_text_superscript));
        actionCollection->addAction("format_text_superscript", d->action_text_superscript);

        connect(d->action_text_superscript, SIGNAL(triggered(bool)),
                this, SLOT(setTextSuperScript(bool)));
    } else {
        actionCollection->removeAction(d->action_text_subscript);
        d->action_text_subscript = 0;

        actionCollection->removeAction(d->action_text_superscript);
        d->action_text_superscript = 0;
    }
    

    disconnect(this, SIGNAL(currentCharFormatChanged(const QTextCharFormat &)),
               this, SLOT(_k_updateCharFormatActions(const QTextCharFormat &)));
    disconnect(this, SIGNAL(cursorPositionChanged()),
               this, SLOT(_k_updateMiscActions()));
    connect(this, SIGNAL(currentCharFormatChanged(const QTextCharFormat &)),
            this, SLOT(_k_updateCharFormatActions(const QTextCharFormat &)));
    connect(this, SIGNAL(cursorPositionChanged()),
            this, SLOT(_k_updateMiscActions()));

    d->_k_updateMiscActions();
    d->_k_updateCharFormatActions(currentCharFormat());
}


void KRichTextWidget::setActionsEnabled(bool enabled)
{
    foreach(QAction* action, d->richTextActionList)
    {
        action->setEnabled(enabled);
    }
    d->richTextEnabled = enabled;
}

void KRichTextWidget::Private::_k_setListStyle(int index)
{
    q->setListStyle(index);
    _k_updateMiscActions();
}

void KRichTextWidget::Private::_k_updateCharFormatActions(const QTextCharFormat &format)
{
    QFont f = format.font();

    if (richTextSupport & SupportFontFamily) {
        action_font_family->setFont(f.family());
    }
    if (richTextSupport & SupportFontSize) {
        if (f.pointSize() > 0)
            action_font_size->setFontSize((int)f.pointSize());
    }

    if (richTextSupport & SupportBold) {
        action_text_bold->setChecked(f.bold());
    }

    if (richTextSupport & SupportItalic) {
        action_text_italic->setChecked(f.italic());
    }

    if (richTextSupport & SupportUnderline) {
        action_text_underline->setChecked(f.underline());
    }

    if (richTextSupport & SupportStrikeOut) {
        action_text_strikeout->setChecked(f.strikeOut());
    }

    if (richTextSupport & SupportSuperScriptAndSubScript) {
        QTextCharFormat::VerticalAlignment vAlign = format.verticalAlignment();
        action_text_superscript->setChecked(vAlign == QTextCharFormat::AlignSuperScript);
        action_text_subscript->setChecked(vAlign == QTextCharFormat::AlignSubScript);
    }
}

void KRichTextWidget::Private::_k_updateMiscActions()
{
    if (richTextSupport & SupportAlignment) {
        Qt::Alignment a = q->alignment();
        if (a & Qt::AlignLeft) {
            action_align_left->setChecked(true);
        } else if (a & Qt::AlignHCenter) {
            action_align_center->setChecked(true);
        } else if (a & Qt::AlignRight) {
            action_align_right->setChecked(true);
        } else if (a & Qt::AlignJustify) {
            action_align_justify->setChecked(true);
        }
    }


    if (richTextSupport & SupportChangeListStyle) {
        if (q->textCursor().currentList()) {
            action_list_style->setCurrentItem(-q->textCursor().currentList()->format().style());
        } else {
            action_list_style->setCurrentItem(0);
        }
    }


    if ( richTextSupport & SupportIndentLists ) {
        if ( richTextEnabled ) {
            action_list_indent->setEnabled( q->canIndentList() );
        } else {
            action_list_indent->setEnabled( false );
        }
    }

    if ( richTextSupport & SupportDedentLists ) {
        if ( richTextEnabled ) {
            action_list_dedent->setEnabled( q->canDedentList() );   
        } else {
            action_list_dedent->setEnabled( false );
        }
    }

    if (richTextSupport & SupportDirection) {
        const Qt::LayoutDirection direction = q->textCursor().blockFormat().layoutDirection();
        action_direction_ltr->setChecked(direction == Qt::LeftToRight);
        action_direction_rtl->setChecked(direction == Qt::RightToLeft);
    }
}

void KRichTextWidget::Private::_k_setTextForegroundColor()
{
    QColor currentTextForegroundColor = q->textColor();

    int result = KColorDialog::getColor(currentTextForegroundColor, KColorScheme(QPalette::Active, KColorScheme::View).foreground().color() , q);
    if (!currentTextForegroundColor.isValid())
        currentTextForegroundColor = KColorScheme(QPalette::Active, KColorScheme::View).foreground().color() ;
    if (result != QDialog::Accepted)
        return;

    q->setTextForegroundColor(currentTextForegroundColor);

}

void KRichTextWidget::Private::_k_setTextBackgroundColor()
{
    QTextCharFormat fmt = q->textCursor().charFormat();
    QColor currentTextBackgroundColor = fmt.background().color();

    int result = KColorDialog::getColor(currentTextBackgroundColor, KColorScheme(QPalette::Active, KColorScheme::View).foreground().color() , q);
    if (!currentTextBackgroundColor.isValid())
        currentTextBackgroundColor = KColorScheme(QPalette::Active, KColorScheme::View).foreground().color() ;
    if (result != QDialog::Accepted)
        return;

    q->setTextBackgroundColor(currentTextBackgroundColor);

}

void KRichTextWidget::Private::_k_manageLink()
{
    q->selectLinkText();
    KLinkDialog *linkDialog = new KLinkDialog(q);
    linkDialog->setLinkText(q->currentLinkText());
    linkDialog->setLinkUrl(q->currentLinkUrl());

    if (linkDialog->exec()) {
        q->updateLink(linkDialog->linkUrl(), linkDialog->linkText());
    }

    delete linkDialog;

}

void KRichTextWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (d->painterActive) {
        // If the painter is active, paint the selection with the
        // correct format.
        if (textCursor().hasSelection()) {
            textCursor().setCharFormat(d->painterFormat);
        }
        d->painterActive = false;
        d->action_format_painter->setChecked(false);
    }
    KRichTextEdit::mouseReleaseEvent(event);
}

void KRichTextWidget::Private::_k_formatPainter(bool active)
{
    if (active) {
        painterFormat = q->currentCharFormat();
        painterActive = true;
        q->viewport()->setCursor(QCursor(KIcon("draw-brush").pixmap(32, 32), 0, 32));
    } else {
        painterFormat = QTextCharFormat();
        painterActive = false;
        q->viewport()->setCursor(Qt::IBeamCursor);
    }
}

void KRichTextWidget::updateActionStates()
{
    d->_k_updateMiscActions();
    d->_k_updateCharFormatActions(currentCharFormat());
}

// kate: space-indent on; indent-width 4; encoding utf-8; replace-tabs on;
#include "krichtextwidget.moc"
