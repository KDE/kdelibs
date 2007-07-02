/*****************************************************************************
 * Copyright (C) 2006 by Peter Penz <peter.penz@gmx.at>                      *
 * Copyright (C) 2006 by Aaron J. Seigo <aseigo@kde.org>                     *
 * Copyright (C) 2007 by Kevin Ottens <ervin@kde.org>                        *
 * Copyright (C) 2007 by Urs Wolfer <uwolfer @ kde.org>                      *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License version 2 as published by the Free Software Foundation.           *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#include "kurlnavigator.h"

#include "kfileplacesselector_p.h"
#include "kprotocolcombo_p.h"
#include "kurldropdownbutton_p.h"
#include "kurlnavigatorbutton_p.h"
#include "kurltogglebutton_p.h"

#include <kfileitem.h>
#include <kicon.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmenu.h>
#include <kprotocolinfo.h>
#include <kurlcombobox.h>
#include <kurlcompletion.h>

#include <QtCore/QDir>
#include <QtCore/QLinkedList>
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QKeyEvent>
#include <QtGui/QBoxLayout>
#include <QtGui/QLabel>

/**
 * @brief Represents the history element of an URL.
 *
 * A history element contains the URL and
 * the x- and y-position of the content.
 */
class HistoryElem
{
public:
    HistoryElem();
    HistoryElem(const KUrl& url);
    ~HistoryElem(); // non virtual

    const KUrl& url() const;

    void setContentsX(int x);
    int contentsX() const;

    void setContentsY(int y);
    int contentsY() const;

private:
    KUrl m_url;
    int m_contentsX;
    int m_contentsY;
};

HistoryElem::HistoryElem() :
    m_url(),
    m_contentsX(0),
    m_contentsY(0)
{
}

HistoryElem::HistoryElem(const KUrl& url) :
    m_url(url),
    m_contentsX(0),
    m_contentsY(0)
{
}

HistoryElem::~HistoryElem()
{
}

inline const KUrl& HistoryElem::url() const
{
    return m_url;
}

inline void HistoryElem::setContentsX(int x)
{
    m_contentsX = x;
}

inline int HistoryElem::contentsX() const
{
    return m_contentsX;
}

inline void HistoryElem::setContentsY(int y)
{
    m_contentsY = y;
}

inline int HistoryElem::contentsY() const
{
    return m_contentsY;
}

class KUrlNavigator::Private
{
public:
    Private(KUrlNavigator* q, KFilePlacesModel* placesModel);

    void slotReturnPressed(const QString&);
    void slotRemoteHostActivated();
    void slotProtocolChanged(const QString&);
    void openPathSelectorMenu();

    /**
     * Appends the widget at the end of the URL navigator. It is assured
     * that the filler widget remains as last widget to fill the remaining
     * width.
     */
    void appendWidget(QWidget* widget, int stretch = 0);

    /**
     * Switches the navigation bar between the breadcrumb view and the
     * traditional view (see setUrlEditable()) and is connected to the clicked signal
     * of the navigation bar button.
     */
    void switchView();

    /** Emits the signal urlsDropped(). */
    void dropUrls(const KUrl::List& urls, const KUrl& destination);

    void updateContent();

    /**
     * Updates all buttons to have one button for each part of the
     * path \a path. Existing buttons, which are available by m_navButtons,
     * are reused if possible. If the path is longer, new buttons will be
     * created, if the path is shorter, the remaining buttons will be deleted.
     * @param startIndex    Start index of path part (/), where the buttons
     *                      should be created for each following part.
     */
    void updateButtons(const QString& path, int startIndex);

    /**
     * Updates the visibility state of all buttons describing the URL. If the
     * width of the URL navigator is too small, the buttons representing the upper
     * paths of the URL will be hidden and moved to a drop down menu.
     */
    void updateButtonVisibility();

    /**
     * Deletes all URL navigator buttons. m_navButtons is
     * empty after this operation.
     */
    void deleteButtons();

    /**
     * Retrieves the place path for the path \a path.
     * E. g. for the path "fish://root@192.168.0.2/var/lib" the string
     * "fish://root@192.168.0.2/" will be returned, which leads to the
     * navigation indication 'Custom Path > var > lib". For e. g.
     * "settings:///System/" the path "settings:///" will be returned.
     */
    QString retrievePlacePath(const QString& path) const;

    bool m_editable;
    bool m_active;
    int m_historyIndex;

    QHBoxLayout* m_layout;

