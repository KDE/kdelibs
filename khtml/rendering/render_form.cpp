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
#include <iostream.h>

using namespace khtml;

RenderFormElement::RenderFormElement(QScrollView *view,
				     HTMLGenericFormElementImpl *element)
    : RenderWidget(view)
{
    m_element = element;
    m_form = m_element->form();
    if(m_form) m_form->registerFormElement(this);

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
                           HTMLGenericFormElementImpl *element)
    : RenderFormElement(view, element)
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
                                       HTMLInputElementImpl *element)
    : RenderButton(view, element)
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

RenderCheckBox::RenderCheckBox(QScrollView *view,
                               HTMLInputElementImpl *element)
    : RenderButton(view, element)
{
    QCheckBox *b = new QCheckBox(view->viewport());
    setQWidget(b);
    connect(b,SIGNAL(stateChanged(int)),this,SLOT(slotStateChanged(int)));
}

QCString RenderCheckBox::encoding()
{
    QCString encoding;
    if (m_name.isEmpty()) return encoding;
    if (static_cast<HTMLInputElementImpl*>(m_element)->checked())
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

QString RenderCheckBox::state()
{
   return static_cast<HTMLInputElementImpl*>(m_element)->checked() ?
             QString::fromLatin1("on") :
             QString::fromLatin1("off");
}

void RenderCheckBox::restoreState(const QString &state)
{
   static_cast<QCheckBox *>(m_widget)->setChecked(state == QString::fromLatin1("on"));
}

void RenderCheckBox::reset()
{
//    setChecked(static_cast<HTMLInputElementImpl*>(m_element)->checked()); // this changes with javascript!

    // ### allow for resettting to default as specified in html file (not modified by javascript)
    static_cast<QCheckBox*>(m_widget)->setChecked(static_cast<HTMLInputElementImpl*>(m_element)->checked());
}

void RenderCheckBox::layout(bool deep)
{
    static_cast<QCheckBox*>(m_widget)->setChecked(static_cast<HTMLInputElementImpl*>(m_element)->checked());
    RenderButton::layout(deep);
}

void RenderCheckBox::slotStateChanged(int state)
{
    m_element->setAttribute(ATTR_CHECKED,state == 2 ? "" : 0);
}


// -------------------------------------------------------------------------------


RenderRadioButton::RenderRadioButton(QScrollView *view,
				     HTMLInputElementImpl *element)
    : RenderButton(view, element)
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
//    setChecked(static_cast<HTMLInputElementImpl*>(m_element)->checked());

    // ### allow for resettting to default as specified in html file (not modified by javascript)
    m_element->setAttribute(ATTR_CHECKED,0);
}

void RenderRadioButton::slotClicked()
{
    // ### make radio button groups work properly when not inside a form
//    if (m_form)
//	m_form->radioClicked(this);

    m_element->setAttribute(ATTR_CHECKED,"");
}

void RenderRadioButton::layout(bool deep)
{
    static_cast<QRadioButton*>(m_widget)->setChecked(static_cast<HTMLInputElementImpl*>(m_element)->checked());
    RenderButton::layout(deep);
}


// -------------------------------------------------------------------------------


RenderSubmitButton::RenderSubmitButton(QScrollView *view, HTMLInputElementImpl *element)
    : RenderButton(view, element)
{
    QPushButton *p = new QPushButton(view->viewport());
    setQWidget(p);

    connect(p, SIGNAL(clicked()), this, SLOT(slotClicked()));
    m_clicked = false;
}

RenderSubmitButton::~RenderSubmitButton()
{
}

void RenderSubmitButton::slotClicked()
{
    m_clicked = true;
    static_cast<HTMLInputElementImpl*>(m_element)->mouseEventHandler( 0, DOM::NodeImpl::MouseClick, true );
    if (m_form)
	m_form->submit();
}

