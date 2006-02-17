/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
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

#undef FORMS_DEBUG
//#define FORMS_DEBUG

#include "html/html_formimpl.h"

#include "khtmlview.h"
#include "khtml_part.h"
#include "html/html_documentimpl.h"
#include "khtml_settings.h"
#include "misc/htmlhashes.h"

#include "css/cssstyleselector.h"
#include "css/cssproperties.h"
#include "css/cssvalues.h"
#include "css/csshelper.h"
#include "xml/dom_textimpl.h"
#include "xml/dom_docimpl.h"
#include "xml/dom2_eventsimpl.h"
#include "khtml_ext.h"

#include "rendering/render_form.h"

#include <kcharsets.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kmimetype.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <klocale.h>
#ifndef KHTML_NO_WALLET
#include <kwallet.h>
#endif
#include <netaccess.h>
#include <kfileitem.h>
#include <qfile.h>
#include <qdir.h>
#include <qtextcodec.h>

// for keygen
#include <qstring.h>
#include <ksslkeygen.h>

#include <assert.h>


using namespace DOM;
using namespace khtml;

HTMLFormElementImpl::HTMLFormElementImpl(DocumentPtr *doc, bool implicit)
    : HTMLElementImpl(doc)
{
    m_implicit = implicit;
    m_post = false;
    m_multipart = false;
    m_autocomplete = true;
    m_insubmit = false;
    m_doingsubmit = false;
    m_inreset = false;
    m_enctype = "application/x-www-form-urlencoded";
    m_boundary = "----------" + KApplication::randomString( 42 + 13 );
    m_acceptcharset = "UNKNOWN";
    m_malformed = false;
}

HTMLFormElementImpl::~HTMLFormElementImpl()
{
    if (getDocument() && getDocument()->view() && getDocument()->view()->part()) {
        getDocument()->view()->part()->dequeueWallet(this);
    }
    QPtrListIterator<HTMLGenericFormElementImpl> it(formElements);
    for (; it.current(); ++it)
        it.current()->m_form = 0;
    QPtrListIterator<HTMLImageElementImpl> it2(imgElements);
    for (; it2.current(); ++it2)
        it2.current()->m_form = 0;
}

NodeImpl::Id HTMLFormElementImpl::id() const
{
    return ID_FORM;
}

long HTMLFormElementImpl::length() const
{
    int len = 0;
    QPtrListIterator<HTMLGenericFormElementImpl> it(formElements);
    for (; it.current(); ++it)
	if (it.current()->isEnumeratable())
	    ++len;

    return len;
}

static QCString encodeCString(const QCString& e)
{
    // http://www.w3.org/TR/html4/interact/forms.html#h-17.13.4.1
    // safe characters like NS handles them for compatibility
    static const char *safe = "-._*";
    QCString encoded(( e.length()+e.contains( '\n' ) )*3
                     +e.contains('\r') * 3 + 1);
    int enclen = 0;
    bool crmissing = false;
    unsigned char oldc;
    unsigned char c ='\0';

    //QCString orig(e.data(), e.size());

    unsigned len = e.length();
    for(unsigned pos = 0; pos < len; pos++) {
        oldc = c;
        c = e[pos];

        if (crmissing && c != '\n') {
            encoded[enclen++] = '%';
            encoded[enclen++] = '0';
            encoded[enclen++] = 'D';
            crmissing = false;
        }

        if ( (( c >= 'A') && ( c <= 'Z')) ||
             (( c >= 'a') && ( c <= 'z')) ||
             (( c >= '0') && ( c <= '9')) ||
             (strchr(safe, c))
            )
            encoded[enclen++] = c;
        else if ( c == ' ' )
            encoded[enclen++] = '+';
        else if ( c == '\n' )
        {
            encoded[enclen++] = '%';
            encoded[enclen++] = '0';
            encoded[enclen++] = 'D';
            encoded[enclen++] = '%';
            encoded[enclen++] = '0';
            encoded[enclen++] = 'A';
            crmissing = false;
        }
        else if (c == '\r' && oldc != '\n') {
            crmissing = true;
        }
        else if ( c != '\r' )
        {
            encoded[enclen++] = '%';
            unsigned int h = c / 16;
            h += (h > 9) ? ('A' - 10) : '0';
            encoded[enclen++] = h;

            unsigned int l = c % 16;
            l += (l > 9) ? ('A' - 10) : '0';
            encoded[enclen++] = l;
        }
    }
    encoded[enclen++] = '\0';
    encoded.truncate(enclen);

    return encoded;
}

// ### This function only encodes to numeric ampersand escapes,
// ### we could use standard ampersand values as well.
inline static QString escapeUnencodeable(const QTextCodec* codec, const QString& s) {
    QString enc_string;
    const int len = s.length();
    for(int i=0; i <len; ++i) {
        const QChar c = s[i];
        if (codec->canEncode(c))
            enc_string.append(c);
        else {
            QString ampersandEscape;
            ampersandEscape.sprintf("&#%u;", c.unicode());
            enc_string.append(ampersandEscape);
        }
    }
    return enc_string;
}

inline static QCString fixUpfromUnicode(const QTextCodec* codec, const QString& s)
{
    QCString str = codec->fromUnicode(escapeUnencodeable(codec,s));
    str.truncate(str.length());
    return str;
}

QByteArray HTMLFormElementImpl::formData(bool& ok)
{
#ifdef FORMS_DEBUG
    kdDebug( 6030 ) << "form: formData()" << endl;
#endif

    QByteArray form_data(0);
    QCString enc_string = ""; // used for non-multipart data

    // find out the QTextcodec to use
    const QString str = m_acceptcharset.string();
    const QChar space(' ');
    const unsigned int strLength = str.length();
    for(unsigned int i=0; i < strLength; ++i) if(str[i].latin1() == ',') str[i] = space;
    const QStringList charsets = QStringList::split(' ', str);
    QTextCodec* codec = 0;
    KHTMLView *view = getDocument()->view();
    {
        QStringList::ConstIterator it = charsets.begin();
        const QStringList::ConstIterator itEnd = charsets.end();

        for ( ; it != itEnd; ++it )
        {
            QString enc = (*it);
            if(enc.contains("UNKNOWN"))
            {
                // use standard document encoding
                enc = "ISO 8859-1";
                if(view && view->part())
                    enc = view->part()->encoding();
            }
            if((codec = KGlobal::charsets()->codecForName(enc.latin1())))
                break;
        }
    }
    if(!codec)
        codec = QTextCodec::codecForLocale();

    // we need to map visual hebrew to logical hebrew, as the web
    // server alsways expects responses in logical ordering
    if ( codec->mibEnum() == 11 )
	codec = QTextCodec::codecForMib( 85 );

    m_encCharset = codec->name();
    const unsigned int m_encCharsetLength = m_encCharset.length();
    for(unsigned int i=0; i < m_encCharsetLength; ++i)
        m_encCharset[i] = m_encCharset[i].latin1() == ' ' ? QChar('-') : m_encCharset[i].lower();

    QStringList fileUploads, fileNotUploads;

    for (QPtrListIterator<HTMLGenericFormElementImpl> it(formElements); it.current(); ++it) {
        HTMLGenericFormElementImpl* const current = it.current();
        khtml::encodingList lst;

        if (!current->disabled() && current->encoding(codec, lst, m_multipart))
        {
            //kdDebug(6030) << "adding name '" << current->name().string() << "'" << endl;
            khtml::encodingList::ConstIterator it = lst.begin();
            const khtml::encodingList::ConstIterator itEnd = lst.end();
            for( it = lst.begin(); it != itEnd; ++it )
            {
                if (!m_multipart)
                {
                    // handle ISINDEX / <input name=isindex> special
                    // but only if its the first entry
                    if ( enc_string.isEmpty() && *it == "isindex" ) {
                        ++it;
                        enc_string += encodeCString( *it );
                    }
                    else {
                        if(!enc_string.isEmpty())
                            enc_string += '&';

                        enc_string += encodeCString(*it);
                        enc_string += "=";
                        ++it;
                        enc_string += encodeCString(*it);
                    }
                }
                else
                {
                    QCString hstr("--");
                    hstr += m_boundary.latin1();
                    hstr += "\r\n";
                    hstr += "Content-Disposition: form-data; name=\"";
                    hstr += (*it).data();
                    hstr += "\"";

                    // if the current type is FILE, then we also need to
                    // include the filename
                    if (current->id() == ID_INPUT &&
                        static_cast<HTMLInputElementImpl*>(current)->inputType() == HTMLInputElementImpl::FILE &&
                        current->renderer())
                    {
                        KURL path;
                        QString val = static_cast<HTMLInputElementImpl*>(current)->value().string().stripWhiteSpace();
                        if (!val.isEmpty() &&
                            QDir::isRelativePath(val) &&
                            QFile::exists(KGlobalSettings::documentPath() + val)) {
                            path.setPath(KGlobalSettings::documentPath() + val);
                        } else {
                            path = KURL::fromPathOrURL(val);
                        }

                        hstr += fixUpfromUnicode(codec, "; filename=\"" + path.fileName() + "\"");
                        if (path.isValid()) {
                            fileUploads << path.prettyURL(0, KURL::StripFileProtocol);
                            const KMimeType::Ptr ptr = KMimeType::findByURL(path);
                            if (!ptr->name().isEmpty()) {
                                hstr += "\r\nContent-Type: ";
                                hstr += ptr->name().ascii();
                            }
                        } else if (!val.isEmpty()) {
                            fileNotUploads << path.prettyURL(0, KURL::StripFileProtocol);
                        }
                    }

                    hstr += "\r\n\r\n";
                    ++it;

                    // append body
                    const unsigned int old_size = form_data.size();
		    form_data.resize( old_size + hstr.length() + (*it).size() + 1);
                    memcpy(form_data.data() + old_size, hstr.data(), hstr.length());
		    memcpy(form_data.data() + old_size + hstr.length(), *it, (*it).size());
                    form_data[form_data.size()-2] = '\r';
                    form_data[form_data.size()-1] = '\n';

		    // reset unsubmittedFormChange flag
                    if (current->id() == ID_INPUT &&
                        static_cast<HTMLInputElementImpl*>(current)->inputType() == HTMLInputElementImpl::TEXT)
                        static_cast<HTMLInputElementImpl*>(current)->setUnsubmittedFormChange(false);

                    if (current->id() == ID_TEXTAREA)
                        static_cast<HTMLTextAreaElementImpl*>(current)->setUnsubmittedFormChange(false);

                }
            }
        }
    }

    if (fileNotUploads.count()) {
        const int result = KMessageBox::warningContinueCancelList( 0,
                                                             i18n("The following files will not be uploaded"
                                                                  " because they could not be found.\n"
                                                                  "Do you want to continue?"),
                                                             fileNotUploads,
                                                             i18n("Submit Confirmation"),KGuiItem(i18n("&Submit Anyway")));


        if (result == KMessageBox::Cancel) {
            ok = false;
            return QByteArray();
        }
    }

    if (fileUploads.count()) {
        const int result = KMessageBox::warningContinueCancelList( 0,
                                                             i18n("You're about to transfer the following files from "
                                                                  "your local computer to the Internet.\n"
                                                                  "Do you really want to continue?"),
                                                             fileUploads,
                                                             i18n("Send Confirmation"),KGuiItem(i18n("&Send Files")));


        if (result == KMessageBox::Cancel) {
            ok = false;
            return QByteArray();
        }
    }

    if (m_multipart)
        enc_string = ("--" + m_boundary + "--\r\n").ascii();

    const int old_size = form_data.size();
    form_data.resize( form_data.size() + enc_string.length() );
    memcpy(form_data.data() + old_size, enc_string.data(), enc_string.length() );

    ok = true;
    return form_data;
}

