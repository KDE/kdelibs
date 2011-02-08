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

#include <QtCore/QSet>
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

// Form parsing JS code adapted from Arora.
// See https://github.com/Arora/arora/blob/master/src/data/parseForms.js
#define FORM_PARSING_JS "(function (){ \
    var forms = new Array; \
    for (var i = 0; i < document.forms.length; ++i) { \
        var form = document.forms[i]; \
        if (form.method.toLowerCase() != 'post') \
            continue; \
        var formObject = new Object; \
        formObject.name = form.name; \
        formObject.index = i; \
        var elements = new Array; \
        for (var j = 0; j < form.elements.length; ++j) { \
            var e = form.elements[j]; \
            var element = new Object; \
            element.name = e.name; \
            element.value = e.value; \
            element.type = e.type; \
            element.readonly = e.hasAttribute('readonly'); \
            element.disabled = e.hasAttribute('disabled'); \
            if (element.autocomplete != null)  \
                element.autocomplete = element.autocomplete.value; \
            elements.push(element); \
        } \
        formObject.elements = elements; \
        forms.push(formObject); \
    } \
    return forms; \
}())"


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

static void collectAllChildFrames(QWebFrame* frame, QList<QWebFrame*>& list)
{
    list << frame->childFrames();
    QListIterator<QWebFrame*> it(frame->childFrames());
    while (it.hasNext()) {
        collectAllChildFrames(it.next(), list);
    }
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

    // Private slots...
    void _k_openWalletDone(bool);
    void _k_walletClosed();

    WId wid;
    KWebWallet *q;
    QPointer<KWallet::Wallet> wallet;
    KWebWallet::WebFormList pendingRemoveRequests;
    QHash<KUrl, FormsData> pendingFillRequests;
    QHash<QString, KWebWallet::WebFormList> pendingSaveRequests;
    QSet<KUrl> confirmSaveRequestOverwrites;
};

KWebWallet::KWebWalletPrivate::KWebWalletPrivate(KWebWallet *parent)
                              :wid (0), q(parent)
{
}

KWebWallet::WebFormList KWebWallet::KWebWalletPrivate::parseFormData(QWebFrame *frame, bool fillform, bool ignorepasswd)
{
    Q_ASSERT(frame);

    KWebWallet::WebFormList list;

    // Execute the javscript to obtain the necessary fields...
    QVariantList results = frame->evaluateJavaScript(QL1S(FORM_PARSING_JS)).toList();
    Q_FOREACH (const QVariant &formVariant, results) {
        QVariantMap map = formVariant.toMap();
        KWebWallet::WebForm form;
        form.url = frame->url();
        form.name = map[QL1S("name")].toString();
        form.index = map[QL1S("index")].toString();
        bool formHasPasswords = false;
        const QVariantList elements = map[QL1S("elements")].toList();
        QList<KWebWallet::WebForm::WebField> inputFields;
        Q_FOREACH (const QVariant &element, elements) {
            QVariantMap elementMap = element.toMap();
            const QString name = elementMap[QL1S("name")].toString();
            const QString value = (ignorepasswd ? QString() : elementMap[QL1S("value")].toString());
            const QString type = elementMap[QL1S("type")].toString();
            const bool isPasswdInput = (type.compare(QL1S("password"), Qt::CaseInsensitive) == 0);
            const bool isTextInput = (type.compare(QL1S("text"), Qt::CaseInsensitive) == 0);
            const bool autoCompleteOff = (elementMap[QL1S("autocomplete")].toString().compare(QL1S("off"), Qt::CaseInsensitive) == 0);
            if (name.isEmpty())
                continue;
            if (!isPasswdInput && !isTextInput)
                continue;
            if (autoCompleteOff)
                continue;
            if (elementMap[QL1S("disabled")].toBool())
                continue;
            if (fillform && elementMap[QL1S("readonly")].toBool())
                continue;
            if (isPasswdInput && !fillform && value.isEmpty())
                continue;
            if (isPasswdInput)
                formHasPasswords = true;
            inputFields.append(qMakePair(name, value));
        }

        // Only add the input fields on form save requests...
        if (formHasPasswords && !fillform)
            form.fields = inputFields;

        // Add the form to the list if we are saving it or it has cached data.
        if ((fillform && q->hasCachedFormData(form)) || (!fillform  && !form.fields.isEmpty()))
            list << form;
    }
    return list;
}

void KWebWallet::KWebWalletPrivate::fillDataFromCache(KWebWallet::WebFormList &formList)
{
    if (!wallet) {
        kWarning(800) << "Unable to retrieve form data from wallet";
        return;
    }

    QMap<QString, QString> cachedValues;
    QMutableListIterator <WebForm> formIt (formList);

    while (formIt.hasNext()) {
        KWebWallet::WebForm &form = formIt.next();
        const QString key (walletKey(form));
        if (wallet->readMap(key, cachedValues) != 0) {
            kWarning(800) << "Unable to read form data for key:" << key;
            continue;
        }

        QMapIterator<QString, QString> valuesIt (cachedValues);
        while (valuesIt.hasNext()) {
            valuesIt.next();
            //kDebug(800) << "wallet key:" << key << valuesIt.key() << valuesIt.value();
            form.fields << qMakePair(valuesIt.key(), valuesIt.value());
        }
    }
}

