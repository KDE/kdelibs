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
#include <QtCore/QScopedPointer>
#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QWebElementCollection>
#include <qwindowdefs.h>

#define QL1S(x)   QLatin1String(x)
#define QL1C(x)   QLatin1Char(x)

// Javascript used to extract/set data from <form> elements.
#define FILLABLE_FORM_ELEMENT_EXTRACTOR_JS "(function (){ \
    var forms; \
    var formList = document.forms; \
    if (formList.length > 0) { \
        forms = new Array; \
        for (var i = 0; i < formList.length; ++i) { \
            var inputList = formList[i].elements; \
            if (inputList.length < 1) { \
                continue; \
            } \
            var formObject = new Object; \
            formObject.name = formList[i].name; \
            if (typeof(formObject.name) != 'string') { \
                formObject.name = String(formList[i].id); \
            } \
            formObject.index = i; \
            formObject.elements = new Array; \
            for (var j = 0; j < inputList.length; ++j) { \
                if (inputList[j].type != 'text' && inputList[j].type != 'email' && inputList[j].type != 'password') { \
                    continue; \
                } \
                if (inputList[j].disabled || inputList[j].autocomplete == 'off') { \
                    continue; \
                } \
                var element = new Object; \
                element.name = inputList[j].name; \
                if (typeof(element.name) != 'string' ) { \
                    element.name = String(inputList[j].id); \
                } \
                element.value = String(inputList[j].value); \
                element.type = String(inputList[j].type); \
                element.readonly = Boolean(inputList[j].readOnly); \
                formObject.elements.push(element); \
            } \
            if (formObject.elements.length > 0) { \
                forms.push(formObject); \
            } \
        } \
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

static QUrl urlForFrame(QWebFrame* frame)
{
    return (frame->url().isEmpty() ? frame->baseUrl().resolved(frame->url()) : frame->url());
}

/*
  Returns the top most window associated with widget.

  Unlike QWidget::window(), this function does its best to find and return the
  main application window associated with a given widget. It will not stop when
  it encounters a dialog which likely "has (or could have) a window-system frame".
*/
static QWidget* topLevelWindow(QObject* obj)
{
    QWebPage *page = qobject_cast<QWebPage*>(obj);
    QWidget* widget = (page ? page->view() : qobject_cast<QWidget*>(page));
    while (widget && widget->parentWidget()) {
        widget = widget->parentWidget();
    }
    return (widget ? widget->window() : 0);
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
    void openWallet();

    // Private slots...
    void _k_openWalletDone(bool);
    void _k_walletClosed();

    WId wid;
    KWebWallet *q;
    QScopedPointer<KWallet::Wallet> wallet;
    KWebWallet::WebFormList pendingRemoveRequests;
    QHash<QUrl, FormsData> pendingFillRequests;
    QHash<QString, KWebWallet::WebFormList> pendingSaveRequests;
    QSet<QUrl> confirmSaveRequestOverwrites;
};

KWebWallet::KWebWalletPrivate::KWebWalletPrivate(KWebWallet *parent)
                              :wid (0), q(parent)
{
}

KWebWallet::WebFormList KWebWallet::KWebWalletPrivate::parseFormData(QWebFrame *frame, bool fillform, bool ignorepasswd)
{
    Q_ASSERT(frame);

    KWebWallet::WebFormList list;

    const QVariant result (frame->evaluateJavaScript(QL1S(FILLABLE_FORM_ELEMENT_EXTRACTOR_JS)));
    const QVariantList results (result.toList());
    Q_FOREACH (const QVariant &formVariant, results) {
        QVariantMap map = formVariant.toMap();
        KWebWallet::WebForm form;
        form.url = urlForFrame(frame);
        form.name = map[QL1S("name")].toString();
        form.index = map[QL1S("index")].toString();
        bool formHasPasswords = false;
        const QVariantList elements = map[QL1S("elements")].toList();
        QList<KWebWallet::WebForm::WebField> inputFields;
        Q_FOREACH (const QVariant &element, elements) {
            QVariantMap elementMap (element.toMap());
            const QString name (elementMap[QL1S("name")].toString());
            const QString value (ignorepasswd ? QString() : elementMap[QL1S("value")].toString());
            if (name.isEmpty()) {
                continue;
            }
            if (fillform && elementMap[QL1S("readonly")].toBool()) {
                continue;
            }
            if (elementMap[QL1S("type")].toString().compare(QL1S("password"), Qt::CaseInsensitive) == 0) {
                if (!fillform && value.isEmpty())
                    continue;
                formHasPasswords = true;
            }
            inputFields.append(qMakePair(name, value));
        }

        // Only add the input fields on form save requests...
        if (formHasPasswords || fillform) {
            form.fields = inputFields;
        }

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

    QString lastKey;
    QMap<QString, QString> cachedValues;
    QMutableListIterator <WebForm> formIt (formList);

    while (formIt.hasNext()) {
        KWebWallet::WebForm &form = formIt.next();
        const QString key (walletKey(form));
        if (key != lastKey && wallet->readMap(key, cachedValues) != 0) {
            kWarning(800) << "Unable to read form data for key:" << key;
            continue;
        }

        for (int i = 0, count = form.fields.count(); i < count; ++i) {
            form.fields[i].second = cachedValues.value(form.fields[i].first);
        }
        lastKey = key;
    }
}

void KWebWallet::KWebWalletPrivate::saveDataToCache(const QString &key)
{
    // Make sure the specified keys exists before acting on it. See BR# 270209.
    if (!pendingSaveRequests.contains(key)) {
        return;
    }

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

void KWebWallet::KWebWalletPrivate::openWallet()
{
    if (!wallet.isNull()) {
        return;
    }

    wallet.reset(KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(),
                                             wid, KWallet::Wallet::Asynchronous));

    if (wallet.isNull()) {
        return;
    }

    connect(wallet.data(), SIGNAL(walletOpened(bool)), q, SLOT(_k_openWalletDone(bool)));
    connect(wallet.data(), SIGNAL(walletClosed()), q, SLOT(_k_walletClosed()));
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
            QList<QUrl> urlList;
            QMutableHashIterator<QUrl, FormsData> requestIt (pendingFillRequests);
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
        delete wallet.take();
    }
}

void KWebWallet::KWebWalletPrivate::_k_walletClosed()
{
    if (wallet)
      wallet.take()->deleteLater();

    emit q->walletClosed();
}

KWebWallet::KWebWallet(QObject *parent, WId wid)
           :QObject(parent), d(new KWebWalletPrivate(this))
{
    if (!wid) {
        // If wid is 0, make a best effort attempt to discern it from our
        // parent object.
        QWidget* widget = topLevelWindow(parent);
        if (widget) {
            wid = widget->winId();
        }
    }

    d->wid = wid;
}

KWebWallet::~KWebWallet()
{
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

    QList<QUrl> urlList;
    WebFormList formsList = d->parseFormData(frame);
    if (!formsList.isEmpty()) {
        const QUrl url (urlForFrame(frame));
        if (d->pendingFillRequests.contains(url)) {
            kWarning(800) << "Duplicate request rejected!";
        } else {
            KWebWalletPrivate::FormsData data;
            data.frame = QPointer<QWebFrame>(frame);
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
                data.frame = QPointer<QWebFrame>(childFrame);
                data.forms << formsList;
                d->pendingFillRequests.insert(url, data);
                urlList << url;
            }
        }
    }

    if (!urlList.isEmpty())
        fillFormDataFromCache(urlList);
}