void HTMLFormElementImpl::setEnctype( const DOMString& type )
{
    if(type.string().find("multipart", 0, false) != -1 || type.string().find("form-data", 0, false) != -1)
    {
        m_enctype = "multipart/form-data";
        m_multipart = true;
        m_post = true;
    } else if (type.string().find("text", 0, false) != -1 || type.string().find("plain", 0, false) != -1)
    {
        m_enctype = "text/plain";
        m_multipart = false;
    }
    else
    {
        m_enctype = "application/x-www-form-urlencoded";
        m_multipart = false;
    }
    m_encCharset = QString::null;
}

static QString calculateAutoFillKey(const HTMLFormElementImpl& e)
{
    KURL k(e.getDocument()->URL());
    k.setRef(QString::null);
    k.setQuery(QString::null);
    // ensure that we have the user / password inside the url
    // otherwise we might have a potential security problem
    // by saving passwords under wrong lookup key.
    const QString name = e.getAttribute(ATTR_NAME).string().stripWhiteSpace();
    const QRegExp re("[;,!]");
    const QStringList url = QStringList::split(re, k.url());
    return url[0] + '#' + name;
}

void HTMLFormElementImpl::doAutoFill()
{
#ifndef KHTML_NO_WALLET
    const QString key = calculateAutoFillKey(*this);

    if (KWallet::Wallet::keyDoesNotExist(KWallet::Wallet::NetworkWallet(),
                                         KWallet::Wallet::FormDataFolder(),
                                         key))
        return;

    // assert(view())
    getDocument()->view()->part()->openWallet(this);
#endif // KHTML_NO_WALLET
}


void HTMLFormElementImpl::walletOpened(KWallet::Wallet *w) {
#ifndef KHTML_NO_WALLET
    assert(w);
    const QString key = calculateAutoFillKey(*this);
    if (!w->hasFolder(KWallet::Wallet::FormDataFolder())) {
        return; // failed
    }
    w->setFolder(KWallet::Wallet::FormDataFolder());
    QMap<QString, QString> map;
    if (w->readMap(key, map))
        return; // failed, abort

    for (QPtrListIterator<HTMLGenericFormElementImpl> it(formElements); it.current(); ++it) {
        if (it.current()->id() == ID_INPUT) {
            HTMLInputElementImpl* const current = static_cast<HTMLInputElementImpl*>(it.current());
            if ((current->inputType() == HTMLInputElementImpl::PASSWORD ||
                    current->inputType() == HTMLInputElementImpl::TEXT) &&
                    !current->readOnly() &&
                    map.contains(current->name().string())) {
                getDocument()->setFocusNode(current);
                current->setValue(map[current->name().string()]);
            }
        }
    }
#endif // KHTML_NO_WALLET
}

void HTMLFormElementImpl::submitFromKeyboard()
{
    // Activate the first nondisabled submit button
    // if there is none, do a submit anyway if not more
    // than one <input type=text> or <input type=password>
    unsigned int inputtext = 0;
    for (QPtrListIterator<HTMLGenericFormElementImpl> it(formElements); it.current(); ++it) {
        if (it.current()->id() == ID_BUTTON) {
            HTMLButtonElementImpl* const current = static_cast<HTMLButtonElementImpl *>(it.current());
            if (current->buttonType() == HTMLButtonElementImpl::SUBMIT && !current->disabled()) {
                current->click();
                return;
            }
        } else if (it.current()->id() == ID_INPUT) {
            HTMLInputElementImpl* const current = static_cast<HTMLInputElementImpl *>(it.current());
            switch(current->inputType())  {
            case HTMLInputElementImpl::SUBMIT:
            case HTMLInputElementImpl::IMAGE:
		if(!current->disabled()) {
			current->click();
			return;
		}
		break;
            case HTMLInputElementImpl::TEXT:
            case HTMLInputElementImpl::PASSWORD:
                ++inputtext;
            default:
                break;
            }
        }
    }

    if (inputtext <= 1)
        prepareSubmit();
}


void HTMLFormElementImpl::gatherWalletData()
{
#ifndef KHTML_NO_WALLET
    KHTMLView* const view = getDocument()->view();
    // check if we have any password input's
    m_walletMap.clear();
    m_havePassword = false;
    m_haveTextarea = false;
    const KURL formUrl(getDocument()->URL());
    if (!view->nonPasswordStorableSite(formUrl.host())) {
        for (QPtrListIterator<HTMLGenericFormElementImpl> it(formElements); it.current(); ++it) {
            if (it.current()->id() == ID_INPUT)  {
                HTMLInputElementImpl* const c = static_cast<HTMLInputElementImpl*> (it.current());
                if ((c->inputType() == HTMLInputElementImpl::TEXT ||
                        c->inputType() == HTMLInputElementImpl::PASSWORD) &&
                        !c->readOnly())  {
                    m_walletMap.insert(c->name().string(), c->value().string());
                    if (c->inputType() == HTMLInputElementImpl::PASSWORD &&
                            !c->value().isEmpty())
                        m_havePassword = true;
                }
            }
            else if (it.current()->id() == ID_TEXTAREA)
                m_haveTextarea = true;
        }
    }
#endif // KHTML_NO_WALLET
}


bool HTMLFormElementImpl::prepareSubmit()
{
    KHTMLView* const view = getDocument()->view();
    if(m_insubmit || !view || !view->part() || view->part()->onlyLocalReferences())
        return m_insubmit;

    gatherWalletData();

    m_insubmit = true;
    m_doingsubmit = false;

    if ( dispatchHTMLEvent(EventImpl::SUBMIT_EVENT,true,true) && !m_doingsubmit )
        m_doingsubmit = true;

    m_insubmit = false;

    if ( m_doingsubmit )
        submit();

    return m_doingsubmit;
}

void HTMLFormElementImpl::submit(  )
{
    if ( m_insubmit ) {
        m_doingsubmit = true;
        return;
    }

    m_insubmit = true;

#ifdef FORMS_DEBUG
    kdDebug( 6030 ) << "submitting!" << endl;
#endif

    bool ok;
    KHTMLView* const view = getDocument()->view();
    const QByteArray form_data = formData(ok);
    const KURL formUrl(getDocument()->URL());

    if (ok && view) {
        if (m_walletMap.isEmpty()) {
            gatherWalletData();
        }
#ifndef KHTML_NO_WALLET
        if (m_havePassword && !m_haveTextarea && KWallet::Wallet::isEnabled()) {
            const QString key = calculateAutoFillKey(*this);
            const bool doesnotexist = KWallet::Wallet::keyDoesNotExist(KWallet::Wallet::NetworkWallet(), KWallet::Wallet::FormDataFolder(), key);
            KWallet::Wallet* const w = view->part()->wallet();
            bool login_changed = false;

            if (!doesnotexist && w) {
                // check if the login information changed from what
                // we had so far.
                if (w->hasFolder(KWallet::Wallet::FormDataFolder())) {
                    w->setFolder(KWallet::Wallet::FormDataFolder());
                    QMap<QString, QString> map;
                    if (!w->readMap(key, map)) {
                        QMapConstIterator<QString, QString> it = map.begin();
                        const QMapConstIterator<QString, QString> itEnd = map.end();
                        for ( ; it != itEnd; ++it )
                            if ( map[it.key()] != m_walletMap[it.key()] ) {
                                login_changed = true;
                                break;
                            }
                    } else {
                        login_changed = true;
                    }
                }
            }

            if ( doesnotexist || !w || login_changed ) {
                // TODO use KMessageBox::questionYesNoCancel() again, if you can pass a KGuiItem for Cancel
                KDialogBase* const dialog = new KDialogBase(i18n("Save Login Information"),
                                                          KDialogBase::Yes | KDialogBase::No | KDialogBase::Cancel,
                                                          KDialogBase::Yes, KDialogBase::Cancel,
                                                          0, "questionYesNoCancel", true, true,
                                                          i18n("Store"), KGuiItem(i18n("Ne&ver for This Site")), i18n("Do Not Store"));

                bool checkboxResult = false;
                const int savePassword = KMessageBox::createKMessageBox(dialog, QMessageBox::Information,
                                                                            formUrl.host().isEmpty() ? // e.g. local file
                                                                            i18n("Konqueror has the ability to store the password "
                                                                                 "in an encrypted wallet. When the wallet is unlocked, it "
                                                                                 "can then automatically restore the login information "
                                                                                 "next time you submit this form. Do you want to store "
                                                                                 "the information now?") :
                                                                            i18n("Konqueror has the ability to store the password "
                                                                                 "in an encrypted wallet. When the wallet is unlocked, it "
                                                                                 "can then automatically restore the login information "
                                                                                 "next time you visit %1. Do you want to store "
                                                                                 "the information now?").arg(formUrl.host()),
                                                                            QStringList(), QString::null, &checkboxResult, KMessageBox::Notify);

                if ( savePassword == KDialogBase::Yes ) {
                    // ensure that we have the user / password inside the url
                    // otherwise we might have a potential security problem
                    // by saving passwords under wrong lookup key.

                    getDocument()->view()->part()->saveToWallet(key, m_walletMap);
                } else if ( savePassword == KDialogBase::No ) {
                    view->addNonPasswordStorableSite(formUrl.host());
                }
            }
        }
#endif // KHTML_NO_WALLET

        const DOMString url(khtml::parseURL(getAttribute(ATTR_ACTION)));
        if(m_post) {
            view->part()->submitForm( "post", url.string(), form_data,
                                      m_target.string(),
                                      enctype().string(),
                                      m_boundary );
        }
        else {
            view->part()->submitForm( "get", url.string(), form_data,
                                      m_target.string() );
        }
    }

    m_walletMap.clear(); // done with it
    m_havePassword = m_haveTextarea= false;
    m_doingsubmit = m_insubmit = false;
}

void HTMLFormElementImpl::reset(  )
{
    KHTMLView* const view = getDocument()->view();
    if(m_inreset || !view || !view->part()) return;

    m_inreset = true;

#ifdef FORMS_DEBUG
    kdDebug( 6030 ) << "reset pressed!" << endl;
#endif

    // ### DOM2 labels this event as not cancelable, however
    // common browsers( sick! ) allow it be cancelled.
    if ( !dispatchHTMLEvent(EventImpl::RESET_EVENT,true, true) ) {
        m_inreset = false;
        return;
    }

    for (QPtrListIterator<HTMLGenericFormElementImpl> it(formElements); it.current(); ++it)
        it.current()->reset();

    m_inreset = false;
}

void HTMLFormElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch(attr->id())
    {
    case ATTR_ACTION:
        break;
    case ATTR_TARGET:
        m_target = attr->value();
        break;
    case ATTR_METHOD:
        m_post = ( strcasecmp( attr->value(), "post" ) == 0 );
        break;
    case ATTR_ENCTYPE:
        setEnctype( attr->value() );
        break;
    case ATTR_ACCEPT_CHARSET:
        // space separated list of charsets the server
        // accepts - see rfc2045
        m_acceptcharset = attr->value();
        break;
    case ATTR_ACCEPT:
        // ignore this one for the moment...
        break;
    case ATTR_AUTOCOMPLETE:
        m_autocomplete = strcasecmp( attr->value(), "off" );
        break;
    case ATTR_ONSUBMIT:
        setHTMLEventListener(EventImpl::SUBMIT_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string(), "onsubmit", this));
        break;
    case ATTR_ONRESET:
        setHTMLEventListener(EventImpl::RESET_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string(), "onreset", this));
        break;
    case ATTR_NAME:
        if (inDocument() && m_name != attr->value()) {
            getDocument()->underDocNamedCache().remove(m_name.string(),        this);
            getDocument()->underDocNamedCache().add   (attr->value().string(), this);
        }
        m_name = attr->value();
        //Fallthrough intentional
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLFormElementImpl::removedFromDocument()
{
    getDocument()->underDocNamedCache().remove(m_name.string(), this);
    HTMLElementImpl::removedFromDocument();
}

void HTMLFormElementImpl::insertedIntoDocument()
{
    getDocument()->underDocNamedCache().add(m_name.string(), this);
    HTMLElementImpl::insertedIntoDocument();
}

void HTMLFormElementImpl::removeId(const QString& id)
{
    getDocument()->underDocNamedCache().remove(id, this);
    HTMLElementImpl::removeId(id);
}

void HTMLFormElementImpl::addId   (const QString& id)
{
    getDocument()->underDocNamedCache().add(id, this);
    HTMLElementImpl::addId(id);
}


void HTMLFormElementImpl::radioClicked( HTMLGenericFormElementImpl *caller )
{
    for (QPtrListIterator<HTMLGenericFormElementImpl> it(formElements); it.current(); ++it) {
        HTMLGenericFormElementImpl* const current = it.current();
        if (current->id() == ID_INPUT &&
            static_cast<HTMLInputElementImpl*>(current)->inputType() == HTMLInputElementImpl::RADIO &&
            current != caller && current->form() == caller->form() && current->name() == caller->name())
            static_cast<HTMLInputElementImpl*>(current)->setChecked(false);
    }
}

void HTMLFormElementImpl::registerFormElement(HTMLGenericFormElementImpl *e)
{
    formElements.append(e);
}

void HTMLFormElementImpl::removeFormElement(HTMLGenericFormElementImpl *e)
{
    formElements.remove(e);
}

void HTMLFormElementImpl::registerImgElement(HTMLImageElementImpl *e)
{
    imgElements.append(e);
}

void HTMLFormElementImpl::removeImgElement(HTMLImageElementImpl *e)
{
    imgElements.remove(e);
}

// -------------------------------------------------------------------------

HTMLGenericFormElementImpl::HTMLGenericFormElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f)
    : HTMLElementImpl(doc)
{
    m_disabled = m_readOnly = false;
    m_name = 0;

    if (f)
	m_form = f;
    else
	m_form = getForm();
    if (m_form)
        m_form->registerFormElement(this);
}

void HTMLGenericFormElementImpl::insertedIntoDocument()
{
    HTMLElementImpl::insertedIntoDocument();

    if (!m_form) {
        HTMLFormElementImpl* const newform = getForm();
        if (newform) {
            m_form = newform;
            m_form->registerFormElement(this);
        }
    }
}

void HTMLGenericFormElementImpl::removedFromDocument()
{
    HTMLElementImpl::removedFromDocument();

    if (m_form)
        m_form->removeFormElement(this);

    m_form = 0;
}

HTMLGenericFormElementImpl::~HTMLGenericFormElementImpl()
{
    if (m_form)
        m_form->removeFormElement(this);
    if (m_name) m_name->deref();
}

void HTMLGenericFormElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch(attr->id())
    {
    case ATTR_DISABLED:
        setDisabled( attr->val() != 0 );
        break;
    case ATTR_READONLY:
    {
        const bool m_oldreadOnly = m_readOnly;
        m_readOnly = attr->val() != 0;
        if (m_oldreadOnly != m_readOnly) setChanged();
        break;
    }
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLGenericFormElementImpl::attach()
{
    assert(!attached());

    if (m_render) {
        assert(m_render->style());
        parentNode()->renderer()->addChild(m_render, nextRenderer());
    }

    // FIXME: This handles the case of a new form element being created by
    // JavaScript and inserted inside a form. What it does not handle is
    // a form element being moved from inside a form to outside, or from one
    // inside one form to another. The reason this other case is hard to fix
    // is that during parsing, we may have been passed a form that we are not
    // inside, DOM-tree-wise. If so, it's hard for us to know when we should
    // be removed from that form's element list.
    if (!m_form) {
	m_form = getForm();
	if (m_form)
	    m_form->registerFormElement(this);
    }

    NodeBaseImpl::attach();

    // The call to updateFromElement() needs to go after the call through
    // to the base class's attach() because that can sometimes do a close
    // on the renderer.
    if (m_render)
        m_render->updateFromElement();

}

HTMLFormElementImpl *HTMLGenericFormElementImpl::getForm() const
{
    NodeImpl *p = parentNode();
    while(p)
    {
        if( p->id() == ID_FORM )
            return static_cast<HTMLFormElementImpl *>(p);
        NodeImpl *s = p->previousSibling();
        if (!s)
            p = p->parentNode();
        else
            p = s;
    }
#ifdef FORMS_DEBUG
    kdDebug( 6030 ) << "couldn't find form!" << endl;
    kdDebug( 6030 ) << kdBacktrace() << endl;
#endif
    return 0;
}

DOMString HTMLGenericFormElementImpl::name() const
{
    if (m_name) return m_name;

// ###
//     DOMString n = getDocument()->htmlMode() != DocumentImpl::XHtml ?
//                   getAttribute(ATTR_NAME) : getAttribute(ATTR_ID);
    const DOMString n = getAttribute(ATTR_NAME);
    if (n.isNull())
        return new DOMStringImpl("");

    return n;
}

void HTMLGenericFormElementImpl::setName(const DOMString& name)
{
    if (m_name) m_name->deref();
    m_name = name.implementation();
    setAttribute( ATTR_NAME, name );
    if (m_name) m_name->ref();
}

void HTMLGenericFormElementImpl::onSelect()
{
    // ### make this work with new form events architecture
    dispatchHTMLEvent(EventImpl::SELECT_EVENT,true,false);
}

void HTMLGenericFormElementImpl::onChange()
{
    // ### make this work with new form events architecture
    dispatchHTMLEvent(EventImpl::CHANGE_EVENT,true,false);
}

void HTMLGenericFormElementImpl::setDisabled( bool _disabled )
{
    if ( m_disabled != _disabled ) {
        m_disabled = _disabled;
        setChanged();
    }
}

bool HTMLGenericFormElementImpl::isFocusable() const
{
    if (disabled())
	return false;

    //Non-widget INPUT TYPE="image" and <BUTTON> support focus, too.
    if (id() == ID_INPUT && static_cast<const HTMLInputElementImpl *>(this)->inputType() == HTMLInputElementImpl::IMAGE)
	return true;

    if (id() == ID_BUTTON)
	return true;

    if (!m_render)
	return false;

    QWidget* widget = static_cast<RenderWidget*>(m_render)->widget();
    return widget && widget->focusPolicy() >= QWidget::TabFocus;
}

class FocusHandleWidget : public QWidget
{
public:
    void focusNextPrev(bool n) {
        if (!focusNextPrevChild(n) && inherits("QTextEdit"))
            QWidget::focusNextPrevChild(n);
    }
};

void HTMLGenericFormElementImpl::defaultEventHandler(EventImpl *evt)
{
    if (evt->target() == this && renderer() && renderer()->isWidget()) {
        switch(evt->id())  {
        case EventImpl::MOUSEDOWN_EVENT:
        case EventImpl::MOUSEUP_EVENT:
        case EventImpl::MOUSEMOVE_EVENT:
        case EventImpl::MOUSEOUT_EVENT:
        case EventImpl::MOUSEOVER_EVENT:
        case EventImpl::KEYDOWN_EVENT:
        case EventImpl::KEYUP_EVENT:
        case EventImpl::KEYPRESS_EVENT:
            if (static_cast<RenderWidget*>(renderer())->handleEvent(*evt))
		evt->setDefaultHandled();
        default:
            break;
        }
    }

    if (evt->target()==this && !m_disabled)
    {
        // Report focus in/out changes to the browser extension (editable widgets only)
        KHTMLView* const view = getDocument()->view();
        if (view && evt->id() == EventImpl::DOMFOCUSIN_EVENT && isEditable() && m_render && m_render->isWidget()) {
            KHTMLPartBrowserExtension *ext = static_cast<KHTMLPartBrowserExtension *>(view->part()->browserExtension());
            QWidget* const widget = static_cast<RenderWidget*>(m_render)->widget();
            if (ext)
                ext->editableWidgetFocused(widget);
        }
        if (evt->id()==EventImpl::MOUSEDOWN_EVENT || evt->id()==EventImpl::KEYDOWN_EVENT)
        {
            setActive();
        }
        else if (evt->id() == EventImpl::MOUSEUP_EVENT || evt->id()==EventImpl::KEYUP_EVENT)
        {
	    if (m_active)
	    {
		setActive(false);
		setFocus();
	    }
	    else {
                setActive(false);
            }
        }

	if (!evt->defaultHandled() && m_render && m_render->isWidget()) {
	    // handle tabbing out, either from a single or repeated key event.
	    if ( evt->id() == EventImpl::KEYPRESS_EVENT && evt->isKeyRelatedEvent() ) {
	        QKeyEvent* const k = static_cast<KeyEventBaseImpl *>(evt)->qKeyEvent();
	        if ( k && (k->key() == Qt::Key_Tab || k->key() == Qt::Key_BackTab) ) {
		    QWidget* const widget = static_cast<RenderWidget*>(m_render)->widget();
		    QFocusEvent::setReason( k->key() == Qt::Key_Tab ? QFocusEvent::Tab : QFocusEvent::Backtab );
		    if (widget)
                        static_cast<FocusHandleWidget *>(widget)
			    ->focusNextPrev(k->key() == Qt::Key_Tab);
		    QFocusEvent::resetReason();
                    evt->setDefaultHandled();
	        }
            }
	}


	if (view && evt->id() == EventImpl::DOMFOCUSOUT_EVENT && isEditable() && m_render && m_render->isWidget()) {
	    KHTMLPartBrowserExtension* const ext = static_cast<KHTMLPartBrowserExtension *>(view->part()->browserExtension());
	    QWidget* const widget = static_cast<RenderWidget*>(m_render)->widget();
	    if (ext)
		ext->editableWidgetBlurred(widget);

	    // ### Don't count popup as a valid reason for losing the focus (example: opening the options of a select
	    // combobox shouldn't emit onblur)
	}
    }
    if (evt->target() == this && evt->isMouseEvent() && renderer())
        evt->setDefaultHandled();

    HTMLElementImpl::defaultEventHandler(evt);
}

bool HTMLGenericFormElementImpl::isEditable()
{
    return false;
}

// -------------------------------------------------------------------------

HTMLButtonElementImpl::HTMLButtonElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
    m_clicked = false;
    m_type = SUBMIT;
    m_dirty = true;
    m_activeSubmit = false;
}

HTMLButtonElementImpl::~HTMLButtonElementImpl()
{
}

NodeImpl::Id HTMLButtonElementImpl::id() const
{
    return ID_BUTTON;
}

