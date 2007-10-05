/*  This file is part of the KDE libraries

    Copyright (C) 1997 Torben Weis <weis@stud.uni-frankfurt.de>
    Copyright (C) 1999 Dirk Mueller <mueller@kde.org>
    Portions copyright (C) 1999 Preston Brown <pbrown@kde.org>
    Copyright (C) 2007 Pino Toscano <pino@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kopenwithdialog.h"
#include "kopenwithdialog_p.h"

#include <QtCore/QtAlgorithms>
#include <QtCore/QList>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QCheckBox>

#include <kauthorized.h>
#include <khistorycombobox.h>
#include <kdesktopfile.h>
#include <klineedit.h>
#include <klocale.h>
#include <kiconloader.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <kmimetypetrader.h>
#include <kurlcompletion.h>
#include <kurlrequester.h>
#include <kmimetype.h>
#include <kservicegroup.h>
#include <kserviceoffer.h>
#include <kdebug.h>

#include <assert.h>
#include <stdlib.h>
#include <kbuildsycocaprogressdialog.h>
#include <kconfiggroup.h>

template <> inline
void KConfigGroup::writeEntry( const QByteArray &key,
                              const KGlobalSettings::Completion& aValue,
                              KConfigBase::WriteConfigFlags flags)
{
  writeEntry(key, int(aValue), flags);
}

class AppNode
{
public:
    AppNode()
        : isDir(false), parent(0), fetched(false)
    {
    }
    ~AppNode()
    {
        qDeleteAll(children);
    }

    QString icon;
    QString text;
    QString relPath;
    QString exec;
    bool isDir;

    AppNode *parent;
    bool fetched;

    QList<AppNode*> children;
};

static bool AppNodeLessThan(AppNode *n1, AppNode *n2)
{
    if (n1->isDir) {
        if (n2->isDir) {
            return n1->text.compare(n2->text, Qt::CaseInsensitive) < 0;
        } else {
            return true;
        }
    } else {
        if (n2->isDir) {
            return false;
        } else {
            return n1->text.compare(n2->text, Qt::CaseInsensitive) < 0;
        }
    }
    return true;
}


class KApplicationModelPrivate
{
public:
    KApplicationModelPrivate(KApplicationModel *qq)
        : q(qq), root(new AppNode())
    {
    }
    ~KApplicationModelPrivate()
    {
        delete root;
    }

    void fillNode(const QString &relPath, AppNode *node);

    KApplicationModel *q;

    AppNode *root;
};

void KApplicationModelPrivate::fillNode(const QString &_relPath, AppNode *node)
{
   KServiceGroup::Ptr root = KServiceGroup::group(_relPath);
   if (!root || !root->isValid()) return;

   KServiceGroup::List list = root->entries();

   for( KServiceGroup::List::ConstIterator it = list.begin();
       it != list.end(); ++it)
   {
      QString icon;
      QString text;
      QString relPath = _relPath;
      QString exec;
      bool isDir = false;
      const KSycocaEntry::Ptr p = (*it);
      if (p->isType(KST_KService))
      {
         const KService::Ptr service = KService::Ptr::staticCast(p);

         if (service->noDisplay())
            continue;

         icon = service->icon();
         text = service->name();
         exec = service->exec();
      }
      else if (p->isType(KST_KServiceGroup))
      {
         const KServiceGroup::Ptr serviceGroup = KServiceGroup::Ptr::staticCast(p);

         if (serviceGroup->noDisplay() || serviceGroup->childCount() == 0)
            continue;

         icon = serviceGroup->icon();
         text = serviceGroup->caption();
         relPath = serviceGroup->relPath();
         isDir = true;
      }
      else
      {
         kWarning(250) << "KServiceGroup: Unexpected object in list!";
         continue;
      }

      AppNode *newnode = new AppNode();
      newnode->icon = icon;
      newnode->text = text;
      newnode->relPath = relPath;
      newnode->exec = exec;
      newnode->isDir = isDir;
      newnode->parent = node;
      node->children.append(newnode);
   }
   qStableSort(node->children.begin(), node->children.end(), AppNodeLessThan);
}



KApplicationModel::KApplicationModel(QObject *parent)
    : QAbstractItemModel(parent), d(new KApplicationModelPrivate(this))
{
    d->fillNode(QString(), d->root);
}

KApplicationModel::~KApplicationModel()
{
    delete d;
}

bool KApplicationModel::canFetchMore(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return false;

    AppNode *node = static_cast<AppNode*>(parent.internalPointer());
    return node->isDir && !node->fetched;
}

int KApplicationModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant KApplicationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    AppNode *node = static_cast<AppNode*>(index.internalPointer());

    switch (role) {
    case Qt::DisplayRole:
        return node->text;
        break;
    case Qt::DecorationRole:
        if (!node->icon.isEmpty()) {
            return KIcon(node->icon);
        }
        break;
    default:
        ;
    }
    return QVariant();
}

void KApplicationModel::fetchMore(const QModelIndex &parent)
{
    if (!parent.isValid())
        return;

    AppNode *node = static_cast<AppNode*>(parent.internalPointer());
    if (!node->isDir)
        return;

    emit layoutAboutToBeChanged();
    d->fillNode(node->relPath, node);
    node->fetched = true;
    emit layoutChanged();
}

bool KApplicationModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return true;

    AppNode *node = static_cast<AppNode*>(parent.internalPointer());
    return node->isDir;
}

QVariant KApplicationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || section != 0)
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        return i18n("Known Applications");
        break;
    default:
        return QVariant();
    }
}

QModelIndex KApplicationModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column != 0)
        return QModelIndex();

    AppNode *node = d->root;
    if (parent.isValid())
        node = static_cast<AppNode*>(parent.internalPointer());

    if (row >= node->children.count())
        return QModelIndex();
    else
        return createIndex(row, 0, node->children.at(row));
}

QModelIndex KApplicationModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    AppNode *node = static_cast<AppNode*>(index.internalPointer());
    if (node->parent->parent) {
        int id = node->parent->parent->children.indexOf(node->parent);

        if (id >= 0 && id < node->parent->parent->children.count())
           return createIndex(id, 0, node->parent);
        else
            return QModelIndex();
    }
    else
        return QModelIndex();
}

int KApplicationModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return d->root->children.count();

    AppNode *node = static_cast<AppNode*>(parent.internalPointer());
    return node->children.count();
}

QString KApplicationModel::nameFor(const QModelIndex &index) const
{
    if (!index.isValid())
        return QString();

    AppNode *node = static_cast<AppNode*>(index.internalPointer());
    return node->text;
}

QString KApplicationModel::execFor(const QModelIndex &index) const
{
    if (!index.isValid())
        return QString();

    AppNode *node = static_cast<AppNode*>(index.internalPointer());
    return node->exec;
}

bool KApplicationModel::isDirectory(const QModelIndex &index) const
{
    if (!index.isValid())
        return false;

    AppNode *node = static_cast<AppNode*>(index.internalPointer());
    return node->isDir;
}

class KApplicationViewPrivate
{
public:
    KApplicationViewPrivate()
        : appModel(0)
    {
    }

    KApplicationModel *appModel;
};

KApplicationView::KApplicationView(QWidget *parent)
    : QTreeView(parent), d(new KApplicationViewPrivate)
{
}

KApplicationView::~KApplicationView()
{
    delete d;
}

void KApplicationView::setModel(QAbstractItemModel *model)
{
    if (d->appModel) {
        disconnect(selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
                this, SLOT(slotSelectionChanged(QItemSelection, QItemSelection)));
    }

    QTreeView::setModel(model);

    d->appModel = qobject_cast<KApplicationModel*>(model);
    if (d->appModel) {
        connect(selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
                this, SLOT(slotSelectionChanged(QItemSelection, QItemSelection)));
    }
}

bool KApplicationView::isDirSel() const
{
    if (d->appModel) {
        QModelIndex index = selectionModel()->currentIndex();
        return d->appModel->isDirectory(index);
    }
    return false;
}

void KApplicationView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTreeView::currentChanged(current, previous);

    if (d->appModel && !d->appModel->isDirectory(current)) {
        QString exec = d->appModel->execFor(current);
        if (!exec.isEmpty()) {
            emit highlighted(d->appModel->nameFor(current), exec);
        }
    }
}

void KApplicationView::slotSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected)

    QModelIndexList indexes = selected.indexes();
    if (indexes.count() == 1 && !d->appModel->isDirectory(indexes.at(0))) {
        QString exec = d->appModel->execFor(indexes.at(0));
        if (!exec.isEmpty()) {
            emit this->selected(d->appModel->nameFor(indexes.at(0)), exec);
        }
    }
}



/***************************************************************
 *
 * KOpenWithDialog
 *
 ***************************************************************/
