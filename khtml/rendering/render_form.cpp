/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 *           (C) 2006 Maksim Orlovich (maksim@kde.org)
 *           (C) 2007 Germain Garand (germain@ebooksfrance.org)
 *           (C) 2007 Mitz Pettel (mitz@webkit.org)
 *           (C) 2007 Charles Samuels (charles@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "render_form.h"

#include <kcompletionbox.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kfind.h>
#include <kfinddialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kreplace.h>
#include <kreplacedialog.h>
#include <dialog.h>
#include <backgroundchecker.h>
#include <kurlcompletion.h>
#include <kwindowsystem.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <kdeuiwidgetsproxystyle_p.h>

#include <QAbstractItemView>
#include <QAbstractTextDocumentLayout>
#include <QtGui/QStyle>
#include <QStyleOptionButton>

#include <misc/helper.h>
#include <xml/dom2_eventsimpl.h>
#include <html/html_formimpl.h>
#include <misc/htmlhashes.h>

#include <assert.h>

#include <khtmlview.h>
#include <khtml_ext.h>
#include <xml/dom_docimpl.h>

#include <QtGui/QMenu>
#include <QtGui/QBitmap>

using namespace khtml;

RenderFormElement::RenderFormElement(HTMLGenericFormElementImpl *element)
    : RenderWidget(element)
//     , m_state(0)
    , proxyStyle(0)
{
    // init RenderObject attributes
    setInline(true);   // our object is Inline

}

RenderFormElement::~RenderFormElement()
{
}

void RenderFormElement::setStyle(RenderStyle *style)
{
    RenderWidget::setStyle(style);
    setPadding();
}

int RenderFormElement::paddingTop() const
{
    return !includesPadding() ? RenderWidget::paddingTop() : 0;
}
int RenderFormElement::paddingBottom() const
{
    return !includesPadding() ? RenderWidget::paddingBottom() : 0;
}
int RenderFormElement::paddingLeft() const
{
    return !includesPadding() ? RenderWidget::paddingLeft() : 0;
}
int RenderFormElement::paddingRight() const
{
    return !includesPadding() ? RenderWidget::paddingRight() : 0;
}

bool RenderFormElement::includesPadding() const
{
    return style()->boxSizing() == BORDER_BOX;
}


void RenderFormElement::setPadding()
{
    if (!includesPadding())
        return;

    struct AddPadding : public KdeUiProxyStyle
    {
        AddPadding(QWidget *parent)
            : KdeUiProxyStyle(parent)
        { }
        QSize sizeFromContents(
                ContentsType type, const QStyleOption * option, const QSize &contentsSize,
                const QWidget * widget
            ) const
        {
            QSize s = KdeUiProxyStyle::sizeFromContents(type, option, contentsSize, widget);
            return s + QSize(left+right, top+bottom);
        }
        QRect subElementRect(
                SubElement element, const QStyleOption *option, const QWidget *widget
            ) const
        {
            QRect r = KdeUiProxyStyle::subElementRect(element, option, widget);
            r.adjust(left, top, -right, -bottom);
            return r;
        }

        int left, right, top, bottom;
    };


    AddPadding *style = new AddPadding(widget());
    style->left = RenderWidget::paddingLeft();
    style->right = RenderWidget::paddingRight();
    style->top = RenderWidget::paddingTop();
    style->bottom = RenderWidget::paddingBottom();

    widget()->setStyle(style);
    delete proxyStyle;
    proxyStyle = style;
}

short RenderFormElement::baselinePosition( bool f ) const
{
    return RenderWidget::baselinePosition( f ) - 2 - style()->fontMetrics().descent();
}

void RenderFormElement::setQWidget( QWidget *w )
{
    // sets the Qt Object Name for the purposes
    // of setPadding() -- this is because QStyleSheet
    // will propagate children of 'w' even if they are toplevel, like
    // the "find" dialog or the popup menu
    w->setObjectName("RenderFormElementWidget");
    RenderWidget::setQWidget(w);
}

void RenderFormElement::updateFromElement()
{
    m_widget->setEnabled(!element()->disabled());

    // If we've disabled/made r/o a focused element, clear its focus,
    // so Qt doesn't do funny stuff like let one type into a disabled
    // line edit.
    if ((element()->disabled() || element()->readOnly()) && element()->focused())
        document()->quietResetFocus();

    RenderWidget::updateFromElement();
    setPadding();
}

void RenderFormElement::layout()
{
    KHTMLAssert( needsLayout() );
    KHTMLAssert( minMaxKnown() );

    // minimum height
    m_height = 0;

    calcWidth();
    calcHeight();

    if ( m_widget )
        resizeWidget(m_width-borderLeft()-borderRight()-paddingLeft()-paddingRight(),
                     m_height-borderTop()-borderBottom()-paddingTop()-paddingBottom());

    setNeedsLayout(false);
}


Qt::AlignmentFlag RenderFormElement::textAlignment() const
{
    switch (style()->textAlign()) {
        case LEFT:
        case KHTML_LEFT:
            return Qt::AlignLeft;
        case RIGHT:
        case KHTML_RIGHT:
            return Qt::AlignRight;
        case CENTER:
        case KHTML_CENTER:
            return Qt::AlignHCenter;
        case JUSTIFY:
            // Just fall into the auto code for justify.
        case TAAUTO:
            return style()->direction() == RTL ? Qt::AlignRight : Qt::AlignLeft;
    }
    assert(false); // Should never be reached.
    return Qt::AlignLeft;
}

// -------------------------------------------------------------------------

RenderButton::RenderButton(HTMLGenericFormElementImpl *element)
    : RenderFormElement(element)
{
}

short RenderButton::baselinePosition( bool f ) const
{
    return RenderWidget::baselinePosition( f ) - 2;
}

// -------------------------------------------------------------------------------


RenderCheckBox::RenderCheckBox(HTMLInputElementImpl *element)
    : RenderButton(element)
{
    CheckBoxWidget* b = new CheckBoxWidget(view()->widget());
    //b->setAutoMask(true);
    b->setMouseTracking(true);
    setQWidget(b);

    // prevent firing toggled() signals on initialization
    b->setChecked(element->checked());

    connect(b,SIGNAL(stateChanged(int)),this,SLOT(slotStateChanged(int)));
    m_ignoreStateChanged = false;
}


void RenderCheckBox::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    QCheckBox *cb = static_cast<QCheckBox *>( m_widget );
    QSize s( qMin(22, qMax(14, cb->style()->pixelMetric( QStyle::PM_IndicatorWidth ))),
             qMin(20, qMax(12, cb->style()->pixelMetric( QStyle::PM_IndicatorHeight ))) );
    setIntrinsicWidth( s.width() );
    setIntrinsicHeight( s.height() );

    RenderButton::calcMinMaxWidth();
}

void RenderCheckBox::updateFromElement()
{
    if (widget()->isChecked() != element()->checked()) {
        m_ignoreStateChanged = true; // We don't want an onchange here,
                                     // or us getting yanked in a recalcStyle in the process, etc.
        widget()->setChecked(element()->checked());
        m_ignoreStateChanged = false;
    }

    RenderButton::updateFromElement();
}

void RenderCheckBox::slotStateChanged(int state)
{
    if (m_ignoreStateChanged) return;
    element()->setChecked(state == Qt::Checked);

    ref();
    element()->onChange();
    deref();
}