    QList<HistoryElem> m_history;
    KFilePlacesSelector* m_placesSelector;
    KUrlComboBox* m_pathBox;
    KProtocolCombo* m_protocols;
    QLabel* m_protocolSeparator;
    KLineEdit* m_host;
    KUrlDropDownButton* m_dropDownButton;
    QLinkedList<KUrlNavigatorButton*> m_navButtons;
    KUrlButton* m_toggleEditableMode;
    QString m_homeUrl;
    QStringList m_customProtocols;
    KUrlNavigator* q;
};


KUrlNavigator::Private::Private(KUrlNavigator* q, KFilePlacesModel* placesModel) :
    m_editable(false),
    m_active(true),
    m_historyIndex(0),
    m_layout(new QHBoxLayout),
    m_placesSelector(0),
    m_pathBox(0),
    m_protocols(0),
    m_protocolSeparator(0),
    m_host(0),
    m_dropDownButton(0),
    m_toggleEditableMode(0),
    m_customProtocols(QStringList()),
    q(q)
{
    m_layout->setSpacing(0);
    m_layout->setMargin(0);

    // initialize the places selector
    QPalette p;
    p.setColor(QPalette::Background, Qt::transparent);
    q->setPalette(p);

    if (placesModel) {
        m_placesSelector = new KFilePlacesSelector(q, placesModel);
        connect(m_placesSelector, SIGNAL(placeActivated(const KUrl&)),
                q, SLOT(setUrl(const KUrl&)));
    }

    m_dropDownButton = new KUrlDropDownButton(q);
    connect(m_dropDownButton, SIGNAL(clicked()),
            q, SLOT(openPathSelectorMenu()));

    // initialize the path box of the traditional view
    m_pathBox = new KUrlComboBox(KUrlComboBox::Directories, true, q);
    m_pathBox->setMinimumWidth(50);

    KUrlCompletion* kurlCompletion = new KUrlCompletion(KUrlCompletion::DirCompletion);
    m_pathBox->setCompletionObject(kurlCompletion);
    m_pathBox->setAutoDeleteCompletionObject(true);

    connect(m_pathBox, SIGNAL(returnPressed(QString)),
            q, SLOT(slotReturnPressed(QString)));
    connect(m_pathBox, SIGNAL(returnPressed()),
            q, SIGNAL(returnPressed()));
    connect(m_pathBox, SIGNAL(urlActivated(KUrl)),
            q, SLOT(setUrl(KUrl)));

    m_toggleEditableMode = new KUrlToggleButton(q);
    connect(m_toggleEditableMode, SIGNAL(clicked()),
            q, SLOT(switchView()));

    if (m_placesSelector) {
        m_layout->addWidget(m_placesSelector);
    }
    m_layout->addWidget(m_dropDownButton);
    m_layout->addWidget(m_pathBox, 1);
    m_layout->addWidget(m_toggleEditableMode);
}

void KUrlNavigator::Private::appendWidget(QWidget* widget, int stretch)
{
    m_layout->insertWidget(m_layout->count() - 1, widget, stretch);
}

void KUrlNavigator::Private::slotReturnPressed(const QString& text)
{
    // Parts of the following code have been taken
    // from the class KateFileSelector located in
    // kate/app/katefileselector.hpp of Kate.
    // Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
    // Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
    // Copyright (C) 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>

    KUrl typedUrl(text);
    if (typedUrl.hasPass()) {
        typedUrl.setPass(QString());
    }

    QStringList urls = m_pathBox->urls();
    urls.removeAll(typedUrl.url());
    urls.prepend(typedUrl.url());
    m_pathBox->setUrls(urls, KUrlComboBox::RemoveBottom);

    q->setUrl(typedUrl);
    // The URL might have been adjusted by KUrlNavigator::setUrl(), hence
    // synchronize the result in the path box.
    m_pathBox->setUrl(q->url());
}

void KUrlNavigator::Private::slotRemoteHostActivated()
{
    KUrl u = q->url();

    KUrl n(m_protocols->currentProtocol() + "://" + m_host->text());

    if (n.scheme() != u.scheme() ||
            n.host() != u.host() ||
            n.user() != u.user() ||
            n.port() != u.port()) {
        u.setScheme(n.scheme());
        u.setHost(n.host());
        u.setUser(n.user());
        u.setPort(n.port());

        //TODO: get rid of this HACK for file:///!
        if (u.scheme() == "file") {
            u.setHost("");
            if (u.path().isEmpty()) {
                u.setPath("/");
            }
        }

        q->setUrl(u);
    }
}