class KOpenWithDialogPrivate
{
public:
    KOpenWithDialogPrivate(KOpenWithDialog *qq)
        : q(qq), saveNewApps(false)
    {
    }

    KOpenWithDialog *q;

    /**
     * Determine mime type from URLs
     */
    void setMimeType(const KUrl::List &_urls);

    /**
     * Create a dialog that asks for a application to open a given
     * URL(s) with.
     *
     * @param text   appears as a label on top of the entry box.
     * @param value  is the initial value of the line
     */
    void init(const QString &text, const QString &value);

    // slots
    void _k_slotDbClick();
    void _k_slotOK();

    bool saveNewApps;
    bool m_terminaldirty;
    KService::Ptr curService;
    KApplicationView *view;
    KUrlRequester *edit;
    QString m_command;
    QLabel *label;
    QString qName;
    QString qMimeType;
    QCheckBox *terminal;
    QCheckBox *remember;
    QCheckBox *nocloseonexit;
    KService::Ptr m_pService;
};

KOpenWithDialog::KOpenWithDialog( const KUrl::List& _urls, QWidget* parent )
    : KDialog(parent), d(new KOpenWithDialogPrivate(this))
{
    setObjectName( QLatin1String( "openwith" ) );
    setModal( true );
    setCaption( i18n( "Open With" ) );

    QString text;
    if( _urls.count() == 1 )
    {
        text = i18n("<qt>Select the program that should be used to open <b>%1</b>. "
                     "If the program is not listed, enter the name or click "
                     "the browse button.</qt>",  _urls.first().fileName() );
    }
    else
        // Should never happen ??
        text = i18n( "Choose the name of the program with which to open the selected files." );
    d->setMimeType(_urls);
    d->init(text, QString());
}