bool RenderCheckBox::handleEvent(const DOM::EventImpl& ev)
{
    switch(ev.id()) {
      case EventImpl::DOMFOCUSIN_EVENT:
      case EventImpl::DOMFOCUSOUT_EVENT:
      case EventImpl::MOUSEMOVE_EVENT:
      case EventImpl::MOUSEOUT_EVENT:
      case EventImpl::MOUSEOVER_EVENT:
          return RenderButton::handleEvent(ev);
      default:
          break;
    }
    return false;
}

// -------------------------------------------------------------------------------

RenderRadioButton::RenderRadioButton(HTMLInputElementImpl *element)
    : RenderButton(element)
{
    RadioButtonWidget* b = new RadioButtonWidget(view()->widget());
    b->setMouseTracking(true);
    b->setAutoExclusive(false);
    setQWidget(b);

    // prevent firing toggled() signals on initialization
    b->setChecked(element->checked());

    connect(b,SIGNAL(toggled(bool)),this,SLOT(slotToggled(bool)));
    m_ignoreToggled = false;
}

void RenderRadioButton::updateFromElement()
{
    m_ignoreToggled = true;
    widget()->setChecked(element()->checked());
    m_ignoreToggled = false;

    RenderButton::updateFromElement();
}

void RenderRadioButton::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    QRadioButton *rb = static_cast<QRadioButton *>( m_widget );
    QSize s( qMin(22, qMax(14, rb->style()->pixelMetric( QStyle::PM_ExclusiveIndicatorWidth ))),
             qMin(20, qMax(12, rb->style()->pixelMetric( QStyle::PM_ExclusiveIndicatorHeight ))) );
    setIntrinsicWidth( s.width() );
    setIntrinsicHeight( s.height() );

    RenderButton::calcMinMaxWidth();
}

void RenderRadioButton::slotToggled(bool activated)
{
    if (m_ignoreToggled)
      return;

    if(activated) {
      ref();
      element()->onChange();
      deref();
    }
}

bool RenderRadioButton::handleEvent(const DOM::EventImpl& ev)
{
    switch(ev.id()) {
      case EventImpl::DOMFOCUSIN_EVENT:
      case EventImpl::DOMFOCUSOUT_EVENT:
      case EventImpl::MOUSEMOVE_EVENT:
      case EventImpl::MOUSEOUT_EVENT:
      case EventImpl::MOUSEOVER_EVENT:
          return RenderButton::handleEvent(ev);
      default:
          break;
    }
    return false;
}

// -------------------------------------------------------------------------------


RenderSubmitButton::RenderSubmitButton(HTMLInputElementImpl *element)
    : RenderButton(element)
{
    PushButtonWidget* p = new PushButtonWidget(view()->widget());
    setQWidget(p);
    //p->setAutoMask(true);
    p->setMouseTracking(true);
}

QString RenderSubmitButton::rawText()
{
    QString value = element()->valueWithDefault().string();
    value = value.trimmed();
    QString raw;
    for(int i = 0; i < value.length(); i++) {
        raw += value[i];
        if(value[i] == '&')
            raw += '&';
    }
    return raw;
}

void RenderSubmitButton::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    QString raw = rawText();
    QPushButton* pb = static_cast<QPushButton*>(m_widget);
    pb->setText(raw);
    pb->setFont(style()->font());

    bool empty = raw.isEmpty();
    if ( empty )
        raw = QLatin1Char('X');
    QFontMetrics fm = pb->fontMetrics();
    QSize ts = fm.size( Qt::TextShowMnemonic, raw);
    //Oh boy.
    QStyleOptionButton butOpt;
    butOpt.init(pb);
    butOpt.text = raw;
    QSize s = pb->style()->sizeFromContents( QStyle::CT_PushButton, &butOpt, ts, pb );

    s = s.expandedTo(QApplication::globalStrut());
    int margin = pb->style()->pixelMetric( QStyle::PM_ButtonMargin) +
              pb->style()->pixelMetric( QStyle::PM_DefaultFrameWidth ) * 2;
    int w = ts.width() + margin;

    int h = s.height();
    if (pb->isDefault() || pb->autoDefault()) {
        int dbw = pb->style()->pixelMetric( QStyle::PM_ButtonDefaultIndicator ) * 2;
        w += dbw;
    }
    // add 30% margins to the width (heuristics to make it look similar to IE)
    w = w*13/10;

    // the crazy heuristic code overrides some changes made by the
    // AddPadding proxy style, so reapply them
    w += RenderWidget::paddingLeft() + RenderWidget::paddingRight();

    s = QSize(w,h).expandedTo(QApplication::globalStrut());

    setIntrinsicWidth( s.width() );
    setIntrinsicHeight( s.height() );

    RenderButton::calcMinMaxWidth();
}

void RenderSubmitButton::updateFromElement()
{
    QString oldText = static_cast<QPushButton*>(m_widget)->text();
    QString newText = rawText();
    static_cast<QPushButton*>(m_widget)->setText(newText);
    if ( oldText != newText )
        setNeedsLayoutAndMinMaxRecalc();
    RenderFormElement::updateFromElement();
}

short RenderSubmitButton::baselinePosition( bool f ) const
{
    int ret = (height()-paddingTop()-paddingBottom()+1)/2;
    ret += marginTop() + paddingTop();
    ret += ((fontMetrics( f ).ascent())/2)-2;
    return ret;
}



// -------------------------------------------------------------------------------

RenderResetButton::RenderResetButton(HTMLInputElementImpl *element)
    : RenderSubmitButton(element)
{
}

// -------------------------------------------------------------------------------

class CompletionWidget: public KCompletionBox
{
public:
    CompletionWidget( QWidget *parent = 0 ) : KCompletionBox( parent ) {}
    virtual QPoint globalPositionHint() const
    {
        QWidget* pw = parentWidget();
        KHTMLWidget* kwp = dynamic_cast<KHTMLWidget*>(pw);
        if (!kwp) {
            qDebug() << "CompletionWidget has no KHTMLWidget parent" << endl;
            return KCompletionBox::globalPositionHint();
        }
        QPoint dest;
        KHTMLView* v = kwp->m_kwp->rootViewPos(dest);
        QPoint ret;
        if (v) {
            ret = v->mapToGlobal( dest + QPoint(0, pw->height()) );
            int zoomLevel = v->zoomLevel();
            if (zoomLevel != 100) {
                ret.setX( ret.x()*zoomLevel/100 );
                ret.setY( ret.y()*zoomLevel/100 );
            }
        }
        return ret;
    }
};

LineEditWidget::LineEditWidget(DOM::HTMLInputElementImpl* input, KHTMLView* view, QWidget* parent)
    : KLineEdit(parent), m_input(input), m_view(view)
{
    m_kwp->setIsRedirected( true );
    setMouseTracking(true);
    KActionCollection *ac = new KActionCollection(this);
    m_spellAction = KStandardAction::spelling( this, SLOT( slotCheckSpelling() ), ac );

    setCompletionBox( new CompletionWidget( this ) );
    completionBox()->setObjectName("completion box");
    completionBox()->setFont(font());
}

LineEditWidget::~LineEditWidget()
{
}