DOMString HTMLButtonElementImpl::type() const
{
    return getAttribute(ATTR_TYPE);
}

void HTMLButtonElementImpl::blur()
{
    if(getDocument()->focusNode() == this)
        getDocument()->setFocusNode(0);
}

void HTMLButtonElementImpl::focus()
{
    getDocument()->setFocusNode(this);
}

void HTMLButtonElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch(attr->id())
    {
    case ATTR_TYPE:
        if ( strcasecmp( attr->value(), "submit" ) == 0 )
            m_type = SUBMIT;
        else if ( strcasecmp( attr->value(), "reset" ) == 0 )
            m_type = RESET;
        else if ( strcasecmp( attr->value(), "button" ) == 0 )
            m_type = BUTTON;
        break;
    case ATTR_VALUE:
        m_value = attr->value();
        m_currValue = m_value.string();
        break;
    case ATTR_ACCESSKEY:
        break;
    case ATTR_ALIGN:
        break;
    default:
        HTMLGenericFormElementImpl::parseAttribute(attr);
    }
}

void HTMLButtonElementImpl::defaultEventHandler(EventImpl *evt)
{
    if (m_type != BUTTON && !m_disabled) {
	bool act = (evt->id() == EventImpl::DOMACTIVATE_EVENT);
	if (!act && evt->id()==EventImpl::KEYUP_EVENT && evt->isKeyRelatedEvent()) {
	    QKeyEvent* const ke = static_cast<KeyEventBaseImpl *>(evt)->qKeyEvent();
	    if (ke && active() && (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Space))
		act = true;
	}
	if (act)
	    activate();
    }
    HTMLGenericFormElementImpl::defaultEventHandler(evt);
}

void HTMLButtonElementImpl::activate()
{
    m_clicked = true;

    if(m_form && m_type == SUBMIT) {
        m_activeSubmit = true;
        m_form->prepareSubmit();
        m_activeSubmit = false; // in case we were canceled
    }
    if(m_form && m_type == RESET)
        m_form->reset();
}

void HTMLButtonElementImpl::click()
{
    QMouseEvent me(QEvent::MouseButtonRelease, QPoint(0,0),Qt::LeftButton, 0);
    dispatchMouseEvent(&me,EventImpl::CLICK_EVENT, 1);
}

bool HTMLButtonElementImpl::encoding(const QTextCodec* codec, khtml::encodingList& encoding, bool /*multipart*/)
{
    if (m_type != SUBMIT || name().isEmpty() || !m_activeSubmit)
        return false;

    encoding += fixUpfromUnicode(codec, name().string());
    const QString enc_str = m_currValue.isNull() ? QString("") : m_currValue;
    encoding += fixUpfromUnicode(codec, enc_str);

    return true;
}

void HTMLButtonElementImpl::attach()
{
    // skip the generic handler
    HTMLElementImpl::attach();
}

// -------------------------------------------------------------------------

HTMLFieldSetElementImpl::HTMLFieldSetElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
}

HTMLFieldSetElementImpl::~HTMLFieldSetElementImpl()
{
}

NodeImpl::Id HTMLFieldSetElementImpl::id() const
{
    return ID_FIELDSET;
}

void HTMLFieldSetElementImpl::attach()
{
    assert(!attached());
    assert(!m_render);
    assert(parentNode());

    RenderStyle* const _style = getDocument()->styleSelector()->styleForElement(this);
    _style->ref();
    if (parentNode()->renderer() && _style->display() != NONE) {
        m_render = new (getDocument()->renderArena()) RenderFieldset(this);
        m_render->setStyle(_style);
    }
    HTMLGenericFormElementImpl::attach();
    _style->deref();
}

void HTMLFieldSetElementImpl::parseAttribute(AttributeImpl *attr)
{
    HTMLElementImpl::parseAttribute(attr);
}

// -------------------------------------------------------------------------

HTMLInputElementImpl::HTMLInputElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
    m_type = TEXT;
    m_maxLen = -1;
    m_size = 20;
    m_clicked = false;
    m_checked = false;

    m_haveType = false;
    m_activeSubmit = false;
    m_autocomplete = true;
    m_inited = false;
    m_unsubmittedFormChange = false;

    xPos = 0;
    yPos = 0;

    if ( m_form )
        m_autocomplete = f->autoComplete();
}

HTMLInputElementImpl::~HTMLInputElementImpl()
{
    if (getDocument()) getDocument()->deregisterMaintainsState(this);
}

NodeImpl::Id HTMLInputElementImpl::id() const
{
    return ID_INPUT;
}

// Called from JS. Can't merge with parseType since we
// also need to actually set ATTR_TYPE, which can't be done there.
void HTMLInputElementImpl::setType(const DOMString& t)
{
    setAttribute(ATTR_TYPE, t);
}

void HTMLInputElementImpl::parseType(const DOMString& t)
{
    typeEnum newType;

    if ( strcasecmp( t, "password" ) == 0 )
        newType = PASSWORD;
    else if ( strcasecmp( t, "checkbox" ) == 0 )
        newType = CHECKBOX;
    else if ( strcasecmp( t, "radio" ) == 0 )
        newType = RADIO;
    else if ( strcasecmp( t, "submit" ) == 0 )
        newType = SUBMIT;
    else if ( strcasecmp( t, "reset" ) == 0 )
        newType = RESET;
    else if ( strcasecmp( t, "file" ) == 0 )
        newType = FILE;
    else if ( strcasecmp( t, "hidden" ) == 0 )
        newType = HIDDEN;
    else if ( strcasecmp( t, "image" ) == 0 )
        newType = IMAGE;
    else if ( strcasecmp( t, "button" ) == 0 )
        newType = BUTTON;
    else if ( strcasecmp( t, "khtml_isindex" ) == 0 )
        newType = ISINDEX;
    else
        newType = TEXT;

    // ### IMPORTANT: Don't allow the type to be changed to FILE after the first
    // type change, otherwise a JavaScript programmer would be able to set a text
    // field's value to something like /etc/passwd and then change it to a file field.
    if (m_type != newType) {
        if (newType == FILE && m_haveType) {
            // Set the attribute back to the old value.
            // Note that this calls parseAttribute again.
            setAttribute(ATTR_TYPE, type());
        } else {
            m_type = newType;

            // force reattach if need be.
            if (attached()) {
                detach();
                attach();
            }
        }
    }
    m_haveType = true;
}

DOMString HTMLInputElementImpl::type() const
{
    // needs to be lowercase according to DOM spec
    switch (m_type) {
    case TEXT: return "text";
    case PASSWORD: return "password";
    case CHECKBOX: return "checkbox";
    case RADIO: return "radio";
    case SUBMIT: return "submit";
    case RESET: return "reset";
    case FILE: return "file";
    case HIDDEN: return "hidden";
    case IMAGE: return "image";
    case BUTTON: return "button";
    default: return "";
    }
}

QString HTMLInputElementImpl::state( )
{
    switch (m_type) {
    case PASSWORD:
        return QString::fromLatin1("."); // empty string, avoid restoring
    case CHECKBOX:
    case RADIO:
        return QString::fromLatin1(m_checked ? "on" : "off");
    case TEXT:
        if (autoComplete() && value() != getAttribute(ATTR_VALUE) && getDocument()->view())
            getDocument()->view()->addFormCompletionItem(name().string(), value().string());
        /* nobreak */
    default:
        return value().string() + (m_unsubmittedFormChange ? 'M' : '.');
    }
}

void HTMLInputElementImpl::restoreState(const QString &state)
{
    switch (m_type) {
    case CHECKBOX:
    case RADIO:
        setChecked((state == QString::fromLatin1("on")));
        break;
    case FILE:
        m_value = DOMString(state.left(state.length()-1));
        setChanged();
        break;
    default:
        setValue(DOMString(state.left(state.length()-1)));
        m_unsubmittedFormChange = state.endsWith("M");
        break;
    }
}

void HTMLInputElementImpl::select(  )
{
    if(!m_render) return;

    if (m_type == TEXT || m_type == PASSWORD)
        static_cast<RenderLineEdit*>(m_render)->select();
    else if (m_type == FILE)
        static_cast<RenderFileButton*>(m_render)->select();
}

void HTMLInputElementImpl::click()
{
    QMouseEvent me(QEvent::MouseButtonRelease, QPoint(0,0),Qt::LeftButton, 0);
    dispatchMouseEvent(&me,0, 1);
    dispatchMouseEvent(&me,EventImpl::CLICK_EVENT, 1);
}

void HTMLInputElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch(attr->id())
    {
    case ATTR_AUTOCOMPLETE:
        m_autocomplete = strcasecmp( attr->value(), "off" );
        break;
    case ATTR_TYPE:
        parseType(attr->value());
        break;
    case ATTR_VALUE:
        if (m_value.isNull()) {// We only need to setChanged if the form is looking
            setChanged();      // at the default value right now.
            if (m_type == TEXT && m_render)
                m_render->updateFromElement();
        }
        break;
    case ATTR_CHECKED:
        // WebCore has m_defaultChecked and m_useDefaultChecked code here....
        break;
    case ATTR_MAXLENGTH:
    {
        m_maxLen = -1;
        if (!attr->val()) break;
        bool ok;
        const int ml = attr->val()->toInt(&ok);
        if (ml > 0 && ml < 1024)
            m_maxLen = ml;
        else if (ok && ml <= 0)
            m_maxLen = 0;
        setChanged();
    }
    break;
    case ATTR_SIZE:
        m_size = attr->val() ? attr->val()->toInt() : 20;
        break;
    case ATTR_ALT:
    case ATTR_SRC:
        if (m_type == IMAGE)
            setChanged();
        break;
    case ATTR_USEMAP:
        // ### ignore for the moment
        break;
    case ATTR_ALIGN:
        if ( m_inited && m_type == IMAGE )
            addHTMLAlignment( attr->value() );
        break;
    case ATTR_ACCESSKEY:
        break;
    case ATTR_WIDTH:
        if ( m_type == IMAGE )
            addCSSLength(CSS_PROP_WIDTH, attr->value() );
        break;
    case ATTR_HEIGHT:
        if ( m_type == IMAGE )
            addCSSLength(CSS_PROP_HEIGHT, attr->value() );
        break;
    case ATTR_ONSELECT:
        setHTMLEventListener(EventImpl::SELECT_EVENT,
            getDocument()->createHTMLEventListener(attr->value().string(), "onselect", this));
        break;
    case ATTR_ONCHANGE:
        setHTMLEventListener(EventImpl::CHANGE_EVENT,
            getDocument()->createHTMLEventListener(attr->value().string(), "onchange", this));
        break;
    default:
        HTMLGenericFormElementImpl::parseAttribute(attr);
    }
}