KOpenWithDialog::KOpenWithDialog( const KUrl::List& _urls, const QString&_text,
                            const QString& _value, QWidget *parent)
    : KDialog(parent), d(new KOpenWithDialogPrivate(this))
{
  setObjectName( QLatin1String( "openwith" ) );
  setModal( true );
  QString caption;
  if (_urls.count()>0 && !_urls.first().isEmpty())
     caption = KStringHandler::csqueeze( _urls.first().prettyUrl() );
  if (_urls.count() > 1)
      caption += QString::fromLatin1("...");
  setCaption(caption);
    d->setMimeType(_urls);
    d->init(_text, _value);
}

KOpenWithDialog::KOpenWithDialog( const QString &mimeType, const QString& value,
                            QWidget *parent)
    : KDialog(parent), d(new KOpenWithDialogPrivate(this))
{
  setObjectName( QLatin1String( "openwith" ) );
  setModal( true );
  setCaption(i18n("Choose Application for %1", mimeType));
  QString text = i18n("<qt>Select the program for the file type: <b>%1</b>. "
                      "If the program is not listed, enter the name or click "
                      "the browse button.</qt>", mimeType);
    d->qMimeType = mimeType;
    d->init(text, value);
    if (d->remember) {
        d->remember->hide();
    }
}

