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

#include <kurl.h>
#include <klocale.h>
#include <klistbox.h>
#include <kfiledialog.h>
#include <netaccess.h>

#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qcombobox.h>
#include <qstack.h>
#include <qlayout.h>

#include "dom_nodeimpl.h"
#include "dom_textimpl.h"
#include "dom_docimpl.h"

#include "html/html_formimpl.h"
#include "misc/htmlhashes.h"

#include "rendering/render_form.h"
#include "rendering/render_style.h"

using namespace khtml;

RenderFormElement::RenderFormElement(QScrollView *view,
				     HTMLFormElementImpl *form)
    : RenderWidget(view)
{
    m_form = form;
    if(m_form) m_form->registerFormElement(this);

    m_gform = 0;
}

RenderFormElement::~RenderFormElement()
{
     if(m_form) m_form->removeFormElement(this);
}

QCString RenderFormElement::encodeString( QString e )
{
    static const char *safe = "$-._!*(),"; /* RFC 1738 */
    unsigned pos = 0;
    QString encoded;

    while ( pos < e.length() )
    {
        QChar c = e[pos];

        if ( (( c >= 'A') && ( c <= 'Z')) ||
             (( c >= 'a') && ( c <= 'z')) ||
             (( c >= '0') && ( c <= '9')) ||
             (strchr(safe, c.latin1()))
            )
        {
            encoded += c;
        }
        else if ( c.latin1() == ' ' )
        {
            encoded += '+';
        }
        else if ( c.latin1() == '\n' )
        {
            encoded += QString::fromLatin1("%0D%0A");
        }
        else if ( c.latin1() != '\r' )
        {
            // HACK! sprintf( buffer, "%%%02X", (int)c );
            //       encoded += buffer;

            // Instead of this hack, use KURL's method.
            // For non-latin1 characters, the (int) cast can lead to wrong values.
            // (even negative ones!) (David)
            encoded += KURL::encode_string( QString(c) );
        }
        pos++;
    }

    return encoded.latin1();
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
    //kdDebug( 6040 ) << "inside RenderFormElement::layout()" << endl;

#if 0
    // honor style sheet stuff
    int h = 0;

    if(parent()) {
        if((h = m_style->width().width(containingBlockWidth())) > 0) {
            kdDebug( 6040 ) << "overwriting width to " << h << endl;
            m_width = h;
        }


        if((h = m_style->height().width(containingBlockHeight())) > 0) {
            kdDebug( 6040 ) << "overwriting height to " << h << endl;
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

    //kdDebug( 6040 ) << "inside RenderFormElement::calcMinMaxWidth()" << endl;

    m_minWidth = m_width;
    m_maxWidth = m_width;
}

// -------------------------------------------------------------------------

RenderButton::RenderButton(QScrollView *view,
                           HTMLFormElementImpl *form)
    : RenderFormElement(view, form)
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

RenderHiddenButton::RenderHiddenButton(QScrollView *view,
                                       HTMLFormElementImpl *form)
    : RenderButton(view, form)
{
}

QCString RenderHiddenButton::encoding()
{
    QCString encoding;
    if (m_name.isEmpty()) return encoding;
    if ( m_form->enctype() == "application/x-www-form-urlencoded" )
    {
	encoding = encodeString( m_name.string() );
	encoding += '=';
	encoding += encodeString( m_value.string() );
    }
    else
	encoding = m_value.string().latin1();
    return encoding;
}


// -------------------------------------------------------------------------------

RenderCheckBox::RenderCheckBox(QScrollView *view, HTMLFormElementImpl *form)
    : RenderButton(view, form)
{
    QCheckBox *b = new QCheckBox(view->viewport());
    setQWidget(b);
}

QCString RenderCheckBox::encoding()
{
    QCString encoding;
    if (m_name.isEmpty()) return encoding;
    if ( static_cast<QCheckBox *>(m_widget)->isChecked() )
    {
      if ( m_form->enctype() == "application/x-www-form-urlencoded" )
      {
          encoding = encodeString( m_name.string() );
          encoding += '=';
          encoding += ( m_value.isEmpty() ? QCString("on") : encodeString( m_value.string() ) );
      }
      else
	        encoding = ( m_value.isEmpty() ? QCString("on") : QCString(m_value.string().latin1()) );
    }
    return encoding;
}

void RenderCheckBox::setChecked(bool checked)
{
    static_cast<QCheckBox*>(m_widget)->setChecked(checked);
}

QString RenderCheckBox::state()
{
   return static_cast<QCheckBox *>(m_widget)->isChecked() ?
             QString::fromLatin1("on") :
             QString::fromLatin1("off");
}

void RenderCheckBox::restoreState(const QString &state)
{
   static_cast<QCheckBox *>(m_widget)->setChecked(state == QString::fromLatin1("on"));
}

void RenderCheckBox::reset()
{
    setChecked(static_cast<HTMLInputElementImpl*>(m_gform)->checked());
}

// -------------------------------------------------------------------------------


RenderRadioButton::RenderRadioButton(QScrollView *view,
				     HTMLFormElementImpl *form)
    : RenderButton(view, form)
{
    QRadioButton *b = new QRadioButton(view->viewport());

    setQWidget(b);
    connect(b, SIGNAL(clicked()), this, SLOT(slotClicked()));
}

QCString RenderRadioButton::encoding()
{
    QCString encoding;
    if (m_name.isEmpty()) return encoding;
    if ( static_cast<QRadioButton *>(m_widget)->isChecked() )
    {
        if ( m_form->enctype() == "application/x-www-form-urlencoded" )
        {
            encoding = encodeString( m_name.string() );
            encoding += '=';
            encoding += encodeString( m_value.string() );
        }
        else
            encoding = m_value.string().latin1();
    }

    return encoding;
}


void RenderRadioButton::setChecked(bool checked)
{
    static_cast<QRadioButton *>(m_widget)->setChecked(checked);
}

QString RenderRadioButton::state()
{
   return static_cast<QRadioButton *>(m_widget)->isChecked() ?
             QString::fromLatin1("on") :
             QString::fromLatin1("off");
}

void RenderRadioButton::restoreState(const QString &state)
{
   static_cast<QRadioButton *>(m_widget)->setChecked(state == QString::fromLatin1("on"));
}

void RenderRadioButton::reset()
{
    setChecked(static_cast<HTMLInputElementImpl*>(m_gform)->checked());
}

void RenderRadioButton::slotClicked()
{
    // ### make radio button groups work properly when not inside a form
    if (m_form)
	m_form->radioClicked(this);
}

// -------------------------------------------------------------------------------


RenderSubmitButton::RenderSubmitButton(QScrollView *view, HTMLFormElementImpl *form,
		      HTMLInputElementImpl *domParent)
    : RenderButton(view, form)
{
    QPushButton *p = new QPushButton(view->viewport());
    setQWidget(p);

    connect(p, SIGNAL(clicked()), this, SLOT(slotClicked()));
    m_clicked = false;
    m_domParent = domParent;
}

RenderSubmitButton::~RenderSubmitButton()
{
}

void RenderSubmitButton::slotClicked()
{
    m_clicked = true;
    m_domParent->mouseEventHandler( 0, DOM::NodeImpl::MouseClick, true );
    if (m_form)
	m_form->submit();
}

QCString RenderSubmitButton::encoding()
{
    QCString encoding;

    if (m_name.isEmpty()) return encoding;

    if (m_clicked)
    {
        if ( m_form->enctype() == "application/x-www-form-urlencoded" )
        {
            encoding = encodeString( m_name.string() );
            encoding += '=';
            encoding += encodeString( m_value.string() );
        }
        else
            encoding = m_value.string().latin1();
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

RenderImageButton::RenderImageButton(QScrollView *view,
				     HTMLFormElementImpl *form,
		      HTMLInputElementImpl *domParent)
    : RenderSubmitButton(view, form, domParent)
{
    image = 0;
}

RenderImageButton::~RenderImageButton()
{
    if(image) image->deref(this);
}

void RenderImageButton::setImageUrl(DOMString url, DOMString baseUrl, DocLoader *docLoader)
{
    kdDebug( 6060 ) << "RenderImageButton::setImageURL" << endl;
    if(image) image->deref(this);
    image = docLoader->requestImage(url, baseUrl);
    image->ref(this);
}

void RenderImageButton::setPixmap( const QPixmap &p, CachedObject *, bool *manualUpdate )
{
    static_cast<QPushButton *>(m_widget)->setPixmap(p);
    // Image dimensions have been changed, recalculate layout
    calcMinMaxWidth(); // does the layout and resizing of the button too.

    if(m_parent) 
    {
       if (manualUpdate)
          *manualUpdate = true;
       else
          updateSize();	
    }
}


// -------------------------------------------------------------------------------

RenderResetButton::RenderResetButton(QScrollView *view, HTMLFormElementImpl *form,
		      HTMLInputElementImpl *domParent)
    : RenderSubmitButton(view, form, domParent)
{
}

RenderResetButton::~RenderResetButton()
{
}

void RenderResetButton::setValue(const DOMString &value)
{
    m_value = value;
    if(m_value != 0)
	static_cast<QPushButton *>(m_widget)->setText(m_value.string());
    else
	static_cast<QPushButton *>(m_widget)->setText(i18n("Reset"));
}

void RenderResetButton::slotClicked()
{
    m_domParent->mouseEventHandler( 0, DOM::NodeImpl::MouseClick, true );
    if (m_form)
	m_form->reset();
}

// -------------------------------------------------------------------------------

RenderPushButton::RenderPushButton(QScrollView *view,
				     HTMLFormElementImpl *form, HTMLInputElementImpl *domParent)
    : RenderSubmitButton(view, form, domParent)
{
}

RenderPushButton::~RenderPushButton()
{
}

void RenderPushButton::setValue(const DOMString &value)
{
    m_value = value;
    if(m_value != 0)
	static_cast<QPushButton *>(m_widget)->setText(m_value.string());
    else
	static_cast<QPushButton *>(m_widget)->setText(i18n(""));
}

void RenderPushButton::slotClicked()
{
    m_domParent->mouseEventHandler( 0, DOM::NodeImpl::MouseClick, true );
}


// -------------------------------------------------------------------------------


RenderLineEdit::RenderLineEdit(QScrollView *view, HTMLFormElementImpl *form,
			       int maxLen, int size, bool passwd)
    : RenderFormElement(view, form)
{
    QLineEdit *edit = new QLineEdit(view);
    connect(edit, SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));

    if(maxLen > 0) edit->setMaxLength(maxLen);
    if(passwd) edit->setEchoMode( QLineEdit::Password );

    m_size = size;
    setQWidget(edit);
}

void RenderLineEdit::slotReturnPressed()
{
    if (m_form)
	m_form->maybeSubmit();
}

void RenderLineEdit::setValue(const DOMString &value)
{
    m_value = value;
    if(m_value != 0)
	static_cast<QLineEdit *>(m_widget)->setText(m_value.string());
}

QCString RenderLineEdit::encoding()
{
    m_value = static_cast<QLineEdit *>(m_widget)->text();
    QCString encoding;
    if (m_name.isEmpty()) return encoding;
    if ( m_form->enctype() == "application/x-www-form-urlencoded" )
    {
        encoding = encodeString( m_name.string() );
        encoding += '=';
        encoding += encodeString( m_value.string() );
    }
    else
        encoding = m_value.string().latin1();

    return encoding;
}

QString RenderLineEdit::state()
{
   // Make sure the string is not empty!
   return static_cast<QLineEdit *>(m_widget)->text()+'.';
}

void RenderLineEdit::restoreState(const QString &state)
{
   static_cast<QLineEdit *>(m_widget)->setText(state.left(state.length()-1));
}

void RenderLineEdit::layout(bool)
{
    QFontMetrics fm( m_widget->font() );
    QSize s;

    int h = fm.height();
    int w = fm.width( 'x' ) * (m_size > 0 ? m_size : 17); // "some"
    if ( static_cast<QLineEdit*>(m_widget)->frame() ) {
        h += 8;
	// ### this is not really portable between all styles.
	// I think one should try to find a generic solution which
	// works with all possible styles. Lars.
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

RenderFieldset::RenderFieldset(QScrollView *view,
                               HTMLFormElementImpl* form)
    : RenderFormElement(view, form)
{
}

RenderFieldset::~RenderFieldset()
{
}


// -------------------------------------------------------------------------


RenderFileButton::RenderFileButton(QScrollView *view,
                         HTMLFormElementImpl* form, int maxLen, int size)
    : RenderFormElement(view, form)
{
    QWidget *w = new QWidget(view->viewport());
    QHBoxLayout *layout = new QHBoxLayout(w);

    m_edit = new QLineEdit(w);
    connect(m_edit, SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));
    m_button = new QPushButton(i18n("Browse..."), w);
    connect(m_button, SIGNAL(clicked()), this, SLOT(slotClicked()));

    if (maxLen > 0) m_edit->setMaxLength(maxLen);

    layout->addWidget(m_edit);
    layout->addWidget(m_button);

    m_size = size;
    setQWidget(w);
}

RenderFileButton::~RenderFileButton()
{
}

void RenderFileButton::slotClicked()
{
    QString file_name = KFileDialog::getOpenFileName(QString::null, QString::null, 0, i18n("Browse..."));
    setValue( DOMString(file_name) );
}

void RenderFileButton::layout( bool )
{
    // this is largely taken from the RenderLineEdit layout
    QFontMetrics fm( m_edit->font() );
    QSize s;

    int h = fm.height();
    int w = fm.width( 'x' ) * (m_size > 0 ? m_size : 17);
    w += m_button->sizeHint().width();

    if ( m_edit->frame() ) {
        h += 8;
        if ( m_widget->style() == QWidget::WindowsStyle && h < 26 )
            h = 22;
        s = QSize( w + 8, h );
    } else
        s = QSize( w + 4, h + 4 );

    m_edit->setReadOnly(m_readonly);

    m_height = s.height();
    m_width  = s.width();

    RenderFormElement::layout(false);
}

QString RenderFileButton::state()
{
    // Make sure the string is not empty!
    return m_edit->text()+'.';
}

void RenderFileButton::restoreState(const QString &state)
{
    setValue( DOMString( state.left(state.length()-1) ) );
}

void RenderFileButton::reset()
{
    setValue( DOMString(QString::null) );
}

void RenderFileButton::slotReturnPressed()
{
    if (m_form)
	m_form->maybeSubmit();
}

void RenderFileButton::setValue(const DOMString &value)
{
    m_value = value;
    if(m_value != 0)
        m_edit->setText(m_value.string());
    else
        m_edit->setText(QString::null);
}

QCString RenderFileButton::encoding()
{
    m_value = m_edit->text();

    QCString encoding;
    if (m_name.isEmpty()) return encoding;
    if ( m_form->enctype() == "application/x-www-form-urlencoded" )
    {
        encoding = encodeString( m_name.string() );
        encoding +=  '=';
        encoding += encodeString( m_edit->text() );
    }
    else
    {
        QString local;
        if ( !KIO::NetAccess::download(KURL(m_value.string()), local) );
        {
            QFile file(local);
            if (file.open(IO_ReadOnly))
            {
                uint size = file.size();
                char *buf = new char[ size ];
                file.readBlock( buf, size );
                file.close();

                encoding.duplicate(buf, size);

                delete[] buf;
            }
            KIO::NetAccess::removeTempFile( local );
        }
    }

    return encoding;
}

// -------------------------------------------------------------------------

RenderLabel::RenderLabel(QScrollView *view,
                         HTMLFormElementImpl* form)
    : RenderFormElement(view, form)
{

}

RenderLabel::~RenderLabel()
{
}


// -------------------------------------------------------------------------

RenderLegend::RenderLegend(QScrollView *view,
                           HTMLFormElementImpl* form)
    : RenderFormElement(view, form)
{
}

RenderLegend::~RenderLegend()
{
}


// -------------------------------------------------------------------------

RenderSelect::RenderSelect(int size, bool multiple,
                           QScrollView *view, HTMLFormElementImpl *form,
                           HTMLSelectElementImpl *domParent)
    : RenderFormElement(view, form)
{
    m_multiple = multiple;
    m_size = size;
    m_domParent = domParent;

    if(m_multiple || m_size > 1) {
        KListBox* w = new KListBox(view);
        w->setSelectionMode(m_multiple ? QListBox::Multi : QListBox::Single);
        setQWidget(w);
        connect(w,SIGNAL(highlighted(int)),this,SLOT(slotActivated(int)));
    }
    else {
        QComboBox *w = new QComboBox(view);
        m_size = 1;
        setQWidget(w);
        connect(w,SIGNAL(activated(int)),this,SLOT(slotActivated(int)));
    }
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

    recalcOptions();

    // Restore state
    QString state = f->ownerDocument()->registerElement(f);
    if ( !state.isEmpty())
    {
       kdDebug( 6040 ) << "Restoring SelectElem name=" << m_name.string() <<
                            " state=" << state << endl;
       restoreState( state );
    }

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

            if(m_multiple || m_size > 1) {
                static_cast<KListBox*>(m_widget)
                    ->setSelected(i, p->selected());
		m_domParent->setSelectedIndex(i,false);
	    }
            else if(p->selected()) {
                static_cast<QComboBox*>(m_widget)->setCurrentItem(i);
		m_domParent->setSelectedIndex(i,false);
                // we only honor the first option that is selected
                // in case there is more than one selected
                break;
            }
        }
        current = current->nextSibling();
        i++;
    }
}

QCString RenderSelect::encoding()
{
    QCString encoding = "";

    if(m_name.isEmpty() || m_gform->disabled()) return encoding;

    QCString prefix;
    if ( m_form->enctype() == "application/x-www-form-urlencoded" )
    {
        prefix = encodeString( m_name.string() );
        prefix += '=';
    }
    if(m_multiple || m_size > 1) {
        KListBox* w = static_cast<KListBox*>(m_widget);
        uint i;
        bool first = true;
        for (i = 0; i < w->count(); i++) {
	    HTMLOptionElementImpl* p = listOptions[i];
            if (w->isSelected(i) && p) {
                if(p->m_value.isNull()) {
                    if(w->item(i))
                        encoding += prefix + encodeString(w->item(i)->text());
                }
                else
                    encoding += prefix + encodeString(p->m_value.string());

                if (first)
                {
                   first = false;
                   prefix = '&' + prefix;
                }
            }
        }
    }
    else
    {
        QComboBox* w = static_cast<QComboBox*>(m_widget);
	HTMLOptionElementImpl* p = listOptions[w->currentItem()];
	if (p) {
	    if(p->m_value.isNull())
		encoding += prefix + encodeString(w->currentText());
	    else
		encoding += prefix + encodeString(p->m_value.string());
	}
    }

    return encoding;
}


QString RenderSelect::state()
{
    QString state;

    if(m_multiple || m_size > 1) {

        KListBox* w = static_cast<KListBox*>(m_widget);

        HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_gform);

        NodeImpl* current = f->firstChild();
        int i = 0;
        while(current) {
            if (current->id() == ID_OPTION)
            {
               if( w->isSelected(i))
                  state += 'X';
               else
                  state += 'O';
            }
            current = current->nextSibling();
            i++;
        }
    }
    else
    {
        QComboBox* w = static_cast<QComboBox*>(m_widget);
        HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_gform);

        NodeImpl* current = f->firstChild();
        int i = 0;
        while(current) {
            if (current->id() == ID_OPTION )
            {
               if( i == w->currentItem())
                  state += 'X';
               else
                  state += 'O';
            }
            current = current->nextSibling();
            i++;
        }
    }

    return state;
}

