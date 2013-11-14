if (QT_QT3SUPPORT_FOUND)
install( FILES
  K3BookmarkDrag
  K3ButtonBox
  K3ColorDrag
  K3DictSpellingHighlighter
  K3DockArea
  K3DockMainWindow
  K3DockManager
  K3DockTabGroup
  K3DockWidget
  K3DockWidgetAbstractHeader
  K3DockWidgetAbstractHeaderDrag
  K3DockWidgetHeader
  K3DockWidgetHeaderDrag
  K3Icon
  K3IconView
  K3IconViewItem
  K3IconViewSearchLine
  K3ListBox
  K3ListView
  K3ListViewItem
  K3ListViewSearchLine
  K3ListViewSearchLineWidget
  K3MimeSourceFactory
  K3MultipleDrag
  K3PopupMenu
  K3Spell
  K3SpellConfig
  K3SpellDlg
  K3SpellingHighlighter
  K3StaticDeleter
  K3StaticDeleterBase
  K3SyntaxHighlighter
  K3TempFile
  K3TextEdit
  K3URLDrag
  K3Wizard
  K3Command
  K3CommandHistory
  K3AboutApplication
  K3AboutContainer
  K3AboutContributor
  K3AboutDialog
  K3AboutWidget
  K3ActiveLabel
  K3FileTreeView
  K3FileTreeViewItem
  K3MacroCommand
  K3NamedCommand
  K3RFCDate
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE COMPONENT Devel)
endif ()