KOpenWithDialog::KOpenWithDialog( QWidget *parent)
    : KDialog(parent), d(new KOpenWithDialogPrivate(this))
{
  setObjectName( QLatin1String( "openwith" ) );
  setModal( true );
  setCaption(i18n("Choose Application"));
  QString text = i18n("<qt>Select a program. "
                      "If the program is not listed, enter the name or click "
                      "the browse button.</qt>");
    d->qMimeType.clear();
    d->init(text, QString());
}

void KOpenWithDialogPrivate::setMimeType(const KUrl::List &_urls)
{
  if ( _urls.count() == 1 )
  {
    qMimeType = KMimeType::findByUrl( _urls.first())->name();
    if (qMimeType == QLatin1String("application/octet-stream"))
      qMimeType.clear();
  }
  else
      qMimeType.clear();
}

void KOpenWithDialogPrivate::init(const QString &_text, const QString &_value)
{
  bool bReadOnly = !KAuthorized::authorize("shell_access");
  m_terminaldirty = false;
    view = 0;
  m_pService = 0L;
    curService = 0L;

    q->setButtons(KDialog::Ok | KDialog::Cancel);

    QWidget *mainWidget = q->mainWidget();

  QBoxLayout *topLayout = new QVBoxLayout( mainWidget );
  topLayout->setMargin(0);
  topLayout->setSpacing( KDialog::spacingHint() );
    label = new QLabel(_text, q);
  label->setWordWrap(true);
  topLayout->addWidget(label);

  if (!bReadOnly)
  {
    // init the history combo and insert it into the URL-Requester
    KHistoryComboBox *combo = new KHistoryComboBox();
    KLineEdit *lineEdit = new KLineEdit();
    lineEdit->setClearButtonShown(true);
    combo->setLineEdit(lineEdit);
    combo->setDuplicatesEnabled( false );
    KConfigGroup cg( KGlobal::config(), QString::fromLatin1("Open-with settings") );
    int max = cg.readEntry( QString::fromLatin1("Maximum history"), 15 );
    combo->setMaxCount( max );
    int mode = cg.readEntry(QString::fromLatin1("CompletionMode"),
				int(KGlobalSettings::completionMode()));
    combo->setCompletionMode((KGlobalSettings::Completion)mode);
    QStringList list = cg.readEntry( QString::fromLatin1("History"), QStringList() );
    combo->setHistoryItems( list, true );
    edit = new KUrlRequester( combo, mainWidget );
  }
  else
  {
    edit = new KUrlRequester( mainWidget );
    edit->lineEdit()->setReadOnly(true);
    edit->button()->hide();
  }

  edit->setUrl( KUrl(_value) );
  edit->setWhatsThis(i18n(
    "Following the command, you can have several place holders which will be replaced "
    "with the actual values when the actual program is run:\n"
    "%f - a single file name\n"
    "%F - a list of files; use for applications that can open several local files at once\n"
    "%u - a single URL\n"
    "%U - a list of URLs\n"
    "%d - the directory of the file to open\n"
    "%D - a list of directories\n"
    "%i - the icon\n"
    "%m - the mini-icon\n"
    "%c - the comment"));

  topLayout->addWidget(edit);

  if ( edit->comboBox() ) {
    KUrlCompletion *comp = new KUrlCompletion( KUrlCompletion::ExeCompletion );
    edit->comboBox()->setCompletionObject( comp );
    edit->comboBox()->setAutoDeleteCompletionObject( true );
  }

    QObject::connect(edit, SIGNAL(returnPressed()), q, SLOT(_k_slotOK()));
    QObject::connect(q, SIGNAL(okClicked()), q, SLOT(_k_slotOK()));
    QObject::connect(edit, SIGNAL(textChanged(QString)), q, SLOT(slotTextChanged()));

    view = new KApplicationView(mainWidget);
    view->setModel(new KApplicationModel(view));
    topLayout->addWidget(view);

    QObject::connect(view, SIGNAL(selected(QString, QString)),
                     q, SLOT(slotSelected(QString, QString)));
    QObject::connect(view, SIGNAL(highlighted(QString, QString)),
                     q, SLOT(slotHighlighted(QString, QString)));
    QObject::connect(view, SIGNAL(doubleClicked(QModelIndex)),
                     q, SLOT(_k_slotDbClick()));

  terminal = new QCheckBox( i18n("Run in &terminal"), mainWidget );
  if (bReadOnly)
     terminal->hide();
    QObject::connect(terminal, SIGNAL(toggled(bool)), q, SLOT(slotTerminalToggled(bool)));

  topLayout->addWidget(terminal);

  QBoxLayout* nocloseonexitLayout = new QHBoxLayout();
  nocloseonexitLayout->setMargin( 0 );
  nocloseonexitLayout->setSpacing( KDialog::spacingHint() );
  QSpacerItem* spacer = new QSpacerItem( 20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum );
  nocloseonexitLayout->addItem( spacer );

  nocloseonexit = new QCheckBox( i18n("&Do not close when command exits"), mainWidget );
  nocloseonexit->setChecked( false );
  nocloseonexit->setDisabled( true );

  // check to see if we use konsole if not disable the nocloseonexit
  // because we don't know how to do this on other terminal applications
  KConfigGroup confGroup( KGlobal::config(), QString::fromLatin1("General") );
  QString preferredTerminal = confGroup.readPathEntry("TerminalApplication", QString::fromLatin1("konsole"));

  if (bReadOnly || preferredTerminal != "konsole")
     nocloseonexit->hide();

  nocloseonexitLayout->addWidget( nocloseonexit );
  topLayout->addLayout( nocloseonexitLayout );

  if (!qMimeType.isNull())
  {
    remember = new QCheckBox(i18n("&Remember application association for this type of file"), mainWidget);
    //    remember->setChecked(true);
    topLayout->addWidget(remember);
  }
  else
    remember = 0L;

  //edit->setText( _value );
  // This is what caused "can't click on items before clicking on Name header".
  // Probably due to the resizeEvent handler using width().
  //resize( minimumWidth(), sizeHint().height() );
  edit->setFocus();
    q->slotTextChanged();
}


