/* This file is part of the KDE libraries
    Copyright
    (C) 2000 Reginald Stadlbauer (reggie@kde.org)
    (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
    (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
    (C) 1997, 1998 Sven Radej (radej@kde.org)
    (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)
    (C) 1999 Chris Schlaeger (cs@kde.org)
    (C) 1999 Kurt Granroth (granroth@kde.org)
    (C) 2005-2006 Hamish Rodda (rodda@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "ktoolbar.h"

#include <config.h>

#include <QtCore/QPointer>
#include <QtGui/QDesktopWidget>
#include <QtGui/QFrame>
#include <QtGui/QLayout>
#include <QtGui/QMouseEvent>
#include <QtGui/QToolButton>
#include <QtXml/QDomElement>

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kauthorized.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kglobalsettings.h>
#include <kguiitem.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kxmlguiwindow.h>
#include <kmenu.h>
#include <kstandardaction.h>
#include <ktoggleaction.h>
#include <kxmlguifactory.h>

#include <kconfiggroup.h>

class KToolBar::Private
{
  public:
    Private(KToolBar *qq)
      : q(qq),
        honorStyle(false),
        enableContext(true),
        modified(false),
        unlockedMovable(true),
        xmlguiClient(0),
        contextLockAction(0),
        IconSizeDefault(22),
        ToolButtonStyleDefault(Qt::ToolButtonTextBesideIcon),
        HiddenDefault(false),
        NewLineDefault(false),
        PositionDefault("Top"),
        dropIndicatorAction(0),
        context(0),
        dragAction(0)
    {
        /**
          TODO: if we get complaints about text beside icons on small screens,
                try the following code out on such systems - aseigo.
        // if we are on a small screen with a non-landscape ratio, then
        // we revert to text under icons since width is probably not our
        // friend in such cases
        QDesktopWidget *desktop = QApplication::desktop();
        QRect screenGeom = desktop->screenGeometry(desktop->primaryScreen());
        qreal ratio = screenGeom.width() / qreal(screenGeom.height());

        if (screenGeom.width() < 1024 && ratio <= 1.4) {
            ToolButtonStyleDefault = Qt::ToolButtonTextUnderIcon;
        }
        **/
    }

    void slotReadConfig();
    void slotAppearanceChanged();
    void slotContextAboutToShow();
    void slotContextAboutToHide();
    void slotContextLeft();
    void slotContextRight();
    void slotContextTop();
    void slotContextBottom();
    void slotContextIcons();
    void slotContextText();
    void slotContextTextRight();
    void slotContextTextUnder();
    void slotContextIconSize();
    void slotLockToolBars(bool lock);

    void init(bool readConfig = true, bool honorStyle = false);
    void getAttributes(QString &position, Qt::ToolButtonStyle &toolButtonStyle, int &index) const;
    int dockWindowIndex() const;
    KMenu *contextMenu();
    bool isMainToolBar() const;
    void setLocked(bool locked);
    void adjustSeparatorVisibility();

    static Qt::ToolButtonStyle toolButtonStyleFromString(const QString& style);
    static QString toolButtonStyleToString(Qt::ToolButtonStyle);
    static Qt::ToolBarArea positionFromString(const QString& position);


    KToolBar *q;
    bool honorStyle : 1;
    bool enableContext : 1;
    bool modified : 1;
    bool unlockedMovable : 1;
    static bool s_editable;
    static bool s_locked;

    KXMLGUIClient *xmlguiClient;

    QList<int> iconSizes;

    QMenu* contextOrient;
    QMenu* contextMode;
    QMenu* contextSize;

    QAction* contextTop;
    QAction* contextLeft;
    QAction* contextRight;
    QAction* contextBottom;
    QAction* contextIcons;
    QAction* contextTextRight;
    QAction* contextText;
    QAction* contextTextUnder;
    KToggleAction* contextLockAction;
    QMap<QAction*,int> contextIconSizes;

    // Default Values.
    int IconSizeDefault;
    Qt::ToolButtonStyle ToolButtonStyleDefault;
    bool HiddenDefault : 1;
    bool NewLineDefault : 1;
    QString PositionDefault;

    QList<QAction*> actionsBeingDragged;
    QAction* dropIndicatorAction;

    KMenu* context;
    KAction* dragAction;
    QPoint dragStartPosition;
};

bool KToolBar::Private::s_editable = false;
bool KToolBar::Private::s_locked = false;

void KToolBar::Private::init(bool readConfig, bool _honorStyle)
{
  honorStyle = _honorStyle;

  // finally, read in our configurable settings
  if (readConfig)
    slotReadConfig();

  if (q->mainWindow()) {
    // Get notified when settings change
    connect(q, SIGNAL(allowedAreasChanged(Qt::ToolBarAreas)),
             q->mainWindow(), SLOT(setSettingsDirty()));
    connect(q, SIGNAL(iconSizeChanged(const QSize&)),
             q->mainWindow(), SLOT(setSettingsDirty()));
    connect(q, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
             q->mainWindow(), SLOT(setSettingsDirty()));
    connect(q, SIGNAL(movableChanged(bool)),
             q->mainWindow(), SLOT(setSettingsDirty()));
    connect(q, SIGNAL(orientationChanged(Qt::Orientation)),
             q->mainWindow(), SLOT(setSettingsDirty()));
  }

  if (!KAuthorized::authorize("movable_toolbars"))
    q->setMovable(false);
  else
    q->setMovable(!KToolBar::toolBarsLocked());

  connect(q, SIGNAL(movableChanged(bool)),
           q, SLOT(slotMovableChanged(bool)));

  q->setAcceptDrops(true);
}