QCString RenderSubmitButton::encoding()
{
    QCString encoding;

    if (m_name.isEmpty()) return encoding;

    QString value = static_cast<HTMLInputElementImpl*>(m_element)->value().isNull() ?
                    defaultLabel() : static_cast<HTMLInputElementImpl*>(m_element)->value().string();

    if (m_clicked)
    {
        if ( m_form->enctype() == "application/x-www-form-urlencoded" )
        {
            encoding = encodeString( m_name.string() );
            encoding += '=';
            encoding += encodeString( value );
        }
        else
            encoding = value.latin1();
    }

    m_clicked = false;
    return encoding;
}

void RenderSubmitButton::reset()
{
    m_clicked = false;
}

void RenderSubmitButton::layout(bool deep)
{
    QString value = static_cast<HTMLInputElementImpl*>(m_element)->value().isNull() ?
                    defaultLabel() : static_cast<HTMLInputElementImpl*>(m_element)->value().string();
    static_cast<QPushButton*>(m_widget)->setText(value);
    RenderButton::layout(deep);
}

QString RenderSubmitButton::defaultLabel() {
  return i18n("Submit");
}


// -------------------------------------------------------------------------------

RenderImageButton::RenderImageButton(QScrollView *view,
				     HTMLInputElementImpl *element)
    : RenderSubmitButton(view, element)
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
    
    if(m_parent) 
    {
        if (manualUpdate)
        {
            if (*manualUpdate)    
                updateSize();
            else
                *manualUpdate = true;
        }
        else
            updateSize();	
    }
}


// -------------------------------------------------------------------------------

RenderResetButton::RenderResetButton(QScrollView *view, HTMLInputElementImpl *element)
    : RenderSubmitButton(view, element)
{
}

RenderResetButton::~RenderResetButton()
{
}

void RenderResetButton::slotClicked()
{
    static_cast<HTMLInputElementImpl*>(m_element)->mouseEventHandler( 0, DOM::NodeImpl::MouseClick, true );
    if (m_form)
	m_form->reset();
}

QString RenderResetButton::defaultLabel() {
  return i18n("Reset");
}


// -------------------------------------------------------------------------------

RenderPushButton::RenderPushButton(QScrollView *view, HTMLInputElementImpl *element)
    : RenderSubmitButton(view, element)
{
}

RenderPushButton::~RenderPushButton()
{
}

void RenderPushButton::slotClicked()
{
    static_cast<HTMLInputElementImpl*>(m_element)->mouseEventHandler( 0, DOM::NodeImpl::MouseClick, true );
}


QString RenderPushButton::defaultLabel() {
  // why doesn't QString("") work? :(
  QString s;
  s.sprintf("");
  return s;
}

// -------------------------------------------------------------------------------


RenderLineEdit::RenderLineEdit(QScrollView *view, HTMLInputElementImpl *element)
    : RenderFormElement(view, element)
{
    QLineEdit *edit = new QLineEdit(view);
    connect(edit, SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));
    connect(edit, SIGNAL(textChanged(const QString &)),this,SLOT(slotTextChanged(const QString &)));

    if(element->type() == "PASSWORD") edit->setEchoMode( QLineEdit::Password );

    setQWidget(edit);
}

void RenderLineEdit::slotReturnPressed()
{
    if (m_form)
	m_form->maybeSubmit();
}

QCString RenderLineEdit::encoding()
{
    DOMString val = static_cast<HTMLInputElementImpl*>(m_element)->value();
    QCString encoding;
    if (m_name.isEmpty()) return encoding;
    if ( m_form->enctype() == "application/x-www-form-urlencoded" )
    {
        encoding = encodeString( m_name.string() );
        encoding += '=';
        encoding += encodeString( val.string() );
    }
    else
        encoding = val.string().latin1();

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

    QLineEdit *edit = static_cast<QLineEdit*>(m_widget);
    HTMLInputElementImpl *input = static_cast<HTMLInputElementImpl*>(m_element);

    int size = input->size();

    int h = fm.height();
    int w = fm.width( 'x' ) * (size > 0 ? size : 17); // "some"
    if ( edit->frame() ) {
        h += 8;
	// ### this is not really portable between all styles.
	// I think one should try to find a generic solution which
	// works with all possible styles. Lars.
        if ( m_widget->style() == QWidget::WindowsStyle && h < 26 )
            h = 22;
        s = QSize( w + 8, h );
    } else
        s = QSize( w + 4, h + 4 );

    edit->setText(static_cast<HTMLInputElementImpl*>(m_element)->value().string());
    // ### what if maxlength goes back to 0? can we unset maxlength in the widget?
    if (input->maxLength() > 0)
	edit->setMaxLength(input->maxLength());
    edit->setReadOnly(m_readonly);

    m_height = s.height();
    m_width = s.width();

    RenderFormElement::layout(false);
}