// ----------------------------------------------------------------------

KOpenWithDialog::~KOpenWithDialog()
{
    delete d;
}


// ----------------------------------------------------------------------

void KOpenWithDialog::slotSelected( const QString& /*_name*/, const QString& _exec )
{
    kDebug(250)<<"KOpenWithDialog::slotSelected";
    KService::Ptr pService = d->curService;
    d->edit->setUrl(KUrl(_exec)); // calls slotTextChanged :(
    d->curService = pService;
}


// ----------------------------------------------------------------------

void KOpenWithDialog::slotHighlighted( const QString& _name, const QString& )
{
    kDebug(250)<<"KOpenWithDialog::slotHighlighted";
    d->qName = _name;
    d->curService = KService::serviceByName(d->qName);
    if (!d->m_terminaldirty)
    {
        // ### indicate that default value was restored
        d->terminal->setChecked(d->curService->terminal());
        QString terminalOptions = d->curService->terminalOptions();
        d->nocloseonexit->setChecked((terminalOptions.contains(QLatin1String("--noclose")) > 0));
        d->m_terminaldirty = false; // slotTerminalToggled changed it
    }
}

// ----------------------------------------------------------------------

void KOpenWithDialog::slotTextChanged()
{
    kDebug(250)<<"KOpenWithDialog::slotTextChanged";
    // Forget about the service
    d->curService = 0L;
    enableButton(Ok, !d->edit->url().isEmpty());
}

// ----------------------------------------------------------------------

void KOpenWithDialog::slotTerminalToggled(bool)
{
    // ### indicate that default value was overridden
    d->m_terminaldirty = true;
    d->nocloseonexit->setDisabled(!d->terminal->isChecked());
}

// ----------------------------------------------------------------------