void KToolBar::Private::getAttributes(QString &position, Qt::ToolButtonStyle &toolButtonStyle, int &index) const
{
  // get all of the stuff to save
  switch (q->mainWindow()->toolBarArea(const_cast<KToolBar*>(q))) {
    case Qt::BottomToolBarArea:
      position = "Bottom";
      break;
    case Qt::LeftToolBarArea:
      position = "Left";
      break;
    case Qt::RightToolBarArea:
      position = "Right";
      break;
    case Qt::TopToolBarArea:
    default:
      position = "Top";
      break;
  }

  toolButtonStyle = q->toolButtonStyle();

  index = dockWindowIndex();
}

int KToolBar::Private::dockWindowIndex() const
{
  Q_ASSERT(q->mainWindow());

  return q->mainWindow()->layout()->indexOf(const_cast<KToolBar*>(q));
}

KMenu *KToolBar::Private::contextMenu()
{
  if (!context) {
    context = new KMenu(q);
    context->addTitle(i18n("Toolbar Menu"));

    contextOrient = new KMenu(i18n("Orientation"), context);
    context->addMenu(contextOrient);

    contextTop = contextOrient->addAction(i18nc("toolbar position string", "Top"), q, SLOT(slotContextTop()));
    contextTop->setChecked(true);
    contextLeft = contextOrient->addAction(i18nc("toolbar position string", "Left"), q, SLOT(slotContextLeft()));
    contextRight = contextOrient->addAction(i18nc("toolbar position string", "Right"), q, SLOT(slotContextRight()));
    contextBottom = contextOrient->addAction(i18nc("toolbar position string", "Bottom"), q, SLOT(slotContextBottom()));

    QActionGroup* positionGroup = new QActionGroup(contextOrient);
    foreach (QAction* action, contextOrient->actions()) {
      action->setActionGroup(positionGroup);
      action->setCheckable(true);
    }

    contextMode = new KMenu(i18n("Text Position"), context);
    context->addMenu(contextMode);

    contextIcons = contextMode->addAction(i18n("Icons Only"), q, SLOT(slotContextIcons()));
    contextIcons->setChecked(true);
    contextText = contextMode->addAction(i18n("Text Only"), q, SLOT(slotContextText()));
    contextTextRight = contextMode->addAction(i18n("Text Alongside Icons"), q, SLOT(slotContextTextRight()));
    contextTextUnder = contextMode->addAction(i18n("Text Under Icons"), q, SLOT(slotContextTextUnder()));

    QActionGroup* textGroup = new QActionGroup(contextMode);
    foreach (QAction* action, contextMode->actions()) {
      action->setActionGroup(textGroup);
      action->setCheckable(true);
    }

    contextSize = new KMenu(i18n("Icon Size"), context);
    context->addMenu(contextSize);

    contextIconSizes.insert(contextSize->addAction(i18nc("@item:inmenu Icon size", "Default"), q, SLOT(slotContextIconSize())), IconSizeDefault);

    // Query the current theme for available sizes
    KIconTheme *theme = KIconLoader::global()->theme();
    QList<int> avSizes;
    if (theme) {
      if (isMainToolBar())
        avSizes = theme->querySizes(KIconLoader::MainToolbar);
      else
        avSizes = theme->querySizes(KIconLoader::Toolbar);
    }

    iconSizes = avSizes;
    qSort(avSizes);

    if (avSizes.count() < 10) {
      // Fixed or threshold type icons
      foreach (int it, avSizes) {
        QString text;
        if (it < 19)
          text = i18n("Small (%1x%2)", it, it);
        else if (it < 25)
          text = i18n("Medium (%1x%2)", it, it);
        else if (it < 35)
          text = i18n("Large (%1x%2)", it, it);
        else
          text = i18n("Huge (%1x%2)", it, it);

        // save the size in the contextIconSizes map
        contextIconSizes.insert(contextSize->addAction(text, q, SLOT(slotContextIconSize())), it);
      }
    } else {
      // Scalable icons.
      const int progression[] = { 16, 22, 32, 48, 64, 96, 128, 192, 256 };

      for (uint i = 0; i < 9; i++) {
        foreach (int it, avSizes) {
          if (it >= progression[ i ]) {
            QString text;
            if (it < 19)
              text = i18n("Small (%1x%2)", it, it);
            else if (it < 25)
              text = i18n("Medium (%1x%2)", it, it);
            else if (it < 35)
              text = i18n("Large (%1x%2)", it, it);
            else
              text = i18n("Huge (%1x%2)", it, it);

            // save the size in the contextIconSizes map
            contextIconSizes.insert(contextSize->addAction(text, q, SLOT(slotContextIconSize())), it);
            break;
          }
        }
      }
    }

    QActionGroup* sizeGroup = new QActionGroup(contextSize);
    foreach (QAction* action, contextSize->actions()) {
      action->setActionGroup(sizeGroup);
      action->setCheckable(true);
    }

    if (!q->toolBarsLocked() && !q->isMovable())
      unlockedMovable = false;

    delete contextLockAction;
    contextLockAction = new KToggleAction(KIcon("system-lock-screen"), (q->toolBarsLocked())?i18n("Unlock Toolbars"):i18n("Lock Toolbars"), q);
    context->addAction(contextLockAction);
    contextLockAction->setChecked(q->toolBarsLocked());
    connect(contextLockAction, SIGNAL(toggled(bool)), q, SLOT(slotLockToolBars(bool)));

    connect(context, SIGNAL(aboutToShow()), q, SLOT(slotContextAboutToShow()));
  }

  contextOrient->menuAction()->setVisible(!q->toolBarsLocked());
  contextMode->menuAction()->setVisible(!q->toolBarsLocked());
  contextSize->menuAction()->setVisible(!q->toolBarsLocked());

  // Unplugging a submenu from abouttohide leads to the popupmenu floating around
  // So better simply call that code from after exec() returns (DF)
  //connect(context, SIGNAL(aboutToHide()), this, SLOT(slotContextAboutToHide()));

  return context;
}