void KUrlNavigator::Private::slotProtocolChanged(const QString& protocol)
{
    KUrl url;
    url.setScheme(protocol);
    //url.setPath(KProtocolInfo::protocolClass(protocol) == ":local" ? "/" : "");
    url.setPath("/");
    QLinkedList<KUrlNavigatorButton*>::const_iterator it = m_navButtons.begin();
    const QLinkedList<KUrlNavigatorButton*>::const_iterator itEnd = m_navButtons.end();
    while (it != itEnd) {
        (*it)->close();
        (*it)->deleteLater();
        ++it;
    }
    m_navButtons.clear();

    if (KProtocolInfo::protocolClass(protocol) == ":local") {
        q->setUrl(url);
    } else {
        if (!m_host) {
            m_protocolSeparator = new QLabel("://", q);
            appendWidget(m_protocolSeparator);
            m_host = new KLineEdit(q);
            m_host->setClearButtonShown(true);
            appendWidget(m_host, 1);

            connect(m_host, SIGNAL(editingFinished()),
                    q, SLOT(slotRemoteHostActivated()));
            connect(m_host, SIGNAL(returnPressed()),
                    q, SIGNAL(returnPressed()));
        } else {
            m_host->setText("");
        }
        m_protocolSeparator->show();
        m_host->show();
        m_host->setFocus();
    }
}

void KUrlNavigator::Private::openPathSelectorMenu()
{
    KMenu* popup = new KMenu(q);

    QString spacer;
    QLinkedList<KUrlNavigatorButton*>::iterator it = m_navButtons.begin();
    const QLinkedList<KUrlNavigatorButton*>::const_iterator itEnd = m_navButtons.end();
    while (it != itEnd) {
        const QString text = spacer + (*it)->text();
        spacer.append("  ");

        QAction* action = new QAction(text, popup);
        action->setData(QVariant((*it)->index()));
        popup->addAction(action);

        ++it;
    }

    const QAction* activatedAction = popup->exec(QCursor::pos());
    if (activatedAction != 0) {
        const int index = activatedAction->data().toInt();
        q->setUrl(q->url(index));
    }

    popup->deleteLater();
}

#if 0
void KUrlNavigator::slotRedirection(const KUrl& oldUrl, const KUrl& newUrl)
{
// kDebug() << "received redirection to " << newUrl << endl;
    kDebug() << "received redirection from " << oldUrl << " to " << newUrl << endl;
    /*    UrlStack::iterator it = m_urls.find(oldUrl);
        if (it != m_urls.end())
        {
            m_urls.erase(++it, m_urls.end());
        }

        m_urls.append(newUrl);*/
}
#endif

void KUrlNavigator::Private::switchView()
{
    m_toggleEditableMode->setFocus();
    m_editable = !m_editable;
    m_toggleEditableMode->setChecked(m_editable);
    updateContent();
    if (q->isUrlEditable()) {
        m_pathBox->setFocus();
    } else {
        q->setUrl(m_pathBox->currentText());
    }
    emit q->requestActivation();
}

void KUrlNavigator::Private::dropUrls(const KUrl::List& urls,
                                      const KUrl& destination)
{
    emit q->urlsDropped(urls, destination);
}