void KOpenWithDialogPrivate::_k_slotDbClick()
{
    // check if a directory is selected
    if (view->isDirSel()) {
        return;
    }
    _k_slotOK();
}

void KOpenWithDialog::setSaveNewApplications(bool b)
{
  d->saveNewApps = b;
}

void KOpenWithDialogPrivate::_k_slotOK()
{
  QString typedExec(edit->url().pathOrUrl());
  QString fullExec(typedExec);

  QString serviceName;
  QString initialServiceName;
  QString preferredTerminal;
  m_pService = curService;
  if (!m_pService) {
    // No service selected - check the command line

    // Find out the name of the service from the command line, removing args and paths
    serviceName = KRun::binaryName( typedExec, true );
    if (serviceName.isEmpty())
    {
      // TODO add a KMessageBox::error here after the end of the message freeze
      return;
    }
    initialServiceName = serviceName;
    kDebug(250) << "initialServiceName=" << initialServiceName;
    int i = 1; // We have app, app-2, app-3... Looks better for the user.
    bool ok = false;
    // Check if there's already a service by that name, with the same Exec line
    do {
        kDebug(250) << "looking for service " << serviceName;
        KService::Ptr serv = KService::serviceByDesktopName( serviceName );
        ok = !serv; // ok if no such service yet
        // also ok if we find the exact same service (well, "kwrite" == "kwrite %U"
        if (serv && serv->isApplication())
        {
            QString exec = serv->exec();
            fullExec = exec;
            exec.replace("%u", "", Qt::CaseInsensitive);
            exec.replace("%f", "", Qt::CaseInsensitive);
            exec.replace("-caption %c", "");
            exec.replace("-caption \"%c\"", "");
            exec.replace("%i", "");
            exec.replace("%m", "");
            exec = exec.simplified();
            if (exec == typedExec)
            {
                ok = true;
                m_pService = serv;
                kDebug(250) << "OK, found identical service: " << serv->entryPath();
            }
        }
        if (!ok) // service was found, but it was different -> keep looking
        {
            ++i;
            serviceName = initialServiceName + '-' + QString::number(i);
        }
    }
    while (!ok);
  }
  if ( m_pService )
  {
    // Existing service selected
    serviceName = m_pService->name();
    initialServiceName = serviceName;
    fullExec = m_pService->exec();
  }

  if (terminal->isChecked())
  {
    KConfigGroup confGroup( KGlobal::config(), QString::fromLatin1("General") );
    preferredTerminal = confGroup.readPathEntry("TerminalApplication", QString::fromLatin1("konsole"));
    m_command = preferredTerminal;
    // only add --noclose when we are sure it is konsole we're using
    if (preferredTerminal == "konsole" && nocloseonexit->isChecked())
      m_command += QString::fromLatin1(" --noclose");
    m_command += QString::fromLatin1(" -e ");
    m_command += edit->url().pathOrUrl();
    kDebug(250) << "Setting m_command to " << m_command;
  }
  if ( m_pService && terminal->isChecked() != m_pService->terminal() )
      m_pService = 0L; // It's not exactly this service we're running

  bool bRemember = remember && remember->isChecked();

  if ( !bRemember && m_pService)
  {
        q->accept();
    return;
  }

    if (!bRemember && !saveNewApps)
  {
    // Create temp service
    m_pService = new KService(initialServiceName, fullExec, QString());
    if (terminal->isChecked())
    {
      m_pService->setTerminal(true);
      // only add --noclose when we are sure it is konsole we're using
      if (preferredTerminal == "konsole" && nocloseonexit->isChecked())
         m_pService->setTerminalOptions("--noclose");
    }
        q->accept();
    return;
  }

  // if we got here, we can't seem to find a service for what they
  // wanted.  The other possibility is that they have asked for the
  // association to be remembered.  Create/update service.

  QString newPath;
  QString oldPath;
  QString menuId;
  if (m_pService)
  {
    oldPath = m_pService->entryPath();
    newPath = m_pService->locateLocal();
    menuId = m_pService->menuId();
    kDebug(250) << "Updating exitsing service " << m_pService->entryPath() << " ( " << newPath << " ) ";
  }
  else
  {
    newPath = KService::newServicePath(false /* hidden */, serviceName, &menuId);
    kDebug(250) << "Creating new service " << serviceName << " ( " << newPath << " ) ";
  }

  int maxPreference = 1;
  if (!qMimeType.isEmpty())
  {
    // Find max highest preference from either initial preference or from profile
    const KServiceOfferList offerList = KMimeTypeTrader::self()->weightedOffers( qMimeType );
    if (!offerList.isEmpty())
      maxPreference = offerList.first().preference();
  }

  KDesktopFile *desktop = 0;
  if (!oldPath.isEmpty() && (oldPath != newPath))
  {
     KDesktopFile orig( oldPath );
     desktop = orig.copyTo( newPath );
  }
  else
  {
     desktop = new KDesktopFile(newPath);
  }
  KConfigGroup cg = desktop->desktopGroup();
  cg.writeEntry("Type", QString::fromLatin1("Application"));
  cg.writeEntry("Name", initialServiceName);
  cg.writePathEntry("Exec", fullExec);
  if (terminal->isChecked())
  {
    cg.writeEntry("Terminal", true);
    // only add --noclose when we are sure it is konsole we're using
    if (preferredTerminal == "konsole" && nocloseonexit->isChecked())
      cg.writeEntry("TerminalOptions", "--noclose");
  }
  else
  {
    cg.writeEntry("Terminal", false);
  }
  cg.writeEntry("InitialPreference", maxPreference + 1);


    if (bRemember || saveNewApps)
  {
    QStringList mimeList = cg.readEntry("MimeType", QStringList(), ';');
    if (!qMimeType.isEmpty() && !mimeList.contains(qMimeType))
      mimeList.append(qMimeType);
    cg.writeEntry("MimeType", mimeList, ';');

    if ( !qMimeType.isEmpty() )
    {
      // Also make sure the "auto embed" setting for this mimetype is off
      KDesktopFile mimeDesktop( KStandardDirs::locateLocal( "mime", qMimeType + ".desktop" ) );
      mimeDesktop.desktopGroup().writeEntry( "X-KDE-AutoEmbed", false );
      mimeDesktop.sync();
    }
  }

  // write it all out to the file
  cg.sync();
  delete desktop;

    KBuildSycocaProgressDialog::rebuildKSycoca(q);

  m_pService = KService::serviceByMenuId( menuId );

  Q_ASSERT( m_pService );

    q->accept();
}