bool KToolBar::Private::isMainToolBar() const
{
  return q->objectName() == QLatin1String("mainToolBar");
}

void KToolBar::Private::setLocked(bool locked)
{
  if (unlockedMovable)
    q->setMovable(!locked);
}

void KToolBar::Private::adjustSeparatorVisibility()
{
  bool visibleNonSeparator = false;
  int separatorToShow = -1;

  for (int index = 0; index < q->actions().count(); ++index) {
    QAction* action = q->actions()[ index ];
    if (action->isSeparator()) {
      if (visibleNonSeparator) {
        separatorToShow = index;
        visibleNonSeparator = false;
      } else {
        action->setVisible(false);
      }
    } else if (!visibleNonSeparator) {
      if (action->isVisible()) {
        visibleNonSeparator = true;
        if (separatorToShow != -1) {
          q->actions()[ separatorToShow ]->setVisible(true);
          separatorToShow = -1;
        }
      }
    }
  }

  if (separatorToShow != -1)
    q->actions()[ separatorToShow ]->setVisible(false);
}

Qt::ToolButtonStyle KToolBar::Private::toolButtonStyleFromString(const QString & _style)
{
  QString style = _style.toLower();
  if (style == "textbesideicon" || style == "icontextright")
    return Qt::ToolButtonTextBesideIcon;
  else if (style == "textundericon" || style == "icontextbottom")
    return Qt::ToolButtonTextUnderIcon;
  else if (style == "textonly")
    return Qt::ToolButtonTextOnly;
  else
    return Qt::ToolButtonIconOnly;
}

QString KToolBar::Private::toolButtonStyleToString(Qt::ToolButtonStyle style)
{
  switch(style)
  {
    case Qt::ToolButtonIconOnly:
    default:
      return "IconOnly";
    case Qt::ToolButtonTextBesideIcon:
      return "TextBesideIcon";
    case Qt::ToolButtonTextOnly:
      return "TextOnly";
    case Qt::ToolButtonTextUnderIcon:
      return "TextUnderIcon";
  }
}

Qt::ToolBarArea KToolBar::Private::positionFromString(const QString& position)
{
    Qt::ToolBarArea newposition = Qt::TopToolBarArea;
    if (position == QLatin1String("left")) {
        newposition = Qt::LeftToolBarArea;
    } else if (position == QLatin1String("bottom")) {
        newposition = Qt::BottomToolBarArea;
    } else if (position == QLatin1String("right")) {
        newposition = Qt::RightToolBarArea;
    }
    return newposition;
}

void KToolBar::Private::slotReadConfig()
{
  /**
   * Read appearance settings (hmm, we used to do both here,
   * but a well behaved application will call applyMainWindowSettings
   * anyway, right ?)
   */
  KConfigGroup cg(KGlobal::config(), QString());
  q->applyAppearanceSettings(cg);
}

void KToolBar::Private::slotAppearanceChanged()
{
  // Read appearance settings from global file.
  KConfigGroup cg(KGlobal::config(), QString());
  q->applyAppearanceSettings(cg , true /* lose local settings */);

  // And remember to save the new look later
  KMainWindow *kmw = qobject_cast<KMainWindow *>(q->mainWindow());
  if (kmw)
    kmw->setSettingsDirty();
}

void KToolBar::Private::slotContextAboutToShow()
{
  /**
   * The idea here is to reuse the "static" part of the menu to save time.
   * But the "Toolbars" action is dynamic (can be a single action or a submenu)
   * and ToolBarHandler::setupActions() deletes it, so better not keep it around.
   * So we currently plug/unplug the last two actions of the menu.
   * Another way would be to keep around the actions and plug them all into a (new each time) popupmenu.
   */
  KXmlGuiWindow *kmw = qobject_cast<KXmlGuiWindow *>(q->mainWindow());
  if (kmw) {
    kmw->setupToolbarMenuActions();
    // Only allow hiding a toolbar if the action is also plugged somewhere else (e.g. menubar)
    QAction *tbAction = kmw->toolBarMenuAction();
    if (!q->toolBarsLocked() && tbAction && tbAction->associatedWidgets().count() > 0)
      contextMenu()->addAction(tbAction);
  }

  // try to find "configure toolbars" action
  QAction *configureAction = 0;
  const char* actionName = KStandardAction::name(KStandardAction::ConfigureToolbars);
  if (xmlguiClient)
    configureAction = xmlguiClient->actionCollection()->action(actionName);

  if (!configureAction && kmw)
    configureAction = kmw->actionCollection()->action(actionName);

  if (configureAction)
    context->addAction(configureAction);

  KEditToolBar::setGlobalDefaultToolBar(q->QObject::objectName().toLatin1().constData());

  // Check the actions that should be checked
  switch (q->toolButtonStyle()) {
    case Qt::ToolButtonIconOnly:
    default:
      contextIcons->setChecked(true);
      break;
    case Qt::ToolButtonTextBesideIcon:
      contextTextRight->setChecked(true);
      break;
    case Qt::ToolButtonTextOnly:
      contextText->setChecked(true);
      break;
    case Qt::ToolButtonTextUnderIcon:
      contextTextUnder->setChecked(true);
      break;
  }

  QMapIterator< QAction*, int > it = contextIconSizes;
  while (it.hasNext()) {
    it.next();
    if (it.value() == q->iconSize().width()) {
      it.key()->setChecked(true);
      break;
    }
  }

  switch (q->mainWindow()->toolBarArea(q)) {
    case Qt::BottomToolBarArea:
      contextBottom->setChecked(true);
      break;
    case Qt::LeftToolBarArea:
      contextLeft->setChecked(true);
      break;
    case Qt::RightToolBarArea:
      contextRight->setChecked(true);
      break;
    default:
    case Qt::TopToolBarArea:
      contextTop->setChecked(true);
      break;
  }
}

