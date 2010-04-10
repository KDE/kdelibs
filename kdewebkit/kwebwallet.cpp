/*
 * This file is part of the KDE project.
 *
 * Copyright (C) 2009 Dawit Alemayehu <adawit@kde.org>
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

#include "kwebwallet.h"

#include <kwallet.h>
#include <kdebug.h>

#include <QtCore/QHash>
#include <QtCore/QFile>
#include <QtCore/QPointer>
#include <QtWebKit/QWebPage>
#include <QtWebKit/QWebFrame>
#include <QtWebKit/QWebElement>
#include <QtWebKit/QWebElementCollection>
#include <qwindowdefs.h>

#define QL1S(x)   QLatin1String(x)
#define QL1C(x)   QLatin1Char(x)

/**
 * Creates key used to store and retrieve form data.
 *
 */
static QString walletKey(KWebWallet::WebForm form)
{
    QString key = form.url.toString(QUrl::RemoveQuery|QUrl::RemoveFragment);
    key += QL1C('#');
    key += form.name;

    return key;
}

static QString escapeValue (const QString& _value)
{
    QString value (_value);
    value.replace(QL1C('\\'), QL1S("\\\\"));
    value.replace(QL1C('\"'), QL1S("\\\""));
    return value;
}

static int getWebFields(const QWebElement &formElement,
                        const QString& selector, QList<KWebWallet::WebForm::WebField> &fields)
{
    QWebElementCollection collection = formElement.findAll(selector);
    const int count = collection.count();

    for(int i = 0; i < count; ++i) {
        QWebElement element = collection.at(i);
        const QString value = element.evaluateJavaScript(QL1S("this.value")).toString();
        if (!value.isEmpty())
            fields << qMakePair(element.attribute(QL1S("name")), value);
    }

    return fields.count();
}

class KWebWallet::KWebWalletPrivate
{  
public:
    struct FormsData
    {
        QPointer<QWebFrame> frame;
        KWebWallet::WebFormList forms;
    };
  
    KWebWalletPrivate(KWebWallet* parent);
    KWebWallet::WebFormList parseFormData(QWebFrame* frame, bool fillform = true, bool ignorepasswd = false);
    void fillDataFromCache(KWebWallet::WebFormList &formList);
    void saveDataToCache(const QString &key);
    void removeDataFromCache(const WebFormList &formList);

    // Private Q_SLOT...
    void _k_openWalletDone(bool);
    void _k_walletClosed();

    WId wid;
    KWebWallet *q;
    QPointer<KWallet::Wallet> wallet;
    KWebWallet::WebFormList pendingRemoveRequests;
    QHash<KUrl, FormsData> pendingFillRequests;
    QHash<QString, KWebWallet::WebFormList> pendingSaveRequests;
};

KWebWallet::KWebWalletPrivate::KWebWalletPrivate(KWebWallet *parent)
                              :wid (0), q(parent)
{
}

KWebWallet::WebFormList KWebWallet::KWebWalletPrivate::parseFormData(QWebFrame *frame, bool fillform, bool ignorepasswd)
{
    Q_ASSERT(frame);

    KWebWallet::WebFormList list;
    QWebElementCollection formElements = frame->findAllElements(QL1S("form[method=post]"));
    const int formElementCount = formElements.count();

    if (fillform) {
        for ( int i = 0; i < formElementCount; ++i ) {
            const QWebElement formElement = formElements.at(i);

            KWebWallet::WebForm form;
            form.url = frame->url();
            form.index = QString::number(i);
            form.name = formElement.attribute(QL1S("name"));
            if (q->hasCachedFormData(form))
                list << form;
        }
    } else {
        int numPasswdFields = 0;
        QString passwdSelector;

        if (!ignorepasswd)
            passwdSelector = QL1S("input[type=password]:not([autocomplete=off])");

        for (int i = 0; i < formElementCount; ++i) {
            const QWebElement formElement = formElements.at(i);

            KWebWallet::WebForm form;
            form.url = frame->url();
            form.index = QString::number(i);
            form.name = formElement.attribute(QL1S("name"));

            // Get all <input> elements of type 'password'
            numPasswdFields = getWebFields(formElement, passwdSelector, form.fields);

            // Get all <input> elements of type 'text'
            getWebFields(formElement, QL1S("input[type=text]:not([autocomplete=off]), input:not([type])"), form.fields);

            // Add the form the list if it contains a password field...
            if ((ignorepasswd || numPasswdFields == 1) && !form.fields.isEmpty())
                list << form;
        }
    }

    return list;
}

