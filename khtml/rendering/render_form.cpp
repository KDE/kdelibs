/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk A. Mueller (mueller@kde.org)
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */

//#define BUTTON_DEBUG

#include <kdebug.h>

#include <klocale.h>
#include <klistbox.h>

#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qcombobox.h>
#include <qstack.h>

#include "dom_nodeimpl.h"
#include "dom_textimpl.h"

#include "html/html_formimpl.h"
#include "misc/htmlhashes.h"

#include "rendering/render_form.h"
#include "rendering/render_style.h"

using namespace khtml;

RenderFormElement::RenderFormElement(RenderStyle *style, QScrollView *view,
				     HTMLFormElementImpl *form)
    : RenderWidget(style, view)
{
    m_form = form;
    if(m_form) m_form->registerFormElement(this);

    m_gform = 0;
}

RenderFormElement::~RenderFormElement()
{
     if(m_form) m_form->removeFormElement(this);
}

QString RenderFormElement::encodeString( QString e )
{
    static const char *safe = "$-._!*(),"; /* RFC 1738 */
    unsigned pos = 0;
    QString encoded;
    char buffer[5];

    while ( pos < e.length() )
    {
        QChar c = e[pos];

        if ( (( c >= 'A') && ( c <= 'Z')) ||
             (( c >= 'a') && ( c <= 'z')) ||
             (( c >= '0') && ( c <= '9')) ||
             (strchr(safe, c))
            )
        {
            encoded += c;
        }
        else if ( c == ' ' )
        {
            encoded += '+';
        }
        else if ( c == '\n' )
        {
            encoded += "%0D%0A";
        }
        else if ( c != '\r' )
        {
            sprintf( buffer, "%%%02X", (int)c );
            encoded += buffer;
        }
        pos++;
    }

    return encoded;
}

QString RenderFormElement::decodeString( QString e )
{
    unsigned int pos = 0;
    unsigned int len = e.length();
    QString decoded;

    while ( pos < len )
    {
        if (e[pos] == QChar('%'))
        {
            if (pos+2 < len)
            {
                DOMString buffer(e.unicode()+pos+1, 2);
                bool ok;
                unsigned char val = buffer.string().toInt(&ok, 16);
                if (((char) val) != '\r')
                {
                    decoded += (char) val;
                }
            }
            else
            {
                decoded += e[pos];
            }
        }
        else if (e[pos] == '+')
        {
            decoded += ' ';
        }
        else
        {
            decoded += e[pos];
        }
        pos++;
    }
    return decoded;
}

void RenderFormElement::layout(bool)
{
    kdDebug(300) << "inside RenderFormElement::layout()" << endl;

    // honor style sheet stuff
    int h = 0;

#if 0
    if(parent()) {
        if((h = m_style->width().width(containingBlockWidth())) > 0) {
            kdDebug(300) << "overwriting width to " << h << endl;
            m_width = h;
        }


        if((h = m_style->height().width(containingBlockHeight())) > 0) {
            kdDebug(300) << "overwriting height to " << h << endl;
            m_height = h;
        }
    }
#endif

    // now Layout the stuff
    if(m_widget)
        m_widget->resize(m_width, m_height);
}


void RenderFormElement::calcMinMaxWidth()
{
    layout(false);

    kdDebug(300) << "inside RenderFormElement::calcMinMaxWidth()" << endl;

    m_minWidth = m_width;
    m_maxWidth = m_width;
}

// -------------------------------------------------------------------------

RenderButton::RenderButton(RenderStyle *style, QScrollView *view,
                           HTMLFormElementImpl *form)
    : RenderFormElement(style, view, form)
{
}

void RenderButton::layout(bool)
{
    QSize s(0, 0);

    if(m_widget)
        s = m_widget->sizeHint();

    m_height = s.height();
    m_width = s.width();

    RenderFormElement::layout(false);
}


// ------------------------------------------------------------------------------

RenderHiddenButton::RenderHiddenButton(RenderStyle *style, QScrollView *view,
                                       HTMLFormElementImpl *form)
    : RenderButton(style, view, form)
{
    // make it segfault cleanly :-)
    m_widget = 0;
}