void KToolBar::Private::slotContextAboutToHide()
{
  // We have to unplug whatever slotContextAboutToShow plugged into the menu.
  // Unplug the toolbar menu action
  KXmlGuiWindow *kmw = qobject_cast<KXmlGuiWindow *>(q->mainWindow());
  if (kmw && kmw->toolBarMenuAction())
    if (kmw->toolBarMenuAction()->associatedWidgets().count() > 1)
      contextMenu()->removeAction(kmw->toolBarMenuAction());

  // Unplug the configure toolbars action too, since it's afterwards anyway
  QAction *configureAction = 0;
  const char* actionName = KStandardAction::name(KStandardAction::ConfigureToolbars);
  if (xmlguiClient)
    configureAction = xmlguiClient->actionCollection()->action(actionName);

  if (!configureAction && kmw)
    configureAction = kmw->actionCollection()->action(actionName);

  if (configureAction)
    context->removeAction(configureAction);
}

void KToolBar::Private::slotContextLeft()
{
  q->mainWindow()->addToolBar(Qt::LeftToolBarArea, q);
}

void KToolBar::Private::slotContextRight()
{
  q->mainWindow()->addToolBar(Qt::RightToolBarArea, q);
}

void KToolBar::Private::slotContextTop()
{
  q->mainWindow()->addToolBar(Qt::TopToolBarArea, q);
}

void KToolBar::Private::slotContextBottom()
{
  q->mainWindow()->addToolBar(Qt::BottomToolBarArea, q);
}

void KToolBar::Private::slotContextIcons()
{
  q->setToolButtonStyle(Qt::ToolButtonIconOnly);
}

void KToolBar::Private::slotContextText()
{
  q->setToolButtonStyle(Qt::ToolButtonTextOnly);
}

void KToolBar::Private::slotContextTextUnder()
{
  q->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
}

void KToolBar::Private::slotContextTextRight()
{
  q->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
}

void KToolBar::Private::slotContextIconSize()
{
  QAction* action = qobject_cast<QAction*>(q->sender());
  if (action && contextIconSizes.contains(action)) {
    q->setIconDimensions(contextIconSizes.value(action));
  }
}

void KToolBar::Private::slotLockToolBars(bool lock)
{
  q->setToolBarsLocked(lock);
}



KToolBar::KToolBar(QWidget *parent, bool honorStyle, bool readConfig)
  : QToolBar(parent),
    d(new Private(this))
{
  d->init(readConfig, honorStyle);

  // KToolBar is auto-added to the top area of the main window if parent is a QMainWindow
  if (QMainWindow* mw = qobject_cast<QMainWindow*>(parent))
    mw->addToolBar(this);
}

KToolBar::KToolBar(const QString& objectName, QMainWindow* parent, Qt::ToolBarArea area,
                    bool newLine, bool honorStyle, bool readConfig)
  : QToolBar(parent),
    d(new Private(this))
{
  setObjectName(objectName);
  d->init(readConfig, honorStyle);

  if (newLine)
    mainWindow()->addToolBarBreak(area);

  mainWindow()->addToolBar(area, this);

  if (newLine)
    mainWindow()->addToolBarBreak(area);
}

KToolBar::~KToolBar()
{
  delete d->contextLockAction;
  delete d;
}

void KToolBar::setContextMenuEnabled(bool enable)
{
  d->enableContext = enable;
}

bool KToolBar::contextMenuEnabled() const
{
  return d->enableContext;
}

void KToolBar::saveSettings(KConfigGroup &cg)
{
    Q_ASSERT(!cg.name().isEmpty());

  QString position;
  Qt::ToolButtonStyle ToolButtonStyle;
  int index;
  d->getAttributes(position, ToolButtonStyle, index);

  if (!cg.hasDefault("Position") && position == d->PositionDefault)
    cg.revertToDefault("Position");
  else
    cg.writeEntry("Position", position);

  if (d->honorStyle && ToolButtonStyle == d->ToolButtonStyleDefault && !cg.hasDefault("ToolButtonStyle"))
    cg.revertToDefault("ToolButtonStyle");
  else
    cg.writeEntry("ToolButtonStyle", d->toolButtonStyleToString(ToolButtonStyle));

  if (!cg.hasDefault("IconSize") && iconSize().width() == iconSizeDefault())
    cg.revertToDefault("IconSize");
  else
    cg.writeEntry("IconSize", iconSize().width());

  if (!cg.hasDefault("Hidden") && isHidden() == d->HiddenDefault)
    cg.revertToDefault("Hidden");
  else
    cg.writeEntry("Hidden", isHidden());

    // Note that index, unlike the other settings, depends on the other toolbars
    // So on the first run with a clean local config file, even the usual
    // hasDefault/==IndexDefault test would save the toolbar indexes
    // (IndexDefault was 0, whereas index is the real index in the GUI)
    //
    // Saving the whole set of indexes is necessary though. When moving only
    // one toolbar, if we only saved the changed indexes, the toolbars wouldn't
    // reappear at the same position the next time.
    // The whole set of indexes has to be saved.
    //kDebug(220) << name() << "                writing index " << index;

    KMainWindow* kmw = mainWindow();
    QList<KToolBar*> toolbarList = kmw->findChildren<KToolBar*>();

    // don't save if there's only one toolbar
    if (!kmw || toolbarList.count() > 1)
        cg.writeEntry("Index", index);
    else
        cg.revertToDefault("Index");

    /* FIXME KToolBar port - need to implement
    if(!cg.hasDefault("NewLine") && newLine() == d->NewLineDefault)
      cg.revertToDefault("NewLine");
    else
      cg.writeEntry("NewLine", newLine());*/
}