install( FILES
  KACL
  KAboutApplicationDialog
  KAboutData
  KAboutPerson
  KAcceleratorManager
  KAction
  KActionCategory
  KActionCollection
  KActionMenu
  KActionSelector
  KAnimatedButton
  KApplication
  KAr
  KArchive
  KArchiveDirectory
  KArchiveEntry
  KArchiveFile
  KAscii
  KAssistantDialog
  KAuthorized
  KAutoMount
  KAutoUnmount
  KAutoSaveFile
  KAutostart
  KBookmark
  KBookmarkAction
  KBookmarkActionInterface
  KBookmarkActionMenu
  KBookmarkDialog
  KBookmarkDomBuilder
  KBookmarkExporterBase
  KBookmarkGroup
  KBookmarkGroupTraverser
  KBookmarkImporterBase
  KBookmarkManager
  KBookmarkMenu
  KBookmarkOwner
  KBreadcrumbSelectionModel
  KBugReport
  KBuildSycocaProgressDialog
  KButtonGroup
  KCapacityBar
  KCModule
  KCModuleContainer
  KCModuleInfo
  KCModuleLoader
  KCModuleProxy
  KCMultiDialog
  KIdleTime
  KCalendarSystem
  KCalendarSystemFactory
  KCharMacroExpander
  KCharSelect
  KCharsets
  KCheckableProxyModel
  KCmdLineArgs
  KCmdLineOptions
  KCodecAction
  KCodecs
  KColor
  KColorButton
  KColorCells
  KColorCollection
  KColorCombo
  KColorDialog
  KColorMimeData
  KColorPatch
  KColorScheme
  KColorTable
  KColorUtils
  KColorValueSelector
  KComboBox
  KCompletion
  KCompletionBase
  KCompletionBox
  KCompletionMatches
  KCompositeJob
  KConfig
  KConfigBase
  KConfigDialog
  KConfigDialogManager
  KConfigGroup
  KConfigSkeleton
  KConfigSkeletonGenericItem
  KConfigSkeletonItem
  KSharedConfigPtr
  KCrash
  KCrashBookmarkImporter
  KCrashBookmarkImporterImpl
  KCurrencyCode
  KCursor
  KDBusServiceStarter
  KDebug
  KDeviceListModel
  KDEDModule
  KDEPrintDialog
  KDataTool
  KDataToolAction
  KDataToolInfo
  KDateComboBox
  KDatePicker
  KDateTable
  KDateTime
  KDateTimeEdit
  KDateTimeWidget
  KDateValidator
  KDateWidget
  KDescendantsProxyModel
  KDesktopFile
  KDialog
  KDialogButtonBox
  KDialogJobUiDelegate
  KDirLister
  KDirModel
  KDirNotify
  KDirOperator
  KDirSortFilterProxyModel
  KDirSelectDialog
  KDirWatch
  KDualAction
  KFilePreviewGenerator
  KFilePlacesView
  KFileWidget
  KFileWritePlugin
  KDiskFreeSpaceInfo
  KDoubleNumInput
  KDoubleValidator
  KEMailSettings
  KEditListWidget
  KEditToolBar
  KEncodingFileDialog
  KEncodingProber
  KExtendableItemDelegate
  KFadeWidgetEffect
  KFile
  KFileDialog
  KFileFilterCombo
  KFileItem
  KFileItemActions
  KFileItemList
  KFileItemListProperties
  KFileItemDelegate
  KFileMetaDataWidget
  KFileMetaInfo
  KFileMetaInfoGroup
  KFileMetaInfoItem
  KFilePlacesModel
  KFileShare
  KFileSharePropsPlugin
  KFileTreeBranch
  KFileTreeView
  KFilterBase
  KFilterDev
  KFind
  KFindDialog
  KFontAction
  KFontChooser
  KFontComboBox
  KFontDialog
  KFontRequester
  KFontSizeAction
  KFontUtils
  KGenericFactory
  KGenericFactoryBase
  KGlobal
  KGlobalAccel
  KGlobalSettings
  KGlobalShortcutInfo
  KGradientSelector
  KGuiItem
  KHBox
  KHTMLPart
  KHTMLSettings
  KHTMLView
  KHelpMenu
  KHistoryComboBox
  KHueSaturationSelector
  KIEBookmarkExporterImpl
  KIEBookmarkImporter
  KIEBookmarkImporterImpl
  KIMProxy
  KIcon
  KIconButton
  KIconDialog
  KIconEffect
  KIconLoader
  KIconTheme
  KIdentityProxyModel
  KImageCache
  KImageFilePreview
  KImageIO
  KInputDialog
  KComponentData
  KIntNumInput
  KIntSpinBox
  KIntValidator
  KJob
  KJobTrackerInterface
  KJobUiDelegate
  KKeySequenceWidget
  KLanguageButton
  KLed
  KLibLoader
  KLibrary
  KLineEdit
  KLinkItemSelectionModel
  KListWidget
  KListWidgetSearchLine
  KLocale
  KLocalizedDate
  KLocalizedString
  KLockFile
  KMD5
  KMacroExpanderBase
  KMainWindow
  KMakeTypeList
  KMenu
  KMenuBar
  KMessage
  KMessageBox
  KMessageBoxMessageHandler
  KMessageHandler
  KMessageWidget
  KMimeType
  KMimeTypeChooser
  KMimeTypeChooserDialog
  KMimeTypeTrader
  KModelIndexProxyMapper
  KMozillaBookmarkImporterImpl
  KModifierKeyInfo
  KMountPoint
  KMultiTabBar
  KMultiTabBarButton
  KMultiTabBarTab
  KNameAndUrlInputDialog
  KNFSShare
  KNSBookmarkExporter
  KNSBookmarkExporterImpl
  KNSBookmarkImporter
  KNSBookmarkImporterImpl
  KNTLM
  KNewFileMenu
  KNewPasswordDialog
  KNotification
  KNotificationRestrictions
  KNotifyConfigWidget
  KNumInput
  KOCRDialog
  KOpenWithDialog
  KOperaBookmarkExporterImpl
  KOperaBookmarkImporter
  KOperaBookmarkImporterImpl
  KPageDialog
  KPageModel
  KPageView
  KPageWidget
  KPageWidgetItem
  KPageWidgetModel
  KPassivePopup
  KPassivePopupMessageHandler
  KPasswordDialog
  KPasteTextAction
  KPixmapCache
  KPixmapProvider
  KPixmapRegionSelectorDialog
  KPixmapRegionSelectorWidget
  KPlotAxis
  KPlotObject
  KPlotPoint
  KPlotWidget
  KPluginFactory
  KPluginInfo
  KPluginLoader
  KPluginSelector
  KPopupFrame
  KPreviewWidgetBase
  KPrintPreview
  KProcess
  KProgressDialog
  KPropertiesDialog
  KProtocolInfo
  KProtocolManager
  KPushButton
  KRandom
  KRandomSequence
  KRatingPainter
  KRatingWidget
  KRecentDocument
  KRecentFilesAction
  KRecursiveFilterProxyModel
  KRegExpEditorInterface
  KRemoteEncoding
  KReplace
  KReplaceDialog
  KRestrictedLine
  KRichTextEdit
  KRichTextWidget
  KRuler
  KRun
  KSambaShare
  KSambaShareData
  KSaveFile
  KScanDialog
  KSelectAction
  KSelectionOwner
  KSelectionProxyModel
  KSelectionWatcher
  KSelector
  KSeparator
  KService
  KServiceAction
  KServiceGroup
  KServiceType
  KServiceTypeProfile
  KServiceTypeTrader
  KSessionManager
  KSharedConfig
  KSharedDataCache
  KSharedPtr
  KShell
  KShellCompletion
  KShortcut
  KShortcutsDialog
  KShortcutsEditor
  KShortcutWidget
  KSocks
  KSortableItem
  KSortableList
  KSpeech
  KSplashScreen
  KSqueezedTextLabel
  KStandardDirs
  KStandardAction
  KStandardGuiItem
  KStandardShortcut
  KStartupInfo
  KStartupInfoData
  KStartupInfoId
  KStatusBar
  KStatusBarJobTracker
  KStatusBarOfflineIndicator
  KStatusNotifierItem
  KStringHandler
  KStringListValidator
  KStyle
  KStyleFactory
  KSycoca
  KSycocaEntry
  KSystemEventFilter
  KSystemTimeZone
  KSystemTimeZoneSource
  KSystemTimeZones
  KSystemTrayIcon
  KTabBar
  KTabWidget
  KTar
  KTempDir
  KTemporaryFile
  KTextBrowser
  KTextEdit
  KTimeComboBox
  KTimeZone
  KTimeZoneData
  KTimeZoneSource
  KTimeZoneWidget
  KTimeZones
  KTipDatabase
  KTipDialog
  KTitleWidget
  KToggleAction
  KToggleFullScreenAction
  KToggleToolBarAction
  KToolBar
  KToolBarLabelAction
  KToolBarPopupAction
  KToolBarSpacerAction
  KToolInvocation
  KTreeWidgetSearchLine
  KTreeWidgetSearchLineWidget
  KTypeList
  KTypeListIndexOf
  KTypeListLength
  KTzfileTimeZone
  KTzfileTimeZoneSource
  KUndoActions
  KUniqueApplication
  KUriFilter
  KUriFilterData
  KUriFilterPlugin
  KUrl
  KUrlAuthorized
  KUrlComboBox
  KUrlComboRequester
  KUrlCompletion
  KUrlLabel
  KUrlNavigator
  KUrlPixmapProvider
  KUrlRequester
  KUrlRequesterDialog
  KUser
  KUserGroup
  KVBox
  KViewStateMaintainer
  KViewStateSaver
  KWindowInfo
  KWindowSystem
  KWordMacroExpander
  KWordWrap
  KXBELBookmarkImporterImpl
  KXErrorHandler
  KXMLGUIBuilder
  KXMLGUIClient
  KXMLGUIFactory
  KXmlGuiWindow
  KXMessages
  KXYSelector
  KZip
  KZipFileEntry
  KonqBookmarkMenu
  KonqBookmarkOwner
  NET
  NETRootInfo
  NETWinInfo
  OrgKdeKDirNotifyInterface
  ThumbCreator
  kdbgstream
  kndbgstream
  KCategorizedView
  KCategoryDrawer
  KCategorizedSortFilterProxyModel
  KEmoticons
  KEmoticonsProvider
  KEmoticonsTheme
  KFilterProxySearchLine
  KWidgetItemDelegate
  KWidgetJobTracker
  KPixmapSequence
  KPixmapSequenceWidget
  KPixmapSequenceOverlayPainter
  KGraphicsWebView
  KWebPage
  KWebView
  KWebPluginFactory
  KWebWallet
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE COMPONENT Devel)