void RenderLineEdit::reset()
{
    // ### I don't think this is correct - supposed to be *default* value
    if(static_cast<HTMLInputElementImpl*>(m_element)->value() != 0)
	static_cast<QLineEdit *>(m_widget)->setText(static_cast<HTMLInputElementImpl*>(m_element)->value().string());
    else
	static_cast<QLineEdit *>(m_widget)->setText(QString::null);
}

void RenderLineEdit::slotTextChanged(const QString &string)
{
    static_cast<HTMLInputElementImpl*>(m_element)->setAttribute(ATTR_VALUE,DOMString(string));
}


// ---------------------------------------------------------------------------

RenderFieldset::RenderFieldset(QScrollView *view,
                               HTMLGenericFormElementImpl *element)
    : RenderFormElement(view, element)
{
}

RenderFieldset::~RenderFieldset()
{
}


// -------------------------------------------------------------------------


RenderFileButton::RenderFileButton(QScrollView *view, HTMLInputElementImpl *element)
    : RenderFormElement(view, element)
{
    QWidget *w = new QWidget(view->viewport());
    QHBoxLayout *layout = new QHBoxLayout(w);

    m_edit = new QLineEdit(w);
    connect(m_edit, SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));
    connect(m_edit, SIGNAL(textChanged(const QString &)),this,SLOT(slotTextChanged(const QString &)));
    m_button = new QPushButton(i18n("Browse..."), w);
    connect(m_button, SIGNAL(clicked()), this, SLOT(slotClicked()));

    if (element->maxLength() > 0) m_edit->setMaxLength(element->maxLength());

    layout->addWidget(m_edit);
    layout->addWidget(m_button);

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
    HTMLInputElementImpl *input = static_cast<HTMLInputElementImpl*>(m_element);
    int size = input->size();

    int h = fm.height();
    int w = fm.width( 'x' ) * (size > 0 ? size : 17);
    w += m_button->sizeHint().width();

    if ( m_edit->frame() ) {
        h += 8;
        if ( m_widget->style() == QWidget::WindowsStyle && h < 26 )
            h = 22;
        s = QSize( w + 8, h );
    } else
        s = QSize( w + 4, h + 4 );

    m_edit->setText(input->value().string());
    m_edit->setMaxLength(input->maxLength());

    m_edit->setReadOnly(m_readonly);

    m_height = s.height();
    m_width  = s.width();

    RenderFormElement::layout(false);
}

QString RenderFileButton::state()
{
    // Make sure the string is not empty!
    return static_cast<HTMLInputElementImpl*>(m_element)->value().string()+'.';
}

void RenderFileButton::restoreState(const QString &state)
{
    m_element->setAttribute(ATTR_VALUE,DOMString(state.left(state.length()-1)));
}

void RenderFileButton::reset()
{
    m_element->setAttribute(ATTR_VALUE,DOMString(QString::null));
}