void KToolBar::setXMLGUIClient(KXMLGUIClient *client)
{
  d->xmlguiClient = client;
}

void KToolBar::contextMenuEvent(QContextMenuEvent* event)
{
  if (mainWindow() && d->enableContext) {
    QPointer<KToolBar> guard(this);
    d->contextMenu()->exec(event->globalPos());

    // "Configure Toolbars" recreates toolbars, so we might not exist anymore.
    if (guard)
      d->slotContextAboutToHide();
    return;
  }

  QToolBar::contextMenuEvent(event);
}

Qt::ToolButtonStyle KToolBar::toolButtonStyleSetting()
{
  KConfigGroup saver(KGlobal::config(), "Toolbar style");

  return KToolBar::Private::toolButtonStyleFromString(saver.readEntry("ToolButtonStyle", "TextUnderIcon"));
}

void KToolBar::loadState(const QDomElement &element)
{
  QMainWindow *mw = mainWindow();
  if (!mw)
    return;

  {
    QByteArray text = element.namedItem("text").toElement().text().toUtf8();
    if (text.isEmpty())
      text = element.namedItem("Text").toElement().text().toUtf8();

    if (!text.isEmpty())
      setWindowTitle(i18n(text));
  }

  /*
    This method is called in order to load toolbar settings from XML.
    However this can be used in two rather different cases:
    - for the initial loading of the app's XML. In that case the settings
      are only the defaults, the user's KConfig settings will override them
      (KDE4 TODO: how about saving those user settings into the local XML file instead?
      Then this whole thing would be simpler, no KConfig settings to apply afterwards.
      OTOH we'd have to migrate those settings when the .rc version increases,
      like we do for shortcuts)

    - for later re-loading when switching between parts in KXMLGUIFactory.
      In that case the XML contains the final settings, not the defaults.
      We do need the defaults, and the toolbar might have been completely
      deleted and recreated meanwhile. So we store the app-default settings
      into the XML.
   */
  bool loadingAppDefaults = true;
  if (element.hasAttribute("newlineDefault")) {
    // this isn't the first time, so the defaults have been saved into the (in-memory) XML
    loadingAppDefaults = false;
    d->NewLineDefault = element.attribute("newlineDefault") == "true";
    d->HiddenDefault = element.attribute("hiddenDefault") == "true";
    d->IconSizeDefault = element.attribute("iconSizeDefault").toInt();
    d->PositionDefault = element.attribute("positionDefault");
    d->ToolButtonStyleDefault = d->toolButtonStyleFromString(element.attribute("toolButtonStyleDefault"));
  }

  {
    QString attrIconText = element.attribute("iconText").toLower().toLatin1();
    if (!attrIconText.isEmpty()) {
      setToolButtonStyle(d->toolButtonStyleFromString(attrIconText));
    } else {
      if (d->honorStyle)
        setToolButtonStyle(toolButtonStyleSetting());
      else
        setToolButtonStyle(d->ToolButtonStyleDefault);
    }
  }

  QString attrIconSize = element.attribute("iconSize").toLower().trimmed();
  int iconSize = d->IconSizeDefault;

  {
    bool ok;
    int newIconSize = attrIconSize.toInt(&ok);
    if (ok)
      iconSize = newIconSize;
  }

  setIconDimensions(iconSize);

  int index = -1; // append by default. This is very important, otherwise

  // with all 0 indexes, we keep reversing the toolbars.
  {
    QString attrIndex = element.attribute("index").toLower();
    if (!attrIndex.isEmpty())
      index = attrIndex.toInt();
  }

  bool newLine = d->NewLineDefault;
  bool hidden = d->HiddenDefault;

  {
    QString attrNewLine = element.attribute("newline").toLower();
    if (!attrNewLine.isEmpty())
      newLine = attrNewLine == "true";

    if (newLine && mw)
      mw->insertToolBarBreak(this);
  }

  {
    QString attrHidden = element.attribute("hidden").toLower();
    if (!attrHidden.isEmpty())
      hidden = attrHidden  == "true";
  }

  Qt::ToolBarArea pos = Qt::NoToolBarArea;
  {
    QString attrPosition = element.attribute("position").toLower();
    if (!attrPosition.isEmpty())
      pos = KToolBar::Private::positionFromString(attrPosition);
  }
  if (pos != Qt::NoToolBarArea)
    mw->addToolBar(pos, this);

  if (hidden)
    hide();
  else
    show();

  if (loadingAppDefaults) {
    d->getAttributes(d->PositionDefault, d->ToolButtonStyleDefault, index);

    d->NewLineDefault = newLine;
    d->HiddenDefault = hidden;
    d->IconSizeDefault = iconSize;
  }
}