if(NOT KDE_NO_DEPRECATED)
install( FILES
  KArrowButton
  KEditListBox
  KDiskFreeSpace
  KMimeTypeResolver
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE COMPONENT Devel)
endif()

if (UNIX)
   if (QT_QT3SUPPORT_FOUND)
      install( FILES
        K3ProcIO
        K3Process
        K3ProcessController
        K3ShellProcess
      DESTINATION ${INCLUDE_INSTALL_DIR}/KDE COMPONENT Devel)
   endif ()

   install( FILES
     KDEsuClient
     KPty
     KPtyDevice
     KPtyProcess
     PtyProcess
     SshProcess
     StubProcess
     SuProcess
   DESTINATION ${INCLUDE_INSTALL_DIR}/KDE COMPONENT Devel)
endif ()

install( FILES
  ConversionCheck/QVconvertible
  ConversionCheck/type_toQString
  ConversionCheck/type_toQVariant
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/ConversionCheck COMPONENT Devel)


install( FILES
  DNSSD/Configuration
  DNSSD/DomainBrowser
  DNSSD/DomainModel
  DNSSD/PublicService
  DNSSD/RemoteService
  DNSSD/ServiceBase
  DNSSD/ServiceBrowser
  DNSSD/ServiceModel
  DNSSD/ServiceTypeBrowser
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/DNSSD COMPONENT Devel)