void LineEditWidget::slotCheckSpelling()
{
    if ( text().isEmpty() ) {
        return;
    }
    Sonnet::Dialog *spellDialog = new Sonnet::Dialog(new Sonnet::BackgroundChecker(this), 0);
    connect(spellDialog, SIGNAL(replace( const QString&, int,const QString&)), this, SLOT(spellCheckerCorrected( const QString&, int,const QString&)));
    connect(spellDialog, SIGNAL(misspelling( const QString&, int)), this, SLOT(spellCheckerMisspelling(const QString &,int)));
    connect(spellDialog, SIGNAL(done(const QString&)), this, SLOT(slotSpellCheckDone(const QString&)));
    connect(spellDialog, SIGNAL(cancel()), this, SLOT(spellCheckerFinished()));
    connect(spellDialog, SIGNAL(stop()), this, SLOT(spellCheckerFinished()));
    spellDialog->setBuffer(text());
    spellDialog->show();
}

void LineEditWidget::spellCheckerMisspelling( const QString &_text, int pos)
{
    highLightWord( _text.length(),pos );
}

void LineEditWidget::setFocus()
{
    KLineEdit::setFocus();
    end( false );
}

void LineEditWidget::highLightWord( unsigned int length, unsigned int pos )
{
    setSelection ( pos, length );
}

void LineEditWidget::spellCheckerCorrected( const QString &old, int pos, const QString &corr )
{
    if( old!= corr )
    {
        setSelection ( pos, old.length() );
        insert( corr );
        setSelection ( pos, corr.length() );
    }
}

void LineEditWidget::spellCheckerFinished()
{
}

void LineEditWidget::slotSpellCheckDone( const QString &s )
{
    if( s != text() )
        setText( s );
}

void LineEditWidget::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu* popup = createStandardContextMenu();

    if ( !popup )
        return;

    if (m_input->autoComplete()) {
        popup->addSeparator();
        QAction* act = popup->addAction( KIcon("edit-clear-history"), i18n("Clear &History"));
        act->setEnabled(compObj() && !compObj()->isEmpty());
        connect(act, SIGNAL(triggered()),
                this, SLOT(clearHistoryActivated()));
    }

    if (echoMode() == QLineEdit::Normal &&
        !isReadOnly()) {
        popup->addSeparator();

        popup->addAction( m_spellAction );
        m_spellAction->setEnabled( !text().isEmpty() );
    }
    popup->exec(e->globalPos());
    delete popup;
}


void LineEditWidget::clearHistoryActivated()
{
    m_view->clearCompletionHistory(m_input->name().string());
    if (compObj())
      compObj()->clear();
}

void LineEditWidget::paintEvent( QPaintEvent *pe )
{
    if (!hasFrame()) {
        QPainter p(this);
        p.fillRect(pe->rect(), palette().brush(QPalette::Base));
        p.end();
    }   
    KLineEdit::paintEvent( pe );
}

bool LineEditWidget::event( QEvent *e )
{
    if (KLineEdit::event(e))
	return true;
#if 0
    if ( e->type() == QEvent::AccelAvailable && isReadOnly() ) {
        QKeyEvent* ke = (QKeyEvent*) e;
        if ( ke->modifiers() & Qt::ControlModifier ) {
            switch ( ke->key() ) {
                case Qt::Key_Left:
                case Qt::Key_Right:
                case Qt::Key_Up:
                case Qt::Key_Down:
                case Qt::Key_Home:
                case Qt::Key_End:
                    ke->accept();
                default:
                break;
            }
        }
    }
#endif
    return false;
}

void LineEditWidget::mouseMoveEvent(QMouseEvent *e)
{
    // hack to prevent Qt from calling setCursor on the widget
    setDragEnabled(false);
    KLineEdit::mouseMoveEvent(e);
    setDragEnabled(true);
}


// -----------------------------------------------------------------------------

RenderLineEdit::RenderLineEdit(HTMLInputElementImpl *element)
    : RenderFormElement(element)
{
    LineEditWidget *edit = new LineEditWidget(element, view(), view()->widget());
    connect(edit,SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));
    connect(edit,SIGNAL(textChanged(const QString &)),this,SLOT(slotTextChanged(const QString &)));

    if(element->inputType() == HTMLInputElementImpl::PASSWORD)
        edit->setEchoMode( QLineEdit::Password );

    if ( element->autoComplete() ) {
        QStringList completions = view()->formCompletionItems(element->name().string());
        if (completions.count()) {
            edit->completionObject()->setItems(completions);
            edit->setContextMenuPolicy(Qt::NoContextMenu);
            edit->completionBox()->setTabHandling( false );
        }
    }

    setQWidget(edit);
}

short RenderLineEdit::baselinePosition( bool f ) const
{
    bool hasFrame = static_cast<LineEditWidget*>(widget())->hasFrame();
    int bTop = hasFrame ? 0 : borderTop();
    int bBottom = hasFrame ? 0 : borderBottom();
    int ret = (height()-paddingTop()-paddingBottom()-bTop-bBottom+1)/2;
    ret += marginTop() + paddingTop() + bTop;
    ret += ((fontMetrics( f ).ascent())/2)-2;
    return ret;
}

void RenderLineEdit::setStyle(RenderStyle* _style)
{
    RenderFormElement::setStyle( _style );

    widget()->setAlignment(textAlignment());
}

void RenderLineEdit::highLightWord( unsigned int length, unsigned int pos )
{
    LineEditWidget* w = static_cast<LineEditWidget*>(m_widget);
    if ( w )
        w->highLightWord( length, pos );
}


void RenderLineEdit::slotReturnPressed()
{
    // don't submit the form when return was pressed in a completion-popup
    KCompletionBox *box = widget()->completionBox(false);

    if ( box && box->isVisible() && box->currentRow() != -1 ) {
      box->hide();
      return;
    }

    // Emit onChange if necessary
    // Works but might not be enough, dirk said he had another solution at
    // hand (can't remember which) - David
    handleFocusOut();

    HTMLFormElementImpl* fe = element()->form();
    if ( fe )
        fe->submitFromKeyboard();
}

void RenderLineEdit::handleFocusOut()
{
    if ( widget() && widget()->isModified() ) {
        element()->onChange();
        widget()->setModified( false );
    }
}

void RenderLineEdit::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    const QFontMetrics &fm = style()->fontMetrics();
    QSize s;


    int size = element()->size();

    int h = fm.lineSpacing();
    int w = fm.width( 'x' ) * (size > 0 ? size+1 : 17); // "some"

    QStyleOptionFrame opt;
    opt.initFrom(widget());
    if (static_cast<LineEditWidget*>(widget())->hasFrame())
        opt.lineWidth = widget()->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &opt, widget());

    s = QSize(w, qMax(h, 14));
    s = widget()->style()->sizeFromContents(QStyle::CT_LineEdit, &opt, s, widget());
    s = s.expandedTo(QApplication::globalStrut());

    setIntrinsicWidth( s.width() );
    setIntrinsicHeight( s.height() );

    RenderFormElement::calcMinMaxWidth();
}