void KToolBar::saveState(QDomElement &current) const
{
  Q_ASSERT(!current.isNull());

  QString position;
  Qt::ToolButtonStyle ToolButtonStyle;
  int index = -1;
  d->getAttributes(position, ToolButtonStyle, index);
  position = position.toLower();

  current.setAttribute("noMerge", "1");
  current.setAttribute("position", position);
  current.setAttribute("toolButtonStyle", d->toolButtonStyleToString(ToolButtonStyle));
  current.setAttribute("index", index);
  // FIXME KAction port
  //current.setAttribute("newline", newLine());
  if (isHidden())
    current.setAttribute("hidden", "true");
  d->modified = true;

  // TODO if this method is used by more than KXMLGUIBuilder, e.g. to save XML settings to *disk*,
  // then the stuff below shouldn't always be done.
  current.setAttribute("newlineDefault", d->NewLineDefault);
  current.setAttribute("hiddenDefault", d->HiddenDefault ? "true" : "false");
  current.setAttribute("iconSizeDefault", d->IconSizeDefault);
  current.setAttribute("positionDefault", d->PositionDefault);
  current.setAttribute("toolButtonStyleDefault", d->toolButtonStyleToString(d->ToolButtonStyleDefault));
}

void KToolBar::applySettings(const KConfigGroup &cg, bool force)
{
    Q_ASSERT(!cg.name().isEmpty());

  /*
    Let's explain this a bit more in details.
    The order in which we apply settings is :
     Global config / <appnamerc> user settings                        if no XMLGUI is used
     Global config / App-XML attributes / <appnamerc> user settings   if XMLGUI is used

    So in the first case, we simply read everything from KConfig as below,
    but in the second case we don't do anything here if there is no app-specific config,
    and the XMLGUI-related code (loadState()) uses the static methods of this class
    to get the global defaults.

    Global config doesn't include position (index, newline and hidden/shown).
  */

  // First the appearance stuff - the one which has a global config
  applyAppearanceSettings(cg);

  // ...and now the position stuff
  if (cg.exists() || force) {
#if 0 // currently unused
    QString position = cg.readEntry("Position", d->PositionDefault);
    int index = cg.readEntry("Index", int(-1));
    bool newLine = cg.readEntry("NewLine", d->NewLineDefault);

    Qt::ToolBarArea pos = Qt::TopToolBarArea;
    if (position == "Top")
      pos = Qt::TopToolBarArea;
    else if (position == "Bottom")
      pos = Qt::BottomToolBarArea;
    else if (position == "Left")
      pos = Qt::LeftToolBarArea;
    else if (position == "Right")
      pos = Qt::RightToolBarArea;
#endif

    bool hidden = cg.readEntry("Hidden", d->HiddenDefault);
    if (hidden)
      hide();
    else
      show();
  }
}

void KToolBar::applyAppearanceSettings(const KConfigGroup &cg, bool forceGlobal)
{
  Q_ASSERT(! cg.name().isEmpty());

  // If we have application-specific settings in the XML file,
  // and nothing in the application's config file, then
  // we don't apply the global defaults, the XML ones are preferred
  // (see applySettings for a full explanation)
  // This is the reason for the xmlgui tests below.
  bool xmlgui = d->xmlguiClient && !d->xmlguiClient->xmlFile().isEmpty();

  KSharedConfig::Ptr gconfig = KGlobal::config();

  // we actually do this in two steps.
  // First, we read in the global styles [Toolbar style] (from the KControl module).
  // Then, if the toolbar is NOT 'mainToolBar', we will also try to read in [barname Toolbar style]
  bool applyToolButtonStyle = !xmlgui; // if xmlgui is used, global defaults won't apply
  bool applyIconSize = !xmlgui;

  int iconSize = d->IconSizeDefault;
  Qt::ToolButtonStyle ToolButtonStyle = d->ToolButtonStyleDefault;

  // this is the first iteration

  { // start block for KConfigGroup
    KConfigGroup globals(gconfig, "Toolbar style");

    if (applyToolButtonStyle)
    {
      // we read in the ToolButtonStyle property *only* if we intend on actually
      // honoring it
      if (d->honorStyle)
      d->ToolButtonStyleDefault = d->toolButtonStyleFromString(globals.readEntry("ToolButtonStyle",
                                          d->toolButtonStyleToString(d->ToolButtonStyleDefault)));
      else
      d->ToolButtonStyleDefault = Qt::ToolButtonTextUnderIcon;

      // Use the default icon size for toolbar icons.
      d->IconSizeDefault = globals.readEntry("IconSize", int(d->IconSizeDefault));
    }

    iconSize = d->IconSizeDefault;
    ToolButtonStyle = d->ToolButtonStyleDefault;

    if (!forceGlobal && cg.exists()) {

      // read in the ToolButtonStyle property
      if (cg.hasKey("ToolButtonStyle")) {
          ToolButtonStyle = d->toolButtonStyleFromString(cg.readEntry("ToolButtonStyle", QString()));
          applyToolButtonStyle = true;
      }

      // now get the size
      if (cg.hasKey("IconSize")) {
          iconSize = cg.readEntry("IconSize", 0);
          applyIconSize = true;
      }
    }
  } // end block for KConfigGroup

  // check if the icon/text has changed
  if (ToolButtonStyle != toolButtonStyle() && applyToolButtonStyle)
    setToolButtonStyle(ToolButtonStyle);

  // ...and check if the icon size has changed
  if (iconSize != KToolBar::iconSize().width() && applyIconSize)
    setIconDimensions(iconSize);
}

KMainWindow * KToolBar::mainWindow() const
{
  return qobject_cast<KMainWindow*>(const_cast<QObject*>(parent()));
}