void HTMLInputElementImpl::attach()
{
    assert(!attached());
    assert(!m_render);
    assert(parentNode());

    if (!m_inited) {
        // FIXME: This needs to be dynamic, doesn't it, since someone could set this
        // after attachment?
        if ((uint) m_type <= ISINDEX && !m_value.isEmpty()) {
            const QString value = m_value.string();
            // remove newline stuff..
            QString nvalue;
            unsigned int valueLength = value.length();
            for (unsigned int i = 0; i < valueLength; ++i)
                if (value[i] >= ' ')
                    nvalue += value[i];
            m_value = nvalue;
        }
        m_checked = (getAttribute(ATTR_CHECKED) != 0);
        if ( m_type == IMAGE )
            addHTMLAlignment( getAttribute( ATTR_ALIGN ) );
        m_inited = true;
    }

    switch( m_type ) {
    case PASSWORD:
        if (getDocument()->isHTMLDocument())
            static_cast<HTMLDocumentImpl*>(getDocument())->setAutoFill();
        break;
    case HIDDEN:
    case IMAGE:
        if (!getAttribute(ATTR_WIDTH).isNull())
            addCSSLength(CSS_PROP_WIDTH, getAttribute(ATTR_WIDTH));
        if (!getAttribute(ATTR_HEIGHT).isNull())
            addCSSLength(CSS_PROP_HEIGHT, getAttribute(ATTR_HEIGHT));
    default:
        break;
    };

    RenderStyle* const _style = getDocument()->styleSelector()->styleForElement(this);
    _style->ref();
    if (parentNode()->renderer() && _style->display() != NONE) {
        switch(m_type)
        {
        case TEXT:
        case PASSWORD:
        case ISINDEX:      m_render = new (getDocument()->renderArena()) RenderLineEdit(this);   break;
        case CHECKBOX:  m_render = new (getDocument()->renderArena()) RenderCheckBox(this); break;
        case RADIO:        m_render = new (getDocument()->renderArena()) RenderRadioButton(this); break;
        case SUBMIT:      m_render = new (getDocument()->renderArena()) RenderSubmitButton(this); break;
        case IMAGE:       m_render =  new (getDocument()->renderArena()) RenderImageButton(this); break;
        case RESET:      m_render = new (getDocument()->renderArena()) RenderResetButton(this);   break;
        case FILE:         m_render =  new (getDocument()->renderArena()) RenderFileButton(this);    break;
        case BUTTON:  m_render = new (getDocument()->renderArena()) RenderPushButton(this);
        case HIDDEN:   break;
        }
    }

    if (m_render)
        m_render->setStyle(_style);

    HTMLGenericFormElementImpl::attach();
    _style->deref();
}

DOMString HTMLInputElementImpl::altText() const
{
    // http://www.w3.org/TR/1998/REC-html40-19980424/appendix/notes.html#altgen
    // also heavily discussed by Hixie on bugzilla
    // note this is intentionally different to HTMLImageElementImpl::altText()
    DOMString alt = getAttribute( ATTR_ALT );
    // fall back to title attribute
    if ( alt.isNull() )
        alt = getAttribute( ATTR_TITLE );
    if ( alt.isNull() )
        alt = getAttribute( ATTR_VALUE );
    if ( alt.isEmpty() )
        alt = i18n( "Submit" );

    return alt;
}

bool HTMLInputElementImpl::encoding(const QTextCodec* codec, khtml::encodingList& encoding, bool multipart)
{
    const QString nme = name().string();

    // image generates its own name's
    if (nme.isEmpty() && m_type != IMAGE) return false;

    // IMAGE needs special handling later
    if(m_type != IMAGE) encoding += fixUpfromUnicode(codec, nme);

    switch (m_type) {
        case CHECKBOX:

            if( checked() ) {
                encoding += fixUpfromUnicode(codec, value().string());
                return true;
            }
            break;

        case RADIO:

            if( checked() ) {
                encoding += fixUpfromUnicode(codec, value().string());
                return true;
            }
            break;

        case BUTTON:
        case RESET:
            // those buttons are never successful
            return false;

        case IMAGE:

            if(m_clicked)
            {
                m_clicked = false;
                QString astr(nme.isEmpty() ? QString::fromLatin1("x") : nme + ".x");

                encoding += fixUpfromUnicode(codec, astr);
                astr.setNum(KMAX( clickX(), 0 ));
                encoding += fixUpfromUnicode(codec, astr);
                astr = nme.isEmpty() ? QString::fromLatin1("y") : nme + ".y";
                encoding += fixUpfromUnicode(codec, astr);
                astr.setNum(KMAX( clickY(), 0 ) );
                encoding += fixUpfromUnicode(codec, astr);
		astr = value().string();
		if(astr.length() > 0) {
		    encoding += fixUpfromUnicode(codec, nme);
		    encoding += fixUpfromUnicode(codec, astr);
		}

                return true;
            }
            break;

        case SUBMIT:

            if (m_activeSubmit)
            {
                QString enc_str = valueWithDefault().string();
                if(!enc_str.isEmpty())
                {
                    encoding += fixUpfromUnicode(codec, enc_str);
                    return true;
                }
            }
            break;

	case FILE: // hmm, we have the type FILE also.  bad choice here...
        {
            // don't submit if display: none or display: hidden
            if(!renderer() || renderer()->style()->visibility() != khtml::VISIBLE)
                return false;

            QString local;
            KURL fileurl;
            QString val = value().string();
            if (!val.isEmpty() &&
                QDir::isRelativePath(val) &&
                QFile::exists(KGlobalSettings::documentPath() + val)) {
                fileurl.setPath(KGlobalSettings::documentPath() + val);
            } else {
                fileurl = KURL::fromPathOrURL(val);
            }

            KIO::UDSEntry filestat;

            // can't submit file in www-url-form encoded
            QWidget* const toplevel = static_cast<RenderSubmitButton*>(m_render)->widget()->topLevelWidget();
            if (multipart) {
                QCString filearray( "" );
                if ( KIO::NetAccess::stat(fileurl, filestat, toplevel)) {
                    const KFileItem fileitem(filestat, fileurl, true, false);
                    if ( fileitem.isFile() &&
                         KIO::NetAccess::download(fileurl, local, toplevel) ) {
                        QFile file(local);
                        filearray.resize(file.size()+1);
                        if ( file.open( IO_ReadOnly ) ) {
                            const int readbytes = file.readBlock( filearray.data(), file.size());
                            if ( readbytes >= 0 )
                                filearray[readbytes] = '\0';
                            file.close();
                        }
                        KIO::NetAccess::removeTempFile( local );
                    }
                }
                encoding += filearray;
                return true;
            }
            // else fall through
        }
        case HIDDEN:
        case TEXT:
        case PASSWORD:
            // always successful
            encoding += fixUpfromUnicode(codec, value().string());
            return true;
        case ISINDEX:
            encoding += fixUpfromUnicode(codec, value().string());
            return true;
    }
    return false;
}

void HTMLInputElementImpl::reset()
{
    setValue(getAttribute(ATTR_VALUE));
    setChecked(getAttribute(ATTR_CHECKED) != 0);
}

void HTMLInputElementImpl::setChecked(bool _checked)
{
    if (m_form && m_type == RADIO && _checked && !name().isEmpty())
        m_form->radioClicked(this);

    if (m_checked == _checked) return;
    m_checked = _checked;
    setChanged();
}


DOMString HTMLInputElementImpl::value() const
{
    if (m_type == CHECKBOX || m_type == RADIO) {
        const DOMString val = getAttribute(ATTR_VALUE);
        // If no attribute exists, then just use "on" or "" based off the checked() state
        // of the control.
        if (val.isNull()) {
            if (checked())
                return DOMString("on");
            else
                return DOMString("");
        }
        return val;
    }

    DOMString val = m_value;
    // It's important *not* to fall back to the value attribute for file inputs,
    // because that would allow a malicious web page to upload files by setting the
    // value attribute in markup.
    if (val.isNull() && m_type != FILE)
        val = getAttribute(ATTR_VALUE);

    return val.isNull() ? DOMString("") : val;
}


void HTMLInputElementImpl::setValue(DOMString val)
{
    if (m_type == FILE) return;

    m_value = (val.isNull() ? DOMString("") : val);
    if (m_type == TEXT && m_render)
        m_render->updateFromElement();
    setChanged();
}

void HTMLInputElementImpl::blur()
{
    if(getDocument()->focusNode() == this)
	getDocument()->setFocusNode(0);
}

void HTMLInputElementImpl::focus()
{
    getDocument()->setFocusNode(this);
}

void HTMLInputElementImpl::defaultEventHandler(EventImpl *evt)
{
    if ( !m_disabled )
    {

        if (evt->isMouseEvent()) {
	    MouseEventImpl* const me = static_cast<MouseEventImpl*>(evt);
            if ((m_type == RADIO || m_type == CHECKBOX)
		&& me->id() == EventImpl::MOUSEUP_EVENT && me->detail() > 0) {
		// click will follow
		setChecked(m_type == RADIO ? true : !checked());
	    }
            if (evt->id() == EventImpl::CLICK_EVENT && m_type == IMAGE && m_render) {
		// record the mouse position for when we get the DOMActivate event
		int offsetX, offsetY;
		m_render->absolutePosition(offsetX,offsetY);
		xPos = me->clientX()-offsetX;
		yPos = me->clientY()-offsetY;
                KHTMLView* v = getDocument()->view();
                if ( v ) {
                    xPos += v->contentsX();
                    yPos += v->contentsY();
                }
	    }
	}

        if (m_type == RADIO || m_type == CHECKBOX || m_type == SUBMIT || m_type == RESET || m_type == BUTTON ) {
	    bool check = false;
	    if (active() && ( evt->id() == EventImpl::KEYUP_EVENT ||
	                      evt->id() == EventImpl::KEYPRESS_EVENT ) ) {
		TextEventImpl* const te = static_cast<TextEventImpl *>(evt);
		if (te->keyVal() == ' ')
		    check = true;
		else if (te->keyVal() == '\r' && (m_type == BUTTON || m_type == RESET || m_type == SUBMIT))
		    check = true;
	    }
	    if (check) {
	        if (evt->id() == EventImpl::KEYUP_EVENT)
		    click();
	        // Tell the parent that we handle this key (keyup and keydown), even though only keyup activates (#70478)
	        evt->setDefaultHandled();
	    }
        }


        // DOMActivate events cause the input to be "activated" - in the case of image and submit inputs, this means
        // actually submitting the form. For reset inputs, the form is reset. These events are sent when the user clicks
        // on the element, or presses enter while it is the active element. Javascript code wishing to activate the element
        // must dispatch a DOMActivate event - a click event will not do the job.
        if (m_type == IMAGE || m_type == SUBMIT || m_type == RESET) {
	    bool act = (evt->id() == EventImpl::DOMACTIVATE_EVENT);
	    if (!act && evt->id() == EventImpl::KEYUP_EVENT && evt->isKeyRelatedEvent()) {
		QKeyEvent* const ke = static_cast<KeyEventBaseImpl *>(evt)->qKeyEvent();
		if (ke && active() && (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Space))
		    act = true;
	    }
	    if (act)
		activate();
	}
    }
    HTMLGenericFormElementImpl::defaultEventHandler(evt);
}

void HTMLInputElementImpl::activate()
{
    if (!m_form)
        return;

    m_clicked = true;
    if (m_type == RESET) {
        m_form->reset();
    }
    else {
        m_activeSubmit = true;
        if (!m_form->prepareSubmit()) {
            xPos = 0;
            yPos = 0;
        }
        m_activeSubmit = false;
    }
}

bool HTMLInputElementImpl::isEditable()
{
    return ((m_type == TEXT) || (m_type == PASSWORD) || (m_type == ISINDEX) || (m_type == FILE));
}

long HTMLInputElementImpl::selectionStart()
{
    if (m_type != TEXT || !m_render) return -1;
    return static_cast<RenderLineEdit*>(m_render)->selectionStart();
}

long HTMLInputElementImpl::selectionEnd()
{
    if (m_type != TEXT || !m_render) return -1;
    return static_cast<RenderLineEdit*>(m_render)->selectionEnd();
}