void KWebWallet::KWebWalletPrivate::saveDataToCache(const QString &key)
{
    bool success = false;
    const QUrl url = pendingSaveRequests.value(key).first().url;

    if (wallet) {
        int count = 0;
        const KWebWallet::WebFormList list = pendingSaveRequests.value(key);
        QListIterator<KWebWallet::WebForm> formIt (list);

        while (formIt.hasNext()) {
            QMap<QString, QString> values, storedValues;
            const KWebWallet::WebForm form = formIt.next();
            const QString accessKey = walletKey(form);
            if (confirmSaveRequestOverwrites.contains(url)) {
                confirmSaveRequestOverwrites.remove(url);
                const int status = wallet->readMap(accessKey, storedValues);
                if (status == 0 && storedValues.count()) {
                    QListIterator<KWebWallet::WebForm::WebField> fieldIt (form.fields);
                    while (fieldIt.hasNext()) {
                        const KWebWallet::WebForm::WebField field = fieldIt.next();
                        if (storedValues.contains(field.first) &&
                            storedValues.value(field.first) != field.second) {
                            emit q->saveFormDataRequested(key, url);
                            return;
                        }
                    }
                    // If we got here it means the new credential is exactly
                    // the same as the one already cached ; so skip the
                    // re-saving part...
                    success = true;
                    continue;
                }
            }
            QListIterator<KWebWallet::WebForm::WebField> fieldIt (form.fields);
            while (fieldIt.hasNext()) {
                const KWebWallet::WebForm::WebField field = fieldIt.next();
                values.insert(field.first, field.second);
            }

            if (wallet->writeMap(accessKey, values) == 0)
                count++;
            else
                kWarning(800) << "Unable to write form data to wallet";
        }

        if (list.isEmpty() || count > 0)
          success = true;

        pendingSaveRequests.remove(key);
    } else {
        kWarning(800) << "NULL KWallet instance!";
    }

    emit q->saveFormDataCompleted(url, success);
}

void KWebWallet::KWebWalletPrivate::removeDataFromCache(const WebFormList &formList)
{
    if (!wallet) {
        kWarning(800) << "NULL KWallet instance!";
        return;
    }

    QListIterator<WebForm> formIt (formList);
    while (formIt.hasNext())
        wallet->removeEntry(walletKey(formIt.next()));
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
            QList<QWebFrame*> childFrameList;
            collectAllChildFrames(frame, childFrameList);
            QListIterator <QWebFrame *> framesIt (childFrameList);
            while (framesIt.hasNext()) {
                list << d->parseFormData(framesIt.next());
            }
        }
    }

    return list;
}

void KWebWallet::fillFormData(QWebFrame *frame, bool recursive)
{
    if (!frame)
        return;

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
        QList<QWebFrame*> childFrameList;
        collectAllChildFrames(frame, childFrameList);
        QListIterator<QWebFrame*> frameIt (childFrameList);
        while (frameIt.hasNext()) {
            QWebFrame *childFrame = frameIt.next();
            formsList = d->parseFormData(childFrame);
            if (formsList.isEmpty())
                continue;
            const QUrl url (childFrame->url());
            if (d->pendingFillRequests.contains(url)) {
                kWarning(800) << "Duplicate request rejected!!!";
            } else {
                KWebWalletPrivate::FormsData data;
                data.frame = childFrame;
                data.forms << formsList;
                d->pendingFillRequests.insert(url, data);
                urlList << url;
            }
        }
    }

    if (!urlList.isEmpty())
        fillFormDataFromCache(urlList);
}

void KWebWallet::saveFormData(QWebFrame *frame, bool recursive, bool ignorePasswordFields)
{
    if (!frame)
        return;

    WebFormList list = d->parseFormData(frame, false, ignorePasswordFields);
    if (recursive) {
        QList<QWebFrame*> childFrameList;
        collectAllChildFrames(frame, childFrameList);
        QListIterator<QWebFrame*> frameIt (childFrameList);
        while (frameIt.hasNext())
            list << d->parseFormData(frameIt.next(), false, ignorePasswordFields);
    }

    if (list.isEmpty())
        return;

    const QString key = QString::number(qHash(frame->url().toString() + frame->frameName()), 16);
    const bool isAlreadyPending = d->pendingSaveRequests.contains(key);
    d->pendingSaveRequests.insert(key, list);

    if (isAlreadyPending)
        return;

    for (int i = 0 ; i < list.count(); ++i) {
        if (hasCachedFormData(list.at(i)))
            list.takeAt(i);
    }

    if (list.isEmpty()) {
        d->confirmSaveRequestOverwrites.insert(frame->url());
        saveFormDataToCache(key);
        return;
    }

    emit saveFormDataRequested(key, frame->url());
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
    if (!frame)
        return;

    QString script;
    bool wasFilled = false;

    Q_FOREACH (const KWebWallet::WebForm& form, forms) {
        Q_FOREACH(const KWebWallet::WebForm::WebField& field, form.fields)
            script += QString::fromLatin1("document.forms[\"%1\"].elements[\"%2\"].value=\"%3\";\n")
                        .arg(form.name.isEmpty() ? form.index : form.name)
                        .arg(field.first).arg(field.second);
    }

    if (!script.isEmpty()) {
        wasFilled = true;
        frame->evaluateJavaScript(script);
    }

    emit fillFormRequestCompleted(wasFilled);
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
        connect(d->wallet, SIGNAL(walletClosed()),
                this, SLOT(_k_walletClosed()));
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
        connect(d->wallet, SIGNAL(walletClosed()),
                this, SLOT(_k_walletClosed()));
        return;
    }

    d->saveDataToCache(key);
}

void KWebWallet::removeFormDataFromCache(const WebFormList &forms)
{
    if (!d->wallet) {
        d->wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(),
                                                d->wid, KWallet::Wallet::Asynchronous);
        connect(d->wallet, SIGNAL(walletOpened(bool)),
                this, SLOT(_k_openWalletDone(bool)));
        connect(d->wallet, SIGNAL(walletClosed()),
                this, SLOT(_k_walletClosed()));
        return;
    }

    d->removeDataFromCache(forms);
    d->pendingRemoveRequests.clear();
}

#include "kwebwallet.moc"