void KUrlNavigator::Private::updateContent()
{
    if (m_placesSelector) {
        m_placesSelector->updateSelection(q->url());
    }

    if (m_editable) {
        delete m_protocols; m_protocols = 0;
        delete m_protocolSeparator; m_protocolSeparator = 0;
        delete m_host; m_host = 0;
        m_dropDownButton->hide();
        deleteButtons();
        m_toggleEditableMode->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

        q->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        m_pathBox->show();
        m_pathBox->setUrl(q->url());
    } else {
        const QString path = q->url().pathOrUrl();

        q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        m_pathBox->hide();
        m_toggleEditableMode->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        // get the data from the currently selected place
        KUrl placeUrl = KUrl();
        if (m_placesSelector)
            placeUrl = m_placesSelector->selectedPlaceUrl();

        const QString placePath = placeUrl.isValid() ? placeUrl.pathOrUrl() : retrievePlacePath(path);
        const uint len = placePath.length();

        // calculate the start point for the URL navigator buttons by counting
        // the slashs inside the place URL
        int slashCount = 0;
        for (uint i = 0; i < len; ++i) {
            if (placePath.at(i) == QChar('/')) {
                ++slashCount;
            }
        }
        if ((len > 0) && placePath.at(len - 1) == QChar('/')) {
            Q_ASSERT(slashCount > 0);
            --slashCount;
        }

        const KUrl currentUrl = q->url();
        if (!currentUrl.isLocalFile() && !placeUrl.isValid()) {
            QString protocol = currentUrl.scheme();
            if (!m_protocols) {
                deleteButtons();
                m_protocols = new KProtocolCombo(protocol, q);
                appendWidget(m_protocols);
                if (!m_customProtocols.isEmpty()) {
                    m_protocols->setCustomProtocols(m_customProtocols);
                }
                connect(m_protocols, SIGNAL(activated(QString)),
                        q, SLOT(slotProtocolChanged(QString)));
            } else {
                m_protocols->setProtocol(protocol);
            }
            m_protocols->show();

            if (KProtocolInfo::protocolClass(protocol) != ":local") {
                QString hostText = currentUrl.host();

                if (!currentUrl.user().isEmpty()) {
                    hostText = currentUrl.user() + '@' + hostText;
                }

                if (currentUrl.port() != -1) {
                    hostText = hostText + ':' + QString::number(currentUrl.port());
                }

                if (!m_host) {
                    // ######### TODO: this code is duplicated from slotProtocolChanged!
                    m_protocolSeparator = new QLabel("://", q);
                    appendWidget(m_protocolSeparator);
                    m_host = new KLineEdit(hostText, q);
                    m_host->setClearButtonShown(true);
                    appendWidget(m_host, 1);

                    connect(m_host, SIGNAL(editingFinished()),
                            q, SLOT(slotRemoteHostActivated()));
                    connect(m_host, SIGNAL(returnPressed()),
                            q, SIGNAL(returnPressed()));
                } else {
                    m_host->setText(hostText);
                }
                m_protocolSeparator->show();
                m_host->show();
            } else {
                delete m_protocolSeparator; m_protocolSeparator = 0;
                delete m_host; m_host = 0;
            }
        } else if (m_protocols) {
            m_protocols->hide();

            if (m_host) {
                m_protocolSeparator->hide();
                m_host->hide();
            }
        }

        updateButtons(path, slashCount);
    }
}

void KUrlNavigator::Private::updateButtons(const QString& path, int startIndex)
{
    QLinkedList<KUrlNavigatorButton*>::iterator it = m_navButtons.begin();
    const QLinkedList<KUrlNavigatorButton*>::const_iterator itEnd = m_navButtons.end();
    bool createButton = false;
    const KUrl currentUrl = q->url();

    int idx = startIndex;
    bool hasNext = true;
    do {
        createButton = (it == itEnd);

        const QString dirName = path.section('/', idx, idx);
        const bool isFirstButton = (idx == startIndex);
        hasNext = isFirstButton || !dirName.isEmpty();
        if (hasNext) {
            QString text;
            if (isFirstButton) {
                // the first URL navigator button should get the name of the
                // place instead of the directory name
                if (m_placesSelector) {
                    const KUrl placeUrl = m_placesSelector->selectedPlaceUrl();
                    text = m_placesSelector->selectedPlaceText();
                }
                if (text.isEmpty()) {
                    if (currentUrl.isLocalFile()) {
                        text = i18n("Custom Path");
                    } else {
                        ++idx;
                        continue;
                    }
                }
            }

            KUrlNavigatorButton* button = 0;
            if (createButton) {
                button = new KUrlNavigatorButton(idx, q);
                connect(button, SIGNAL(urlsDropped(const KUrl::List&, const KUrl&)),
                        q, SLOT(dropUrls(const KUrl::List&, const KUrl&)));
                appendWidget(button);
            } else {
                button = *it;
                button->setIndex(idx);
            }

            if (isFirstButton) {
                button->setText(text);
            }

            if (createButton) {
                m_navButtons.append(button);
            } else {
                ++it;
            }
            ++idx;
        }
    } while (hasNext);

    // delete buttons which are not used anymore
    QLinkedList<KUrlNavigatorButton*>::iterator itBegin = it;
    while (it != itEnd) {
        (*it)->close();
        (*it)->deleteLater();
        ++it;
    }
    m_navButtons.erase(itBegin, m_navButtons.end());

    updateButtonVisibility();
}