void RenderSelect::restoreState(const QString &state)
{
    if(m_multiple || m_size > 1) {

        KListBox* w = static_cast<KListBox*>(m_widget);

        HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_gform);

        NodeImpl* current = f->firstChild();
        int i = 0;
        while(current) {
            if (current->id() == ID_OPTION)
            {
               w->setSelected(i, (state[i] == 'X'));
            }
            current = current->nextSibling();
            i++;
        }
    }
    else
    {
        QComboBox* w = static_cast<QComboBox*>(m_widget);
        HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_gform);

        NodeImpl* current = f->firstChild();
        int i = 0;
        while(current) {
            if (current->id() == ID_OPTION )
            {
               if (state[i] == 'X')
                  w->setCurrentItem(i);
            }
            current = current->nextSibling();
            i++;
        }
    }

}

void RenderSelect::setSelectedIndex(long index)
{
    if(m_multiple || m_size > 1) {
        KListBox* w = static_cast<KListBox*>(m_widget);
        w->clearSelection();
        w->setSelected(w->item(index),true);
    }
    else
        static_cast<QComboBox*>(m_widget)->setCurrentItem(index);
    m_domParent->setSelectedIndex(index,false);
}

void RenderSelect::slotActivated(int index)
{
    int optionIndex = 0; // actual index of option not counting OPTGROUP entries that may be in list
    if (!listOptions[index])
	optionIndex = -1;
    else {
	int i;
	for (i = 0; i < index; i++) {
	    if (listOptions[i])
		optionIndex++;
	}
    }
    m_domParent->setSelectedIndex(optionIndex,false);

}

