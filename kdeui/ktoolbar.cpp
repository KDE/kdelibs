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

#include <config.h>

#include <string.h>

#include <QMouseEvent>
#include <qdrawutil.h>
#include <qicon.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qrect.h>
#include <qstring.h>
#include <qstyle.h>
#include <qtimer.h>

#include <kaction.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kglobal.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kipc.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kmenu.h>
#include <kseparator.h>
#include <kstdaction.h>
#include <ktoolbar.h>
#include <kxmlguifactory.h>
#include <kwin.h>
#include <kauthorized.h>
#include <kactioncollection.h>
#include <ktoggleaction.h>

class KToolBarPrivate
{
public:
    KToolBarPrivate() {
        honorStyle   = false;

        enableContext  = true;

        unlockedMovable = true;

        m_xmlguiClient   = 0;

        oldPos = Qt::TopToolBarArea;

        modified = false;

        IconSizeDefault = 22;
        ToolButtonStyleDefault = Qt::ToolButtonTextUnderIcon;

        contextLockAction = 0L;

        NewLineDefault = false;
        OffsetDefault = 0;
        PositionDefault = "Top";
        HiddenDefault = false;

        context = 0L;

        dropIndicatorAction = 0L;

        dragAction = 0L;
    }

    bool honorStyle : 1;
    bool enableContext : 1;
    bool modified : 1;
    bool unlockedMovable : 1;
    static bool s_editable;
    static bool s_locked;

    Qt::ToolBarArea oldPos;

    KXMLGUIClient *m_xmlguiClient;

    struct ToolBarInfo
    {
        ToolBarInfo() : index( -1 ), offset( -1 ), newline( false ), area( Qt::TopToolBarArea ) {}
        ToolBarInfo( Qt::ToolBarArea a, int i, bool n, int o ) : index( i ), offset( o ), newline( n ), area( a ) {}
        int index, offset;
        bool newline;
        Qt::ToolBarArea area;
    };

    ToolBarInfo toolBarInfo;
    QList<int> iconSizes;
    QTimer repaintTimer;

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
  bool HiddenDefault;
  int IconSizeDefault;
  Qt::ToolButtonStyle ToolButtonStyleDefault;
  bool NewLineDefault;
  int OffsetDefault;
  QString PositionDefault;

  QList<KAction*> actionsBeingDragged;
  QAction* dropIndicatorAction;

  KMenu* context;
  KAction* dragAction;
  QPoint dragStartPosition;
};

bool KToolBarPrivate::s_editable = false;
bool KToolBarPrivate::s_locked = false;

KToolBar::KToolBar( QWidget *parent, bool honorStyle, bool readConfig )
    : QToolBar( parent )
    , d(new KToolBarPrivate)
{
    init( readConfig, honorStyle );

    // KToolBar is auto-added to the top area of the main window if parent is a QMainWindow
    if (QMainWindow* mw = qobject_cast<QMainWindow*>(parent))
      mw->addToolBar(this);
}