void RenderLineEdit::updateFromElement()
{
    int ml = element()->maxLength();
    if ( ml < 0 )
        ml = 32767;

    if ( widget()->maxLength() != ml )  {
        widget()->setMaxLength( ml );
    }

    if (element()->value().string() != widget()->text()) {
        bool blocked = widget()->blockSignals(true);
        int pos = widget()->cursorPosition();
        widget()->setText(element()->value().string());

        widget()->setModified( false );

        widget()->setCursorPosition(pos);
        widget()->blockSignals(blocked);
    }
    widget()->setReadOnly(element()->readOnly());

    RenderFormElement::updateFromElement();
}

void RenderLineEdit::slotTextChanged(const QString &string)
{
    // don't use setValue here!
    element()->m_value = string;
    element()->m_unsubmittedFormChange = true;
}

void RenderLineEdit::select()
{
    static_cast<LineEditWidget*>(m_widget)->selectAll();
}

long RenderLineEdit::selectionStart()
{
    LineEditWidget* w = static_cast<LineEditWidget*>(m_widget);
    if (w->hasSelectedText())
        return w->selectionStart();
    else
        return w->cursorPosition();
}


long RenderLineEdit::selectionEnd()
{
    LineEditWidget* w = static_cast<LineEditWidget*>(m_widget);
    if (w->hasSelectedText())
        return w->selectionStart() + w->selectedText().length();
    else
        return w->cursorPosition();
}

void RenderLineEdit::setSelectionStart(long pos)
{
    LineEditWidget* w = static_cast<LineEditWidget*>(m_widget);
    //See whether we have a non-empty selection now.
    long end = selectionEnd();
    if (end > pos)
        w->setSelection(pos, end - pos);
    w->setCursorPosition(pos);
}

void RenderLineEdit::setSelectionEnd(long pos)
{
    LineEditWidget* w = static_cast<LineEditWidget*>(m_widget);
    //See whether we have a non-empty selection now.
    long start = selectionStart();
    if (start < pos)
        w->setSelection(start, pos - start);

    w->setCursorPosition(pos);
}

void RenderLineEdit::setSelectionRange(long start, long end)
{
    LineEditWidget* w = static_cast<LineEditWidget*>(m_widget);
    w->setCursorPosition(end);
    w->setSelection(start, end - start);
}

// ---------------------------------------------------------------------------

RenderFieldset::RenderFieldset(HTMLGenericFormElementImpl *element)
    : RenderBlock(element)
{
    m_intrinsicWidth = 0;
}

void RenderFieldset::calcMinMaxWidth()
{
    RenderBlock::calcMinMaxWidth();
    if (style()->htmlHacks()){ if (RenderObject* legend = findLegend()) {
        int legendMinWidth = legend->minWidth();

        Length legendMarginLeft = legend->style()->marginLeft();
        Length legendMarginRight = legend->style()->marginLeft();

        if (legendMarginLeft.isFixed())
            legendMinWidth += legendMarginLeft.value();

        if (legendMarginRight.isFixed())
            legendMinWidth += legendMarginRight.value();

        m_intrinsicWidth = qMax((int)m_minWidth, legendMinWidth + paddingLeft() + paddingRight() + borderLeft() + borderRight());
    }}
}

RenderObject* RenderFieldset::layoutLegend(bool relayoutChildren)
{
    RenderObject* legend = findLegend();
    if (legend) {
        if (relayoutChildren)
            legend->setNeedsLayout(true);
        legend->layoutIfNeeded();

        int xPos = borderLeft() + paddingLeft() + legend->marginLeft();
        if (style()->direction() == RTL)
            xPos = m_width - paddingRight() - borderRight() - legend->width() - legend->marginRight();
        int b = borderTop();
        int h = legend->height();
        legend->setPos(xPos, qMax((b-h)/2, 0));
        m_height = qMax(b,h) + paddingTop();
    }
    return legend;
}

RenderObject* RenderFieldset::findLegend() const
{
    for (RenderObject* legend = firstChild(); legend; legend = legend->nextSibling()) {
      if (!legend->isFloatingOrPositioned() && legend->element() &&
          legend->element()->id() == ID_LEGEND)
        return legend;
    }
    return 0;
}

void RenderFieldset::paintBoxDecorations(PaintInfo& pI, int _tx, int _ty)
{
    //kDebug( 6040 ) << renderName() << "::paintDecorations()";

    RenderObject* legend = findLegend();
    if (!legend)
        return RenderBlock::paintBoxDecorations(pI, _tx, _ty);

    int w = width();
    int h = height() + borderTopExtra() + borderBottomExtra();
    int yOff = (legend->yPos() > 0) ? 0 : (legend->height()-borderTop())/2;
    int legendBottom = _ty + legend->yPos() + legend->height();
    h -= yOff;
    _ty += yOff - borderTopExtra();

    QRect cr = QRect(_tx, _ty, w, h).intersected( pI.r );
    paintOneBackground(pI.p, style()->backgroundColor(), style()->backgroundLayers(), cr, _tx, _ty, w, h);

    if ( style()->hasBorder() )
	    paintBorderMinusLegend(pI.p, _tx, _ty, w, h, style(), legend->xPos(), legend->width(), legendBottom);
}

void RenderFieldset::paintBorderMinusLegend(QPainter *p, int _tx, int _ty, int w, int h,
                                            const RenderStyle* style, int lx, int lw, int lb)
{

    const QColor& tc = style->borderTopColor();
    const QColor& bc = style->borderBottomColor();

    EBorderStyle ts = style->borderTopStyle();
    EBorderStyle bs = style->borderBottomStyle();
    EBorderStyle ls = style->borderLeftStyle();
    EBorderStyle rs = style->borderRightStyle();

    bool render_t = ts > BHIDDEN;
    bool render_l = ls > BHIDDEN;
    bool render_r = rs > BHIDDEN;
    bool render_b = bs > BHIDDEN;

    int borderLeftWidth = style->borderLeftWidth();
    int borderRightWidth = style->borderRightWidth();

    if(render_t) {
        if (lx >= borderLeftWidth)
            drawBorder(p, _tx, _ty, _tx + lx, _ty +  style->borderTopWidth(), BSTop, tc, style->color(), ts,
                   (render_l && (ls == DOTTED || ls == DASHED || ls == DOUBLE)?style->borderLeftWidth():0), 0);
        if (lx + lw <=  w - borderRightWidth)
            drawBorder(p, _tx+lx+lw, _ty, _tx + w, _ty +  style->borderTopWidth(), BSTop, tc, style->color(), ts,
                   0, (render_r && (rs == DOTTED || rs == DASHED || rs == DOUBLE)?style->borderRightWidth():0));
    }

    if(render_b)
        drawBorder(p, _tx, _ty + h - style->borderBottomWidth(), _tx + w, _ty + h, BSBottom, bc, style->color(), bs,
                   (render_l && (ls == DOTTED || ls == DASHED || ls == DOUBLE)?style->borderLeftWidth():0),
                   (render_r && (rs == DOTTED || rs == DASHED || rs == DOUBLE)?style->borderRightWidth():0));

    if(render_l)
    {
	const QColor& lc = style->borderLeftColor();

	bool ignore_top =
	  (tc == lc) &&
	  (ls >= OUTSET) &&
	  (ts == DOTTED || ts == DASHED || ts == SOLID || ts == OUTSET);

	bool ignore_bottom =
	  (bc == lc) &&
	  (ls >= OUTSET) &&
	  (bs == DOTTED || bs == DASHED || bs == SOLID || bs == INSET);

        int startY = _ty;
        if (lx < borderLeftWidth && lx + lw > 0) {
            // The legend intersects the border.
            ignore_top = true;
            startY = lb;
        }

        drawBorder(p, _tx, startY, _tx + borderLeftWidth, _ty + h, BSLeft, lc, style->color(), ls,
		   ignore_top?0:style->borderTopWidth(),
		   ignore_bottom?0:style->borderBottomWidth());
    }

    if(render_r)
    {
	const QColor& rc = style->borderRightColor();

	bool ignore_top =
	  (tc == rc) &&
	  (rs >= DOTTED || rs == INSET) &&
	  (ts == DOTTED || ts == DASHED || ts == SOLID || ts == OUTSET);

	bool ignore_bottom =
	  (bc == rc) &&
	  (rs >= DOTTED || rs == INSET) &&
	  (bs == DOTTED || bs == DASHED || bs == SOLID || bs == INSET);

        int startY = _ty;
        if (lx < w && lx + lw > w - borderRightWidth) {
            // The legend intersects the border.
            ignore_top = true;
            startY = lb;
        }

        drawBorder(p, _tx + w - borderRightWidth, startY, _tx + w, _ty + h, BSRight, rc, style->color(), rs,
		   ignore_top?0:style->borderTopWidth(),
		   ignore_bottom?0:style->borderBottomWidth());
    }
}