void HTMLInputElementImpl::setSelectionStart(long pos)
{
    if (m_type != TEXT || !m_render) return;
    static_cast<RenderLineEdit*>(m_render)->setSelectionStart(pos);
}

void HTMLInputElementImpl::setSelectionEnd  (long pos)
{
    if (m_type != TEXT || !m_render) return;
    static_cast<RenderLineEdit*>(m_render)->setSelectionEnd(pos);
}

void HTMLInputElementImpl::setSelectionRange(long start, long end)
{
    if (m_type != TEXT || !m_render) return;
    static_cast<RenderLineEdit*>(m_render)->setSelectionRange(start, end);
}

// -------------------------------------------------------------------------

HTMLLabelElementImpl::HTMLLabelElementImpl(DocumentPtr *doc)
    : HTMLGenericFormElementImpl(doc)
{
}

HTMLLabelElementImpl::~HTMLLabelElementImpl()
{
}

NodeImpl::Id HTMLLabelElementImpl::id() const
{
    return ID_LABEL;
}

void HTMLLabelElementImpl::attach()
{
    // skip the generic handler
    HTMLElementImpl::attach();
}

NodeImpl* HTMLLabelElementImpl::getFormElement()
{
	    const DOMString formElementId = getAttribute(ATTR_FOR);
	    NodeImpl *newNode=0L;
    	    if (!formElementId.isEmpty())
	        newNode=getDocument()->getElementById(formElementId);
    	    if (!newNode){
    		const uint children=childNodeCount();
    		if (children>1)
    		    for (unsigned int i=0;i<children;++i){
			const uint nodeId=childNode(i)->id();
			if (nodeId==ID_INPUT || nodeId==ID_SELECT || nodeId==ID_TEXTAREA){
			    newNode=childNode(i);
			    break;
			}
		    }
		}
return newNode;
}

void HTMLLabelElementImpl::defaultEventHandler(EventImpl *evt)
{
    if ( !m_disabled ) {
	bool act = false;
	if ( evt->id() == EventImpl::CLICK_EVENT ) {
	    act = true;
	}
	else if ( evt->isKeyRelatedEvent() && ( evt->id() == EventImpl::KEYUP_EVENT ||
	                                        evt->id() == EventImpl::KEYPRESS_EVENT ) ) {
	    QKeyEvent* const ke = static_cast<KeyEventBaseImpl *>(evt)->qKeyEvent();
	    if (ke && active() && (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Space))
		act = true;
	}

	if (act) {
	    NodeImpl* const formNode=getFormElement();
	    if (formNode) {
		getDocument()->setFocusNode(formNode);
		if (formNode->id()==ID_INPUT)
		    static_cast<DOM::HTMLInputElementImpl*>(formNode)->click();
	    }
	    evt->setDefaultHandled();
	}
    }
    HTMLGenericFormElementImpl::defaultEventHandler(evt);
}

// -------------------------------------------------------------------------

HTMLLegendElementImpl::HTMLLegendElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
}

HTMLLegendElementImpl::~HTMLLegendElementImpl()
{
}

NodeImpl::Id HTMLLegendElementImpl::id() const
{
    return ID_LEGEND;
}

void HTMLLegendElementImpl::attach()
{
    assert(!attached());
    assert(!m_render);
    assert(parentNode());
    RenderStyle* const _style = getDocument()->styleSelector()->styleForElement(this);
    _style->ref();
    if (parentNode()->renderer() && _style->display() != NONE) {
        m_render = new (getDocument()->renderArena()) RenderLegend(this);
        m_render->setStyle(_style);
    }
    HTMLGenericFormElementImpl::attach();
    _style->deref();
}

void HTMLLegendElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch(attr->id())
    {
    case ATTR_ACCESSKEY:
        break;
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

// -------------------------------------------------------------------------

HTMLSelectElementImpl::HTMLSelectElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
    m_multiple = false;
    m_recalcListItems = false;
    // 0 means invalid (i.e. not set)
    m_size = 0;
    m_minwidth = 0;
    m_length   = 0;
}

HTMLSelectElementImpl::~HTMLSelectElementImpl()
{
    if (getDocument()) getDocument()->deregisterMaintainsState(this);
}

NodeImpl::Id HTMLSelectElementImpl::id() const
{
    return ID_SELECT;
}

DOMString HTMLSelectElementImpl::type() const
{
    return (m_multiple ? "select-multiple" : "select-one");
}

long HTMLSelectElementImpl::selectedIndex() const
{
    // return the number of the first option selected
    uint o = 0;
    QMemArray<HTMLGenericFormElementImpl*> items = listItems();
    const unsigned int itemsSize = items.size();
    for (unsigned int i = 0; i < itemsSize; ++i) {
        if (items[i]->id() == ID_OPTION) {
            if (static_cast<HTMLOptionElementImpl*>(items[i])->selected())
                return o;
            o++;
        }
    }
    Q_ASSERT(m_multiple || items.isEmpty());
    return -1;
}

void HTMLSelectElementImpl::setSelectedIndex( long  index )
{
    // deselect all other options and select only the new one
    QMemArray<HTMLGenericFormElementImpl*> items = listItems();
    int listIndex;
    const int itemsSize = int(items.size());
    for (listIndex = 0; listIndex < itemsSize; ++listIndex) {
        if (items[listIndex]->id() == ID_OPTION)
            static_cast<HTMLOptionElementImpl*>(items[listIndex])->setSelected(false);
    }
    listIndex = optionToListIndex(index);
    if (listIndex >= 0)
        static_cast<HTMLOptionElementImpl*>(items[listIndex])->setSelected(true);

    setChanged(true);
}

long HTMLSelectElementImpl::length() const
{
    if (m_recalcListItems)
        recalcListItems();
    return m_length;
}

void HTMLSelectElementImpl::add( const HTMLElement &element, const HTMLElement &before, int& exceptioncode )
{
    if(element.isNull() || element.handle()->id() != ID_OPTION)
        return;

    HTMLOptionElementImpl* option = static_cast<HTMLOptionElementImpl*>(element.handle());;
    //Fast path for appending an item. Can't be done if it is selected and 
    //we're single-select, since we may need to drop an implicitly-selected item
    bool fastAppendLast = false;
    if (before.handle() == 0 && (m_multiple || !option->selected()) && !m_recalcListItems)
        fastAppendLast = true;

    insertBefore(option, before.handle(), exceptioncode );

    if (fastAppendLast) {
        m_listItems.resize(m_listItems.size() + 1);
        m_listItems[m_listItems.size() - 1] = option;
        ++m_length;
        m_recalcListItems = false;
    } else if (!exceptioncode)
        setRecalcListItems();
}

void HTMLSelectElementImpl::remove( long index )
{
    int exceptioncode = 0;
    const int listIndex = optionToListIndex(index);

    QMemArray<HTMLGenericFormElementImpl*> items = listItems();
    if(listIndex < 0 || index >= int(items.size()))
        return; // ### what should we do ? remove the last item?

    //Fast path for last element, for e.g. clearing the box
    //Note that if this is a single-select, we may have to recompute
    //anyway if the item was selected, since we may want to set 
    //a different one
    bool fastRemoveLast = false;
    if ((listIndex == (signed)items.size() - 1) && !m_recalcListItems && 
        (m_multiple || !static_cast<HTMLOptionElementImpl*>(items[listIndex])->selected()))
            fastRemoveLast = true;

    removeChild(items[listIndex], exceptioncode);

    if (fastRemoveLast) {
        m_listItems.resize(m_listItems.size() - 1);
        --m_length;
        m_recalcListItems = false;
    } else if( !exceptioncode)
        setRecalcListItems();
}

void HTMLSelectElementImpl::blur()
{
    if(getDocument()->focusNode() == this)
	getDocument()->setFocusNode(0);
}

void HTMLSelectElementImpl::focus()
{
    getDocument()->setFocusNode(this);
}

DOMString HTMLInputElementImpl::valueWithDefault() const
{
    DOMString v = value();
    if (v.isEmpty()) {
        switch (m_type) {
            case RESET:
#ifdef APPLE_CHANGES
                v = resetButtonDefaultLabel();
#else
                v = i18n("Reset");
#endif
                break;

            case SUBMIT:
#ifdef APPLE_CHANGES
                v = submitButtonDefaultLabel();
#else
                v = i18n("Submit");
#endif
                break;

            case BUTTON:
            case CHECKBOX:
            case FILE:
            case HIDDEN:
            case IMAGE:
            case ISINDEX:
            case PASSWORD:
            case RADIO:
        #ifdef APPLE_CHANGES
            case RANGE:
            case SEARCH:
        #endif
            case TEXT:
                break;
        }
    }
    return v;
}

DOMString HTMLSelectElementImpl::value( ) const
{
    uint i;
    QMemArray<HTMLGenericFormElementImpl*> items = listItems();
    const uint itemsSize = items.size();
    for (i = 0; i < itemsSize; ++i) {
        if ( items[i]->id() == ID_OPTION
            && static_cast<HTMLOptionElementImpl*>(items[i])->selected())
            return static_cast<HTMLOptionElementImpl*>(items[i])->value();
    }
    return DOMString("");
}

void HTMLSelectElementImpl::setValue(DOMStringImpl* value)
{
    // find the option with value() matching the given parameter
    // and make it the current selection.
    QMemArray<HTMLGenericFormElementImpl*> items = listItems();
    for (unsigned i = 0; i < items.size(); i++)
        if (items[i]->id() == ID_OPTION && static_cast<HTMLOptionElementImpl*>(items[i])->value() == value) {
            static_cast<HTMLOptionElementImpl*>(items[i])->setSelected(true);
            return;
        }
}

QString HTMLSelectElementImpl::state( )
{
    QString state;
    QMemArray<HTMLGenericFormElementImpl*> items = listItems();

    const int l = items.count();

    state.fill('.', l);
    for(int i = 0; i < l; ++i)
        if(items[i]->id() == ID_OPTION && static_cast<HTMLOptionElementImpl*>(items[i])->selected())
            state[i] = 'X';

    return state;
}

void HTMLSelectElementImpl::restoreState(const QString &_state)
{
    recalcListItems();

    QString state = _state;
    if(!state.isEmpty() && !state.contains('X') && !m_multiple && m_size <= 1) {
        qWarning("should not happen in restoreState!");
        state[0] = 'X';
    }

    QMemArray<HTMLGenericFormElementImpl*> items = listItems();

    const int l = items.count();
    for(int i = 0; i < l; ++i) {
        if(items[i]->id() == ID_OPTION) {
            HTMLOptionElementImpl* const oe = static_cast<HTMLOptionElementImpl*>(items[i]);
            oe->setSelected(state[i] == 'X');
        }
    }
    setChanged(true);
}

NodeImpl *HTMLSelectElementImpl::insertBefore ( NodeImpl *newChild, NodeImpl *refChild, int &exceptioncode )
{
    NodeImpl* const result = HTMLGenericFormElementImpl::insertBefore(newChild,refChild, exceptioncode );
    if (!exceptioncode)
        setRecalcListItems();
    return result;
}

NodeImpl *HTMLSelectElementImpl::replaceChild ( NodeImpl *newChild, NodeImpl *oldChild, int &exceptioncode )
{
    NodeImpl* const result = HTMLGenericFormElementImpl::replaceChild(newChild,oldChild, exceptioncode);
    if( !exceptioncode )
        setRecalcListItems();
    return result;
}