void KUrlNavigator::Private::updateButtonVisibility()
{
    if (m_editable) {
        return;
    }

    const int buttonsCount = m_navButtons.count();
    if (buttonsCount == 0) {
        m_dropDownButton->hide();
        return;
    }

    int hiddenButtonsCount = 0;

    int availableWidth = q->width() - 20;

    if (m_placesSelector && m_placesSelector->isVisible())
        availableWidth -= m_placesSelector->width();

    if (m_dropDownButton->isVisible()) {
        availableWidth -= m_dropDownButton->width();
    }

    if ((m_protocols != 0) && m_protocols->isVisible()) {
        availableWidth -= m_protocols->width();
    }

    QLinkedList<KUrlNavigatorButton*>::iterator it = m_navButtons.end();
    const QLinkedList<KUrlNavigatorButton*>::const_iterator itBegin = m_navButtons.begin();
    while (it != itBegin) {
        --it;
        KUrlNavigatorButton* button = (*it);
        availableWidth -= button->minimumWidth();
        if (availableWidth <= 0) {
            button->hide();
            ++hiddenButtonsCount;
        }
        else {
            button->show();
        }
    }

    Q_ASSERT(hiddenButtonsCount <= buttonsCount);
    if (hiddenButtonsCount == buttonsCount) {
        // assure that at least one button is visible
        hiddenButtonsCount = buttonsCount - 1;
    }

    int index = 0;
    it = m_navButtons.begin();
    const QLinkedList<KUrlNavigatorButton*>::const_iterator itEnd = m_navButtons.end();
    while (it != itEnd) {
        (*it)->setVisible(index >= hiddenButtonsCount);
        ++it;
        ++index;
    }

    m_dropDownButton->setVisible(hiddenButtonsCount != 0);
}

void KUrlNavigator::Private::deleteButtons()
{
    QLinkedList<KUrlNavigatorButton*>::iterator itBegin = m_navButtons.begin();
    QLinkedList<KUrlNavigatorButton*>::iterator itEnd = m_navButtons.end();
    QLinkedList<KUrlNavigatorButton*>::iterator it = itBegin;
    while (it != itEnd) {
        (*it)->close();
        (*it)->deleteLater();
        ++it;
    }
    m_navButtons.erase(itBegin, itEnd);
}

QString KUrlNavigator::Private::retrievePlacePath(const QString& path) const
{
    int idx = path.indexOf(QString("///"));
    if (idx >= 0) {
        idx += 3;
    } else {
        idx = path.indexOf(QString("//"));
        idx = path.indexOf("/", (idx < 0) ? 0 : idx + 2);
    }
    return (idx < 0) ? path : path.left(idx);
}

////

KUrlNavigator::KUrlNavigator(KFilePlacesModel* placesModel,
                             const KUrl& url,
                             QWidget* parent) :
    QWidget(parent),
    d(new Private(this, placesModel))
{
    d->m_history.prepend(HistoryElem(url));

    QFontMetrics fontMetrics(font());
    setMinimumHeight(fontMetrics.height() + 10);

    setLayout(d->m_layout);
    setMinimumWidth(100);

    d->updateContent();
}

KUrlNavigator::~KUrlNavigator()
{
    delete d;
}

const KUrl& KUrlNavigator::url() const
{
    Q_ASSERT(!d->m_history.empty());
    return d->m_history[d->m_historyIndex].url();
}

KUrl KUrlNavigator::uncommittedUrl() const
{
    if (isUrlEditable()) {
        return KUrl(d->m_pathBox->currentText());
    } else {
        return KUrl(d->m_protocols->currentProtocol() + "://" + d->m_host->text());
    }
}

KUrl KUrlNavigator::url(int index) const
{
    if (index < 0) {
        index = 0;
    }

    // keep scheme, hostname etc. maybe we will need this in the future
    // for e.g. browsing ftp repositories.
    KUrl newUrl(url());
    newUrl.setPath(QString());

    QString path(url().path());
    if (!path.isEmpty()) {
        if (index == 0) {
            // prevent the last "/" from being stripped
            // or we end up with an empty path
            path = "/";
        } else {
            path = path.section('/', 0, index);
        }
    }

    newUrl.setPath(path);
    return newUrl;
}

bool KUrlNavigator::goBack()
{
    const int count = d->m_history.count();
    if (d->m_historyIndex < count - 1) {
        ++d->m_historyIndex;
        d->updateContent();
        emit urlChanged(url());
        emit historyChanged();
        return true;
    }

    return false;
}

bool KUrlNavigator::goForward()
{
    if (d->m_historyIndex > 0) {
        --d->m_historyIndex;
        d->updateContent();
        emit urlChanged(url());
        emit historyChanged();
        return true;
    }

    return false;
}