void KWebWallet::KWebWalletPrivate::fillDataFromCache(KWebWallet::WebFormList &formList)
{
    if (wallet) {
        QMap<QString, QString> cachedValues;
        QMutableListIterator <WebForm> formIt (formList);

        while (formIt.hasNext()) {
            KWebWallet::WebForm &form = formIt.next();
            const QString key (walletKey(form));
            if (wallet->readMap(key, cachedValues) == 0) {
                QMapIterator<QString, QString> valuesIt (cachedValues);
                while (valuesIt.hasNext()) {
                    valuesIt.next();
                    //kDebug(800) << "wallet key:" << key << valuesIt.key() << valuesIt.value();
                    form.fields << qMakePair(valuesIt.key(), valuesIt.value());
                }
            } else {
                  kWarning(800) << "Unable to read form data for key:" << key;
            }
        }
    } else {
        kWarning(800) << "Unable to retreive form data from wallet";
    }
}

void KWebWallet::KWebWalletPrivate::saveDataToCache(const QString &key)
{
    bool success = false;
    const QUrl url = pendingSaveRequests.value(key).first().url;

    if (wallet) {
        int count = 0;
        const KWebWallet::WebFormList list = pendingSaveRequests.take(key);
        QListIterator<KWebWallet::WebForm> formIt (list);

        while (formIt.hasNext()) {
            QMap<QString, QString> values;
            const KWebWallet::WebForm form = formIt.next();
            QListIterator<KWebWallet::WebForm::WebField> fieldIt (form.fields);
            while (fieldIt.hasNext()) {
                const KWebWallet::WebForm::WebField field = fieldIt.next();
                values.insert(field.first, field.second);
            }

            if (wallet->writeMap(walletKey(form), values) == 0)
                count++;
            else
                kWarning(800) << "Unable to write form data to wallet";
        }

        if (list.isEmpty())
          success = true;
    } else {
        kWarning(800) << "NULL KWallet instance!";
    }

    emit q->saveFormDataCompleted(url, success);
}

void KWebWallet::KWebWalletPrivate::removeDataFromCache(const WebFormList &formList)
{
    if (wallet) {
        QListIterator<WebForm> formIt (formList);
        while (formIt.hasNext()) {
            wallet->removeEntry(walletKey(formIt.next()));
        }
    } else {
        kWarning(800) << "NULL KWallet instance!";
    }
}

void KWebWallet::KWebWalletPrivate::_k_openWalletDone(bool ok)
{ 
    Q_ASSERT (wallet);

    if (ok &&
        (wallet->hasFolder(KWallet::Wallet::FormDataFolder()) ||
         wallet->createFolder(KWallet::Wallet::FormDataFolder())) &&
         wallet->setFolder(KWallet::Wallet::FormDataFolder())) {

        // Do pending fill requests...
        if (!pendingFillRequests.isEmpty()) {
            KUrl::List urlList;
            QMutableHashIterator<KUrl, FormsData> requestIt (pendingFillRequests);
            while (requestIt.hasNext()) {
               requestIt.next();
               KWebWallet::WebFormList list = requestIt.value().forms;
               fillDataFromCache(list);
               q->fillWebForm(requestIt.key(), list);
            }

            pendingFillRequests.clear();
        }

         // Do pending save requests...
        if (!pendingSaveRequests.isEmpty()) {
            QListIterator<QString> keysIt (pendingSaveRequests.keys());
            while (keysIt.hasNext())
                saveDataToCache(keysIt.next());

            pendingSaveRequests.clear();
        }

        // Do pending remove requests...
        if (!pendingRemoveRequests.isEmpty()) {
            removeDataFromCache(pendingRemoveRequests);
            pendingRemoveRequests.clear();
        }
    } else {
        // Delete the wallet if opening the wallet failed or we were unable
        // to change to the folder we wanted to change to.
        delete wallet;
    }
}