install( FILES
  DOM/AbstractView
  DOM/Attr
  DOM/CDATASection
  DOM/CSSCharsetRule
  DOM/CSSException
  DOM/CSSFontFaceRule
  DOM/CSSImportRule
  DOM/CSSMediaRule
  DOM/CSSPageRule
  DOM/CSSPrimitiveValue
  DOM/CSSRule
  DOM/CSSRuleList
  DOM/CSSStyleDeclaration
  DOM/CSSStyleRule
  DOM/CSSStyleSheet
  DOM/CSSUnknownRule
  DOM/CSSValue
  DOM/CSSValueList
  DOM/CharacterData
  DOM/Comment
  DOM/Counter
  DOM/CustomNodeFilter
  DOM/DOMException
  DOM/DOMImplementation
  DOM/DOMString
  DOM/Document
  DOM/DocumentFragment
  DOM/DocumentStyle
  DOM/DocumentType
  DOM/DomShared
  DOM/Element
  DOM/Entity
  DOM/EntityReference
  DOM/Event
  DOM/EventException
  DOM/EventListener
  DOM/HTMLAnchorElement
  DOM/HTMLAppletElement
  DOM/HTMLAreaElement
  DOM/HTMLBRElement
  DOM/HTMLBaseElement
  DOM/HTMLBaseFontElement
  DOM/HTMLBlockquoteElement
  DOM/HTMLBodyElement
  DOM/HTMLButtonElement
  DOM/HTMLCollection
  DOM/HTMLDListElement
  DOM/HTMLDirectoryElement
  DOM/HTMLDivElement
  DOM/HTMLDocument
  DOM/HTMLElement
  DOM/HTMLFieldSetElement
  DOM/HTMLFontElement
  DOM/HTMLFormCollection
  DOM/HTMLFormElement
  DOM/HTMLFrameElement
  DOM/HTMLFrameSetElement
  DOM/HTMLHRElement
  DOM/HTMLHeadElement
  DOM/HTMLHeadingElement
  DOM/HTMLHtmlElement
  DOM/HTMLIFrameElement
  DOM/HTMLImageElement
  DOM/HTMLInputElement
  DOM/HTMLIsIndexElement
  DOM/HTMLLIElement
  DOM/HTMLLabelElement
  DOM/HTMLLayerElement
  DOM/HTMLLegendElement
  DOM/HTMLLinkElement
  DOM/HTMLMapElement
  DOM/HTMLMenuElement
  DOM/HTMLMetaElement
  DOM/HTMLModElement
  DOM/HTMLOListElement
  DOM/HTMLObjectElement
  DOM/HTMLOptGroupElement
  DOM/HTMLOptionElement
  DOM/HTMLParagraphElement
  DOM/HTMLParamElement
  DOM/HTMLPreElement
  DOM/HTMLQuoteElement
  DOM/HTMLScriptElement
  DOM/HTMLSelectElement
  DOM/HTMLStyleElement
  DOM/HTMLTableCaptionElement
  DOM/HTMLTableCellElement
  DOM/HTMLTableColElement
  DOM/HTMLTableElement
  DOM/HTMLTableRowElement
  DOM/HTMLTableSectionElement
  DOM/HTMLTextAreaElement
  DOM/HTMLTitleElement
  DOM/HTMLUListElement
  DOM/KeyboardEvent
  DOM/LinkStyle
  DOM/MediaList
  DOM/MouseEvent
  DOM/MutationEvent
  DOM/NamedNodeMap
  DOM/Node
  DOM/NodeFilter
  DOM/NodeIterator
  DOM/NodeList
  DOM/Notation
  DOM/ProcessingInstruction
  DOM/RGBColor
  DOM/Range
  DOM/RangeException
  DOM/Rect
  DOM/StyleSheet
  DOM/StyleSheetList
  DOM/Text
  DOM/TextEvent
  DOM/TreeWalker
  DOM/UIEvent
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/DOM COMPONENT Devel)