bool KUrlNavigator::goUp()
{
    const KUrl& currentUrl = url();
    const KUrl upUrl = currentUrl.upUrl();
    if (upUrl != currentUrl) {
        setUrl(upUrl);
        return true;
    }

    return false;
}

void KUrlNavigator::goHome()
{
    if (d->m_homeUrl.isEmpty()) {
        setUrl(QDir::homePath());
    } else {
        setUrl(d->m_homeUrl);
    }
}


void KUrlNavigator::setHomeUrl(const QString& homeUrl)
{
    d->m_homeUrl = homeUrl;
}

void KUrlNavigator::setUrlEditable(bool editable)
{
    if (d->m_editable != editable) {
        d->switchView();
    }
}

bool KUrlNavigator::isUrlEditable() const
{
    return d->m_editable;
}

void KUrlNavigator::setActive(bool active)
{
    if (active != d->m_active) {
        d->m_active = active;
        update();
        if (active) {
            emit activated();
        }
    }
}

void KUrlNavigator::setUrl(const KUrl& url)
{
    QString urlStr(url.pathOrUrl());

    // TODO: a patch has been submitted by Filip Brcic which adjusts
    // the URL for tar and zip files. See https://bugs.kde.org/show_bug.cgi?id=142781
    // for details. The URL navigator part of the patch has not been committed yet,
    // as the URL navigator will be subject of change and
    // we might think of a more generic approach to check the protocol + MIME type for
    // this use case.

    //kDebug() << "setUrl(" << url << ")" << endl;
    if (urlStr.length() > 0 && urlStr.at(0) == '~') {
        // replace '~' by the home directory
        urlStr.remove(0, 1);
        urlStr.insert(0, QDir::homePath());
    }

    const KUrl transformedUrl(urlStr);

    if (d->m_historyIndex > 0) {
        // Check whether the previous element of the history has the same Url.
        // If yes, just go forward instead of inserting a duplicate history
        // element.
        HistoryElem& prevHistoryElem = d->m_history[d->m_historyIndex - 1];
        if (transformedUrl == prevHistoryElem.url()) {
            goForward();
//             kDebug() << "goin' forward in history" << endl;
            return;
        }
    }

    if (this->url() != transformedUrl) {
        // don't insert duplicate history elements
//         kDebug() << "current url == transformedUrl" << endl;
        d->m_history.insert(d->m_historyIndex, HistoryElem(transformedUrl));

        emit urlChanged(transformedUrl);
        emit historyChanged();

        // Prevent an endless growing of the history: remembering
        // the last 100 Urls should be enough...
        if (d->m_historyIndex > 100) {
            d->m_history.removeFirst();
            --d->m_historyIndex;
        }
    }

    d->updateContent();

    requestActivation();
}

void KUrlNavigator::requestActivation()
{
    setActive(true);
}

void KUrlNavigator::savePosition(int x, int y)
{
    HistoryElem& hist = d->m_history[d->m_historyIndex];
    hist.setContentsX(x);
    hist.setContentsY(y);
}

void KUrlNavigator::keyReleaseEvent(QKeyEvent* event)
{
    QWidget::keyReleaseEvent(event);
    if (isUrlEditable() && (event->key() == Qt::Key_Escape)) {
        setUrlEditable(false);
    }
}

void KUrlNavigator::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MidButton) {
        QClipboard* clipboard = QApplication::clipboard();
        const QMimeData* mimeData = clipboard->mimeData();
        if (mimeData->hasText()) {
            const QString text = mimeData->text();
            setUrl(KUrl(text));
        }
    }
    QWidget::mouseReleaseEvent(event);
}

void KUrlNavigator::resizeEvent(QResizeEvent* event)
{
    QTimer::singleShot(0, this, SLOT(updateButtonVisibility()));
    QWidget::resizeEvent(event);
}

bool KUrlNavigator::isActive() const
{
    return d->m_active;
}

int KUrlNavigator::historySize() const
{
    return d->m_history.count();
}

int KUrlNavigator::historyIndex() const
{
    return d->m_historyIndex;
}

QPoint KUrlNavigator::savedPosition() const
{
    const HistoryElem& histElem = d->m_history[d->m_historyIndex];
    return QPoint(histElem.contentsX(), histElem.contentsY());
}

void KUrlNavigator::setCustomProtocols(const QStringList &protocols)
{
    d->m_customProtocols = protocols;

    d->m_protocols->setCustomProtocols(d->m_customProtocols);
}

QStringList KUrlNavigator::customProtocols() const
{
    return d->m_customProtocols;
}

#include "kurlnavigator.moc"