QString RenderHiddenButton::encoding()
{
    QString encoding;
    if (!m_name.isEmpty() )
    {
	encoding = encodeString( m_name.string() );
	encoding += '=';
	encoding += encodeString( m_value.string() );
    }
    return encoding;
}


// -------------------------------------------------------------------------------

RenderCheckBox::RenderCheckBox(RenderStyle *style, QScrollView *view, HTMLFormElementImpl *form)
    : RenderButton(style, view, form)
{
    QCheckBox *b = new QCheckBox(view->viewport());
    m_widget = b;
}

QString RenderCheckBox::encoding()
{
    QString encoding;
    if ( static_cast<QCheckBox *>(m_widget)->isChecked() && !m_name.isEmpty() )
    {
	encoding = encodeString( m_name.string() );
	encoding += '=';
	encoding += m_value.isEmpty() ? QString("on") : encodeString( m_value.string() );
    }
    return encoding;
}

void RenderCheckBox::setChecked(bool checked)
{
    static_cast<QCheckBox*>(m_widget)->setChecked(checked);
}


void RenderCheckBox::reset()
{
    setChecked(static_cast<HTMLInputElementImpl*>(m_gform)->checked());
}

// -------------------------------------------------------------------------------


RenderRadioButton::RenderRadioButton(RenderStyle *style, QScrollView *view,
				     HTMLFormElementImpl *form)
    : RenderButton(style, view, form)
{
    QRadioButton *b = new QRadioButton(view->viewport());

    m_widget = b;
    connect(b, SIGNAL(clicked()), this, SLOT(slotClicked()));
}

QString RenderRadioButton::encoding()
{
    QString encoding;
    if ( static_cast<QRadioButton *>(m_widget)->isChecked() && !m_name.isEmpty())
    {
	encoding = encodeString( m_name.string() );
	encoding += '=';
	encoding += encodeString( m_value.string() );
    }
    return encoding;
}

void RenderRadioButton::setChecked(bool checked)
{
    static_cast<QRadioButton *>(m_widget)->setChecked(checked);
}

void RenderRadioButton::reset()
{
    setChecked(static_cast<HTMLInputElementImpl*>(m_gform)->checked());
}

void RenderRadioButton::slotClicked()
{
    m_form->radioClicked(this);
}

// -------------------------------------------------------------------------------


RenderSubmitButton::RenderSubmitButton(RenderStyle *style, QScrollView *view,
				     HTMLFormElementImpl *form)
    : RenderButton(style, view, form)
{
    QPushButton *p = new QPushButton(view->viewport());
    m_widget = p;
    m_widget->setFont(m_style->font());

    connect(p, SIGNAL(clicked()), this, SLOT(slotClicked()));
    m_clicked = false;
}

RenderSubmitButton::~RenderSubmitButton()
{
}

void RenderSubmitButton::slotClicked()
{
    m_clicked = true;
    m_form->submit();
}

QString RenderSubmitButton::encoding()
{
    QString encoding;
    if ( m_clicked && !m_name.isEmpty())
    {
	encoding = encodeString( m_name.string() );
	encoding += '=';
	encoding += encodeString( QString(m_value.unicode(), m_value.length()) );
    }
    m_clicked = false;
    return encoding;
}


void RenderSubmitButton::setValue(const DOMString &value)
{
    m_value = value;
    if(m_value != 0)
	static_cast<QPushButton *>(m_widget)->setText(m_value.string());
    else
	static_cast<QPushButton *>(m_widget)->setText(i18n("Submit Query"));
}

void RenderSubmitButton::reset()
{
    m_clicked = false;
}

// -------------------------------------------------------------------------------

RenderImageButton::RenderImageButton(RenderStyle *style, QScrollView *view,
				     HTMLFormElementImpl *form)
    : RenderSubmitButton(style, view, form)
{
}

RenderImageButton::~RenderImageButton()
{
}

void RenderImageButton::setImageUrl(DOMString url, DOMString baseUrl)
{
    if(m_bgImage) m_bgImage->deref(this);
    m_bgImage = Cache::requestImage(url, baseUrl);
    m_bgImage->ref(this);
}