void RenderFieldset::setStyle(RenderStyle* _style)
{
    RenderBlock::setStyle(_style);

    // WinIE renders fieldsets with display:inline like they're inline-blocks.  For us,
    // an inline-block is just a block element with replaced set to true and inline set
    // to true.  Ensure that if we ended up being inline that we set our replaced flag
    // so that we're treated like an inline-block.
    if (isInline())
        setReplaced(true);
}

// -------------------------------------------------------------------------

RenderFileButton::RenderFileButton(HTMLInputElementImpl *element)
    : RenderFormElement(element)
{
    FileButtonWidget* w = new FileButtonWidget( view()->widget() );

    w->setMode(KFile::File | KFile::ExistingOnly);
    w->lineEdit()->setCompletionBox( new CompletionWidget(w) );
    w->completionObject()->setDir(KGlobalSettings::documentPath());

    connect(w->lineEdit(), SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));
    connect(w->lineEdit(), SIGNAL(textChanged(const QString &)),this,SLOT(slotTextChanged(const QString &)));
    connect(w, SIGNAL(urlSelected(const KUrl &)),this,SLOT(slotUrlSelected(const KUrl &)));

    setQWidget(w);
    m_haveFocus = false;
}

short RenderFileButton::baselinePosition( bool f ) const
{
    int bTop = borderTop();
    int bBottom = borderBottom();
    int ret = (height()-paddingTop()-paddingBottom()-bTop-bBottom+1)/2;
    ret += marginTop() + paddingTop() + bTop;
    ret += ((fontMetrics( f ).ascent())/2)-2;
    return ret;
}

void RenderFileButton::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    const QFontMetrics &fm = style()->fontMetrics();
    int size = element()->size();

    int h = fm.lineSpacing();
    int w = fm.width( 'x' ) * (size > 0 ? size+1 : 17); // "some"
    KLineEdit* edit = static_cast<KUrlRequester*>( m_widget )->lineEdit();

    QStyleOptionFrame opt;
    opt.initFrom(edit);
    if (edit->hasFrame())
        opt.lineWidth = edit->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &opt, edit);
    QSize s = edit->style()->sizeFromContents(QStyle::CT_LineEdit,
                                             &opt,
          QSize(w, qMax(h, 14)), edit)
        .expandedTo(QApplication::globalStrut());
    QSize bs = static_cast<KUrlRequester*>( m_widget )->minimumSizeHint() - edit->minimumSizeHint();

    setIntrinsicWidth( s.width() + bs.width() );
    setIntrinsicHeight( qMax(s.height(), bs.height()) );

    RenderFormElement::calcMinMaxWidth();
}

void RenderFileButton::handleFocusOut()
{
    if ( widget()->lineEdit() && widget()->lineEdit()->isModified() ) {
        element()->onChange();
        widget()->lineEdit()->setModified( false );
    }
}

void RenderFileButton::updateFromElement()
{
    KLineEdit* edit = widget()->lineEdit();
    bool blocked = edit->blockSignals(true);
    edit->setText(element()->value().string());
    edit->blockSignals(false);
    edit->setModified(blocked );

    RenderFormElement::updateFromElement();
}

void RenderFileButton::slotReturnPressed()
{
    handleFocusOut();

    if (element()->form())
	element()->form()->submitFromKeyboard();
}

void RenderFileButton::slotTextChanged(const QString &/*string*/)
{
   element()->m_value = KUrl( widget()->url() ).pathOrUrl();
}

void RenderFileButton::slotUrlSelected(const KUrl &)
{
	element()->onChange();
}

void RenderFileButton::select()
{
    widget()->lineEdit()->selectAll();
}

// -------------------------------------------------------------------------

RenderLabel::RenderLabel(HTMLGenericFormElementImpl *element)
    : RenderFormElement(element)
{

}

// -------------------------------------------------------------------------

RenderLegend::RenderLegend(HTMLGenericFormElementImpl *element)
    : RenderBlock(element)
{
}

// -------------------------------------------------------------------------------

bool ListBoxWidget::event( QEvent * event )
{
    // accept all wheel events so that they are not propagated to the view
    // once either end of the list is reached.
    bool ret = KListWidget::event(event);
    if (event->type() == QEvent::Wheel) {
        event->accept();
        ret = true;
    }
    return ret;
}

ComboBoxWidget::ComboBoxWidget(QWidget *parent)
    : KComboBox(false, parent)
{
    m_kwp->setIsRedirected( true );
    //setAutoMask(true);
    if (view()) view()->installEventFilter(this);
    setMouseTracking(true);
}

void ComboBoxWidget::showPopup()
{
    QPoint p = pos();
    QPoint dest;
    QWidget* parent = parentWidget();
    KHTMLView* v = m_kwp->rootViewPos(dest);
    int zoomLevel = v ? v->zoomLevel() : 100;
    if (zoomLevel != 100) {
        dest.setX( dest.x()*zoomLevel/100 );
        dest.setY( dest.y()*zoomLevel/100 );
    }
    bool blocked = blockSignals(true);
    if (v != parent)
        setParent(v);
    move( dest );
    blockSignals(blocked);

    KComboBox::showPopup();

    blocked = blockSignals(true);
    if (v != parent)
        setParent(parent);
    move( p );
    blockSignals(blocked);
}

void ComboBoxWidget::hidePopup()
{
    KComboBox::hidePopup();
}


bool ComboBoxWidget::event(QEvent *e)
{
    if (KComboBox::event(e))
	return true;
    if (e->type()==QEvent::KeyPress)
    {
	QKeyEvent *ke = static_cast<QKeyEvent *>(e);
	switch(ke->key())
	{
	case Qt::Key_Return:
	case Qt::Key_Enter:
	    showPopup();
	    ke->accept();
	    return true;
	default:
	    return false;
	}
    }
    return false;
}