NodeImpl *HTMLSelectElementImpl::removeChild ( NodeImpl *oldChild, int &exceptioncode )
{
    NodeImpl* const result = HTMLGenericFormElementImpl::removeChild(oldChild, exceptioncode);
    if( !exceptioncode )
        setRecalcListItems();
    return result;
}

NodeImpl *HTMLSelectElementImpl::appendChild ( NodeImpl *newChild, int &exceptioncode )
{
    NodeImpl* const result = HTMLGenericFormElementImpl::appendChild(newChild, exceptioncode);
    if( !exceptioncode )
        setRecalcListItems();
    setChanged(true);
    return result;
}

NodeImpl* HTMLSelectElementImpl::addChild(NodeImpl* newChild)
{
    setRecalcListItems();
    return HTMLGenericFormElementImpl::addChild(newChild);
}

void HTMLSelectElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch(attr->id())
    {
    case ATTR_SIZE:
        m_size = kMax( attr->val()->toInt(), 1 );
        setChanged();
        break;
    case ATTR_WIDTH:
        m_minwidth = kMax( attr->val()->toInt(), 0 );
        break;
    case ATTR_MULTIPLE:
        m_multiple = (attr->val() != 0);
        break;
    case ATTR_ACCESSKEY:
        break;
    case ATTR_ALIGN:
        addHTMLAlignment( attr->value() );
        break;
    case ATTR_ONCHANGE:
        setHTMLEventListener(EventImpl::CHANGE_EVENT,
            getDocument()->createHTMLEventListener(attr->value().string(), "onchange", this));
        break;
    default:
        HTMLGenericFormElementImpl::parseAttribute(attr);
    }
}

void HTMLSelectElementImpl::attach()
{
    assert(!attached());
    assert(parentNode());
    assert(!renderer());

    RenderStyle* const _style = getDocument()->styleSelector()->styleForElement(this);
    _style->ref();
    if (parentNode()->renderer() && _style->display() != NONE) {
        m_render = new (getDocument()->renderArena()) RenderSelect(this);
        m_render->setStyle(_style);
    }

    HTMLGenericFormElementImpl::attach();
    _style->deref();
}

bool HTMLSelectElementImpl::encoding(const QTextCodec* codec, khtml::encodingList& encoded_values, bool)
{
    bool successful = false;
    const QCString enc_name = fixUpfromUnicode(codec, name().string());
    QMemArray<HTMLGenericFormElementImpl*> items = listItems();

    uint i;
    const uint itemsSize = items.size();
    for (i = 0; i < itemsSize; ++i) {
        if (items[i]->id() == ID_OPTION) {
            HTMLOptionElementImpl* const option = static_cast<HTMLOptionElementImpl*>(items[i]);
            if (option->selected()) {
                encoded_values += enc_name;
                encoded_values += fixUpfromUnicode(codec, option->value().string());
                successful = true;
            }
        }
    }

    // ### this case should not happen. make sure that we select the first option
    // in any case. otherwise we have no consistency with the DOM interface. FIXME!
    // we return the first one if it was a combobox select
    if (!successful && !m_multiple && m_size <= 1 && itemsSize &&
        (items[0]->id() == ID_OPTION) ) {
        HTMLOptionElementImpl* const option = static_cast<HTMLOptionElementImpl*>(items[0]);
        encoded_values += enc_name;
        if (option->value().isNull())
            encoded_values += fixUpfromUnicode(codec, option->text().string().stripWhiteSpace());
        else
            encoded_values += fixUpfromUnicode(codec, option->value().string());
        successful = true;
    }

    return successful;
}

int HTMLSelectElementImpl::optionToListIndex(int optionIndex) const
{
    QMemArray<HTMLGenericFormElementImpl*> items = listItems();
    const int itemsSize = int(items.size());
    if (optionIndex < 0 || optionIndex >= itemsSize)
        return -1;

    //See if we're asked for the very last item, and check whether it's an <option>
    //to fastpath clear
    if (optionIndex == (m_length - 1) && items[itemsSize - 1]->id() == ID_OPTION)
        return itemsSize - 1;

    int listIndex = 0;
    int optionIndex2 = 0;
    for (;
         optionIndex2 < itemsSize && optionIndex2 <= optionIndex;
         ++listIndex) { // not a typo!
        if (items[listIndex]->id() == ID_OPTION)
            ++optionIndex2;
    }
    --listIndex;
    return listIndex;
}

int HTMLSelectElementImpl::listToOptionIndex(int listIndex) const
{
    QMemArray<HTMLGenericFormElementImpl*> items = listItems();
    if (listIndex < 0 || listIndex >= int(items.size()) ||
        items[listIndex]->id() != ID_OPTION)
        return -1;

    int optionIndex = 0; // actual index of option not counting OPTGROUP entries that may be in list
    int i;
    for (i = 0; i < listIndex; i++)
        if (items[i]->id() == ID_OPTION)
            optionIndex++;
    return optionIndex;
}

void HTMLSelectElementImpl::recalcListItems() const
{
    NodeImpl* current = firstChild();
    m_listItems.resize(0);
    HTMLOptionElementImpl* foundSelected = 0;
    m_length = 0;
    while(current) {
        if (current->id() == ID_OPTGROUP && current->firstChild()) {
            // ### what if optgroup contains just comments? don't want one of no options in it...
            m_listItems.resize(m_listItems.size()+1);
            m_listItems[m_listItems.size()-1] = static_cast<HTMLGenericFormElementImpl*>(current);
            current = current->firstChild();
        }
        if (current->id() == ID_OPTION) {
            ++m_length;
            m_listItems.resize(m_listItems.size()+1);
            m_listItems[m_listItems.size()-1] = static_cast<HTMLGenericFormElementImpl*>(current);
            if (!foundSelected && !m_multiple && m_size <= 1) {
                foundSelected = static_cast<HTMLOptionElementImpl*>(current);
                foundSelected->m_selected = true;
            }
            else if (foundSelected && !m_multiple && static_cast<HTMLOptionElementImpl*>(current)->selected()) {
                foundSelected->m_selected = false;
                foundSelected = static_cast<HTMLOptionElementImpl*>(current);
            }
        }
        NodeImpl* const parent = current->parentNode();
        current = current->nextSibling();
        if (!current) {
            if (static_cast<const NodeImpl *>(parent) != this)
                current = parent->nextSibling();
        }
    }
    m_recalcListItems = false;
}

void HTMLSelectElementImpl::childrenChanged()
{
    setRecalcListItems();

    HTMLGenericFormElementImpl::childrenChanged();
}

void HTMLSelectElementImpl::setRecalcListItems()
{
    m_recalcListItems = true;
    if (m_render)
        static_cast<khtml::RenderSelect*>(m_render)->setOptionsChanged(true);
    setChanged();
}

void HTMLSelectElementImpl::reset()
{
    QMemArray<HTMLGenericFormElementImpl*> items = listItems();
    uint i;
    const uint itemsSize = items.size();
    bool anySelected = false;
    for (i = 0; i < itemsSize; ++i) {
        if (items[i]->id() == ID_OPTION) {
            HTMLOptionElementImpl* const option = static_cast<HTMLOptionElementImpl*>(items[i]);
            const bool selected = (!option->getAttribute(ATTR_SELECTED).isNull());
            option->setSelected(selected);
            if (selected)
                anySelected = true;
        }
    }
    // If there is no default selection, jump to first option.
    if ( !anySelected ) {
        for (i = 0; i < itemsSize; ++i) {
            if (items[i]->id() == ID_OPTION) {
                static_cast<HTMLOptionElementImpl*>(items[i])->setSelected(true);
                break;
            }
        }
    }
    if ( m_render )
        static_cast<RenderSelect*>(m_render)->setSelectionChanged(true);
    setChanged( true );
}

void HTMLSelectElementImpl::notifyOptionSelected(HTMLOptionElementImpl *selectedOption, bool selected)
{
    if (selected && !m_multiple) {
        // deselect all other options
        QMemArray<HTMLGenericFormElementImpl*> items = listItems();
        uint i;
	const uint itemsSize = items.size();
        for (i = 0; i < itemsSize; ++i) {
            if (items[i]->id() == ID_OPTION)
                static_cast<HTMLOptionElementImpl*>(items[i])->m_selected = (items[i] == selectedOption);
        }
    }
    if (m_render)
        static_cast<RenderSelect*>(m_render)->setSelectionChanged(true);

    setChanged(true);
}

// -------------------------------------------------------------------------

HTMLKeygenElementImpl::HTMLKeygenElementImpl(DocumentPtr* doc, HTMLFormElementImpl* f)
    : HTMLSelectElementImpl(doc, f)
{
    const QStringList keys = KSSLKeyGen::supportedKeySizes();
    QStringList::ConstIterator i = keys.begin();
    const QStringList::ConstIterator iEnd = keys.end();
    for ( ; i != iEnd; ++i) {
        HTMLOptionElementImpl* const o = new HTMLOptionElementImpl(doc, form());
        addChild(o);
        o->addChild(doc->document()->createTextNode(DOMString(*i).implementation()));
    }
}

NodeImpl::Id HTMLKeygenElementImpl::id() const
{
    return ID_KEYGEN;
}

void HTMLKeygenElementImpl::parseAttribute(AttributeImpl* attr)
{
    switch(attr->id())
    {
    case ATTR_CHALLENGE:
        break;
    default:
        // skip HTMLSelectElementImpl parsing!
        HTMLGenericFormElementImpl::parseAttribute(attr);
    }
}

bool HTMLKeygenElementImpl::encoding(const QTextCodec* codec, khtml::encodingList& encoded_values, bool)
{
    bool successful = false;
    const QCString enc_name = fixUpfromUnicode(codec, name().string());

    encoded_values += enc_name;

    // pop up the fancy certificate creation dialog here
    KSSLKeyGen* const kg = new KSSLKeyGen(static_cast<RenderWidget *>(m_render)->widget(), "Key Generator", true);

    kg->setKeySize(0);
    successful = (QDialog::Accepted == kg->exec());

    delete kg;

    encoded_values += "deadbeef";

    return successful;
}

// -------------------------------------------------------------------------

NodeImpl::Id HTMLOptGroupElementImpl::id() const
{
    return ID_OPTGROUP;
}

// -------------------------------------------------------------------------

HTMLOptionElementImpl::HTMLOptionElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
    m_selected = false;
}

NodeImpl::Id HTMLOptionElementImpl::id() const
{
    return ID_OPTION;
}

DOMString HTMLOptionElementImpl::text() const
{
    if (firstChild() && firstChild()->nodeType() == Node::TEXT_NODE) {
	if (firstChild()->nextSibling()) {
	    DOMString ret = "";
	    NodeImpl *n = firstChild();
	    for (; n; n = n->nextSibling()) {
		if (n->nodeType() == Node::TEXT_NODE ||
		    n->nodeType() == Node::CDATA_SECTION_NODE)
		    ret += n->nodeValue();
	    }
	    return ret;
	}
	else
	    return firstChild()->nodeValue();
    }
    return "";
}

long HTMLOptionElementImpl::index() const
{
    // Let's do this dynamically. Might be a bit slow, but we're sure
    // we won't forget to update a member variable in some cases...
    QMemArray<HTMLGenericFormElementImpl*> items = getSelect()->listItems();
    const int l = items.count();
    int optionIndex = 0;
    for(int i = 0; i < l; ++i) {
        if(items[i]->id() == ID_OPTION)
        {
            if (static_cast<HTMLOptionElementImpl*>(items[i]) == this)
                return optionIndex;
            ++optionIndex;
        }
    }
    kdWarning() << "HTMLOptionElementImpl::index(): option not found!" << endl;
    return 0;
}