void RenderImageButton::setPixmap( const QPixmap &p )
{
    static_cast<QPushButton *>(m_widget)->setPixmap(p);
    // Image dimensions have been changed, recalculate layout
    layout(false);
    calcMinMaxWidth();
    m_widget->resize(m_width, m_height);

    if(m_parent) m_parent->updateSize();	
}


// -------------------------------------------------------------------------------

RenderResetButton::RenderResetButton(RenderStyle *style, QScrollView *view,
				     HTMLFormElementImpl *form)
    : RenderSubmitButton(style, view, form)
{
}

RenderResetButton::~RenderResetButton()
{
}

void RenderResetButton::slotClicked()
{
    m_form->reset();
}

// -------------------------------------------------------------------------------

RenderPushButton::RenderPushButton(RenderStyle *style, QScrollView *view,
				     HTMLFormElementImpl *form, HTMLInputElementImpl *input)
    : RenderSubmitButton(style, view, form)
{
    m_domParent = input;
}

RenderPushButton::~RenderPushButton()
{
}

void RenderPushButton::slotClicked()
{
    m_domParent->mouseEventHandler( 0, DOM::NodeImpl::MouseClick, true );
}


// -------------------------------------------------------------------------------


RenderLineEdit::RenderLineEdit(RenderStyle *style, QScrollView *view, HTMLFormElementImpl *form,
			       int maxLen, int size, bool passwd)
    : RenderFormElement(style, view, form)
{
    QLineEdit *edit = new QLineEdit(view);
    connect(edit, SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));

    if(maxLen > 0) edit->setMaxLength(maxLen);
    if(passwd) edit->setEchoMode( QLineEdit::Password );

    m_size = size;
    m_widget = edit;
    m_widget->setFont(m_style->font());
}

void RenderLineEdit::slotReturnPressed()
{
    m_form->maybeSubmit();
}

void RenderLineEdit::setValue(const DOMString &value)
{
    m_value = value;
    if(m_value != 0)
	static_cast<QLineEdit *>(m_widget)->setText(m_value.string());
}

QString RenderLineEdit::encoding()
{
    QString encoding;
    if(!m_name.isEmpty()) {
        encoding = encodeString( m_name.string() );
        encoding += '=';
        encoding += encodeString( static_cast<QLineEdit *>(m_widget)->text() );
    }
    return encoding;
}

void RenderLineEdit::layout(bool deep)
{
    QFontMetrics fm( m_widget->font() );
    QSize s;

    int h = fm.height();
    int w = fm.width( 'x' ) * (m_size > 0 ? m_size : 17); // "some"
    if ( static_cast<QLineEdit*>(m_widget)->frame() ) {
        h += 8;
        if ( m_widget->style() == QWidget::WindowsStyle && h < 26 )
            h = 22;
        s = QSize( w + 8, h );
    } else
        s = QSize( w + 4, h + 4 );

    static_cast<QLineEdit*>(m_widget)->setReadOnly(m_readonly);

    m_height = s.height();
    m_width = s.width();

    RenderFormElement::layout(false);
}

void RenderLineEdit::reset()
{
    if(m_value != 0)
	static_cast<QLineEdit *>(m_widget)->setText(m_value.string());
    else
	static_cast<QLineEdit *>(m_widget)->setText(QString::null);
}


// ---------------------------------------------------------------------------

RenderFieldset::RenderFieldset(RenderStyle *style, QScrollView *view,
                               HTMLFormElementImpl* form)
    : RenderFormElement(style, view, form)
{
}

RenderFieldset::~RenderFieldset()
{
}


// -------------------------------------------------------------------------


RenderFileButton::RenderFileButton(RenderStyle *style, QScrollView *view,
                         HTMLFormElementImpl* form)
    : RenderFormElement(style, view, form)
{
    QLineEdit *edit = new QLineEdit(view);

    m_widget = edit;
    m_widget->setFont(m_style->font());
}

RenderFileButton::~RenderFileButton()
{
}

void RenderFileButton::layout( bool )
{
    QSize s(m_widget->sizeHint());

    m_height = s.height();
    m_width = s.width();

    RenderFormElement::layout(false);
}

void RenderFileButton::reset()
{
    // ###
}

QString RenderFileButton::encoding()
{
    QString _encoding;

    return _encoding;
}

// -------------------------------------------------------------------------