bool ComboBoxWidget::eventFilter(QObject *dest, QEvent *e)
{
    if (dest==view() &&  e->type()==QEvent::KeyPress)
    {
	QKeyEvent *ke = static_cast<QKeyEvent *>(e);
	bool forward = false;
	switch(ke->key())
	{
	case Qt::Key_Tab:
	    forward=true;
	case Qt::Key_Backtab:
	    // ugly hack. emulate popdownlistbox() (private in QComboBox)
	    // we re-use ke here to store the reference to the generated event.
	    ke = new QKeyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
	    QApplication::sendEvent(dest,ke);
	    focusNextPrevChild(forward);
	    delete ke;
	    return true;
	default:
	    return KComboBox::eventFilter(dest, e);
	}
    }
    return KComboBox::eventFilter(dest, e);
}

// -------------------------------------------------------------------------

RenderSelect::RenderSelect(HTMLSelectElementImpl *element)
    : RenderFormElement(element)
{
    m_ignoreSelectEvents = false;
    m_multiple = element->multiple();
    m_size = element->size();
    m_useListBox = (m_multiple || m_size > 1);
    m_selectionChanged = true;
    m_optionsChanged = true;

    if(m_useListBox)
        setQWidget(createListBox());
    else
        setQWidget(createComboBox());
}

void RenderSelect::updateFromElement()
{
    m_ignoreSelectEvents = true;

    // change widget type
    bool oldMultiple = m_multiple;
    unsigned oldSize = m_size;
    bool oldListbox = m_useListBox;

    m_multiple = element()->multiple();
    m_size = element()->size();
    m_useListBox = (m_multiple || m_size > 1);

    if (oldMultiple != m_multiple || oldSize != m_size) {
        if (m_useListBox != oldListbox) {
            // type of select has changed
            if(m_useListBox)
                setQWidget(createListBox());
            else
                setQWidget(createComboBox());
        }

        if (m_useListBox && oldMultiple != m_multiple) {
            static_cast<KListWidget*>(m_widget)->setSelectionMode(m_multiple ?
                                            QListWidget::ExtendedSelection
                                          : QListWidget::SingleSelection);
        }
        m_selectionChanged = true;
        m_optionsChanged = true;
    }

    // update contents listbox/combobox based on options in m_element
    if ( m_optionsChanged ) {
        if (element()->m_recalcListItems)
            element()->recalcListItems();
        const QVector<HTMLGenericFormElementImpl*> listItems = element()->listItems();
        int listIndex;

        if(m_useListBox) {
            static_cast<KListWidget*>(m_widget)->clear();
        }

        else
            static_cast<KComboBox*>(m_widget)->clear();

        for (listIndex = 0; listIndex < int(listItems.size()); listIndex++) {
            if (listItems[listIndex]->id() == ID_OPTGROUP) {
                DOMString text = listItems[listIndex]->getAttribute(ATTR_LABEL);
                if (text.isNull())
                    text = "";

                if(m_useListBox) {
                    QListWidgetItem *item = new QListWidgetItem(QString(text.implementation()->s, text.implementation()->l));
                    static_cast<KListWidget*>(m_widget)
                        ->insertItem(listIndex,item);
                    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
                }
                else {
                    static_cast<KComboBox*>(m_widget)
                        ->insertItem(listIndex, QString(text.implementation()->s, text.implementation()->l));
#ifdef __GNUC__
  #warning "This needs fixing (though did it work in 3?)"
#endif
#if 0
		    static_cast<KComboBox*>(m_widget)->listBox()->item(listIndex)->setSelectable(false);
#endif
		}
            }
            else if (listItems[listIndex]->id() == ID_OPTION) {
                HTMLOptionElementImpl* optElem = static_cast<HTMLOptionElementImpl*>(listItems[listIndex]);
                QString text = optElem->text().string();
                if (optElem->parentNode()->id() == ID_OPTGROUP)
                {
                    // Prefer label if set
                    DOMString label = optElem->getAttribute(ATTR_LABEL);
                    if (!label.isEmpty())
                        text = label.string();
                    text = QLatin1String("    ")+text;
                }

                if(m_useListBox) {
                    KListWidget *l = static_cast<KListWidget*>(m_widget);
                    l->insertItem(listIndex,text);
                    DOMString disabled = optElem->getAttribute(ATTR_DISABLED);
                    if (!disabled.isNull() && l->item( listIndex )) {
                        l->item( listIndex )->setFlags( l->item(listIndex)->flags()
                                                            & ~Qt::ItemIsSelectable );
                    }
                }  else
                    static_cast<KComboBox*>(m_widget)->insertItem(listIndex, text);
            }
            else
                KHTMLAssert(false);
            m_selectionChanged = true;
        }

        // QComboBox caches the size hint unless you call setFont (ref: TT docu)
        if(!m_useListBox) {
            KComboBox *that = static_cast<KComboBox*>(m_widget);
            that->setFont( that->font() );
        }
        setNeedsLayoutAndMinMaxRecalc();
        m_optionsChanged = false;
    }

    // update selection
    if (m_selectionChanged) {
        updateSelection();
    }


    m_ignoreSelectEvents = false;

    RenderFormElement::updateFromElement();
}

short RenderSelect::baselinePosition( bool f ) const
{
    if (m_useListBox)
        return RenderFormElement::baselinePosition(f);

    bool hasFrame = static_cast<KComboBox*>(widget())->hasFrame();
    int bTop = hasFrame ? 0 : borderTop();
    int bBottom = hasFrame ? 0 : borderBottom();
    int ret = (height()-paddingTop()-paddingBottom()-bTop-bBottom+1)/2;
    ret += marginTop() + paddingTop() + bTop;
    ret += ((fontMetrics( f ).ascent())/2)-2;
    return ret;
}

void RenderSelect::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    if (m_optionsChanged)
        updateFromElement();

    // ### ugly HACK FIXME!!!
    setMinMaxKnown();
    layoutIfNeeded();
    setNeedsLayoutAndMinMaxRecalc();
    // ### end FIXME

    RenderFormElement::calcMinMaxWidth();
}

void RenderSelect::layout( )
{
    KHTMLAssert(needsLayout());
    KHTMLAssert(minMaxKnown());

    // ### maintain selection properly between type/size changes, and work
    // out how to handle multiselect->singleselect (probably just select
    // first selected one)

    // calculate size
    if(m_useListBox) {
        KListWidget* w = static_cast<KListWidget*>(m_widget);

        int width = 0;
        int height = 0;

        QAbstractItemModel *m = w->model();
        QAbstractItemDelegate *d = w->itemDelegate();
        QStyleOptionViewItem so;
        so.font = w->font();

        for ( int rowIndex = 0 ; rowIndex < w->count() ; rowIndex++ ) {
            QModelIndex mi = m->index(rowIndex, 0);
            QSize s = d->sizeHint( so, mi);
            width = qMax(width, s.width());
            height = qMax(height, s.height());
        }

        if ( !height )
            height = w->fontMetrics().height();
        if ( !width )
            width = w->fontMetrics().width( 'x' );

        int size = m_size;
        // check if multiple and size was not given or invalid
        // Internet Exploder sets size to qMin(number of elements, 4)
        // Netscape seems to simply set it to "number of elements"
        // the average of that is IMHO qMin(number of elements, 10)
        // so I did that ;-)
        if(size < 1)
            size = qMin(static_cast<KListWidget*>(m_widget)->count(), 10);

        width += 2*w->frameWidth() + w->verticalScrollBar()->sizeHint().width();
        int lhs = m_widget->style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing);
        if (lhs>0)
            width += lhs;
        height = size*height + 2*w->frameWidth();

        setIntrinsicWidth( width );
        setIntrinsicHeight( height );
    }
    else {
        QSize s(m_widget->sizeHint());
        setIntrinsicWidth( s.width() );
        setIntrinsicHeight( s.height() );
    }

    /// uuh, ignore the following line..
    setNeedsLayout(true);
    RenderFormElement::layout();

    // and now disable the widget in case there is no <option> given
    const QVector<HTMLGenericFormElementImpl*> listItems = element()->listItems();

    bool foundOption = false;
    for (int i = 0; i < listItems.size() && !foundOption; i++)
	foundOption = (listItems[i]->id() == ID_OPTION);

    m_widget->setEnabled(foundOption && ! element()->disabled());
}