void RenderFileButton::slotReturnPressed()
{
    if (m_form)
	m_form->maybeSubmit();
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
        encoding += encodeString( static_cast<HTMLInputElementImpl*>(m_element)->value().string() );
    }
    else
    {
        QString local;
        if ( !KIO::NetAccess::download(KURL(static_cast<HTMLInputElementImpl*>(m_element)->value().string()), local) );
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

void RenderFileButton::slotTextChanged(const QString &string)
{
    m_element->setAttribute(ATTR_VALUE,DOMString(string));
}

// -------------------------------------------------------------------------

RenderLabel::RenderLabel(QScrollView *view,
                         HTMLGenericFormElementImpl *element)
    : RenderFormElement(view, element)
{

}

RenderLabel::~RenderLabel()
{
}


// -------------------------------------------------------------------------

RenderLegend::RenderLegend(QScrollView *view,
                           HTMLGenericFormElementImpl *element)
    : RenderFormElement(view, element)
{
}

RenderLegend::~RenderLegend()
{
}


// -------------------------------------------------------------------------

RenderSelect::RenderSelect(QScrollView *view, HTMLSelectElementImpl *element)
    : RenderFormElement(view, element)
{
    m_multiple = element->multiple();
    m_size = element->size();

    m_listBox = (m_multiple || m_size > 1);

    if(m_listBox) {
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
    m_optionsChanged = true; // options will be inserted on first layout
    m_selectedIndex = element->selectedIndex();
}

void RenderSelect::layout( bool )
{
    // ### maintain selection properly between type/size changes, and work
    // out how to handle multiselect->singleselect (probably just select
    // first selected one)

    HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_element);
    bool forceSelectionRecalc = false;

    // change widget type

    bool oldMultiple = m_multiple;
    unsigned oldSize = m_size;
    bool oldListbox = m_listBox;

    m_multiple = f->multiple();
    m_size = f->size();
    m_listBox = (m_multiple || m_size > 1);

    if (oldMultiple != m_multiple || oldSize != m_size) {
	if (m_listBox != oldListbox) {
	    // type of select has changed
	
	    if(oldListbox)
		disconnect(static_cast<KListBox*>(m_widget),SIGNAL(highlighted(int)),this,SLOT(slotActivated(int)));
	    else
		disconnect(static_cast<QComboBox*>(m_widget),SIGNAL(activated(int)),this,SLOT(slotActivated(int)));
	    delete m_widget;

	    if(m_listBox) {
		KListBox* w = new KListBox(m_view);
		w->setSelectionMode(m_multiple ? QListBox::Multi : QListBox::Single);
		setQWidget(w);
		connect(w,SIGNAL(highlighted(int)),this,SLOT(slotActivated(int)));
	    }
	    else {
		QComboBox *w = new QComboBox(m_view);
		m_size = 1;
		setQWidget(w);
		connect(w,SIGNAL(activated(int)),this,SLOT(slotActivated(int)));
	    }
	}

	if (m_listBox && oldMultiple != m_multiple) {
	    static_cast<KListBox*>(m_widget)->setSelectionMode(m_multiple ? QListBox::Multi : QListBox::Single);
	}
    }

    if (m_optionsChanged) {
	forceSelectionRecalc = true;
	// insert all given <option>'s

	NodeImpl* current = f->firstChild();
	int i = 0;
	bool inOptGroup = false;

	listOptions.clear();
	if(m_listBox)
	    static_cast<KListBox*>(m_widget)->clear();
	else
	    static_cast<QComboBox*>(m_widget)->clear();

	while(current) {
	    if (!inOptGroup && current->id() == ID_OPTGROUP && current->firstChild()) {
		if (m_listBox) {
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

		if(m_listBox)
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
	    }
	}
	m_optionsChanged = false;
    }

    if (m_selectedIndex != static_cast<HTMLSelectElementImpl*>(m_element)->selectedIndex() ||
	forceSelectionRecalc) {
	m_selectedIndex = static_cast<HTMLSelectElementImpl*>(m_element)->selectedIndex();
	// selectedIndex changed since last update
	if (m_listBox) {
	    // if multi-select, we select only the new selected index
	    int listIndex;
	    KListBox *listBox = static_cast<KListBox*>(m_widget);
	    for (listIndex = 0; listIndex < int(listBox->count()); listIndex++)
		listBox->setSelected(listIndex,0);
	    listBox->setSelected(optionToListIndex(m_selectedIndex),true);
	}
	else {
	    int listIndex = optionToListIndex(m_selectedIndex);
	    static_cast<QComboBox*>(m_widget)->setCurrentItem(listIndex >= 0 ? listIndex : 0);
	}
    }

    if (m_listBox) {
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

//    reset();


    // calculate size

    if(m_listBox) {
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
    HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_element);

    // Restore state
    QString state = f->ownerDocument()->registerElement(f);
    if ( !state.isEmpty())
    {
       kdDebug( 6040 ) << "Restoring SelectElem name=" << m_name.string() <<
                            " state=" << state << endl;
       restoreState( state );
    }
    setLayouted(false);

    RenderFormElement::close();
}


void RenderSelect::reset()
{
    HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_element);

    NodeImpl* current = f->firstChild();
    int i = 0;

    while(current) {
        if(current->id() == ID_OPTION &&
           current->firstChild() && current->firstChild()->id() == ID_TEXT) {

            HTMLOptionElementImpl* p = static_cast<HTMLOptionElementImpl*>(current);

            if (m_listBox) {
                // we only honor the first option that is selected
                // in case there is more than one selected
                static_cast<KListBox*>(m_widget)
                    ->setSelected(i, p->selected());
		static_cast<HTMLSelectElementImpl*>(m_element)->setSelectedIndex(i);
	    }
            else if(p->selected()) {
                static_cast<QComboBox*>(m_widget)->setCurrentItem(i);
		static_cast<HTMLSelectElementImpl*>(m_element)->setSelectedIndex(i);
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

    if(m_name.isEmpty() || m_element->disabled()) return encoding;

    QCString prefix;
    if ( m_form->enctype() == "application/x-www-form-urlencoded" )
    {
        prefix = encodeString( m_name.string() );
        prefix += '=';
    }
    if (m_listBox) {
        KListBox* w = static_cast<KListBox*>(m_widget);
        uint i;
        bool first = true;
        for (i = 0; i < w->count(); i++) {
	    HTMLOptionElementImpl* p = listOptions[i];
            if (w->isSelected(i) && p) {
                if(p->value().isNull()) {
                    if(w->item(i))
                        encoding += prefix + encodeString(w->item(i)->text());
                }
                else
                    encoding += prefix + encodeString(p->value().string());

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
	    if(p->value().isNull())
		encoding += prefix + encodeString(w->currentText());
	    else
		encoding += prefix + encodeString(p->value().string());
	}
    }

    return encoding;
}


QString RenderSelect::state()
{
    QString state;

    if(m_listBox) {

        KListBox* w = static_cast<KListBox*>(m_widget);

        HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_element);

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
        HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_element);

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
    if (m_listBox) {

        KListBox* w = static_cast<KListBox*>(m_widget);

        HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_element);

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
        HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_element);

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

void RenderSelect::slotActivated(int index)
{
    static_cast<HTMLSelectElementImpl*>(m_element)->setSelectedIndex(listToOptionIndex(index));
}

int RenderSelect::optionToListIndex(int optionIndex)
{
    if (optionIndex < 0 || optionIndex >= int(listOptions.count()))
	return -1;
	
    int listIndex = 0;
    int optionIndex2 = 0;
    for (;
	 optionIndex2 < int(listOptions.count()) && optionIndex2 <= optionIndex;
	 listIndex++) { // not a typo!
	if (listOptions[listIndex])
	    optionIndex2++;
    }
    listIndex--;
    return listIndex;
}

int RenderSelect::listToOptionIndex(int listIndex)
{
    if (!listOptions[listIndex])
	return -1;

    int optionIndex = 0; // actual index of option not counting OPTGROUP entries that may be in list
    int i;
    for (i = 0; i < listIndex; i++)
	if (listOptions[i])
	    optionIndex++;
    return optionIndex;
}


void RenderSelect::setOptionsChanged(bool _optionsChanged)
{
    m_optionsChanged = _optionsChanged;
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

// ### allow contents to be manipulated via DOM - will require updating
// of text node child

RenderTextArea::RenderTextArea(QScrollView *view, HTMLTextAreaElementImpl *element)
    : RenderFormElement(view, element)
{
    TextAreaWidget *edit = new TextAreaWidget(element->wrap(), view);

    setQWidget(edit);
}

void RenderTextArea::layout( bool )
{
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    HTMLTextAreaElementImpl* f = static_cast<HTMLTextAreaElementImpl*>(m_element);

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
    HTMLTextAreaElementImpl *f = static_cast<HTMLTextAreaElementImpl*>(m_element);

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