void KWebWallet::KWebWalletPrivate::_k_walletClosed()
{
    if (wallet)
      wallet->deleteLater();

    emit q->walletClosed();
}

KWebWallet::KWebWallet(QObject *parent, WId wid)
           :QObject(parent), d(new KWebWalletPrivate(this))
{
    if (!wid) {
        // If wid is 0, make the best effort the discern it from our parent.
        QWebPage *page = qobject_cast<QWebPage*>(parent);
        if (page) {
            QWidget *widget = page->view();
            if (widget && widget->window())
                wid = widget->window()->winId();
        }
    }

    d->wid = wid;
}

KWebWallet::~KWebWallet()
{
    delete d->wallet;
    delete d;
}

KWebWallet::WebFormList KWebWallet::formsWithCachedData(QWebFrame* frame, bool recursive) const
{
    WebFormList list;

    if (frame) {
        list << d->parseFormData(frame);

        if (recursive) {
            QListIterator <QWebFrame *> framesIt (frame->childFrames());
            while (framesIt.hasNext()) {
                list << d->parseFormData(framesIt.next());
            }
        }
    }

    return list;
}

void KWebWallet::fillFormData(QWebFrame *frame, bool recursive)
{
    if (frame) {
        KUrl::List urlList;
        WebFormList formsList = d->parseFormData(frame);
        if (!formsList.isEmpty()) {
          const QUrl url (frame->url());
          if (d->pendingFillRequests.contains(url)) {
              kWarning(800) << "Duplicate request rejected!";
          } else {
              KWebWalletPrivate::FormsData data;
              data.frame = frame;
              data.forms << formsList;
              d->pendingFillRequests.insert(url, data);
              urlList << url;
          }
        }

        if (recursive) {
            QListIterator<QWebFrame*> frameIt (frame->childFrames());
            while (frameIt.hasNext()) {
                QWebFrame *childFrame = frameIt.next();
                formsList = d->parseFormData(childFrame);
                if (!formsList.isEmpty()) {
                    const QUrl url (childFrame->url());
                    if (d->pendingFillRequests.contains(url)) {
                        kWarning(800) << "Duplicate request rejected!!!";
                    } else {
                        KWebWalletPrivate::FormsData data;
                        data.frame = frame;
                        data.forms << formsList;
                        d->pendingFillRequests.insert(url, data);
                        urlList << url;
                    }
                }
            }
        }

        if (!urlList.isEmpty())
            fillFormDataFromCache(urlList);
    }
}

void KWebWallet::saveFormData(QWebFrame *frame, bool recursive, bool ignorePasswordFields)
{
    if (frame) {
        WebFormList list = d->parseFormData(frame, false, ignorePasswordFields);
        if (recursive) {
            QListIterator<QWebFrame*> frameIt (frame->childFrames());
            while (frameIt.hasNext()) {
                list << d->parseFormData(frameIt.next(), false, ignorePasswordFields);
            }
        }

        if (!list.isEmpty()) {
            const QString key = QString::number(qHash(frame->url().toString() + frame->frameName()), 16);
            if (!d->pendingSaveRequests.contains(key)) {
                d->pendingSaveRequests.insert(key, list);

                for (int i =0 ; i < list.count(); ++i) {
                    if (hasCachedFormData(list.at(i)))
                        list.takeAt(i);
                }

                if (list.isEmpty())
                    saveFormDataToCache(key);
                else
                    emit saveFormDataRequested(key, frame->url());
            }
        }
    }
}