install( FILES
  KAccelGen/Deref
  KAccelGen/Deref_Key
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/KAccelGen COMPONENT Devel)

install( FILES
  KIO/AccessManager
  KIO/AuthInfo
  KIO/ChmodJob
  KIO/Connection
  KIO/CopyInfo
  KIO/CopyJob
  KIO/DavJob
  KIO/DeleteJob
  KIO/FileCopyJob
  KIO/FileJob
  KIO/ForwardingSlaveBase
  KIO/Job
  KIO/JobClasses
  KIO/JobUiDelegate
  KIO/ListJob
  KIO/MetaData
  KIO/MetaInfoJob
  KIO/MimetypeJob
  KIO/MultiGetJob
  KIO/NetAccess
  KIO/NetRC
  KIO/PreviewJob
  KIO/RenameDialog
  KIO/Scheduler
  KIO/SessionData
  KIO/SimpleJob
  KIO/SkipDialog
  KIO/Slave
  KIO/SlaveBase
  KIO/SlaveConfig
  KIO/SlaveInterface
  KIO/StatJob
  KIO/StoredTransferJob
  KIO/TCPSlaveBase
  KIO/Task
  KIO/TransferJob
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/KIO COMPONENT Devel)

if(NOT KDE_NO_DEPRECATED)
install( FILES
  KIO/PasswordDialog
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/KIO COMPONENT Devel)
endif()


install( FILES
  KMediaPlayer/Player
  KMediaPlayer/View
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/KMediaPlayer COMPONENT Devel)


if(NOT KDE_NO_DEPRECATED)
install( FILES
  KNS/Author
  KNS/Category
  KNS/Engine
  KNS/Entry
  KNS/Installation
  KNS/KTranslatable
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/KNS COMPONENT Devel)
endif()

install( FILES
  KNS3/DownloadDialog
  KNS3/Entry
  KNS3/KNewStuffAction
  KNS3/KNewStuffButton
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/KNS3 COMPONENT Devel)

install( FILES
  KNetwork/KActiveSocketBase
  KNetwork/KBufferedSocket
  KNetwork/KClientSocketBase
  KNetwork/KInetSocketAddress
  KNetwork/KIpAddress
  KNetwork/KPassiveSocketBase
  KNetwork/KResolver
  KNetwork/KResolverEntry
  KNetwork/KResolverResults
  KNetwork/KReverseResolver
  KNetwork/KServerSocket
  KNetwork/KSocketAddress
  KNetwork/KSocketBase
  KNetwork/KSocketDevice
  KNetwork/KSocketDeviceFactory
  KNetwork/KSocksSocketDevice
  KNetwork/KStreamSocket
  KNetwork/KUnixSocketAddress
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/KNetwork COMPONENT Devel)