RenderLabel::RenderLabel(RenderStyle *style, QScrollView *view,
                         HTMLFormElementImpl* form)
    : RenderFormElement(style, view, form)
{

}

RenderLabel::~RenderLabel()
{
}


// -------------------------------------------------------------------------

RenderLegend::RenderLegend(RenderStyle *style, QScrollView *view,
                           HTMLFormElementImpl* form)
    : RenderFormElement(style, view, form)
{
}

RenderLegend::~RenderLegend()
{
}


// -------------------------------------------------------------------------

RenderSelect::RenderSelect(int size, bool multiple, RenderStyle *style,
                           QScrollView *view, HTMLFormElementImpl *form)
    : RenderFormElement(style, view, form)
{
    m_multiple = multiple;
    m_size = size;

    if(m_multiple || m_size > 1) {
        KListBox* w = new KListBox(view);
        w->setSelectionMode(m_multiple ? QListBox::Multi : QListBox::Single);
        m_widget = w;
    }
    else {
        QComboBox *w = new QComboBox(view);
        m_size = 1;
        m_widget = w;
    }
    m_widget->setFont(m_style->font());
}

void RenderSelect::layout( bool )
{
    if(m_multiple || m_size > 1) {
        KListBox* w = static_cast<KListBox*>(m_widget);

        QListBoxItem* p = w->firstItem();
        int width = 0;
        int height = 0;
        while(p) {
            width = QMAX(width, p->width(p->listBox()));
            height = QMAX(height, p->height(p->listBox()));
            p = p->next();
        }

        width += 2*w->frameWidth() + w->verticalScrollBar()->sizeHint().width();
        height = QMAX(m_size, 1)*height + 2*w->frameWidth();

        m_width = width;
        m_height = height;
    }
    else
    {
        QSize s(m_widget->sizeHint());
        m_width = s.width();
        m_height = s.height();
    }

    RenderFormElement::layout(false);
}

void RenderSelect::close()
{
    HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_gform);

    // insert all given <option>'s
    NodeImpl* current = f->firstChild();
    int i = 0;

    while(current) {
        if(current->id() == ID_OPTION &&
           current->firstChild() && current->firstChild()->id() == ID_TEXT) {

            DOMStringImpl *text = static_cast<TextImpl *>(current->firstChild())->string();

            if(m_multiple || m_size > 1)
                static_cast<KListBox*>(m_widget)
                    ->insertItem(QString(text->s, text->l), i);
            else
                static_cast<QComboBox*>(m_widget)
                    ->insertItem(QString(text->s, text->l), i);
        }
        current = current->nextSibling();
        i++;
    }

    if(m_multiple || m_size > 1) {
        // check if multiple and size was not given or invalid
        // Internet Exploder sets size to QMIN(number of elements, 4)
        // Netscape seems to simply set it to "number of elements"
        // the average of that is IMHO QMIN(number of elements, 15)
        // so I did that ;-)
        if(m_multiple && m_size < 1)
            m_size = QMIN(static_cast<KListBox*>(m_widget)->count(), 15);
    }
    else {
        // and now disable the widget in case there is no <option> given
        QComboBox* w = static_cast<QComboBox*>(m_widget);
        if(!w->count())
            w->setEnabled(false);
    }

    reset();
    layout();

    RenderFormElement::close();
}


void RenderSelect::reset()
{
    HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_gform);

    NodeImpl* current = f->firstChild();
    int i = 0;

    while(current) {
        if(current->id() == ID_OPTION &&
           current->firstChild() && current->firstChild()->id() == ID_TEXT) {

            HTMLOptionElementImpl* p = static_cast<HTMLOptionElementImpl*>(current);

            if(m_multiple || m_size > 1)
                static_cast<KListBox*>(m_widget)
                    ->setSelected(i, p->selected());
            else if(p->selected()) {
                static_cast<QComboBox*>(m_widget)->setCurrentItem(i);
                // we only honor the first option that is selected
                // in case there is more than one selected
                break;
            }
        }
        current = current->nextSibling();
        i++;
    }
}