void RenderSelect::recalcOptions()
{
    HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_gform);
    // insert all given <option>'s
    NodeImpl* current = f->firstChild();
    int i = 0;
    bool inOptGroup = false;
    bool finished = false;

    listOptions.clear();
    while(!finished) {
	if (!inOptGroup && current->id() == ID_OPTGROUP && current->firstChild()) {
            if(m_multiple || m_size > 1) {
		// put the optgroup label in the list - ### make this work for combo boxes
		DOMString text = static_cast<HTMLElementImpl*>(current)->getAttribute(ATTR_LABEL);
		if (text.isNull())
		    text = "";	
		QListBoxText *optGroupItem = new QListBoxText(QString(text.implementation()->s, text.implementation()->l));

                static_cast<KListBox*>(m_widget)->insertItem(optGroupItem, i);
		optGroupItem->setSelectable(false);
		i++;
	    }
	
	    current = current->firstChild();
	    inOptGroup = true;
	}
        if (current->id() == ID_OPTION) {
            DOMString text = static_cast<HTMLElementImpl*>(current)->getAttribute(ATTR_LABEL);
            if (text.isNull() && current->firstChild() && current->firstChild()->id() == ID_TEXT)
		text = static_cast<TextImpl *>(current->firstChild())->string();
	    if (text.isNull())
		text = "";
	    if (inOptGroup)
		text = DOMString("    ")+text;

            if(m_multiple || m_size > 1)
                static_cast<KListBox*>(m_widget)
                    ->insertItem(QString(text.implementation()->s, text.implementation()->l), i);
            else
                static_cast<QComboBox*>(m_widget)
                    ->insertItem(QString(text.implementation()->s, text.implementation()->l), i);
            listOptions.insert(i,static_cast<HTMLOptionElementImpl*>(current));
        }
	NodeImpl *parent = current->parentNode();
        current = current->nextSibling();
        i++;
        if (!current) {
	    if (inOptGroup) {
		current = parent->nextSibling();
		inOptGroup = false;
	    }
	    if (!current)
		finished = true;
        }
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

RenderTextArea::RenderTextArea(int wrap, QScrollView *view,
                               HTMLFormElementImpl *form)
    : RenderFormElement(view, form)
{
    TextAreaWidget *edit = new TextAreaWidget(wrap, view);

    setQWidget(edit);
}

void RenderTextArea::layout( bool )
{
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

QCString RenderTextArea::encoding()
{
    QCString encoding;
    if (m_name.isEmpty()) return encoding;

    if ( m_form->enctype() == "application/x-www-form-urlencoded" )
    {
        encoding = encodeString( m_name.string() );
        encoding += '=';
        encoding += encodeString( static_cast<TextAreaWidget *>(m_widget)->text() );
    }
    else
        encoding += static_cast<TextAreaWidget *>(m_widget)->text().latin1();

    return encoding;
}

QString RenderTextArea::state()
{
   // Make sure the string is not empty!
   return static_cast<TextAreaWidget *>(m_widget)->text()+'.';
}

void RenderTextArea::restoreState(const QString &state)
{
   static_cast<TextAreaWidget *>(m_widget)->setText(state.left(state.length()-1));
}


void RenderTextArea::close( )
{
    HTMLTextAreaElementImpl *f = static_cast<HTMLTextAreaElementImpl*>(m_gform);

    if(f->firstChild() && f->firstChild()->id() == ID_TEXT)
        setValue(static_cast<TextImpl*>(f->firstChild())->string());

    reset();
    layout();

    // Restore state
    QString state = f->ownerDocument()->registerElement(f);
    if ( !state.isEmpty())
    {
       kdDebug( 6040 ) << "Restoring TextAreaElem name=" << m_name.string() <<
                            " state=" << state << endl;
       restoreState( state );
    }

    RenderFormElement::close();
}

// ---------------------------------------------------------------------------

#include "render_form.moc"