QString KOpenWithDialog::text() const
{
    if (!d->m_command.isEmpty())
        return d->m_command;
    else
        return d->edit->url().url();
}

void KOpenWithDialog::hideNoCloseOnExit()
{
    // uncheck the checkbox because the value could be used when "Run in Terminal" is selected
    d->nocloseonexit->setChecked(false);
    d->nocloseonexit->hide();
}

void KOpenWithDialog::hideRunInTerminal()
{
    d->terminal->hide();
    hideNoCloseOnExit();
}

KService::Ptr KOpenWithDialog::service() const
{
    return d->m_pService;
}

void KOpenWithDialog::accept()
{
    KHistoryComboBox *combo = static_cast<KHistoryComboBox*>(d->edit->comboBox());
    if ( combo ) {
        combo->addToHistory(d->edit->url().url());

        KConfigGroup cg( KGlobal::config(), QString::fromLatin1("Open-with settings") );
        cg.writeEntry( QString::fromLatin1("History"), combo->historyItems() );
        cg.writeEntry(QString::fromLatin1("CompletionMode"), combo->completionMode());
        // don't store the completion-list, as it contains all of KUrlCompletion's
        // executables
        cg.sync();
    }

    QDialog::accept();
}

#include "kopenwithdialog.moc"
#include "kopenwithdialog_p.moc"