void HTMLOptionElementImpl::setIndex( long  )
{
    kdWarning() << "Unimplemented HTMLOptionElementImpl::setIndex(long) called" << endl;
    // ###
}

void HTMLOptionElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch(attr->id())
    {
    case ATTR_SELECTED:
        m_selected = (attr->val() != 0);
        break;
    case ATTR_VALUE:
        m_value = attr->value();
        break;
    default:
        HTMLGenericFormElementImpl::parseAttribute(attr);
    }
}

DOMString HTMLOptionElementImpl::value() const
{
    if ( !m_value.isNull() )
        return m_value;
    // Use the text if the value wasn't set.
    return text().string().simplifyWhiteSpace();
}

void HTMLOptionElementImpl::setValue(DOMStringImpl* value)
{
    setAttribute(ATTR_VALUE, value);
}

void HTMLOptionElementImpl::setSelected(bool _selected)
{
    if(m_selected == _selected)
        return;
    m_selected = _selected;
    HTMLSelectElementImpl* const select = getSelect();
    if (select)
        select->notifyOptionSelected(this,_selected);
}

HTMLSelectElementImpl *HTMLOptionElementImpl::getSelect() const
{
    NodeImpl *select = parentNode();
    while (select && select->id() != ID_SELECT)
        select = select->parentNode();
    return static_cast<HTMLSelectElementImpl*>(select);
}

// -------------------------------------------------------------------------

HTMLTextAreaElementImpl::HTMLTextAreaElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
    // DTD requires rows & cols be specified, but we will provide reasonable defaults
    m_rows = 2;
    m_cols = 20;
    m_wrap = ta_Virtual;
    m_changed     = false;
    m_dirtyvalue  = true;
    m_initialized = false;
    m_unsubmittedFormChange = false;
}

HTMLTextAreaElementImpl::~HTMLTextAreaElementImpl()
{
    if (getDocument()) getDocument()->deregisterMaintainsState(this);
}

NodeImpl::Id HTMLTextAreaElementImpl::id() const
{
    return ID_TEXTAREA;
}

DOMString HTMLTextAreaElementImpl::type() const
{
    return "textarea";
}

QString HTMLTextAreaElementImpl::state( )
{
    return value().string() + (m_unsubmittedFormChange ? 'M' : '.');
}

void HTMLTextAreaElementImpl::restoreState(const QString &state)
{
    setDefaultValue(state.left(state.length()-1));
    m_unsubmittedFormChange = state.endsWith("M");
    // the close() in the rendertree will take care of transferring defaultvalue to 'value'
}

void HTMLTextAreaElementImpl::select(  )
{
    if (m_render)
        static_cast<RenderTextArea*>(m_render)->select();
    onSelect();
}

void HTMLTextAreaElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch(attr->id())
    {
    case ATTR_ROWS:
        m_rows = attr->val() ? attr->val()->toInt() : 3;
        if (renderer())
            renderer()->setNeedsLayoutAndMinMaxRecalc();
        break;
    case ATTR_COLS:
        m_cols = attr->val() ? attr->val()->toInt() : 60;
        if (renderer())
            renderer()->setNeedsLayoutAndMinMaxRecalc();
        break;
    case ATTR_WRAP:
        // virtual / physical is Netscape extension of HTML 3.0, now deprecated
        // soft/ hard / off is recommendation for HTML 4 extension by IE and NS 4
        if ( strcasecmp( attr->value(), "virtual" ) == 0  || strcasecmp( attr->value(), "soft") == 0)
            m_wrap = ta_Virtual;
        else if ( strcasecmp ( attr->value(), "physical" ) == 0 || strcasecmp( attr->value(), "hard") == 0)
            m_wrap = ta_Physical;
        else if(strcasecmp( attr->value(), "on" ) == 0)
            m_wrap = ta_Physical;
        else if(strcasecmp( attr->value(), "off") == 0)
            m_wrap = ta_NoWrap;
        break;
    case ATTR_ACCESSKEY:
        break;
    case ATTR_ALIGN:
        break;
    case ATTR_ONSELECT:
        setHTMLEventListener(EventImpl::SELECT_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string(), "onselect", this));
        break;
    case ATTR_ONCHANGE:
        setHTMLEventListener(EventImpl::CHANGE_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string(), "onchange", this));
        break;
    default:
        HTMLGenericFormElementImpl::parseAttribute(attr);
    }
}

void HTMLTextAreaElementImpl::attach()
{
    assert(!attached());
    assert(!m_render);
    assert(parentNode());

    RenderStyle* const _style = getDocument()->styleSelector()->styleForElement(this);
    _style->ref();
    if (parentNode()->renderer() && _style->display() != NONE) {
        m_render = new (getDocument()->renderArena()) RenderTextArea(this);
        m_render->setStyle(_style);
    }

    HTMLGenericFormElementImpl::attach();
    _style->deref();
}


static QString expandLF(const QString& s)
{
    // LF -> CRLF
    unsigned crs = s.contains( '\n' );
    if (crs == 0)
	return s;
    unsigned len = s.length();

    QString r;
    r.reserve(len + crs + 1);
    unsigned pos2 = 0;
    for(unsigned pos = 0; pos < len; pos++)
    {
       QChar c = s.at(pos);
       switch(c.unicode())
       {
         case '\n':
           r[pos2++] = '\r';
           r[pos2++] = '\n';
           break;

         case '\r':
           break;

         default:
           r[pos2++]= c;
           break;
       }
    }
    r.squeeze();
    return r;
}


bool HTMLTextAreaElementImpl::encoding(const QTextCodec* codec, encodingList& encoding, bool)
{
    if (name().isEmpty()) return false;

    encoding += fixUpfromUnicode(codec, name().string());
    encoding += fixUpfromUnicode(codec, expandLF(value().string()));

    return true;
}

void HTMLTextAreaElementImpl::reset()
{
    setValue(defaultValue());
}


DOMString HTMLTextAreaElementImpl::value()
{
    if ( m_dirtyvalue) {
        if ( m_render && m_initialized ) {
            RenderTextArea* renderArea = static_cast<RenderTextArea*>( m_render );
            m_value = renderArea->text();
            m_dirtyvalue = false;
        } else {
            m_value = defaultValue().string();
            m_initialized = true;
            m_dirtyvalue = false;
        }
    }

    if ( m_value.isNull() ) return "";

    return m_value;
}

void HTMLTextAreaElementImpl::setValue(DOMString _value)
{
    // \r\n -> \n, \r -> \n
    QString str = _value.string().replace( "\r\n", "\n" );
    m_value = str.replace( '\r', '\n' );
    m_dirtyvalue = false;
    m_initialized = true;
    setChanged(true);
}

DOMString HTMLTextAreaElementImpl::defaultValue()
{
    DOMString val = "";
    // there may be comments - just grab the text nodes
    NodeImpl *n;
    for (n = firstChild(); n; n = n->nextSibling())
        if (n->isTextNode())
            val += static_cast<TextImpl*>(n)->data();

    if (val[0] == '\r' && val[1] == '\n') {
	val = val.copy();
	val.remove(0,2);
    }
    else if (val[0] == '\r' || val[0] == '\n') {
	val = val.copy();
	val.remove(0,1);
    }

    return val;
}

void HTMLTextAreaElementImpl::setDefaultValue(DOMString _defaultValue)
{
    // there may be comments - remove all the text nodes and replace them with one
    QPtrList<NodeImpl> toRemove;
    NodeImpl *n;
    for (n = firstChild(); n; n = n->nextSibling())
        if (n->isTextNode())
            toRemove.append(n);
    QPtrListIterator<NodeImpl> it(toRemove);
    int exceptioncode = 0;
    for (; it.current(); ++it) {
        removeChild(it.current(), exceptioncode);
    }
    insertBefore(getDocument()->createTextNode(_defaultValue.implementation()),firstChild(), exceptioncode);
    setValue(_defaultValue);
}

void HTMLTextAreaElementImpl::blur()
{
    if(getDocument()->focusNode() == this)
	getDocument()->setFocusNode(0);
}

void HTMLTextAreaElementImpl::focus()
{
    getDocument()->setFocusNode(this);
}

bool HTMLTextAreaElementImpl::isEditable()
{
    return true;
}

//Mozilla extensions.
long HTMLTextAreaElementImpl::selectionStart()
{
    if (m_render) {
        RenderTextArea* renderArea = static_cast<RenderTextArea*>( m_render );
        return renderArea->selectionStart();
    }

    return 0;
}

long HTMLTextAreaElementImpl::selectionEnd()
{
    if (m_render) {
        RenderTextArea* renderArea = static_cast<RenderTextArea*>( m_render );
        return renderArea->selectionEnd();
    }

    return 0;
}

void HTMLTextAreaElementImpl::setSelectionStart(long pos)
{
    if (m_render) {
        RenderTextArea* renderArea = static_cast<RenderTextArea*>( m_render );
        renderArea->setSelectionStart( pos );
    }
}

void HTMLTextAreaElementImpl::setSelectionEnd(long pos)
{
    if (m_render) {
        RenderTextArea* renderArea = static_cast<RenderTextArea*>( m_render );
        renderArea->setSelectionEnd( pos );
    }
}

void HTMLTextAreaElementImpl::setSelectionRange(long start, long end)
{
    if (m_render) {
        RenderTextArea* renderArea = static_cast<RenderTextArea*>( m_render );
        renderArea->setSelectionRange( start, end );
    }
}

long HTMLTextAreaElementImpl::textLength()
{
    return value().length();
}

// -------------------------------------------------------------------------

HTMLIsIndexElementImpl::HTMLIsIndexElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f)
    : HTMLInputElementImpl(doc, f)
{
    m_type = TEXT;
    setName("isindex");
}

HTMLIsIndexElementImpl::~HTMLIsIndexElementImpl()
{
}

NodeImpl::Id HTMLIsIndexElementImpl::id() const
{
    return ID_ISINDEX;
}

void HTMLIsIndexElementImpl::parseAttribute(AttributeImpl* attr)
{
    // don't call HTMLInputElement::parseAttribute here, as it would
    // accept attributes this element does not support
    HTMLGenericFormElementImpl::parseAttribute(attr);
}

DOMString HTMLIsIndexElementImpl::prompt() const
{
    // When IsIndex is parsed, <HR/>Prompt: <ISINDEX/><HR/> is created.
    // So we have to look at the previous sibling to find the prompt text
    DOM::NodeImpl* const prev = previousSibling();
    if ( prev && prev->nodeType() == DOM::Node::TEXT_NODE)
        return prev->nodeValue();
    return "";
}

void HTMLIsIndexElementImpl::setPrompt(const DOMString& str)
{
    // When IsIndex is parsed, <HR/>Prompt: <ISINDEX/><HR/> is created.
    // So we have to look at the previous sibling to find the prompt text
    int exceptioncode = 0;
    DOM::NodeImpl* const prev = previousSibling();
    if ( prev && prev->nodeType() == DOM::Node::TEXT_NODE)
        static_cast<DOM::TextImpl *>(prev)->setData(str, exceptioncode);
}

// -------------------------------------------------------------------------