static void createSaveKeyFor(QWebFrame* frame, QString* key)
{
    QUrl frameUrl(urlForFrame(frame));
    frameUrl.setPassword(QString());
    frameUrl.setPassword(QString());

    QString keyStr = frameUrl.toString();
    if (!frame->frameName().isEmpty())
        keyStr += frame->frameName();

    *key = QString::number(qHash(keyStr), 16);
}

void KWebWallet::saveFormData(QWebFrame *frame, bool recursive, bool ignorePasswordFields)
{
    if (!frame)
        return;

    QString key;
    createSaveKeyFor(frame, &key);
    if (d->pendingSaveRequests.contains(key))
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

    d->pendingSaveRequests.insert(key, list);

    QMutableListIterator<WebForm> it (list);
    while (it.hasNext()) {
        const WebForm form (it.next());
        if (hasCachedFormData(form))
            it.remove();
    }

    if (list.isEmpty()) {
        d->confirmSaveRequestOverwrites.insert(urlForFrame(frame));
        saveFormDataToCache(key);
        return;
    }

    emit saveFormDataRequested(key, urlForFrame(frame));
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

void KWebWallet::fillWebForm(const QUrl &url, const KWebWallet::WebFormList &forms)
{
    QPointer<QWebFrame> frame = d->pendingFillRequests.value(url).frame;
    if (!frame)
        return;

    QString script;
    bool wasFilled = false;

    Q_FOREACH (const KWebWallet::WebForm& form, forms) {
        Q_FOREACH(const KWebWallet::WebForm::WebField& field, form.fields) {
            QString value = field.second;
            value.replace(QL1C('\\'), QL1S("\\\\"));
            script += QString::fromLatin1("if (document.forms[\"%1\"].elements[\"%2\"]) document.forms[\"%1\"].elements[\"%2\"].value=\"%3\";\n")
                        .arg((form.name.isEmpty() ? form.index : form.name))
                        .arg(field.first).arg(value);
        }
    }

    if (!script.isEmpty()) {
        wasFilled = true;
        frame.data()->evaluateJavaScript(script);
    }

    emit fillFormRequestCompleted(wasFilled);
}

KWebWallet::WebFormList KWebWallet::formsToFill(const QUrl &url) const
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

void KWebWallet::fillFormDataFromCache(const QList<QUrl> &urlList)
{
    if (d->wallet) {
        QListIterator<QUrl> urlIt (urlList);
        while (urlIt.hasNext()) {
            const QUrl url = urlIt.next();
            WebFormList list = formsToFill(url);
            d->fillDataFromCache(list);
            fillWebForm(url, list);
        }
        d->pendingFillRequests.clear();
    }
    d->openWallet();
}

void KWebWallet::saveFormDataToCache(const QString &key)
{
    if (d->wallet) {
        d->saveDataToCache(key);
        return;
    }
    d->openWallet();
}

void KWebWallet::removeFormDataFromCache(const WebFormList &forms)
{
    if (d->wallet) {
        d->removeDataFromCache(forms);
        d->pendingRemoveRequests.clear();
        return;
    }
    d->openWallet();
}

#include "moc_kwebwallet.cpp"