void KToolBar::setIconDimensions(int size)
{
  QToolBar::setIconSize(QSize(size, size));
}

int KToolBar::iconSizeDefault() const
{
  if (QObject::objectName() == "mainToolBar")
    return KIconLoader::global()->currentSize(KIconLoader::MainToolbar);

  return KIconLoader::global()->currentSize(KIconLoader::Toolbar);
}

void KToolBar::slotMovableChanged(bool movable)
{
  if (movable && !KAuthorized::authorize("movable_toolbars"))
    setMovable(false);
}

void KToolBar::dragEnterEvent(QDragEnterEvent *event)
{
  if (toolBarsEditable() && event->proposedAction() & (Qt::CopyAction | Qt::MoveAction) &&
       event->mimeData()->hasFormat("application/x-kde-action-list")) {
    QByteArray data = event->mimeData()->data("application/x-kde-action-list");

    QDataStream stream(data);

    QStringList actionNames;

    stream >> actionNames;

    foreach (const QString& actionName, actionNames) {
      foreach (KActionCollection* ac, KActionCollection::allCollections()) {
        QAction* newAction = ac->action(actionName.toAscii().constData());
        if (newAction) {
          d->actionsBeingDragged.append(newAction);
          break;
        }
      }
    }

    if (d->actionsBeingDragged.count()) {
      QAction* overAction = actionAt(event->pos());

      QFrame* dropIndicatorWidget = new QFrame(this);
      dropIndicatorWidget->resize(8, height() - 4);
      dropIndicatorWidget->setFrameShape(QFrame::VLine);
      dropIndicatorWidget->setLineWidth(3);

      d->dropIndicatorAction = insertWidget(overAction, dropIndicatorWidget);

      insertAction(overAction, d->dropIndicatorAction);

      event->acceptProposedAction();
      return;
    }
  }

  QToolBar::dragEnterEvent(event);
}

void KToolBar::dragMoveEvent(QDragMoveEvent *event)
{
  if (toolBarsEditable())
    forever {
      if (d->dropIndicatorAction) {
        QAction* overAction = 0L;
        foreach (QAction* action, actions()) {
          // want to make it feel that half way across an action you're dropping on the other side of it
          QWidget* widget = widgetForAction(action);
          if (event->pos().x() < widget->pos().x() + (widget->width() / 2)) {
            overAction = action;
            break;
          }
        }

        if (overAction != d->dropIndicatorAction) {
          // Check to see if the indicator is already in the right spot
          int dropIndicatorIndex = actions().indexOf(d->dropIndicatorAction);
          if (dropIndicatorIndex + 1 < actions().count()) {
            if (actions()[ dropIndicatorIndex + 1 ] == overAction)
              break;
          } else if (!overAction) {
            break;
          }

          insertAction(overAction, d->dropIndicatorAction);
        }

        event->accept();
        return;
      }
      break;
    }

  QToolBar::dragMoveEvent(event);
}

void KToolBar::dragLeaveEvent(QDragLeaveEvent *event)
{
  // Want to clear this even if toolBarsEditable was changed mid-drag (unlikey)
  delete d->dropIndicatorAction;
  d->dropIndicatorAction = 0L;
  d->actionsBeingDragged.clear();

  if (toolBarsEditable()) {
    event->accept();
    return;
  }

  QToolBar::dragLeaveEvent(event);
}

void KToolBar::dropEvent(QDropEvent *event)
{
  if (toolBarsEditable()) {
    foreach (QAction* action, d->actionsBeingDragged) {
      if (actions().contains(action))
        removeAction(action);
      insertAction(d->dropIndicatorAction, action);
    }
  }

  // Want to clear this even if toolBarsEditable was changed mid-drag (unlikey)
  delete d->dropIndicatorAction;
  d->dropIndicatorAction = 0L;
  d->actionsBeingDragged.clear();

  if (toolBarsEditable()) {
    event->accept();
    return;
  }

  QToolBar::dropEvent(event);
}

void KToolBar::mousePressEvent(QMouseEvent *event)
{
  if (toolBarsEditable() && event->button() == Qt::LeftButton) {
    if (KAction* action = qobject_cast<KAction*>(actionAt(event->pos()))) {
      d->dragAction = action;
      d->dragStartPosition = event->pos();
      event->accept();
      return;
    }
  }

  QToolBar::mousePressEvent(event);
}