void KWebWallet::removeFormData(QWebFrame *frame, bool recursive)
{
    if (frame)
        removeFormDataFromCache(formsWithCachedData(frame, recursive));
}

void KWebWallet::removeFormData(const WebFormList &forms)
{
    d->pendingRemoveRequests << forms;
    removeFormDataFromCache(forms);
}

void KWebWallet::acceptSaveFormDataRequest(const QString &key)
{
    saveFormDataToCache(key);
}

void KWebWallet::rejectSaveFormDataRequest(const QString & key)
{
    d->pendingSaveRequests.remove(key);
}

void KWebWallet::fillWebForm(const KUrl &url, const KWebWallet::WebFormList &forms)
{
    QWebFrame *frame = d->pendingFillRequests.value(url).frame;
    QWebElement formElement;
    QString script (QL1S("var e = this.elements[\"%1\"];\n"
                         "if(e && !e.disabled  && !e.readonly)\n"
                         "  e.value=\"%2\";"));
    if (frame) {
        QListIterator<WebForm> formIt (forms);
        while (formIt.hasNext()) {
            const WebForm form = formIt.next();

            QListIterator<WebForm::WebField> fieldIt (form.fields);
            while (fieldIt.hasNext()) {
                const WebForm::WebField field = fieldIt.next();
                if (form.name.isEmpty())
                    formElement = frame->findAllElements(QL1S("form[method=post]")).at(form.index.toInt());
                else
                  formElement = frame->findFirstElement(QString::fromLatin1("form[method=post][name=%1]")
                                                        .arg(form.name));
                //kDebug(800) << script.arg(field.first).arg(escapeValue(field.second));
                formElement.evaluateJavaScript(script
                                               .arg(field.first)
                                               .arg(escapeValue(field.second)));
            }
        }
    }
}

KWebWallet::WebFormList KWebWallet::formsToFill(const KUrl &url) const
{
    return d->pendingFillRequests.value(url).forms;
}

KWebWallet::WebFormList KWebWallet::formsToSave(const QString &key) const
{
    return d->pendingSaveRequests.value(key);
}

bool KWebWallet::hasCachedFormData(const WebForm &form) const
{
    return !KWallet::Wallet::keyDoesNotExist(KWallet::Wallet::NetworkWallet(),
                                             KWallet::Wallet::FormDataFolder(),
                                             walletKey(form));
}

void KWebWallet::fillFormDataFromCache(const KUrl::List &urlList)
{
    if (!d->wallet) {
        d->wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(),
                                                d->wid, KWallet::Wallet::Asynchronous);
        connect(d->wallet, SIGNAL(walletOpened(bool)),
                this, SLOT(_k_openWalletDone(bool)));
        return;
    }

    QListIterator<KUrl> urlIt (urlList);
    while (urlIt.hasNext()) {
        const KUrl url = urlIt.next();
        WebFormList list = formsToFill(url);
        d->fillDataFromCache(list);
        fillWebForm(url, list);
    }

    d->pendingFillRequests.clear();
}

void KWebWallet::saveFormDataToCache(const QString &key)
{
    if (!d->wallet) {
        d->wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(),
                                                d->wid, KWallet::Wallet::Asynchronous);
        connect(d->wallet, SIGNAL(walletOpened(bool)),
                this, SLOT(_k_openWalletDone(bool)));
        return;
    }

    d->saveDataToCache(key);
    d->pendingSaveRequests.clear();
}

void KWebWallet::removeFormDataFromCache(const WebFormList &forms)
{
    if (!d->wallet) {
        d->wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(),
                                                d->wid, KWallet::Wallet::Asynchronous);
        connect(d->wallet, SIGNAL(walletOpened(bool)),
                this, SLOT(_k_openWalletDone(bool)));
        return;
    }

    d->removeDataFromCache(forms);
    d->pendingRemoveRequests.clear();
}

#include "kwebwallet.moc"
