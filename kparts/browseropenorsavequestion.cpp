/*
    Copyright (c) 2009 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "browseropenorsavequestion.h"
#include <kmimetypetrader.h>
#include <kstandardguiitem.h>
#include <kguiitem.h>
#include <kmessagebox.h>
#include <kmimetype.h>

using namespace KParts;

class BrowserOpenOrSaveQuestionPrivate // TODO : public KDialog
{
public:
    BrowserOpenOrSaveQuestionPrivate(const KUrl& url, const QString& mimeType, const QString& suggestedFileName)
        : url(url), mimeType(mimeType), suggestedFileName(suggestedFileName) {}

    KUrl url;
    QString mimeType;
    QString suggestedFileName;
    KService::Ptr selectedService;
};

BrowserOpenOrSaveQuestion::BrowserOpenOrSaveQuestion(const KUrl& url, const QString& mimeType, const QString& suggestedFileName)
    : d(new BrowserOpenOrSaveQuestionPrivate(url, mimeType, suggestedFileName))
{
}

BrowserOpenOrSaveQuestion::~BrowserOpenOrSaveQuestion()
{
    delete d;    
}

static QString makeQuestion(const KUrl& url, const QString& mimeType, const QString& suggestedFileName)
{
    QString surl = url.prettyUrl();
    KMimeType::Ptr mime = KMimeType::mimeType(mimeType, KMimeType::ResolveAliases);
    QString comment = mimeType;

    // Test if the mimeType is not recognize as octet-stream.
    // If so then keep mime-type as comment
    if (mime && mime->name() != KMimeType::defaultMimeType()) {
        // The mime-type is known so display the comment instead of mime-type
        comment = mime->comment();
    }
    // The strange order in the i18n() calls below is due to the possibility
    // of surl containing a '%'
    if (suggestedFileName.isEmpty())
        return i18n("Open '%2'?\nType: %1", comment, surl);
    else
        return i18n("Open '%3'?\nName: %2\nType: %1", comment, suggestedFileName, surl);
}

BrowserOpenOrSaveQuestion::Result BrowserOpenOrSaveQuestion::askOpenOrSave(QWidget* parent)
{
    QString question = makeQuestion(d->url, d->mimeType, d->suggestedFileName);

    // text and icon used for the open button
    KGuiItem openItem;
    // TODO get full list
    KService::Ptr offer = KMimeTypeTrader::self()->preferredService(d->mimeType, "Application");
    if (offer && !offer->name().isEmpty())
        openItem = KGuiItem(i18n("&Open with '%1'", offer->name()), offer->icon());
    else
        openItem = KGuiItem(i18n("&Open with..."), "system-run");

    int choice = KMessageBox::questionYesNoCancel(
        parent, question, d->url.host(),
        KStandardGuiItem::saveAs(), openItem, KStandardGuiItem::cancel(),
        QLatin1String("askSave")+ d->mimeType); // dontAskAgainName, KEEP IN SYNC with kdebase/runtime/keditfiletype/filetypedetails.cpp!!!

    return choice == KMessageBox::Yes ? Save : (choice == KMessageBox::No ? Open : Cancel);
}

#if 0
KService::Ptr BrowserOpenOrSaveQuestion::selectedService() const
{
    // TODO set it somewhere
    return d->selectedService;
}
#endif

BrowserOpenOrSaveQuestion::Result BrowserOpenOrSaveQuestion::askEmbedOrSave(QWidget* parent, int flags)
{
    // SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC
    // NOTE: Keep this function in sync with
    // kdebase/runtime/keditfiletype/filetypedetails.cpp
    //       FileTypeDetails::updateAskSave()

    KMimeType::Ptr mime = KMimeType::mimeType(d->mimeType, KMimeType::ResolveAliases);
    // Don't ask for:
    // - html (even new tabs would ask, due to about:blank!)
    // - dirs obviously (though not common over HTTP :),
    // - images (reasoning: no need to save, most of the time, because fast to see)
    // e.g. postscript is different, because takes longer to read, so
    // it's more likely that the user might want to save it.
    // - multipart/* ("server push", see kmultipart)
    // KEEP IN SYNC!!!
    if (flags != (int)BrowserRun::AttachmentDisposition && mime && (
         mime->is("text/html") ||
         mime->is("application/xml") ||
         mime->is("inode/directory") ||
         d->mimeType.startsWith("image") ||
         mime->is("multipart/x-mixed-replace") ||
         mime->is("multipart/replace")))
        return Open;

    QString question = makeQuestion(d->url, d->mimeType, d->suggestedFileName);

    // don't use KStandardGuiItem::open() here which has trailing ellipsis!
    int choice = KMessageBox::questionYesNoCancel(
        parent, question, d->url.host(),
        KStandardGuiItem::saveAs(), KGuiItem(i18n("&Open"), "document-open"), KStandardGuiItem::cancel(),
        QLatin1String("askEmbedOrSave")+ d->mimeType); // dontAskAgainName, KEEP IN SYNC!!!
    return choice == KMessageBox::Yes ? Save : (choice == KMessageBox::No ? Open : Cancel);
    // SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC
}
