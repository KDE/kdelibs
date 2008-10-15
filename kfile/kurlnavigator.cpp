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
#include <kfileplacesmodel.h>
#include <kglobalsettings.h>
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

    void setRootUrl(const KUrl& url);
    const KUrl& rootUrl() const;

    void setContentsX(int x);
    int contentsX() const;

    void setContentsY(int y);
    int contentsY() const;

private:
    KUrl m_url;
    KUrl m_rootUrl;
    int m_contentsX;
    int m_contentsY;
};

HistoryElem::HistoryElem() :
    m_url(),
    m_rootUrl(),
    m_contentsX(0),
    m_contentsY(0)
{
}

HistoryElem::HistoryElem(const KUrl& url) :
    m_url(url),
    m_rootUrl(),
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

inline void HistoryElem::setRootUrl(const KUrl& url)
{
    m_rootUrl = url;
}

inline const KUrl& HistoryElem::rootUrl() const
{
    return m_rootUrl;
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

/**
 * @brief Line editor for the host part of the URL.
 *
 * Extends the class KLineEdit to return a preferred
 * width in HostLineEdit::sizeHint() which assures that
 * the host part is always fully visible.
 */
class HostLineEdit : public KLineEdit
{
public:
    explicit HostLineEdit(const QString& text, KUrlNavigator* parent = 0);
    virtual ~HostLineEdit();
    virtual QSize sizeHint() const;
    inline void setOptimizeWidth(bool optimize);

protected:
    virtual void mousePressEvent(QMouseEvent* event);

private:
    bool m_optimizeWidth;
    KUrlNavigator* m_urlNavigator;
};

HostLineEdit::HostLineEdit(const QString& text, KUrlNavigator* parent) :
    KLineEdit(text, parent),
    m_optimizeWidth(true),
    m_urlNavigator(parent)
{
}

HostLineEdit::~HostLineEdit()
{
}

void HostLineEdit::setOptimizeWidth(bool optimize)
{
    m_optimizeWidth = optimize;
    setClearButtonShown(!optimize);
}

QSize HostLineEdit::sizeHint() const
{
    QSize size = KLineEdit::sizeHint();

    if (m_optimizeWidth) {
        const QFontMetrics fm(font());
        const int width = fm.width(text()) + 32;
        if (width > size.width()) {
            size.setWidth(width);
        }
    }

    return size;
}

void HostLineEdit::mousePressEvent(QMouseEvent* event)
{
    KLineEdit::mousePressEvent(event);

    if (event->button() == Qt::LeftButton) {
        // behave like a button when a mouse click has been done
        // inside the host editor and go to the host URL
        const KUrl currentUrl = m_urlNavigator->url();
        const KUrl newUrl(currentUrl.protocol() + "://" + text());
        if (currentUrl != newUrl) {
            m_urlNavigator->setUrl(newUrl);
        }
    }
}

////

class KUrlNavigator::Private
{
public:
    Private(KUrlNavigator* q, KFilePlacesModel* placesModel);

    void slotReturnPressed(const QString&);
    void slotReturnPressed();
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

    void switchToBreadcrumbMode();

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

    /**
     * Returns true, if the MIME type of the path represents a
     * compressed file like TAR or ZIP.
     */
    bool isCompressedPath(const KUrl& path) const;

    bool m_editable;
    bool m_active;
    bool m_showPlacesSelector;
    int m_historyIndex;

    QHBoxLayout* m_layout;

    QList<HistoryElem> m_history;
    KFilePlacesSelector* m_placesSelector;
    KUrlComboBox* m_pathBox;
    KProtocolCombo* m_protocols;
    HostLineEdit* m_host;
    KUrlDropDownButton* m_dropDownButton;
    QLinkedList<KUrlNavigatorButton*> m_navButtons;
    KUrlButton* m_toggleEditableMode;
    QString m_homeUrl;
    QStringList m_customProtocols;
    KUrlNavigator* q;

private:
    /**
     * Creates an instance of the class HostLineEdit and
     * assigns it to m_host.
     */
    void createHostLineEdit(const QString& text);
};


KUrlNavigator::Private::Private(KUrlNavigator* q, KFilePlacesModel* placesModel) :
    m_editable(false),
    m_active(true),
    m_showPlacesSelector(placesModel != 0),
    m_historyIndex(0),
    m_layout(new QHBoxLayout),
    m_placesSelector(0),
    m_pathBox(0),
    m_protocols(0),
    m_host(0),
    m_dropDownButton(0),
    m_toggleEditableMode(0),
    m_customProtocols(QStringList()),
    q(q)
{
    m_layout->setSpacing(0);
    m_layout->setMargin(0);

    // initialize the places selector
    q->setAutoFillBackground(false);

    if (placesModel != 0) {
        m_placesSelector = new KFilePlacesSelector(q, placesModel);
        connect(m_placesSelector, SIGNAL(placeActivated(const KUrl&)),
                q, SLOT(setUrl(const KUrl&)));

        connect(placesModel, SIGNAL(rowsInserted(QModelIndex, int, int)),
                q, SLOT(updateContent()));
        connect(placesModel, SIGNAL(rowsRemoved(QModelIndex, int, int)),
                q, SLOT(updateContent()));
        connect(placesModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
                q, SLOT(updateContent()));
    }

    m_dropDownButton = new KUrlDropDownButton(q);
    connect(m_dropDownButton, SIGNAL(clicked()),
            q, SLOT(openPathSelectorMenu()));

    // initialize the path box of the traditional view
    m_pathBox = new KUrlComboBox(KUrlComboBox::Both, true, q);
    m_pathBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    m_pathBox->installEventFilter(q);

    KUrlCompletion* kurlCompletion = new KUrlCompletion(KUrlCompletion::DirCompletion);
    m_pathBox->setCompletionObject(kurlCompletion);
    m_pathBox->setAutoDeleteCompletionObject(true);

    connect(m_pathBox, SIGNAL(returnPressed(QString)),
            q, SLOT(slotReturnPressed(QString)));
    connect(m_pathBox, SIGNAL(returnPressed()),
            q, SLOT(slotReturnPressed()));
    connect(m_pathBox, SIGNAL(urlActivated(KUrl)),
            q, SLOT(setUrl(KUrl)));

    m_toggleEditableMode = new KUrlToggleButton(q);
    m_toggleEditableMode->setMinimumWidth(20);
    connect(m_toggleEditableMode, SIGNAL(clicked()),
            q, SLOT(switchView()));

    if (m_placesSelector != 0) {
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

    emit q->returnPressed();

    if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
        // Pressing Ctrl+Return automatically switches back to the breadcrumb mode.
        // The switch must be done asynchronously, as we are in the context of the
        // editor.
        QMetaObject::invokeMethod(q, "switchToBreadcrumbMode", Qt::QueuedConnection);
    }
}

void KUrlNavigator::Private::slotReturnPressed()
{
    const QString text = q->uncommittedUrl().prettyUrl();
    slotReturnPressed(text);
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
    url.setPath("/");
    QLinkedList<KUrlNavigatorButton*>::const_iterator it = m_navButtons.begin();
    const QLinkedList<KUrlNavigatorButton*>::const_iterator itEnd = m_navButtons.end();
    while (it != itEnd) {
        (*it)->hide();
        (*it)->deleteLater();
        ++it;
    }
    m_navButtons.clear();

    if (KProtocolInfo::protocolClass(protocol) == ":local") {
        q->setUrl(url);
    } else {
        if (m_host == 0) {
            createHostLineEdit("");
        } else {
            m_host->setText("");
        }
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

void KUrlNavigator::Private::switchView()
{
    m_toggleEditableMode->setFocus();
    m_editable = !m_editable;
    m_toggleEditableMode->setChecked(m_editable);
    updateContent();
    if (q->isUrlEditable()) {
        m_pathBox->setFocus();
    }

    emit q->requestActivation();
    emit q->editableStateChanged(m_editable);
}

void KUrlNavigator::Private::dropUrls(const KUrl::List& urls,
                                      const KUrl& destination)
{
    emit q->urlsDropped(urls, destination);
}

void KUrlNavigator::Private::updateContent()
{
    if (m_placesSelector != 0) {
        m_placesSelector->updateSelection(q->url());
    }

    if (m_editable) {
        delete m_protocols;
        m_protocols = 0;
        delete m_host;
        m_host = 0;

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
        if (m_placesSelector != 0) {
            placeUrl = m_placesSelector->selectedPlaceUrl();
        }

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
            if (m_protocols == 0) {
                deleteButtons();

                m_protocols = new KProtocolCombo(protocol, q);
                const int index = m_layout->indexOf(m_dropDownButton);
                m_layout->insertWidget(index, m_protocols);

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

                if (m_host == 0) {
                    createHostLineEdit(hostText);
                } else {
                    m_host->setText(hostText);
                }
                m_host->show();
            } else {
                delete m_host;
                m_host = 0;
            }
        } else if (m_protocols != 0) {
            m_protocols->hide();
            if (m_host != 0) {
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
                if (m_placesSelector != 0) {
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
                button->updateMinimumWidth();
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
        (*it)->hide();
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
    if (m_host != 0) {
        const bool optimize = (buttonsCount != 0);
        m_host->setOptimizeWidth(optimize);
        if (optimize) {
            m_host->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
            m_layout->setStretchFactor(m_host, 0);
        } else {
            m_host->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
            m_layout->setStretchFactor(m_host, 1);
        }
    }

    if (buttonsCount == 0) {
        m_dropDownButton->hide();
        return;
    }

    int hiddenButtonsCount = 0;

    // subtract all widgets from the available width, that must be shown anyway
    int availableWidth = q->width() - m_toggleEditableMode->minimumWidth();

    if ((m_placesSelector != 0) && m_placesSelector->isVisible()) {
        availableWidth -= m_placesSelector->width();
    }

    if ((m_protocols != 0) && m_protocols->isVisible()) {
        availableWidth -= m_protocols->width();
    }

    if ((m_host != 0) && m_host->isVisible()) {
        availableWidth -= m_host->width();
    }

    // check whether buttons must be hidden at all...
    int requiredButtonWidth = 0;
    foreach (KUrlNavigatorButton* button, m_navButtons) {
        requiredButtonWidth += button->minimumWidth();
    }
    if (requiredButtonWidth > availableWidth) {
        // At least one button must be hidden. This implies that the
        // drop-down button must get visible, which again decreases the
        // available width.
        availableWidth -= m_dropDownButton->width();
    }

    // hide buttons...
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

void KUrlNavigator::Private::switchToBreadcrumbMode()
{
    q->setUrlEditable(false);
}

void KUrlNavigator::Private::deleteButtons()
{
    foreach (KUrlNavigatorButton* button, m_navButtons) {
        button->hide();
        button->deleteLater();
    }
    m_navButtons.clear();
}

QString KUrlNavigator::Private::retrievePlacePath(const QString& path) const
{
    int idx = path.indexOf(QLatin1String("///"));
    if (idx >= 0) {
        idx += 3;
    } else {
        idx = path.indexOf(QLatin1String("//"));
        idx = path.indexOf(QLatin1Char('/'), (idx < 0) ? 0 : idx + 2);
    }
    return (idx < 0) ? path : path.left(idx);
}

bool KUrlNavigator::Private::isCompressedPath(const KUrl& url) const
{
    const KMimeType::Ptr mime = KMimeType::findByPath(url.path(KUrl::RemoveTrailingSlash));
    // Note: this list of MIME types depends on the protocols implemented by kio_archive
    return  mime->is("application/x-compressed-tar") ||
            mime->is("application/x-bzip-compressed-tar") ||
            mime->is("application/x-tar") ||
            mime->is("application/x-tarz") ||
            mime->is("application/x-tzo") || // (not sure KTar supports those?)
            mime->is("application/zip") ||
            mime->is("application/x-archive");
}

void KUrlNavigator::Private::createHostLineEdit(const QString& text)
{
    Q_ASSERT(m_host == 0);

    m_host = new HostLineEdit(text, q);
    m_host->setClearButtonShown(true);

    const int index = m_layout->indexOf(m_dropDownButton);
    m_layout->insertWidget(index, m_host);

    connect(m_host, SIGNAL(editingFinished()),
            q, SLOT(slotRemoteHostActivated()));
    connect(m_host, SIGNAL(returnPressed()),
            q, SIGNAL(returnPressed()));
}

////

KUrlNavigator::KUrlNavigator(KFilePlacesModel* placesModel,
                             const KUrl& url,
                             QWidget* parent) :
    QWidget(parent),
    d(new Private(this, placesModel))
{
    d->m_history.prepend(HistoryElem(url));

    const QFont font = KGlobalSettings::generalFont();
    setFont(font);

    const int minHeight = d->m_pathBox->sizeHint().height();
    setMinimumHeight(minHeight);

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
    KUrl newUrl = url();
    newUrl.setPath(QString());

    QString pathOrUrl = url().pathOrUrl();
    if (!pathOrUrl.isEmpty()) {
        if (index == 0) {
            // prevent the last "/" from being stripped
            // or we end up with an empty path
#ifdef Q_OS_WIN
            pathOrUrl = pathOrUrl.length() > 2 ? pathOrUrl.left(3) : QDir::rootPath();
#else
            pathOrUrl = QLatin1String("/");
#endif
        } else {
            pathOrUrl = pathOrUrl.section('/', 0, index);
        }
    }

    newUrl.setPath(KUrl(pathOrUrl).path());
    return newUrl;
}

bool KUrlNavigator::goBack()
{
    const int count = d->m_history.count();
    if (d->m_historyIndex < count - 1) {
        ++d->m_historyIndex;
        d->updateContent();
        emit historyChanged();
        emit urlChanged(url());
        return true;
    }

    return false;
}

bool KUrlNavigator::goForward()
{
    if (d->m_historyIndex > 0) {
        --d->m_historyIndex;
        d->updateContent();
        emit historyChanged();
        emit urlChanged(url());
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

bool KUrlNavigator::isActive() const
{
    return d->m_active;
}

void KUrlNavigator::setPlacesSelectorVisible(bool visible)
{
    if (visible == d->m_showPlacesSelector) {
        return;
    }

    if (visible  && (d->m_placesSelector == 0)) {
        // the places selector cannot get visible as no
        // places model is available
        return;
    }

    d->m_showPlacesSelector = visible;
    d->m_placesSelector->setVisible(visible);
}

bool KUrlNavigator::isPlacesSelectorVisible() const
{
    return d->m_showPlacesSelector;
}

void KUrlNavigator::setUrl(const KUrl& url)
{
    QString urlStr(KUrlCompletion::replacedPath(url.pathOrUrl(), true, true));

    if (urlStr.length() > 0 && urlStr.at(0) == '~') {
        // replace '~' by the home directory
        urlStr.remove(0, 1);
        urlStr.insert(0, QDir::homePath());
    }

    if ((url.protocol() == "tar") || (url.protocol() == "zip")) {
        // The URL represents a tar- or zip-file. Check whether
        // the URL is really part of the tar- or zip-file, otherwise
        // replace it by the local path again.
        bool insideCompressedPath = d->isCompressedPath(url);
        if (!insideCompressedPath) {
            KUrl prevUrl = url;
            KUrl parentUrl = url.upUrl();
            while (parentUrl != prevUrl) {
                if (d->isCompressedPath(parentUrl)) {
                    insideCompressedPath = true;
                    break;
                }
                prevUrl = parentUrl;
                parentUrl = parentUrl.upUrl();
            }
        }
        if (!insideCompressedPath) {
            // drop the tar: or zip: protocol since we are not
            // inside the compressed path anymore
            urlStr = url.path();
        }
    }

    const KUrl transformedUrl(urlStr);

    // Check whether current history element has the same URL.
    // If this is the case, just ignore setting the URL.
    const HistoryElem& historyElem = d->m_history[d->m_historyIndex];
    const bool isUrlEqual = transformedUrl.equals(historyElem.url(), KUrl::CompareWithoutTrailingSlash) ||
                            !transformedUrl.isValid() && (urlStr == historyElem.url().url());
    if (isUrlEqual) {
        return;
    }

    if (d->m_historyIndex > 0) {
        // If an URL is set when the history index is not at the end (= 0),
        // then clear all previous history elements so that a new history
        // tree is started from the current position.
        QList<HistoryElem>::iterator begin = d->m_history.begin();
        QList<HistoryElem>::iterator end = begin + d->m_historyIndex;
        d->m_history.erase(begin, end);
        d->m_historyIndex = 0;
    }

    Q_ASSERT(d->m_historyIndex == 0);
    d->m_history.insert(0, HistoryElem(transformedUrl));

    // Prevent an endless growing of the history: remembering
    // the last 100 Urls should be enough...
    const int historyMax = 100;
    if (d->m_history.size() > historyMax) {
        QList<HistoryElem>::iterator begin = d->m_history.begin() + historyMax;
        QList<HistoryElem>::iterator end = d->m_history.end();
        d->m_history.erase(begin, end);
    }

    emit historyChanged();
    emit urlChanged(transformedUrl);

    d->updateContent();

    requestActivation();
}

void KUrlNavigator::requestActivation()
{
    setActive(true);
}

void KUrlNavigator::saveRootUrl(const KUrl& url)
{
    HistoryElem& hist = d->m_history[d->m_historyIndex];
    hist.setRootUrl(url);
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

bool KUrlNavigator::eventFilter(QObject* watched, QEvent* event)
{
    if ((watched == d->m_pathBox) && (event->type() == QEvent::FocusIn)) {
        requestActivation();
        setFocus();
    }

    return QWidget::eventFilter(watched, event);
}

int KUrlNavigator::historySize() const
{
    return d->m_history.count();
}

int KUrlNavigator::historyIndex() const
{
    return d->m_historyIndex;
}

const KUrl& KUrlNavigator::savedRootUrl() const
{
    const HistoryElem& histElem = d->m_history[d->m_historyIndex];
    return histElem.rootUrl();
}

QPoint KUrlNavigator::savedPosition() const
{
    const HistoryElem& histElem = d->m_history[d->m_historyIndex];
    return QPoint(histElem.contentsX(), histElem.contentsY());
}

KUrlComboBox* KUrlNavigator::editor() const
{
    return d->m_pathBox;
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

void KUrlNavigator::setFocus()
{
    if (isUrlEditable()) {
        d->m_pathBox->setFocus();
    } else if (d->m_host) {
        d->m_host->setFocus();
    } else {
        QWidget::setFocus();
    }
}

#include "kurlnavigator.moc"