void RenderSelect::slotSelected(int index) // emitted by the combobox only
{
    if ( m_ignoreSelectEvents ) return;

    KHTMLAssert( !m_useListBox );

    const QVector<HTMLGenericFormElementImpl*> listItems = element()->listItems();
    if(index >= 0 && index < int(listItems.size()))
    {
        bool found = ( listItems[index]->id() == ID_OPTION );

        if ( !found ) {
            // this one is not selectable,  we need to find an option element
            while ( index < listItems.size() ) {
                if ( listItems[index]->id() == ID_OPTION ) {
                    found = true;
                    break;
                }
                ++index;
            }

            if ( !found ) {
                while ( index >= 0 ) {
                    if ( listItems[index]->id() == ID_OPTION ) {
                        found = true;
                        break;
                    }
                    --index;
                }
            }
        }

        if ( found ) {
            bool changed = false;

            for ( int i = 0; i < listItems.size(); ++i )
                if ( listItems[i]->id() == ID_OPTION && i !=  index )
                {
                    HTMLOptionElementImpl* opt = static_cast<HTMLOptionElementImpl*>( listItems[i] );
                    changed |= (opt->m_selected == true);
                    opt->m_selected = false;
                }

            HTMLOptionElementImpl* opt = static_cast<HTMLOptionElementImpl*>(listItems[index]);
            changed |= (opt->m_selected == false);
            opt->m_selected = true;

            if ( index != static_cast<ComboBoxWidget*>( m_widget )->currentIndex() )
                static_cast<ComboBoxWidget*>( m_widget )->setCurrentIndex( index );

            // When selecting an optgroup item, and we move forward to we
            // shouldn't emit onChange. Hence this bool, the if above doesn't do it.
            if ( changed )
            {
		ref();
                element()->onChange();
                deref();
            }
        }
    }
}


void RenderSelect::slotSelectionChanged() // emitted by the listbox only
{
    if ( m_ignoreSelectEvents ) return;

    // don't use listItems() here as we have to avoid recalculations - changing the
    // option list will make use update options not in the way the user expects them
    const QVector<HTMLGenericFormElementImpl*> listItems = element()->m_listItems;
    for ( int i = 0; i < listItems.count(); i++ )
        // don't use setSelected() here because it will cause us to be called
        // again with updateSelection.
        if ( listItems[i]->id() == ID_OPTION )
            static_cast<HTMLOptionElementImpl*>( listItems[i] )
                ->m_selected = static_cast<KListWidget*>( m_widget )->item(i)->isSelected();

    ref();
    element()->onChange();
    deref();
}

void RenderSelect::setOptionsChanged(bool _optionsChanged)
{
    m_optionsChanged = _optionsChanged;
}

void RenderSelect::setPadding()
{
    if (m_size > 1 || m_multiple)
        RenderFormElement::setPadding();
}

ListBoxWidget* RenderSelect::createListBox()
{
    ListBoxWidget *lb = new ListBoxWidget(view()->widget());
    lb->setSelectionMode(m_multiple ? QListWidget::ExtendedSelection : QListWidget::SingleSelection);
    connect( lb, SIGNAL( itemSelectionChanged() ), this, SLOT( slotSelectionChanged() ) );
    m_ignoreSelectEvents = false;
    lb->setMouseTracking(true);

    return lb;
}

ComboBoxWidget *RenderSelect::createComboBox()
{
    ComboBoxWidget *cb = new ComboBoxWidget(view()->widget());
    connect(cb, SIGNAL(activated(int)), this, SLOT(slotSelected(int)));
    return cb;
}

void RenderSelect::updateSelection()
{
    const QVector<HTMLGenericFormElementImpl*> listItems = element()->listItems();
    int i;
    if (m_useListBox) {
        // if multi-select, we select only the new selected index
        KListWidget *listBox = static_cast<KListWidget*>(m_widget);
        for (i = 0; i < int(listItems.size()); i++)
            listBox->item(i)->setSelected(listItems[i]->id() == ID_OPTION &&
                                 static_cast<HTMLOptionElementImpl*>(listItems[i])->selected());
    }
    else {
        bool found = false;
        int firstOption = i = listItems.size();
        while (i--)
            if (listItems[i]->id() == ID_OPTION) {
                if (found)
                    static_cast<HTMLOptionElementImpl*>(listItems[i])->m_selected = false;
                else if (static_cast<HTMLOptionElementImpl*>(listItems[i])->selected()) {
                    static_cast<KComboBox*>( m_widget )->setCurrentIndex(i);
                    found = true;
                }
                firstOption = i;
            }

        if (!found && firstOption != listItems.size()) {
            // select first option (IE7/Gecko behaviour)
            static_cast<HTMLOptionElementImpl*>(listItems[firstOption])->m_selected = true;
            static_cast<KComboBox*>( m_widget )->setCurrentIndex(firstOption);
        }
    }

    m_selectionChanged = false;
}


// -------------------------------------------------------------------------

TextAreaWidget::TextAreaWidget(int wrap, QWidget* parent)
    : KTextEdit(parent)
{
    m_kwp->setIsRedirected( true );
    if(wrap != DOM::HTMLTextAreaElementImpl::ta_NoWrap) {
        setLineWrapMode(QTextEdit::WidgetWidth);
        setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
        setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    }
    else {
        setLineWrapMode(QTextEdit::NoWrap);
        setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );
        setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
    }
    KCursor::setAutoHideCursor(viewport(), true);
    setAcceptRichText (false);
    setMouseTracking(true);

}

void TextAreaWidget::scrollContentsBy( int dx, int dy )
{
    KTextEdit::scrollContentsBy(dx, dy);
    update();

}

TextAreaWidget::~TextAreaWidget()
{
}



bool TextAreaWidget::event( QEvent *e )
{
#if 0
    if ( e->type() == QEvent::AccelAvailable && isReadOnly() ) {
        QKeyEvent* ke = (QKeyEvent*) e;
        if ( ke->modifiers() & Qt::ControlModifier ) {
            switch ( ke->key() ) {
                case Qt::Key_Left:
                case Qt::Key_Right:
                case Qt::Key_Up:
                case Qt::Key_Down:
                case Qt::Key_Home:
                case Qt::Key_End:
                    ke->accept();
                default:
                break;
            }
        }
    }
#endif
    // accept all wheel events so that they are not propagated to the view
    // once either end of the widget is reached.
    bool ret = KTextEdit::event(e);
    if (e->type() == QEvent::Wheel) {
        e->accept();
        ret = true;
    }
    return ret;
}