KToolBar::KToolBar( const QString& objectName, QMainWindow* parent, Qt::ToolBarArea area, bool newLine, bool honorStyle, bool readConfig )
    : QToolBar(parent)
    , d(new KToolBarPrivate)
{
    setObjectName(objectName);
    init( readConfig, honorStyle );

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

void KToolBar::init( bool readConfig, bool honorStyle )
{
    d->honorStyle = honorStyle;

    // finally, read in our configurable settings
    if ( readConfig )
        slotReadConfig();

    if ( mainWindow() ) {
        // Get notified when settings change
        connect( this, SIGNAL( allowedAreasChanged(Qt::ToolBarAreas) ), mainWindow(), SLOT( setSettingsDirty() ) );
        connect( this, SIGNAL( iconSizeChanged(const QSize&) ), mainWindow(), SLOT( setSettingsDirty() ) );
        connect( this, SIGNAL( toolButtonStyleChanged(Qt::ToolButtonStyle) ), mainWindow(), SLOT( setSettingsDirty() ) );
        connect( this, SIGNAL( movableChanged(bool) ), mainWindow(), SLOT( setSettingsDirty() ) );
        connect( this, SIGNAL( orientationChanged(Qt::Orientation) ), mainWindow(), SLOT( setSettingsDirty() ) );
    }
    if (!KAuthorized::authorize("movable_toolbars"))
        setMovable(false);

    connect( this, SIGNAL( movableChanged(bool) ), SLOT( slotMovableChanged(bool) ) );

    setAcceptDrops(true);

    setFont(KGlobalSettings::toolBarFont());
}

void KToolBar::setEnableContextMenu(bool enable )
{
    d->enableContext = enable;
}


bool KToolBar::contextMenuEnabled() const
{
    return d->enableContext;
}

#if 0
void KToolBar::saveState()
{
    // first, try to save to the xml file
    if ( d->m_xmlguiClient && !d->m_xmlguiClient->xmlFile().isEmpty() ) {
        //kDebug(220) << name() << " saveState: saving to " << d->m_xmlguiClient->xmlFile() << endl;
        QString barname(objectName().isEmpty() ? "mainToolBar" : objectName());
        // try to find our toolbar
        d->modified = false;
        // go down one level to get to the right tags
        QDomElement current;
        for( QDomNode n = d->m_xmlguiClient->domDocument().documentElement().firstChild();
             !n.isNull(); n = n.nextSibling()) {
            current = n.toElement();

            if ( current.tagName().toLower() != "toolbar" )
                continue;

            QString curname(current.attribute( "name" ));

            if ( curname == barname ) {
                saveState( current );
                break;
            }
        }
        // if we didn't make changes, then just return
        if ( !d->modified )
            return;

        // now we load in the (non-merged) local file
        QString local_xml(KXMLGUIFactory::readConfigFile(d->m_xmlguiClient->xmlFile(), true, d->m_xmlguiClient->instance()));
        QDomDocument local;
        local.setContent(local_xml);

        // make sure we don't append if this toolbar already exists locally
        bool just_append = true;

        for( QDomNode n = local.documentElement().firstChild();
             !n.isNull(); n = n.nextSibling()) {
            QDomElement elem = n.toElement();

            if ( elem.tagName().toLower() != "toolbar" )
                continue;

            QString curname(elem.attribute( "name" ));

            if ( curname == barname ) {
                just_append = false;
                local.documentElement().replaceChild( current, elem );
                break;
            }
        }

        if (just_append)
            local.documentElement().appendChild( current );

        KXMLGUIFactory::saveConfigFile(local, d->m_xmlguiClient->localXMLFile(), d->m_xmlguiClient->instance() );

        return;
    }

    // if that didn't work, we save to the config file
    KConfig *config = KGlobal::config();
    saveSettings(config, QString());
    config->sync();
}
#endif

QString KToolBar::settingsGroup() const
{
    QString configGroup;
    if (objectName().isEmpty() || isMainToolBar())
        configGroup = "Toolbar style";
    else
        configGroup = QString(objectName()) + " Toolbar style";
    if ( mainWindow() )
    {
        configGroup.prepend(" ");
        configGroup.prepend( mainWindow()->objectName() );
    }
    return configGroup;
}

void KToolBar::saveSettings(KConfig *config, const QString &_configGroup)
{
    QString configGroup = _configGroup;
    if (configGroup.isEmpty())
        configGroup = settingsGroup();
    //kDebug(220) << name() << " saveSettings() group=" << _configGroup << " -> " << configGroup << endl;

    QString position;
    Qt::ToolButtonStyle ToolButtonStyle;
    int index;
    getAttributes( position, ToolButtonStyle, index );

    //kDebug(220) << name() << "                position=" << position << " index=" << index << " offset=" << offset() << " newLine=" << newLine() << endl;

    KConfigGroup cg(config, configGroup);

    if(!cg.hasDefault("Position") && position == d->PositionDefault )
      cg.revertToDefault("Position");
    else
      cg.writeEntry("Position", position);

    //kDebug(220) << name() << "                ToolButtonStyle=" << ToolButtonStyle << " hasDefault:" << config->hasDefault( "ToolButtonStyle" ) << " d->ToolButtonStyleDefault=" << d->ToolButtonStyleDefault << endl;

    if(d->honorStyle && ToolButtonStyle == d->ToolButtonStyleDefault && !cg.hasDefault("ToolButtonStyle") )
    {
      //kDebug(220) << name() << "                reverting ToolButtonStyle to default" << endl;
      cg.revertToDefault("ToolButtonStyle");
    }
    else
    {
      //kDebug(220) << name() << "                writing ToolButtonStyle " << ToolButtonStyle << endl;
      cg.writeEntry("ToolButtonStyle", toolButtonStyleToString(ToolButtonStyle));
    }

    if(!cg.hasDefault("IconSize") && iconSize().width() == iconSizeDefault() )
      cg.revertToDefault("IconSize");
    else
      cg.writeEntry("IconSize", iconSize().width());

    if(!cg.hasDefault("Hidden") && isHidden() == d->HiddenDefault )
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
    //kDebug(220) << name() << "                writing index " << index << endl;

    // don't save if there's only one toolbar

    // Don't use kmw->toolBarIterator() because you might
    // mess up someone else's iterator.  Make the list on your own

    /* FIXME KMainWindow port - no replacement
    QList<KToolBar*> toolbarList = mainWindow()->findChildren<KToolBar*>();

    if ( !kmw || toolbarList.count() > 1 )
        cg.writeEntry("Index", index);
    else
        cg.revertToDefault("Index");*/

    /* FIXME KMainWindow port - no replacement
    if(!cg.hasDefault("Offset") && offset() == d->OffsetDefault )
      cg.revertToDefault("Offset");
    else
      cg.writeEntry("Offset", offset());*/

    /* FIXME KToolBar port - need to implement
    if(!cg.hasDefault("NewLine") && newLine() == d->NewLineDefault )
      cg.revertToDefault("NewLine");
    else
      cg.writeEntry("NewLine", newLine());*/
}

void KToolBar::setXMLGUIClient( KXMLGUIClient *client )
{
    d->m_xmlguiClient = client;
}

void KToolBar::contextMenuEvent(QContextMenuEvent* event)
{
    if ( mainWindow() && d->enableContext ) {
        QPointer<KToolBar> guard( this );
        contextMenu()->exec( event->globalPos() );
        // "Configure Toolbars" recreates toolbars, so we might not exist anymore.
        if ( guard )
            slotContextAboutToHide();
        return;
    }

    return QToolBar::contextMenuEvent(event);
}

//static
Qt::ToolButtonStyle KToolBar::toolButtonStyleSetting()
{
    QString grpToolbar(QLatin1String("Toolbar style"));
    KConfigGroup saver(KGlobal::config(), grpToolbar);
    return toolButtonStyleFromString(KGlobal::config()->readEntry(QLatin1String("ToolButtonStyle"),QString::fromLatin1("TextUnderIcon")));
}

void KToolBar::loadState( const QDomElement &element )
{
    QMainWindow *mw = mainWindow();

    if ( !mw )
        return;

    {
        QByteArray text = element.namedItem( "text" ).toElement().text().toUtf8();
        if ( text.isEmpty() )
            text = element.namedItem( "Text" ).toElement().text().toUtf8();
        if ( !text.isEmpty() )
            setWindowTitle( i18n( text ) );
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
    if ( element.hasAttribute( "offsetDefault" ) )
    {
        // this isn't the first time, so the defaults have been saved into the (in-memory) XML
        loadingAppDefaults = false;
        d->OffsetDefault = element.attribute( "offsetDefault" ).toInt();
        d->NewLineDefault = element.attribute( "newlineDefault" ) == "true";
        d->HiddenDefault = element.attribute( "hiddenDefault" ) == "true";
        d->IconSizeDefault = element.attribute( "iconSizeDefault" ).toInt();
        d->PositionDefault = element.attribute( "positionDefault" );
        d->ToolButtonStyleDefault = toolButtonStyleFromString(element.attribute( "toolButtonStyleDefault" ));
    }
    //kDebug(220) << name() << " loadState loadingAppDefaults=" << loadingAppDefaults << endl;

    {
        QString attrIconText = element.attribute( "iconText" ).toLower().toLatin1();
        if ( !attrIconText.isEmpty() ) {
            setToolButtonStyle(toolButtonStyleFromString(attrIconText));
        } else {
          if (d->honorStyle)
            setToolButtonStyle(d->ToolButtonStyleDefault);
        }
    }

    QString attrIconSize = element.attribute( "iconSize" ).toLower().trimmed();
    int iconSize = d->IconSizeDefault;
    {
        bool ok;
        int newIconSize = attrIconSize.toInt(&ok);
        if (ok)
            iconSize = newIconSize;
    }
    setIconDimensions( iconSize );

    int index = -1; // append by default. This is very important, otherwise
    // with all 0 indexes, we keep reversing the toolbars.
    {
        QString attrIndex = element.attribute( "index" ).toLower();
        if ( !attrIndex.isEmpty() )
            index = attrIndex.toInt();
    }

    int offset = d->OffsetDefault;
    bool newLine = d->NewLineDefault;
    bool hidden = d->HiddenDefault;

    {
        QString attrOffset = element.attribute( "offset" );
        if ( !attrOffset.isEmpty() )
            offset = attrOffset.toInt();
    }

    {
        QString attrNewLine = element.attribute( "newline" ).toLower();
        if ( !attrNewLine.isEmpty() )
            newLine = attrNewLine == "true";

        if (newLine && mainWindow())
          mainWindow()->insertToolBarBreak(this);
    }

    {
        QString attrHidden = element.attribute( "hidden" ).toLower();
        if ( !attrHidden.isEmpty() ) {
            hidden = attrHidden  == "true";
        }
    }

    if ( hidden )
        hide();
    else
        show();

    if ( loadingAppDefaults )
    {
        getAttributes( d->PositionDefault, d->ToolButtonStyleDefault, index );
        //kDebug(220) << name() << " loadState ToolButtonStyleDefault=" << d->ToolButtonStyleDefault << endl;
        d->OffsetDefault = offset;
        d->NewLineDefault = newLine;
        d->HiddenDefault = hidden;
        d->IconSizeDefault = iconSize;
    }
    //kDebug(220) << name() << " loadState hidden=" << hidden << endl;
}

int KToolBar::dockWindowIndex() const
{
    Q_ASSERT( mainWindow() );
    return mainWindow()->layout()->indexOf(const_cast<KToolBar*>(this));
}

void KToolBar::getAttributes( QString &position, Qt::ToolButtonStyle &toolButtonStyle, int &index ) const
{
    // get all of the stuff to save
    switch ( mainWindow()->toolBarArea(const_cast<KToolBar*>(this)) ) {
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

    toolButtonStyle = KToolBar::toolButtonStyle();

    index = dockWindowIndex();
}

void KToolBar::saveState( QDomElement &current ) const
{
    Q_ASSERT( !current.isNull() );
    QString position;
    Qt::ToolButtonStyle ToolButtonStyle;
    int index = -1;
    getAttributes( position, ToolButtonStyle, index );

    current.setAttribute( "noMerge", "1" );
    current.setAttribute( "position", position );
    current.setAttribute( "toolButtonStyle", toolButtonStyleToString(ToolButtonStyle) );
    current.setAttribute( "index", index );
    // FIXME KAction port
    //current.setAttribute( "offset", offset() );
    //current.setAttribute( "newline", newLine() );
    if ( isHidden() )
        current.setAttribute( "hidden", "true" );
    d->modified = true;

    // TODO if this method is used by more than KXMLGUIBuilder, e.g. to save XML settings to *disk*,
    // then the stuff below shouldn't always be done.
    current.setAttribute( "offsetDefault", d->OffsetDefault );
    current.setAttribute( "newlineDefault", d->NewLineDefault );
    current.setAttribute( "hiddenDefault", d->HiddenDefault ? "true" : "false" );
    current.setAttribute( "iconSizeDefault", d->IconSizeDefault );
    current.setAttribute( "positionDefault", d->PositionDefault );
    current.setAttribute( "toolButtonStyleDefault", toolButtonStyleToString(d->ToolButtonStyleDefault) );

    //kDebug(220) << name() << " saveState: saving index=" << index << " ToolButtonStyle=" << ToolButtonStyle << " hidden=" << isHidden() << endl;
}

KMenu *KToolBar::contextMenu()
{
  if (!d->context) {
    d->context = new KMenu( this );
    d->context->addTitle(i18n("Toolbar Menu"));
  
    d->contextOrient = new KMenu( i18n("Orientation"), d->context );
    d->context->addMenu( d->contextOrient );
  
    d->contextTop = d->contextOrient->addAction( i18nc("toolbar position string","Top"), this, SLOT(slotContextTop()) );
    d->contextTop->setChecked(true);
    d->contextLeft = d->contextOrient->addAction( i18nc("toolbar position string","Left"), this , SLOT(slotContextLeft()) );
    d->contextRight = d->contextOrient->addAction( i18nc("toolbar position string","Right"), this, SLOT(slotContextRight()) );
    d->contextBottom = d->contextOrient->addAction( i18nc("toolbar position string","Bottom"), this, SLOT(slotContextBottom()) );
  
    QActionGroup* positionGroup = new QActionGroup(d->contextOrient);
    foreach (QAction* action, d->contextOrient->actions()) {
      action->setActionGroup(positionGroup);
      action->setCheckable(true);
    }
  
    d->contextMode = new KMenu( i18n("Text Position"), d->context );
    d->context->addMenu( d->contextMode );
  
    d->contextIcons = d->contextMode->addAction( i18n("Icons Only"), this, SLOT(slotContextIcons()) );
    d->contextIcons->setChecked(true);
    d->contextText = d->contextMode->addAction( i18n("Text Only"), this, SLOT(slotContextText()) );
    d->contextTextRight = d->contextMode->addAction( i18n("Text Alongside Icons"), this, SLOT(slotContextTextRight()) );
    d->contextTextUnder = d->contextMode->addAction( i18n("Text Under Icons"), this, SLOT(slotContextTextUnder()) );
  
    QActionGroup* textGroup = new QActionGroup(d->contextMode);
    foreach (QAction* action, d->contextMode->actions()) {
      action->setActionGroup(textGroup);
      action->setCheckable(true);
    }
  
    d->contextSize = new KMenu( i18n("Icon Size"), d->context );
    d->context->addMenu( d->contextSize );
  
    d->contextIconSizes.insert(d->contextSize->addAction( i18n("Default"), this, SLOT(slotContextIconSize())), 0);
  
    // Query the current theme for available sizes
    KIconTheme *theme = KGlobal::instance()->iconLoader()->theme();
    QList<int> avSizes;
    if (theme)
    {
        if (isMainToolBar())
            avSizes = theme->querySizes( K3Icon::MainToolbar);
        else
            avSizes = theme->querySizes( K3Icon::Toolbar);
    }
  
    d->iconSizes = avSizes;
    qSort(avSizes);
  
    if (avSizes.count() < 10) {
        // Fixed or threshold type icons
        foreach ( int it, avSizes ) {
            QString text;
            if ( it < 19 )
                text = i18n("Small (%1x%2)", it, it);
            else if (it < 25)
                text = i18n("Medium (%1x%2)", it, it);
            else if (it < 35)
                text = i18n("Large (%1x%2)", it, it);
            else
                text = i18n("Huge (%1x%2)", it, it);
            // save the size in the contextIconSizes map
            d->contextIconSizes.insert(d->contextSize->addAction( text, this, SLOT(slotContextIconSize())), it );
        }
    }
    else {
        // Scalable icons.
        const int progression[] = {16, 22, 32, 48, 64, 96, 128, 192, 256};
  
        for (uint i = 0; i < 9; i++) {
            foreach ( int it, avSizes ) {
                if (it >= progression[i]) {
                    QString text;
                    if ( it < 19 )
                        text = i18n("Small (%1x%2)", it, it);
                    else if (it < 25)
                        text = i18n("Medium (%1x%2)", it, it);
                    else if (it < 35)
                        text = i18n("Large (%1x%2)", it, it);
                    else
                        text = i18n("Huge (%1x%2)", it, it);
                    // save the size in the contextIconSizes map
                    d->contextIconSizes.insert(d->contextSize->addAction( text, this, SLOT(slotContextIconSize())), it );
                    break;
                }
            }
        }
    }
  
    QActionGroup* sizeGroup = new QActionGroup(d->contextSize);
    foreach (QAction* action, d->contextSize->actions()) {
      action->setActionGroup(sizeGroup);
      action->setCheckable(true);
    }
  
    if (!toolBarsLocked() && !isMovable())
      d->unlockedMovable = false;

    delete d->contextLockAction;
    d->contextLockAction = new KToggleAction(KIcon("lock"), i18n("Lock Toolbars"), 0L, 0L);
    d->context->addAction(d->contextLockAction);
    d->contextLockAction->setChecked(toolBarsLocked());
    d->contextLockAction->setCheckedState(KGuiItem(i18n("Unlock Toolbars"), KIcon("unlock")));
    connect(d->contextLockAction, SIGNAL(toggled(bool)), SLOT(slotLockToolBars(bool)));

    connect( d->context, SIGNAL( aboutToShow() ), this, SLOT( slotContextAboutToShow() ) );
  }

  d->contextOrient->menuAction()->setVisible(!toolBarsLocked());
  d->contextMode->menuAction()->setVisible(!toolBarsLocked());
  d->contextSize->menuAction()->setVisible(!toolBarsLocked());

  // Unplugging a submenu from abouttohide leads to the popupmenu floating around
  // So better simply call that code from after exec() returns (DF)
  //connect( context, SIGNAL( aboutToHide() ), this, SLOT( slotContextAboutToHide() ) );
  return d->context;
}

void KToolBar::slotContextAboutToShow()
{
  // The idea here is to reuse the "static" part of the menu to save time.
  // But the "Toolbars" action is dynamic (can be a single action or a submenu)
  // and ToolBarHandler::setupActions() deletes it, so better not keep it around.
  // So we currently plug/unplug the last two actions of the menu.
  // Another way would be to keep around the actions and plug them all into a (new each time) popupmenu.
  KMainWindow *kmw = qobject_cast<KMainWindow *>(mainWindow());
  if ( kmw ) {
      kmw->setupToolbarMenuActions();
      // Only allow hiding a toolbar if the action is also plugged somewhere else (e.g. menubar)
      KAction *tbAction = kmw->toolBarMenuAction();
      if ( !toolBarsLocked() && tbAction && tbAction->associatedWidgets().count() > 0 )
          contextMenu()->addAction(tbAction);
  }

  // try to find "configure toolbars" action
  KAction *configureAction = 0;
  const char* actionName = KStdAction::name(KStdAction::ConfigureToolbars);
  if ( d->m_xmlguiClient )
    configureAction = d->m_xmlguiClient->actionCollection()->action(actionName);
  if ( !configureAction && kmw )
    configureAction = kmw->actionCollection()->action(actionName);
  if ( configureAction )
    d->context->addAction(configureAction);
  KEditToolbar::setDefaultToolbar(QObject::objectName().toLatin1().constData());

  // Check the actions that should be checked
  switch( toolButtonStyle() )
  {
        case Qt::ToolButtonIconOnly:
        default:
            d->contextIcons->setChecked(true);
            break;
        case Qt::ToolButtonTextBesideIcon:
            d->contextTextRight->setChecked(true);
            break;
        case Qt::ToolButtonTextOnly:
            d->contextText->setChecked(true);
            break;
        case Qt::ToolButtonTextUnderIcon:
            d->contextTextUnder->setChecked(true);
            break;
  }

  QMapIterator<QAction*,int> it = d->contextIconSizes;
  while (it.hasNext()) {
    it.next();
    if (it.value() == iconSize().width()) {
      it.key()->setChecked(true);
      break;
    }
  }

  switch ( mainWindow()->toolBarArea(this) )
  {
  case Qt::BottomToolBarArea:
      d->contextBottom->setChecked( true );
      break;
  case Qt::LeftToolBarArea:
      d->contextLeft->setChecked( true );
      break;
  case Qt::RightToolBarArea:
      d->contextRight->setChecked( true );
      break;
  default:
  case Qt::TopToolBarArea:
      d->contextTop->setChecked( true );
      break;
  }
}

void KToolBar::slotContextAboutToHide()
{
  // We have to unplug whatever slotContextAboutToShow plugged into the menu.
  // Unplug the toolbar menu action
  KMainWindow *kmw = qobject_cast<KMainWindow *>(mainWindow());
  if ( kmw && kmw->toolBarMenuAction() )
    if ( kmw->toolBarMenuAction()->associatedWidgets().count() > 1 )
      contextMenu()->removeAction(kmw->toolBarMenuAction());

  // Unplug the configure toolbars action too, since it's afterwards anyway
  KAction *configureAction = 0;
  const char* actionName = KStdAction::name(KStdAction::ConfigureToolbars);
  if ( d->m_xmlguiClient )
    configureAction = d->m_xmlguiClient->actionCollection()->action(actionName);
  if ( !configureAction && kmw )
    configureAction = kmw->actionCollection()->action(actionName);
  if ( configureAction )
    d->context->removeAction(configureAction);
}

void KToolBar::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KToolBar::slotContextLeft( )
{
    mainWindow()->addToolBar( Qt::LeftToolBarArea, this );
}

void KToolBar::slotContextRight( )
{
    mainWindow()->addToolBar( Qt::RightToolBarArea, this );
}

void KToolBar::slotContextTop( )
{
    mainWindow()->addToolBar( Qt::TopToolBarArea, this );
}

void KToolBar::slotContextBottom( )
{
    mainWindow()->addToolBar( Qt::BottomToolBarArea, this );
}

void KToolBar::slotContextIcons( )
{
    setToolButtonStyle( Qt::ToolButtonIconOnly );
}

void KToolBar::slotContextText( )
{
    setToolButtonStyle( Qt::ToolButtonTextOnly );
}

void KToolBar::slotContextTextUnder( )
{
    setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
}

void KToolBar::slotContextTextRight( )
{
    setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
}

void KToolBar::slotContextIconSize( )
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action && d->contextIconSizes.contains(action)) {
        setIconDimensions(d->contextIconSizes.value(action));
    }
}

void KToolBar::slotReadConfig()
{
    //kdDebug(220) << name() << " slotReadConfig" << endl;
    // Read appearance settings (hmm, we used to do both here,
    // but a well behaved application will call applyMainWindowSettings
    // anyway, right ?)
    applyAppearanceSettings(KGlobal::config(), QString::null );
}

void KToolBar::slotAppearanceChanged()
{
    // Read appearance settings from global file.
    applyAppearanceSettings(KGlobal::config(), QString::null, true /* lose local settings */ );

    // And remember to save the new look later
    KMainWindow *kmw = qobject_cast<KMainWindow *>(mainWindow());
    if ( kmw )
        kmw->setSettingsDirty();
}

void KToolBar::applySettings(KConfig *config, const QString &_configGroup, bool force)
{
    //kdDebug(220) << name() << " applySettings group=" << _configGroup << endl;

    QString configGroup = _configGroup.isEmpty() ? settingsGroup() : _configGroup;

    /*
      Let's explain this a bit more in details.
      The order in which we apply settings is :
       Global config / <appnamerc> user settings                        if no XMLGUI is used
       Global config / App-XML attributes / <appnamerc> user settings   if XMLGUI is used

      So in the first case, we simply read everything from KConfig as below,
      but in the second case we don't do anything here if there is no app-specific config,
      and the XMLGUI-related code (loadState()) uses the static methods of this class
      to get the global defaults.

      Global config doesn't include position (index, offset, newline and hidden/shown).
    */

    // First the appearance stuff - the one which has a global config
    applyAppearanceSettings( config, configGroup );

    // ...and now the position stuff
    if ( config->hasGroup(configGroup) || force )
    {
        KConfigGroup cg(config, configGroup);

        static const QString &attrPosition  = KGlobal::staticQString("Position");
        static const QString &attrIndex  = KGlobal::staticQString("Index");
        static const QString &attrOffset  = KGlobal::staticQString("Offset");
        static const QString &attrNewLine  = KGlobal::staticQString("NewLine");
        static const QString &attrHidden  = KGlobal::staticQString("Hidden");

        QString position = cg.readEntry(attrPosition, d->PositionDefault);
        int index = cg.readEntry(attrIndex, int(-1));
        int offset = cg.readEntry(attrOffset, int(d->OffsetDefault));
        bool newLine = cg.readEntry(attrNewLine, d->NewLineDefault);
        bool hidden = cg.readEntry(attrHidden, d->HiddenDefault);

        Qt::ToolBarArea pos = Qt::TopToolBarArea;
        if ( position == "Top" )
            pos = Qt::TopToolBarArea;
        else if ( position == "Bottom" )
            pos = Qt::BottomToolBarArea;
        else if ( position == "Left" )
            pos = Qt::LeftToolBarArea;
        else if ( position == "Right" )
            pos = Qt::RightToolBarArea;

        //kdDebug(220) << name() << " applySettings hidden=" << hidden << endl;
        if (hidden)
            hide();
        else
            show();

        if ( mainWindow() )
        {
            //kdDebug(220) << name() << " applySettings updating ToolbarInfo" << endl;
            d->toolBarInfo = KToolBarPrivate::ToolBarInfo( pos, index, newLine, offset );
        }
    }
}

void KToolBar::applyAppearanceSettings(KConfig *config, const QString &_configGroup, bool forceGlobal)
{
    QString configGroup = _configGroup.isEmpty() ? settingsGroup() : _configGroup;
    //kdDebug(220) << name() << " applyAppearanceSettings: configGroup=" << configGroup << " forceGlobal=" << forceGlobal << endl;

    // If we have application-specific settings in the XML file,
    // and nothing in the application's config file, then
    // we don't apply the global defaults, the XML ones are preferred
    // (see applySettings for a full explanation)
    // This is the reason for the xmlgui tests below.
    bool xmlgui = d->m_xmlguiClient && !d->m_xmlguiClient->xmlFile().isEmpty();

    KConfig *gconfig = KGlobal::config();

    static const QString &attrToolButtonStyle  = KGlobal::staticQString("ToolButtonStyle");
    static const QString &attrIconSize  = KGlobal::staticQString("IconSize");

    // we actually do this in two steps.
    // First, we read in the global styles [Toolbar style] (from the KControl module).
    // Then, if the toolbar is NOT 'mainToolBar', we will also try to read in [barname Toolbar style]
    bool applyToolButtonStyle = !xmlgui; // if xmlgui is used, global defaults won't apply
    bool applyIconSize = !xmlgui;

    int iconSize = d->IconSizeDefault;
    Qt::ToolButtonStyle ToolButtonStyle = d->ToolButtonStyleDefault;

    // this is the first iteration
    QString grpToolbar(QLatin1String("Toolbar style"));
    { // start block for KConfigGroup
        KConfigGroup cg(gconfig, grpToolbar);

        // we read in the ToolButtonStyle property *only* if we intend on actually
        // honoring it
        if (d->honorStyle)
            d->ToolButtonStyleDefault = toolButtonStyleFromString(cg.readEntry(attrToolButtonStyle, toolButtonStyleToString(d->ToolButtonStyleDefault)));
        else
            d->ToolButtonStyleDefault = Qt::ToolButtonTextUnderIcon;

        // Use the default icon size for toolbar icons.
        d->IconSizeDefault = cg.readEntry(attrIconSize, int(d->IconSizeDefault));

        iconSize = d->IconSizeDefault;
        ToolButtonStyle = d->ToolButtonStyleDefault;

        if ( !forceGlobal && config->hasGroup(configGroup) )
        {
            config->setGroup(configGroup);

            // read in the ToolButtonStyle property
            if ( config->hasKey( attrToolButtonStyle ) ) {
                ToolButtonStyle = toolButtonStyleFromString(config->readEntry(attrToolButtonStyle, QString()));
                applyToolButtonStyle = true;
                //kdDebug(220) << name() << " read ToolButtonStyle=" << d->ToolButtonStyleDefault << ", that will be the default" << endl;
            }

            // now get the size
            if ( config->hasKey( attrIconSize ) ) {
                iconSize = config->readEntry(attrIconSize, 0);
                applyIconSize = true;
            }
        }

        // revert back to the old group
    } // end block for KConfigGroup

    // check if the icon/text has changed
    if (ToolButtonStyle != toolButtonStyle() && applyToolButtonStyle) {
        //kdDebug(220) << name() << " applyAppearanceSettings setToolButtonStyle " << tool_button_style << endl;
        setToolButtonStyle(ToolButtonStyle);
    }

    // ...and check if the icon size has changed
    if (iconSize != KToolBar::iconSize().width() && applyIconSize) {
        setIconDimensions(iconSize);
    }
}

KMainWindow * KToolBar::mainWindow( ) const
{
  return qobject_cast<KMainWindow*>(const_cast<QObject*>(parent()));
}

int KToolBar::iconSizeDefault() const
{
    if (QObject::objectName() ==  "mainToolBar")
        return KGlobal::iconLoader()->currentSize(K3Icon::MainToolbar);

    return KGlobal::iconLoader()->currentSize(K3Icon::Toolbar);
}

void KToolBar::slotMovableChanged( bool movable )
{
    if (movable && !KAuthorized::authorize("movable_toolbars"))
        setMovable(false);
}

bool KToolBar::isMainToolBar( ) const
{
  return objectName() == QLatin1String("mainToolBar");
}

void KToolBar::dragEnterEvent( QDragEnterEvent * event )
{
  if (toolBarsEditable() && event->proposedAction() & (Qt::CopyAction | Qt::MoveAction) && event->mimeData()->hasFormat("application/x-kde-action-list")) {
    QByteArray data = event->mimeData()->data("application/x-kde-action-list");

    QDataStream stream(data);

    QStringList actionNames;

    stream >> actionNames;

    foreach (const QString& actionName, actionNames) {
      foreach (KActionCollection* ac, KActionCollection::allCollections()) {
        KAction* newAction = ac->action(actionName.toAscii().constData());
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

void KToolBar::dragMoveEvent( QDragMoveEvent * event )
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
            if (actions()[dropIndicatorIndex + 1] == overAction)
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

void KToolBar::dragLeaveEvent( QDragLeaveEvent * event )
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

void KToolBar::dropEvent( QDropEvent * event )
{
  if (toolBarsEditable()) {
    foreach (KAction* action, d->actionsBeingDragged) {
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

void KToolBar::mousePressEvent( QMouseEvent * event )
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

void KToolBar::mouseMoveEvent( QMouseEvent * event )
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

void KToolBar::mouseReleaseEvent( QMouseEvent* event )
{
  // Want to clear this even if toolBarsEditable was changed mid-drag (unlikey)
  if (d->dragAction) {
    d->dragAction = 0L;
    event->accept();
    return;
  }

  QToolBar::mouseReleaseEvent(event);
}

Qt::ToolButtonStyle KToolBar::toolButtonStyleFromString( const QString & style )
{
  QString style2 = style.toLower();
  if ( style2 == "textbesideicon" || style2 == "icontextright" )
    return Qt::ToolButtonTextBesideIcon;
  else if ( style2 == "textundericon" || style2 == "icontextbottom" )
    return Qt::ToolButtonTextUnderIcon;
  else if ( style2 == "textonly" )
    return Qt::ToolButtonTextOnly;
  else
    return Qt::ToolButtonIconOnly;
}

QString KToolBar::toolButtonStyleToString( Qt::ToolButtonStyle style )
{
  switch( style )
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

bool KToolBar::eventFilter( QObject * watched, QEvent * event )
{
  // Generate context menu events for disabled buttons too...
  if (event->type() == QEvent::MouseButtonPress) {
    QMouseEvent* me = static_cast<QMouseEvent*>(event);
    if (me->buttons() & Qt::RightButton)
      if (QWidget* ww = qobject_cast<QWidget*>(watched))
        if (ww->parent() == this)
          if (!ww->isEnabled())
            QCoreApplication::postEvent(this, new QContextMenuEvent(QContextMenuEvent::Mouse, me->pos(), me->globalPos()));

  } else if (event->type() == QEvent::ParentChange) {
    // Make sure we're not leaving stale event filters around
    if (QWidget* ww = qobject_cast<QWidget*>(watched)) {
      while (ww) {
        if (ww == this)
          goto found;
      }
      // New parent is not a subwidget - remove event filter
      ww->removeEventFilter(this);
      foreach (QWidget* child, ww->findChildren<QWidget*>())
        child->removeEventFilter(this);
    }
  }

  found:

  // Redirect mouse events to the toolbar when drag + drop editing is enabled
  if (toolBarsEditable()) {
    if (QWidget* ww = qobject_cast<QWidget*>(watched)) {
      switch (event->type()) {
        case QEvent::MouseButtonPress: {
          QMouseEvent* me = static_cast<QMouseEvent*>(event);
          QMouseEvent newEvent(me->type(), mapFromGlobal(ww->mapToGlobal(me->pos())), me->globalPos(), me->button(), me->buttons(), me->modifiers());
          mousePressEvent(&newEvent);
          return true;
        }
        case QEvent::MouseMove: {
          QMouseEvent* me = static_cast<QMouseEvent*>(event);
          QMouseEvent newEvent(me->type(), mapFromGlobal(ww->mapToGlobal(me->pos())), me->globalPos(), me->button(), me->buttons(), me->modifiers());
          mouseMoveEvent(&newEvent);
          return true;
        }
        case QEvent::MouseButtonRelease: {
          QMouseEvent* me = static_cast<QMouseEvent*>(event);
          QMouseEvent newEvent(me->type(), mapFromGlobal(ww->mapToGlobal(me->pos())), me->globalPos(), me->button(), me->buttons(), me->modifiers());
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

void KToolBar::actionEvent( QActionEvent * event )
{
  if (event->type() == QEvent::ActionRemoved) {
    QWidget* widget = widgetForAction(event->action());
    widget->removeEventFilter(this);
    foreach (QWidget* child, widget->findChildren<QWidget*>())
      child->removeEventFilter(this);
  }

  QToolBar::actionEvent(event);

  if (event->type() == QEvent::ActionAdded) {
    QWidget* widget = widgetForAction(event->action());
    widget->installEventFilter(this);
    foreach (QWidget* child, widget->findChildren<QWidget*>())
      child->installEventFilter(this);
  }

  adjustSeparatorVisibility();
}

bool KToolBar::toolBarsEditable( )
{
  return KToolBarPrivate::s_editable;
}

void KToolBar::setToolBarsEditable( bool editable )
{
  if (KToolBarPrivate::s_editable != editable) {
    KToolBarPrivate::s_editable = editable;
  }
}

void KToolBar::setLocked( bool locked )
{
  if (d->unlockedMovable)
    setMovable(!locked);
}

void KToolBar::slotLockToolBars( bool lock )
{
  setToolBarsLocked(lock);
}

void KToolBar::setToolBarsLocked( bool locked )
{
  if (KToolBarPrivate::s_locked != locked) {
    KToolBarPrivate::s_locked = locked;

    foreach (KMainWindow* mw, KMainWindow::memberList())
      foreach (KToolBar* toolbar, mw->findChildren<KToolBar*>())
        toolbar->setLocked(locked);
  }
}

bool KToolBar::toolBarsLocked( )
{
  return KToolBarPrivate::s_locked;
}

void KToolBar::adjustSeparatorVisibility()
{
  bool visibleNonSeparator = false;
  int separatorToShow = -1;
  for (int index = 0; index < actions().count(); ++index) {
    QAction* action = actions()[index];
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
          actions()[separatorToShow]->setVisible(true);
          separatorToShow = -1;
        }
      }
    }
  }

  if (separatorToShow != -1)
    actions()[separatorToShow]->setVisible(false);
}

#include "ktoolbar.moc"