install( FILES
  KParts/BrowserExtension
  KParts/BrowserHostExtension
  KParts/BrowserInterface
  KParts/BrowserRun
  KParts/ComponentFactory
  KParts/DockMainWindow3
  KParts/Event
  KParts/Factory
  KParts/FileInfoExtension
  KParts/GUIActivateEvent
  KParts/GenericFactory
  KParts/GenericFactoryBase
  KParts/HistoryProvider
  KParts/HtmlExtension
  KParts/LiveConnectExtension
  KParts/ListingExtension
  KParts/MainWindow
  KParts/OpenUrlEvent
  KParts/Part
  KParts/PartActivateEvent
  KParts/PartBase
  KParts/PartManager
  KParts/PartSelectEvent
  KParts/Plugin
  KParts/ReadOnlyPart
  KParts/ReadWritePart
  KParts/StatusBarExtension
  KParts/TextExtension
  KParts/WindowArgs
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/KParts COMPONENT Devel)


install( FILES
  Kross/Action
  Kross/ActionCollection
  Kross/ActionCollectionEditor
  Kross/ActionCollectionModel
  Kross/ActionCollectionProxyModel
  Kross/ActionCollectionView
  Kross/ChildrenInterface
  Kross/ErrorInterface
  Kross/Interpreter
  Kross/InterpreterInfo
  Kross/Manager
  Kross/MetaFunction
  Kross/MetaType
  Kross/MetaTypeHandler
  Kross/MetaTypeImpl
  Kross/MetaTypeVariant
  Kross/MetaTypeVoidStar
  Kross/Object
  Kross/Script
  Kross/ScriptingPlugin
  Kross/WrapperInterface
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Kross COMPONENT Devel)


install( FILES
  KSettings/Dialog
  KSettings/Dispatcher
  KSettings/PluginPage
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/KSettings COMPONENT Devel)


#install( FILES
#  Sonnet/GuessLanguage
#  Sonnet/UnicodeData
#  Sonnet/TextBreaks
#  Sonnet/Spell
#DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Sonnet )


install( FILES
  KTextEditor/Attribute
  KTextEditor/CodeCompletionInterface
  KTextEditor/CodeCompletionModel
  KTextEditor/CodeCompletionModelControllerInterface
  KTextEditor/Command
  KTextEditor/CommandExtension
  KTextEditor/CommandInterface
  KTextEditor/ConfigInterface
  KTextEditor/ConfigPage
  KTextEditor/ContainerInterface
  KTextEditor/Cursor
  KTextEditor/Document
  KTextEditor/Editor
  KTextEditor/EditorChooser
  KTextEditor/Factory
  KTextEditor/HighlightInterface
  KTextEditor/MarkInterface
  KTextEditor/ModificationInterface
  KTextEditor/MovingCursor
  KTextEditor/MovingInterface
  KTextEditor/MovingRange
  KTextEditor/ParameterizedSessionConfigInterface
  KTextEditor/Plugin
  KTextEditor/Range
  KTextEditor/SearchInterface
  KTextEditor/SessionConfigInterface
  KTextEditor/SmartCursor
  KTextEditor/SmartCursorNotifier
  KTextEditor/SmartCursorWatcher
  KTextEditor/SmartInterface
  KTextEditor/SmartRange
  KTextEditor/SmartRangeNotifier
  KTextEditor/SmartRangeWatcher
  KTextEditor/TemplateInterface
  KTextEditor/TemplateInterface2
  KTextEditor/TextHintInterface
  KTextEditor/VariableInterface
  KTextEditor/View
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/KTextEditor COMPONENT Devel)


install( FILES
  KUnitTest/Runner
  KUnitTest/SlotTester
  KUnitTest/TestResults
  KUnitTest/Tester
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/KUnitTest COMPONENT Devel)


install( FILES
  KWallet/Wallet
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/KWallet COMPONENT Devel)


install( FILES
  Solid/AcAdapter
  Solid/AudioInterface
  Solid/Battery
  Solid/Block
  Solid/Button
  Solid/Camera
  Solid/Device
  Solid/DeviceInterface
  Solid/DeviceNotifier
  Solid/DvbInterface
  Solid/GenericInterface
  Solid/NetworkInterface
  Solid/NetworkShare
  Solid/Networking
  Solid/OpticalDisc
  Solid/OpticalDrive
  Solid/PortableMediaPlayer
  Solid/PowerManagement
  Solid/Predicate
  Solid/Processor
  Solid/StorageAccess
  Solid/StorageDrive
  Solid/StorageVolume
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Solid COMPONENT Devel)