QString RenderSelect::encoding()
{
    QString encoding;

    if(m_name.isEmpty() || m_gform->disabled()) return encoding;

    if(m_multiple || m_size > 1) {
        QString prefix = encodeString( m_name.string() );
        prefix += '=';

        KListBox* w = static_cast<KListBox*>(m_widget);

        HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_gform);

        NodeImpl* current = f->firstChild();
        int i = 0;
        while(current) {
            if( w->isSelected(i) && current->id() == ID_OPTION ) {
                HTMLOptionElementImpl* p = static_cast<HTMLOptionElementImpl*>(current);

                if(p->m_value.isNull())
                    if(w->item(i))
                        encoding += prefix + encodeString(w->item(i)->text());
                else
                    encoding += prefix + encodeString(p->m_value.string());

            }
            current = current->nextSibling();
            i++;
        }
    }
    else
    {
        QComboBox* w = static_cast<QComboBox*>(m_widget);
        HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_gform);

        encoding = encodeString( m_name.string() );
        encoding += '=';

        NodeImpl* current = f->firstChild();
        int i = 0;
        while(current) {
            if( i == w->currentItem() && current->id() == ID_OPTION ) {
                HTMLOptionElementImpl* p = static_cast<HTMLOptionElementImpl*>(current);

                if(p->m_value.isNull())
                    encoding += encodeString(w->currentText());
                else
                    encoding += encodeString(p->m_value.string());

                break;
            }
            current = current->nextSibling();
            i++;
        }
        if (!current)
            // we didn't find it, something strange is going on here
            encoding = QString::null;
    }

    return encoding;
}

// -------------------------------------------------------------------------

class TextAreaWidget : public QMultiLineEdit
{
public:
    TextAreaWidget(int wrap, QWidget* parent)
        : QMultiLineEdit(parent)
    {
        if(wrap != DOM::HTMLTextAreaElementImpl::ta_NoWrap) {
            setWordWrap(QMultiLineEdit::WidgetWidth);
            clearTableFlags(Tbl_autoScrollBars);
            setTableFlags(Tbl_vScrollBar);
        }
        else {
            clearTableFlags(Tbl_autoScrollBars);
            setTableFlags(Tbl_vScrollBar | Tbl_hScrollBar);
        }
    };

    QScrollBar* verticalScrollBar () const
        { return QTableView::verticalScrollBar(); };
    QScrollBar* horizontalScrollBar () const
        { return QTableView::horizontalScrollBar(); };
};


// -------------------------------------------------------------------------

RenderTextArea::RenderTextArea(int wrap, RenderStyle *style, QScrollView *view,
                               HTMLFormElementImpl *form)
    : RenderFormElement(style, view, form)
{
    TextAreaWidget *edit = new TextAreaWidget(wrap, view);

    m_widget = edit;
    m_widget->setFont(m_style->font());
}

void RenderTextArea::layout( bool )
{
    reset();

    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    HTMLTextAreaElementImpl* f = static_cast<HTMLTextAreaElementImpl*>(m_gform);

    w->setReadOnly(m_readonly);

    QFontMetrics m = w->fontMetrics();
    QSize size( QMAX(f->cols(), 1)*m.maxWidth() + w->frameWidth()*5 +
                w->verticalScrollBar()->sizeHint().width(),
                QMAX(f->rows(), 1)*m.height() + w->frameWidth()*3 +
                (w->wordWrap() == QMultiLineEdit::NoWrap ?
                 w->horizontalScrollBar()->sizeHint().height() : 0)
        );

    m_width  = size.width();
    m_height = size.height();

    RenderFormElement::layout(false);
}

void RenderTextArea::reset()
{
    static_cast<QMultiLineEdit*>(m_widget)->setText(m_value.string());
}

QString RenderTextArea::encoding()
{
    QString encoding;

    if(!m_name.isEmpty()) {
        encoding = encodeString( m_name.string() );
        encoding += '=';
        encoding += encodeString( static_cast<TextAreaWidget *>(m_widget)->text() );
    }

    return encoding;
}

void RenderTextArea::close( )
{
    HTMLTextAreaElementImpl *f = static_cast<HTMLTextAreaElementImpl*>(m_gform);

    if(f->firstChild() && f->firstChild()->id() == ID_TEXT)
        setValue(static_cast<TextImpl*>(f->firstChild())->string());

    RenderFormElement::close();
}

// ---------------------------------------------------------------------------

#include "render_form.moc"