// -------------------------------------------------------------------------

RenderTextArea::RenderTextArea(HTMLTextAreaElementImpl *element)
    : RenderFormElement(element)
{
    scrollbarsStyled = false;

    TextAreaWidget *edit = new TextAreaWidget(element->wrap(), view());
    setQWidget(edit);
    const KHTMLSettings *settings = view()->part()->settings();
    edit->setCheckSpellingEnabled( settings->autoSpellCheck() );
    edit->setTabChangesFocus( ! settings->allowTabulation() );

    connect(edit,SIGNAL(textChanged()),this,SLOT(slotTextChanged()));

    setText(element->value().string());
}

RenderTextArea::~RenderTextArea()
{
    element()->m_value = text();
}

void RenderTextArea::handleFocusOut()
{
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);

    if ( w && element()->m_changed ) {
        element()->m_changed = false;
        element()->onChange();
    }
}

void RenderTextArea::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    const QFontMetrics &m = style()->fontMetrics();
    w->setTabStopWidth(8 * m.width(" "));
    int lvs = qMax(0, w->style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing));
    int lhs = qMax(0, w->style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing));
    int llm = qMax(0, w->style()->pixelMetric(QStyle::PM_LayoutLeftMargin));
    int lrm = qMax(0, w->style()->pixelMetric(QStyle::PM_LayoutRightMargin));
    int lbm = qMax(0, w->style()->pixelMetric(QStyle::PM_LayoutBottomMargin));
    int ltm = qMax(0, w->style()->pixelMetric(QStyle::PM_LayoutTopMargin));
    QSize size( qMax(element()->cols(), 1L)*m.width('x') + w->frameWidth()*2 + llm+lrm +
                w->verticalScrollBar()->sizeHint().width()+lhs,
                qMax(element()->rows(), 1L)*m.lineSpacing() + w->frameWidth()*4 + lbm+ltm +
                (w->lineWrapMode() == QTextEdit::NoWrap ?
                 w->horizontalScrollBar()->sizeHint().height()+lvs : 0)
        );

    setIntrinsicWidth( size.width() );
    setIntrinsicHeight( size.height() );

    RenderFormElement::calcMinMaxWidth();
}

void RenderTextArea::setStyle(RenderStyle* _style)
{
    bool unsubmittedFormChange = element()->m_unsubmittedFormChange;

    RenderFormElement::setStyle(_style);

    bool blocked = widget()->blockSignals(true);
    widget()->setAlignment(textAlignment());
    widget()->blockSignals(blocked);

    scrollbarsStyled = false;

    element()->m_unsubmittedFormChange = unsubmittedFormChange;
}

void RenderTextArea::layout()
{
    KHTMLAssert( needsLayout() );

    RenderFormElement::layout();

    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);

    if (!scrollbarsStyled) {
        w->horizontalScrollBar()->setPalette(style()->palette());
        w->verticalScrollBar()->setPalette(style()->palette());
        scrollbarsStyled=true;
    }
}

void RenderTextArea::setText(const QString& newText)
{
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);

    // When this is called, m_value in the element must have just
    // been set to new value --- see if we have any work to do
    if ( newText != text() ) {
        bool blocked = w->blockSignals(true);
        QTextCursor tc = w->textCursor();
        bool atEnd = tc.atEnd();
        bool atStart = tc.atStart();
        int cx = w->horizontalScrollBar()->value();
        int cy = w->verticalScrollBar()->value();
        QString oldText = w->toPlainText();
        int ex = 0;
        int otl = oldText.length();
        if (otl && newText.length() > otl) {
            while (ex < otl && newText[ex] == oldText[ex])
                ++ex;
            QTextCursor tc(w->document());
            tc.setPosition( ex, QTextCursor::MoveAnchor );
            tc.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
            tc.insertText(newText.right( newText.length()-ex ));
        } else {                            
            w->setPlainText( newText );
        }
        w->setTextCursor(tc);
        if (atEnd)
           tc.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        else if (atStart)
           tc.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
        w->horizontalScrollBar()->setValue( cx );
        w->verticalScrollBar()->setValue( cy );
        w->blockSignals(blocked);
    }
}

void RenderTextArea::updateFromElement()
{
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    w->setReadOnly(element()->readOnly());
    RenderFormElement::updateFromElement();
}

QString RenderTextArea::text()
{
    // ### We may want to cache this when physical, since the DOM no longer caches,
    // but seeing how text() has always been called on textChanged(), it's probably not needed

    QString txt;
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
#ifdef __GNUC__
#warning "Physical wrap mode needs testing (also in ::selection*)"
#endif
    if (element()->wrap() == DOM::HTMLTextAreaElementImpl::ta_Physical) {
        QTextCursor tc(w->document());
        while (!tc.atEnd()) {
           tc.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
           txt += tc.selectedText();
           if (tc.movePosition(QTextCursor::Down)) {
               txt += QLatin1String("\n");
               tc.movePosition(QTextCursor::StartOfLine);
           } else {
               break;
           }
        }
    }
    else
        txt = w->toPlainText();
    return txt;
}

void RenderTextArea::highLightWord( unsigned int length, unsigned int pos )
{
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    if ( w )
        w->highlightWord( length, pos );
}

void RenderTextArea::slotTextChanged()
{
    element()->m_changed    = true;
    if (element()->m_value != text())
        element()->m_unsubmittedFormChange = true;
}

void RenderTextArea::select()
{
    static_cast<TextAreaWidget *>(m_widget)->selectAll();
}

long RenderTextArea::selectionStart()
{
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    return w->textCursor().selectionStart();
}

long RenderTextArea::selectionEnd()
{
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    return w->textCursor().selectionEnd();
}

static void setPhysWrapPos(QTextCursor& otc, bool selStart, int idx)
{
    QTextCursor tc = otc;
    tc.setPosition(0);
    tc.movePosition(QTextCursor::EndOfLine);
    while (!tc.atEnd()) {
        if (tc.movePosition(QTextCursor::Down) && tc.position()< idx)
            --idx;
        if (tc.position() >= idx)
            break;
    }
    otc.setPosition(idx, selStart ? QTextCursor::MoveAnchor : QTextCursor::KeepAnchor );
}

void RenderTextArea::setSelectionStart(long offset) {
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    QTextCursor tc = w->textCursor();
    if (element()->wrap() == DOM::HTMLTextAreaElementImpl::ta_Physical)
        setPhysWrapPos(tc, true /*selStart*/, offset);
    else
        tc.setPosition(offset);
    w->setTextCursor(tc);
}

void RenderTextArea::setSelectionEnd(long offset) {
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    QTextCursor tc = w->textCursor();
    if (element()->wrap() == DOM::HTMLTextAreaElementImpl::ta_Physical)
        setPhysWrapPos(tc, false /*selStart*/, offset);
    else
        tc.setPosition(offset, QTextCursor::KeepAnchor);
    w->setTextCursor(tc);
}

void RenderTextArea::setSelectionRange(long start, long end) {
    setSelectionStart(start);
    setSelectionEnd(end);
}
// ---------------------------------------------------------------------------

#include "render_form.moc"