void KToolBar::mouseMoveEvent(QMouseEvent *event)
{
  if (!toolBarsEditable() || !d->dragAction)
    return QToolBar::mouseMoveEvent(event);

  if ((event->pos() - d->dragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
    event->accept();
    return;
  }

  QDrag *drag = new QDrag(this);
  QMimeData *mimeData = new QMimeData;

  QByteArray data;
  {
    QDataStream stream(&data, QIODevice::WriteOnly);

    QStringList actionNames;
    actionNames << d->dragAction->objectName();

    stream << actionNames;
  }

  mimeData->setData("application/x-kde-action-list", data);

  drag->setMimeData(mimeData);

  Qt::DropAction dropAction = drag->start(Qt::MoveAction);

  if (dropAction == Qt::MoveAction)
    // Only remove from this toolbar if it was moved to another toolbar
    // Otherwise the receiver moves it.
    if (drag->target() != this)
      removeAction(d->dragAction);

  d->dragAction = 0L;
  event->accept();
}

void KToolBar::mouseReleaseEvent(QMouseEvent *event)
{
  // Want to clear this even if toolBarsEditable was changed mid-drag (unlikey)
  if (d->dragAction) {
    d->dragAction = 0L;
    event->accept();
    return;
  }

  QToolBar::mouseReleaseEvent(event);
}

bool KToolBar::eventFilter(QObject * watched, QEvent * event)
{
    // Generate context menu events for disabled buttons too...
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if (me->buttons() & Qt::RightButton)
            if (QWidget* ww = qobject_cast<QWidget*>(watched))
                if (ww->parent() == this && !ww->isEnabled())
                    QCoreApplication::postEvent(this, new QContextMenuEvent(QContextMenuEvent::Mouse, me->pos(), me->globalPos()));

    } else if (event->type() == QEvent::ParentChange) {
        // Make sure we're not leaving stale event filters around,
        // when a child is reparented somewhere else
        if (QWidget* ww = qobject_cast<QWidget*>(watched)) {
            if (!this->isAncestorOf(ww)) {
                // New parent is not a subwidget - remove event filter
                ww->removeEventFilter(this);
                foreach (QWidget* child, ww->findChildren<QWidget*>())
                    child->removeEventFilter(this);
            }
        }
    }

    QToolButton* tb;
    if ((tb = qobject_cast<QToolButton*>(watched)) && !tb->actions().isEmpty()) {
        // Handle MMB on toolbar buttons
        QAction* act = tb->actions().first();
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::MidButton /*&&
                                                 act->receivers(SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)))*/) {
                if (me->type() == QEvent::MouseButtonPress)
                    tb->setDown(true);
                else {
                    tb->setDown(false);
                    QMetaObject::invokeMethod(act, "triggered", Qt::DirectConnection,
                                              Q_ARG(Qt::MouseButtons, me->button()),
                                              Q_ARG(Qt::KeyboardModifiers, QApplication::keyboardModifiers()));
                }
            }
        }

        // CJK languages use more verbose accelerator marker: they add a Latin
        // letter in parenthesis, and put accelerator on that. Hence, the default
        // removal of ampersand only may not be enough there, instead the whole
        // parenthesis construct should be removed. Provide these filtering i18n
        // messages so that translators can use Transcript for custom removal.
        if (event->type() == QEvent::Show || event->type() == QEvent::Paint || event->type() == QEvent::EnabledChange) {
            act = tb->defaultAction();
            if (act) {
                QString text = act->iconText().isEmpty() ? act->text() : act->iconText();
                tb->setText(i18nc("@action:intoolbar Text label of toolbar button", "%1", text));
                tb->setToolTip(i18nc("@info:tooltip Tooltip of toolbar button", "%1", act->toolTip()));
            }
        }
    }

    // Redirect mouse events to the toolbar when drag + drop editing is enabled
    if (toolBarsEditable()) {
        if (QWidget* ww = qobject_cast<QWidget*>(watched)) {
            switch (event->type()) {
            case QEvent::MouseButtonPress: {
                QMouseEvent* me = static_cast<QMouseEvent*>(event);
                QMouseEvent newEvent(me->type(), mapFromGlobal(ww->mapToGlobal(me->pos())), me->globalPos(),
                                      me->button(), me->buttons(), me->modifiers());
                mousePressEvent(&newEvent);
                return true;
            }
            case QEvent::MouseMove: {
                QMouseEvent* me = static_cast<QMouseEvent*>(event);
                QMouseEvent newEvent(me->type(), mapFromGlobal(ww->mapToGlobal(me->pos())), me->globalPos(),
                                      me->button(), me->buttons(), me->modifiers());
                mouseMoveEvent(&newEvent);
                return true;
            }
            case QEvent::MouseButtonRelease: {
                QMouseEvent* me = static_cast<QMouseEvent*>(event);
                QMouseEvent newEvent(me->type(), mapFromGlobal(ww->mapToGlobal(me->pos())), me->globalPos(),
                                      me->button(), me->buttons(), me->modifiers());
                mouseReleaseEvent(&newEvent);
                return true;
            }
            default:
                break;
            }
        }
    }

    return QToolBar::eventFilter(watched, event);
}

void KToolBar::actionEvent(QActionEvent * event)
{
  if (event->type() == QEvent::ActionRemoved) {
    QWidget* widget = widgetForAction(event->action());
    if (widget) {
        widget->removeEventFilter(this);

        foreach (QWidget* child, widget->findChildren<QWidget*>())
            child->removeEventFilter(this);
    }
  }

  QToolBar::actionEvent(event);

  if (event->type() == QEvent::ActionAdded) {
    QWidget* widget = widgetForAction(event->action());
    if (widget) {
        widget->installEventFilter(this);

        foreach (QWidget* child, widget->findChildren<QWidget*>())
            child->installEventFilter(this);
    }
  }

  d->adjustSeparatorVisibility();
}

bool KToolBar::toolBarsEditable()
{
  return KToolBar::Private::s_editable;
}

void KToolBar::setToolBarsEditable(bool editable)
{
  if (KToolBar::Private::s_editable != editable)
    KToolBar::Private::s_editable = editable;
}

void KToolBar::setToolBarsLocked(bool locked)
{
  if (KToolBar::Private::s_locked != locked) {
    KToolBar::Private::s_locked = locked;

    foreach (KMainWindow* mw, KMainWindow::memberList())
      foreach (KToolBar* toolbar, mw->findChildren<KToolBar*>()) {
        toolbar->d->setLocked(locked);
        if (toolbar->d->contextLockAction)
            toolbar->d->contextLockAction->setText(locked ? i18n("Unlock Toolbars") : i18n("Lock Toolbars"));
      }

  }
}

bool KToolBar::toolBarsLocked()
{
  return KToolBar::Private::s_locked;
}

#include "ktoolbar.moc"