install( FILES
  Sonnet/ConfigDialog
  Sonnet/ConfigWidget
  Sonnet/Dialog
  Sonnet/DictionaryComboBox
  Sonnet/Highlighter
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Sonnet COMPONENT Devel)

install( FILES
  KAuth/Action
  KAuth/ActionReply
  KAuth/ActionWatcher
  KAuth/HelperSupport
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/KAuth COMPONENT Devel)

install( FILES
  ThreadWeaver/DependencyPolicy
  ThreadWeaver/Job
  ThreadWeaver/JobCollection
  ThreadWeaver/JobSequence
  ThreadWeaver/QueuePolicy
  ThreadWeaver/ResourceRestrictionPolicy
  ThreadWeaver/State
  ThreadWeaver/Thread
  ThreadWeaver/Weaver
  ThreadWeaver/WeaverInterface
  ThreadWeaver/WeaverObserver
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/ThreadWeaver COMPONENT Devel)


install( FILES
  khtml/DrawContentsEvent
  khtml/MouseDoubleClickEvent
  khtml/MouseEvent
  khtml/MouseMoveEvent
  khtml/MousePressEvent
  khtml/MouseReleaseEvent
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/khtml COMPONENT Devel)


install(FILES
  Nepomuk/File
  Nepomuk/KRatingPainter
  Nepomuk/KRatingWidget
  Nepomuk/MassUpdateJob
  Nepomuk/Resource
  Nepomuk/ResourceManager
  Nepomuk/Service
  Nepomuk/Tag
  Nepomuk/TagWidget
  Nepomuk/Thing
  Nepomuk/Variant
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Nepomuk COMPONENT Devel)


install(FILES
  Nepomuk/Types/Class
  Nepomuk/Types/Literal
  Nepomuk/Types/Ontology
  Nepomuk/Types/Property
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Nepomuk/Types COMPONENT Devel)


install(FILES
  Nepomuk/Query/AndTerm
  Nepomuk/Query/ComparisonTerm
  Nepomuk/Query/FileQuery
  Nepomuk/Query/GroupTerm
  Nepomuk/Query/LiteralTerm
  Nepomuk/Query/NegationTerm
  Nepomuk/Query/OptionalTerm
  Nepomuk/Query/OrTerm
  Nepomuk/Query/Query
  Nepomuk/Query/QueryParser
  Nepomuk/Query/QueryServiceClient
  Nepomuk/Query/ResourceTerm
  Nepomuk/Query/ResourceTypeTerm
  Nepomuk/Query/Result
  Nepomuk/Query/SimpleTerm
  Nepomuk/Query/StandardQuery
  Nepomuk/Query/Term
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Nepomuk/Query COMPONENT Devel)


install(FILES
  Nepomuk/Utils/DynamicResourceFacet
  Nepomuk/Utils/Facet
  Nepomuk/Utils/FacetWidget
  Nepomuk/Utils/ProxyFacet
  Nepomuk/Utils/ResourceModel
  Nepomuk/Utils/SearchWidget
  Nepomuk/Utils/SimpleFacet
  Nepomuk/Utils/SimpleResourceModel
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Nepomuk/Utils COMPONENT Devel)


install(FILES
  Nepomuk/Vocabulary/NCAL
  Nepomuk/Vocabulary/NCO
  Nepomuk/Vocabulary/NDO
  Nepomuk/Vocabulary/NEXIF
  Nepomuk/Vocabulary/NFO
  Nepomuk/Vocabulary/NIE
  Nepomuk/Vocabulary/NMM
  Nepomuk/Vocabulary/NMO
  Nepomuk/Vocabulary/NUAO
  Nepomuk/Vocabulary/PIMO
  Nepomuk/Vocabulary/TMO
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Nepomuk/Vocabulary COMPONENT Devel)


install(FILES
  KUnitConversion/Converter
  KUnitConversion/Unit
  KUnitConversion/UnitCategory
  KUnitConversion/Value
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/KUnitConversion COMPONENT Devel)
