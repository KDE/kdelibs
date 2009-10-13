/**
 * KStyle for KDE4
 * Copyright (C) 2004-2005 Maksim Orlovich <maksim@kde.org>
 * Copyright (C) 2005,2006 Sandro Giessl <giessl@kde.org>
 *
 * Based in part on the following software:
 *  KStyle for KDE3
 *      Copyright (C) 2001-2002 Karol Szwed <gallium@kde.org>
 *      Portions  (C) 1998-2000 TrollTech AS
 *  Keramik for KDE3,
 *      Copyright (C) 2002      Malte Starostik   <malte@kde.org>
 *                (C) 2002-2003 Maksim Orlovich  <maksim@kde.org>
 *      Portions  (C) 2001-2002 Karol Szwed     <gallium@kde.org>
 *                (C) 2001-2002 Fredrik Höglund <fredrik@kde.org>
 *                (C) 2000 Daniel M. Duley       <mosfet@kde.org>
 *                (C) 2000 Dirk Mueller         <mueller@kde.org>
 *                (C) 2001 Martijn Klingens    <klingens@kde.org>
 *                (C) 2003 Sandro Giessl      <sandro@giessl.com>
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
 
#ifndef KDE_KSTYLE_H
#define KDE_KSTYLE_H

#include <kdeui_export.h>

#include <QtGui/QCommonStyle>
#include <QtGui/QPalette>
#include <QtGui/QStylePlugin>
#include <typeinfo>

class QStyleOptionProgressBar;
class QStyleOptionTab;

class KStylePrivate;
/**
 * Makes style coding more convenient.
 *
 * @todo and allows to style KDE specific widgets.
 *
 * KStyle strives to ease style development by implementing various QStyle
 * methods. These implementations are based on
 * -# the concept of Layout Properties. These properties can be set using
 *    setWidgetLayoutProp(). KStyle uses this information to respect various
 *    metrics (like space between primitives or margins around widget contents)
 *    or turn specific features on or off.
 * -# the concept of KStyle Primitives. These can be implemented by overriding
 *    drawKStylePrimitive() and providing drawing methods for specific
 *    primitives. Often, the drawing of more complex widgets consists of
 *    several primitives.
 *
 * In the following modules, information about related members is collected:
 * - \ref OptionGroup
 * - \ref WidgetGroup
 *
 * @author Maksim Orlovich (maksim\@kde.org)
 * @author Sandro Giessl (giessl\@kde.org)
 *
 * @see KStyleFactory for how to implement the style plugin interface.
 */
// TODO: From 'Qt4 Themes' discussion on kde-devel
// - Remi Villatel: extend QStyle enums for KColorButton, KColorCombo, KKeyButton, split PE_HeaderSection into KPopupTitle, PopupMenuTitle, TaskContainer)
// - RV: KLineEdit "plays with its colors" - related to KStyle?
// - RV: KMulitTabBarTab ... does it need support from KStyle (instead of manual button rotation etc.)? Or is it dumped already?
// - RV: KTabCtl draws itself mimicking QDrawShadeThingies
// - RV: fixed colors (e.g. Konqueror, KToolbarButton label text ->KPE_ToolbarButton?): To prevent hacks like "preventing any PaletteChange()"... mor related to KDE4 color schemes... I guess
// - LM: User interface guidelines... related to KStyle?
// - e.g. drawFancyPE() in kdeui for KDE widgets: check "qobject_cast<KStyle*>(style())", or fallback drawing...
// TODO: implement standardIcon().. and what about standardPalette()?
// TODO: maybe the arrow in CE_PushButtonLabel should be painted in CE_PushButtonBevel like QCommonStyle

class KDEUI_EXPORT KStyle: public QCommonStyle
{
    Q_OBJECT

public:
    KStyle();
    ~KStyle();

    /**
     * Returns the default widget style.
     */
    static QString defaultStyle();
    /**
     * Runtime element extension
     * This is just convenience and does /not/ require the using widgets style to inherit KStyle
     * (i.e. calling this while using cleanlooks won't segfault or so but just return 0)
     * Returns a unique id for an element string (e.g. "CE_CapacityBar")
     *
     * For simplicity, only StyleHints, ControlElements and their SubElements are supported
     * If you don't need extended SubElement functionality, just drop it
     * 
     * @param element The style element, represented as string.
     * Naming convention: "appname.(2-char-element-type)_element"
     * where the 2-char-element-type is of {SH, CE, SE}
     * (widgets in kdelibs don't have to pass the appname)
     * examples: "CE_CapacityBar", "amarok.CE_Analyzer"
     * @param widget Your widget ("this") passing this is mandatory, passing NULL will just return 0
     * @returns a unique id for the @p element string or 0, if the element is not supported by the
     * widgets current style
     *
     * Important notes:
     * 1) If your string lacks the matching "SH_", "CE_" or "SE_" token the element
     * request will be ignored (return is 0)
     * 2) Try to avoid custom elements and use default ones (if possible) to get better style support
     * and keep UI coherency
     * 3) If you cache this value (good idea, this requires a map lookup) don't (!) forget to catch
     * style changes in QWidget::changeEvent()
     */
     static StyleHint customStyleHint(const QString &element, const QWidget *widget);
     static ControlElement customControlElement(const QString &element, const QWidget *widget);
     static SubElement customSubElement(const QString &element, const QWidget *widget);

protected:

    /**
    * Runtime element extension, allows inheriting styles to add support custom elements
    * merges supporting inherit chains
    * Supposed to be called e.g. in your constructor.
    *
    * NOTICE: in order to have this work, your style must provide
    * an "X-KDE-CustomElements" classinfo, i.e.
    * class MyStyle : public KStyle
    * {
    *       Q_OBJECT
    *       Q_CLASSINFO ("X-KDE-CustomElements", "true")
    *
    *   public:
    *       .....
    * }
    *
    * @param element The style element, represented as string.
    * Suggested naming convention: appname.(2-char-element-type)_element
    * where the 2-char-element-type is of {SH, CE, SE}
    * widgets in kdelibs don't have to pass the appname
    * examples: "CE_CapacityBar", "amarok.CE_Analyzer"
    *
    * Important notes:
    * 1) If your string lacks the matching "SH_", "CE_" or "SE_" token the element
    * request will be ignored (return is 0)
    * 2) To keep UI coherency, don't support any nonsense in your style, but convince app developers
    * to use standard elements - if available
    */
    StyleHint newStyleHint(const QString &element);
    ControlElement newControlElement(const QString &element);
    SubElement newSubElement(const QString &element);
    
    /** @name Helper Methods
    * These are methods helping with QRect handling, for example.
    */
//@{
    /**
     Draws inside the rectangle using a thinkness 0 pen. This is what drawRect in Qt3 used to do.
    */
    void drawInsideRect(QPainter* p, const QRect& r) const;
    
    /**
     Returns a w x h QRect center inside the 'in' rectangle
    */
    QRect centerRect(const QRect &in, int w, int h) const;

    /**
     Return a size-dimension QRect centered inside the 'in' rectangle
    */
    QRect centerRect(const QRect &in, const QSize &size) const;
//@}

/**
 * \defgroup OptionGroup KStyle option representation
 * Things related to the representation of options passed when drawing things.
 */
//@{
    /**
     * @brief A representation for colors for use as a widget layout property.
     *
     * This marshals to and from integers.
    */
    class KDEUI_EXPORT ColorMode //krazy:exclude=dpointer (lightweight helper)
    {
    public:
        /**
         KStyle understands two kinds of colors:
         * -# Palette entries. This means the item must be painted with a specific
         *    color role from the palette.
         * -# Auto-selected black or white, dependent on the brightness of a certain
         *    color role from the palette.
        */
        enum Mode
        {
            PaletteEntryMode,
            BWAutoContrastMode = 0x8000000
        };
    private:
        Mode                mode;
        QPalette::ColorRole role;
    public:

        /// Constructor, using a the given palette role @p _role
        /// and a default mode.
        ColorMode(QPalette::ColorRole _role);

        /// Constructor with explicit color mode and palette roles.
        ColorMode(Mode _mode, QPalette::ColorRole _role);

        /// Represent as an int to store as a property
        operator int() const;

        /// Decode from an int.
        ColorMode(int encoded);

        /// Return the color corresponding to our role from the palette,
        /// automatically compensating for the contrast mode.
        QColor color(const QPalette& palette);
    };

    
    /**
     Base for our own option classes. 
     The idea here is that Option is the main base, and all the
     public bases inherit off it indirectly using OptionBase,
     which helps implement the default handling
    
     When implementing the actual types, just implement the default ctor,
     filling in defaults, and you're set.
    */
    struct KDEUI_EXPORT Option
    {
        virtual ~Option() {} //So dynamic_cast works, and g++ shuts up
    };

    /**
     Intermediatary base that helps implement subtypes of Option
     that properly handle defaults

     EventualSubtype --- the type of option which will be implemented
     by inheritting of this class

     BaseType        --- the type of option from which this should inherit
     */
    template<typename EventualSubtype, typename BaseType>
    struct KDEUI_EXPORT OptionBase: public BaseType
    {
        /** Default value for this option. Uses the default constructor
            of EventualSubtype to create the option. 
        */
        static EventualSubtype* defaultOption()
        {
            static EventualSubtype* theDefault = 0; //### function static, not very nice,
            //but avoids need for explicit instantiation.

            if (!theDefault)
                theDefault = new EventualSubtype;
                
            return theDefault;
        }
    };
    
    /**
     The extractOption method casts the passed in option object, and returns
     it, if available, or the defaults for the given type. When implementing
     drawing of things with associated options, just use this to extract
     the parameter.
    */
    template<typename T>
    static T extractOption(Option* option);

    /**
     Option representing the color of the thing to draw. Used for arrows, and for text
     (the latter actually uses TextOption)
    */
    struct KDEUI_EXPORT ColorOption: public OptionBase<ColorOption, Option>
    {
        /** Color to use for the drawing. Public, modifiable. */
        ColorMode color;

        ColorOption(): color(QPalette::ButtonText)
        {}
    };

    /**
     Option for drawing icons: represents whether the icon should be active or not.
     The implementation is responsible for all other flags
    */
    struct KDEUI_EXPORT IconOption: public OptionBase<IconOption, Option>
    {
        bool  active; ///< Is the icon active?
        QIcon icon;   ///< Icon drawn by this option
        QSize size;

        IconOption(): active(false)
        {}
    };

    /**
     * Option for drawing double scrollbar buttons, indicating whether
     * a button should be drawn active or not.
     * @sa ScrollBar::Primitive
     */
    struct KDEUI_EXPORT DoubleButtonOption: public OptionBase<DoubleButtonOption, Option>
    {
        /**
         * List of active button possibilities.
         */
        enum ActiveButton
        {
            None,   ///< No button is active
            Top,    ///< Vertical scrollbar: The upper button is active
            Left,   ///< Horizontal scrollbar: The left button is active
            Right,  ///< Horizontal scrollbar: The right button is active
            Bottom  ///< Vertical scrollbar: The lower button is active
        };

	/**
          Whether any of the two buttons is active; and if yes, which
          one.
	*/
        ActiveButton activeButton; 

        DoubleButtonOption(): activeButton(None)
        {}

        /**
         * Convenience constructor.
         *
         * @param ab initializes the active button property
         */
        DoubleButtonOption(ActiveButton ab): activeButton(ab)
        {}
    };

    /**
     * Option for drawing WT_Window titlebar buttons, indicating whether
     * the button is pressed, and containing the window icon
     * @sa Window
     */
    struct KDEUI_EXPORT TitleButtonOption: public OptionBase<TitleButtonOption, Option>
    {
        bool active;  ///< whether the button is pressed
        QIcon icon;   ///< window Icon
//         /// whether the button is hovered, this doesn't work at the moment (not even in any Qt style)...
//         bool hover;

        TitleButtonOption(): active(false)/*, hover(false)*/
        {}

        /**
         * Convenience constructor.
         *
         * @param act initializes the active button property
         */
        TitleButtonOption(bool act): active(act)
        {}
    };
    
    ///Option representing text drawing info. For Generic::Text. 
    struct KDEUI_EXPORT TextOption: public OptionBase<TextOption, ColorOption>
    {
        Qt::Alignment        hAlign; ///< The horizontal alignment, default is Qt::AlignLeft
        QString              text;   ///< The text to draw
        
        TextOption();

        /**
         * Convenience constructor.
         *
         * @param _text initializes the text string property
         */
        TextOption(const QString& _text);

        /**
         * Called by the constructor to set the default value of @c hAlign
         */
        void init();
    };
//@}

/**
 * \defgroup WidgetGroup KStyle widget representation
 * Things related to the representation of widgets.
 */
//@{
    /**
     This enum is used to represent KStyle's concept of
     a widget, and to associate drawing requests and metrics
     with it. The generic value is used for primitives and metrics
     that are common between many widgets
    */
    enum WidgetType
    {
        WT_Generic,         ///< @sa Generic
        WT_PushButton,      ///< @sa PushButton
        WT_Splitter,        ///< @sa Splitter
        WT_CheckBox,        ///< @sa CheckBox
        WT_RadioButton,     ///< @sa RadioButton
        WT_DockWidget,      ///< @sa DockWidget
        WT_ProgressBar,     ///< @sa ProgressBar
        WT_MenuBar,         ///< @sa MenuBar
        WT_MenuBarItem,     ///< @sa MenuBarItem
        WT_Menu,            ///< @sa Menu
        WT_MenuItem,        ///< @sa MenuItem
        WT_ScrollBar,       ///< @sa ScrollBar
        WT_TabBar,          ///< @sa TabBar
        WT_TabWidget,       ///< @sa TabWidget
        WT_Slider,          ///< @sa Slider
        WT_Tree,            ///< @sa Tree
        WT_SpinBox,         ///< @sa SpinBox
        WT_ComboBox,        ///< @sa ComboBox
        WT_Header,          ///< @sa Header
        WT_LineEdit,        ///< @sa LineEdit
        WT_GroupBox,        ///< @sa GroupBox
        WT_StatusBar,       ///< @sa StatusBar
        WT_ToolBar,         ///< @sa ToolBar
        WT_ToolButton,      ///< @sa ToolButton
        WT_ToolBoxTab,      ///< @sa ToolBoxTab
        WT_Window,          ///< @sa Window
        WT_Limit = 0xFFFF ///< For enum extensibility
    };

                
    /**
     These constants describe how to access various fields of a margin property.
     For example, to set an additional top margin of 2 pixels, use
     * \code
     * setWidgetLayoutProp(WT_SomeWidget, SomeWidget::Margin + Top, 2);
     * \endcode
     */
    enum MarginOffsets
    {
        MainMargin, /**< The main margin is applied equally on each side.
                     * In the example above, 'SomeWidget::Margin+MainMargin' is
                     * the same as 'SomeWidget::Margin'. */
        Top,        ///< Apply an additional top margin.
        Bot,        ///< Apply an additional bottom margin.
        Left,       ///< Apply an additional left margin.
        Right,      ///< Apply an additional right margin.
        MarginInc   /**< Used to reserve space in an enum for the Top, Bot,
                     * Left, Right margins. */
    };


    /**
     * Basic primitive drawing operations. Are intended to be used in every
     * WidgetType combination.
     */
    struct Generic
    {
        /**
         * Layout properties. These can be set with setWidgetLayoutProp()
         * Generic LayoutProps contain a few properties which are not
         * directly related to a specific widget type.
         */
        enum LayoutProp
        {
            DefaultFrameWidth,    ///< The FrameWidth used by LineEdit, etc..., default is \b 2 [sets QStyle::PM_DefaultFrameWidth]
            DefaultLayoutSpacing, ///< The spacing used by layouts, unless the style implements layoutSpacingImplementation(), default is \b 6 [sets QStyle::PM_DefaultLayoutSpacing]
            DefaultLayoutMargin   ///< The margin used by layouts, default is \b 9 [sets QStyle::PM_DefaultChildMargin and QStyle::PM_DefaultTopLevelMargin]
        };

        /**
         * Primitive drawing operations.
         * @note The arrows are centering primitives, which means they draw in
         * the center of the specified rectangle.
         */
        enum Primitive
        {
            Text = 0xFFFF,  ///< Passes in TextOption
            Icon,           ///< Passes in IconOption
            FocusIndicator, ///< Indication that this widget has focus
            Frame,          /**< Frame around widget
                             * @note QFrame derived widgets are painted using
                             * WT_Generic widget type and this Generic::Frame primitive. */
            ArrowUp,        ///< Up arrow (pointing up)
            ArrowDown,      ///< Down arrow
            ArrowRight,     ///< Right arrow
            ArrowLeft       ///< Left arrow
        };
    };

    /**
     * @brief Describes widgets like QPushButton.
     *
     * @sa WT_PushButton
     */
    struct PushButton
    {
        /**
         * The layout of a PushButton is structured as follows:
         * -# Between the very outside and the bevel is the default indicator
         *    area, controlled by the @c PushButton::DefaultIndicatorMargin .
         * -# From the bevel, the content and focus rect margins are measured.
         *    Only the content margin is used to size the content area.
         * -# Inside the content area, @c PushButton::MenuIndicatorSize is allocated to the down
         *    arrow if there is a popup menu.
         * -# @c PushButton::TextToIconSpace is allocated between icon and text if both exist
         *
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProp
        {
            ContentsMargin, ///< (\b 5) space between the bevel and the button contents
            FocusMargin            = ContentsMargin + MarginInc, ///< (\b 3) Used to calculate the area of the focus indicator. Measured from the bevel.
            DefaultIndicatorMargin = FocusMargin    + MarginInc, ///< (\b 0 ?) Default indicator between the very outside and the bevel. KStyle may reserve this for auto-default buttons, too, for consistency's sake. [the MainMargin sets QStyle::PM_ButtonDefaultIndicator]
            PressedShiftHorizontal = DefaultIndicatorMargin + MarginInc, ///< (\b 2) horizontal contents shift for pressed buttons [sets QStyle::PM_ButtonShiftHorizontal]
            PressedShiftVertical, ///< (\b 2) vertical contents shift for pressed buttons [sets QStyle::PM_ButtonShiftVertical]
            MenuIndicatorSize, ///< (\b 8) Space inside the content area, which is allocated to the down arrow if there is a popup menu [sets QStyle::PM_MenuButtonIndicator, except for toolbuttons]
            TextToIconSpace ///< (\b 6) space between the icon and the text if both exist
        };

        /**
         * Relevant Generic elements:
         * - @c Generic::Text the button's text
         * - @c Generic::FocusIndicator indicating that the button has keyboard focus
         * - @c Generic::ArrowDown indicating that the button has a popup menu associated to it
         *
         * [KStyle implements @c QStyle::CE_PushButtonBevel to compose the primitives]
         *
         * @sa drawKStylePrimitive()
         */
        enum Primitive
        {
            Panel,              /**< the pushbutton panel
                                 * [implements QStyle::PE_PanelButtonCommand] */
            DefaultButtonFrame  /**< frame indicating a default button, painted before
                                 * the button Panel
                                 * [implements QStyle::PE_FrameDefaultButton] */
        };
    };

    /**
     * @brief Describes widgets like QSplitter.
     *
     * @sa WT_Splitter
     */
    struct Splitter
    {
        /**
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProp
        {
            Width ///< (\b 6) size of the splitter handle [sets QStyle::PM_SplitterWidth]
        };

        /**
         * [the Handles implement QStyle::CE_Splitter]
         *
         * @sa drawKStylePrimitive()
         */
        enum Primitive
        {
            HandleHor,      /**< The splitter handle, horizontal. Flags: @c State_Enabled&&State_MouseOver for mouseOver */
            HandleVert      /**< The splitter handle, vertical. Flags: @c State_Enabled&&State_MouseOver for mouseOver */
        };
    };

    /**
     * @brief Describes widgets like QCheckBox.
     *
     * @sa WT_CheckBox
     */
    struct CheckBox
    {
        /**
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProp
        {
            Size,               ///< (\b 16) size of the checkbox [sets PM_IndicatorWidth, PM_IndicatorHeight]
            BoxTextSpace,       ///< (\b 6) space to leave between checkbox and text (and icon between them in case there is one)
            NoLabelFocusMargin, /**< (\b 1) rectangle to apply to the checkbox rectangle
                                 * to get where to paint the focus rectangle in
                                 * case of a labelless checkbox */
            FocusMargin = NoLabelFocusMargin + MarginInc ///< (\b 0) margin around the checkbox contents reserved for the focus rect @todo have a look, it isn't as nice as plastique etc.
        };

        /**
         * Relevant elements:
         * - @c Generic::Text the CheckBox label alongside the CheckBox
         * - @c Generic::FocusIndicator the focus indicator. Usually drawn around the
         *      text label. If no label exists, it is drawn around the CheckBox.
         *
         * [check primitives implement QStyle::PE_IndicatorCheckBox]
         * [KStyle implements QStyle::CE_CheckBox to compose a CheckBox using
         *  the mentioned primitives]
         *
         * @sa drawKStylePrimitive()
         */
        enum Primitive
        {
            CheckOn,        ///< checkbox which is checked
            CheckOff,       ///< checkbox which is not checked
            CheckTriState   ///< tristate checkbox (neither off nor on)
        };
    };

    /**
     * @brief Describes widgets like QRadioButton.
     */
    struct RadioButton
    {
        /**
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProp
        {
            Size, /**< [sets QStyle::PM_ExclusiveIndicatorWidth,
                   *    QStyle::PM_ExclusiveIndicatorHeight]
                   * @sa CheckBox::Size */
            BoxTextSpace, ///< @sa CheckBox::BoxTextSpace
            FocusMargin   ///< @sa CheckBox::FocusMargin
        };

        /**
         * Relevant Generic elements:
         * - @c Generic::Text the RadioButton label alongside the RadioButton
         * - @c Generic::FocusIndicator the keyboard focus indicator
         *
         * [check primitives implement QStyle::PE_IndicatorRadioButton]
         * [KStyle implements QStyle::CE_RadioButton to compose a RadioButton using
         *  the mentioned primitives]
         *
         * @sa drawKStylePrimitive()
         */
        enum Primitive
        {
            RadioOn,        ///< radiobutton which is checked
            RadioOff        ///< radiobutton which is not checked
        };
    };
    

    /**
     * @brief Describes the title of a dock widget.
     *
     * @sa WT_DockWidget
     */
    struct DockWidget
    {
        /**
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProp
        {
            TitleTextColor, ///< (\b ColorMode(QPalette::HighlightedText)) color mode of the title text
            TitleMargin, ///< (\b 2) Margin around title contents: Note that the symmetric margin (MainMargin) is used to size the title! Additional the Left and Right margins can be used to position the title text a little, though (to set Top and Bottom is not advisable). [the MainMargin sets QStyle::PM_DockWidgetTitleMargin]
            FrameWidth = TitleMargin + MarginInc,  ///< (\b 3) width of the frame around floating dockwidgets [sets QStyle::PM_DockWidgetFrameWidth]
            SeparatorExtent ///< (\b 6) width of the area which separates the (docked) dock window from the window contents [sets QStyle::PM_DockWidgetSeparatorExtent]
        };

        /**
         * Relevant Generic elements:
         * - @c Generic::Text the title text
         * - @c Generic::Frame the frame around floating dockwidgets
         *
         * [KStyle implements @c QStyle::CE_DockWidgetTitle to split it into KStyle primitives]
         *
         * @sa drawKStylePrimitive()
         */
        enum Primitive
        {
            TitlePanel, ///< the panel/background of the title bar
            SeparatorHandle ///< the splitter between dockwidgets
        };
    };

    /**
     * @brief Describes widgets like QProgressBar.
     *
     * @todo support for Qt > 4.1 orientation, bottomToTop, invertedAppearance properties!
     *
     * @sa WT_ProgressBar
     */
    struct ProgressBar
    {
        /**
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProp
        {
            GrooveMargin,        ///< (\b 2) Margin to allocate for the groove. Content area will be inside of it.
            SideText = GrooveMargin + MarginInc, ///< (\b false) set this to true to have the text positionned to the side
            SideTextSpace,       ///< (\b 3) Extra space besides that needed for text to allocate to side indicator (on both sides).
            Precision,           ///< (\b 1) The indicator size will always be a multiple of this (modulo busy indicator size clamping). [sets QStyle::PM_ProgressBarChunkWidth]
            BusyIndicatorSize,   ///< (\b 10) The busy indicator size, in percent of area size.
            MaxBusyIndicatorSize ///< (\b 10000) size limit on the busy indicator size
        };

        /**
         * Relevant Generic elements:
         * - @c Generic::Text the progress label
         *
         * @sa drawKStylePrimitive()
         */
        enum Primitive
        {
            Groove,         /**< the progressbar groove, drawn before the progress
                             * Indicator [implements QStyle::CE_ProgressBarGroove] */
            Indicator,      ///< The actual bar indicating the progress...
            BusyIndicator   /**< Used to indicate business, for example when
                             * no progress is known (minimum and maximum values
                             * both set to 0) */
        };
    };


    /**
     * @brief Describes widgets like QMenuBar.
     *
     * @sa WT_MenuBar
     */
    struct MenuBar
    {
        /**
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProp
        {
            Margin,           /**< (MainMargin \b 2, Left \b 4, Right \b 4)
                               * Margin rectangle for the contents. */
            ItemSpacing = Margin + MarginInc ///< (\b 14) Space between items [sets QStyle::PM_MenuBarItemSpacing]
        };

        /**
         * @sa drawKStylePrimitive()
         */
        enum Property
        {
            EmptyArea /**< Empty area of a menu bar, e.g. background
                       * color. Maybe the place to fake toolbar separators (?)
                       * [implements QStyle::CE_MenuBarEmptyArea] */
        };
    };


    /**
     * @brief Describes MenuBar items.
     *
     * Relevant elements:
     * - @c Generic::Text text appearing as menubar entry
     */
    struct MenuBarItem
    {
        /**
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProp
        {
            Margin,                    /**< (\b 1) Margin rectangle to allocate for any
                                        * bevel, etc. (Text will be drawn with
                                        * the inside rect). */
            Dummy = Margin + MarginInc //Paranoia about underlying type
        };

        /**
         * @sa drawKStylePrimitive()
         */
        enum Primitive
        {
            Panel       ///< The panel/background of a menubar item. Interesting flags: State_Selected && State_HasFocus for mouseOver, State_Sunken for pressed state.
        };
    };

    /**
     * @brief Describes a menu.
     *
     * @sa WT_Menu
     */
    struct Menu
    {
        /**
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProp
        {
            FrameWidth, ///< (\b 1) The width of the frame, note that this does not affect the layout.
            Margin,     ///< (\b 3) The margin of the menu. @todo have a look at comments at PM_MenuHMargin...
            ScrollerHeight = Margin + MarginInc, ///< (\b 10) Height of a menu scroller. [sets QStyle::PM_MenuScrollerHeight]
            TearOffHeight ///< (\b 10) Height of the TearOff area. [sets QStyle::PM_MenuTearoffHeight]
        };

        /**
         * Relevant Generic elements:
         * - @c Generic::Frame frame around the menu panel
         *
         * @sa drawKStylePrimitive()
         */
        enum Primitive
        {
            Background, ///< Menu and MenuItem background
            TearOff,    /**< paints the area where a menu can be teared off
                         * [implements QStyle::CE_MenuTearoff] */
            Scroller    /**< scrolling areas in a QMenu
                         * [implements QStyle::CE_MenuScroller] */
        };
    };

    /**
     * @brief Describes an item in a menu.
     *
     * MenuItems are layouted like this:
     * -# There are two MenuItem modes, toggled by @c LP_MenuItem_CheckAlongsideIcon.
     *    Horizontal layout inside the items is as follow:
     *     - @c LP_MenuItem_CheckAlongsideIcon disabled:
     *          |icon/checkmark|IconSpace|text|AccelSpace|accel|ArrowSpace|ArrowWidth|
     *     - @c LP_MenuItem_CheckAlongsideIcon enabled:
     *          |checkmark|CheckSpace|icon|IconSpace|text|AccelSpace|accel|ArrowSpace|ArrowWidth|
     * -# The icon/checkmark column is at least @c LP_MenuItem_CheckWidth wide in all cases.
     * -# Then Margin is applied outside that.
     *
     * @note For the subprimitives the passed rect is their own.
     * @note Background is erased with WT_Menu/Menu::Background.
     *
     * @sa WT_MenuItem
     */
    struct MenuItem
    {
        /**
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProp
        {
            Margin,         ///< (\b 2) margin for each entry
            CheckAlongsideIcon = Margin + MarginInc,
                            /**< (\b 0) Set to non-zero to have checkmarks painted
                             * separate from icons. */
            CheckWidth,     /**< (\b 12) size of the checkmark column
                             * (CheckAlongsideButton enabled). */
            CheckSpace,     /**< (\b 3) Space between the checkmark column and the icon
                             * column (CheckAlongsideButton enabled).*/
            IconWidth,      ///< (\b 12) minimum size of the icon column
            IconSpace,      ///< (\b 3) space between the icon column and text column
            AccelSpace,     ///< (\b 0 ?) space between text and keyboard accelerator label
            ArrowSpace,     ///< (\b 3) space between the text column and arrow column (indicating sub menus)
            ArrowWidth,     ///< (\b 11) space to reserve for the menu arrow column
            SeparatorHeight,///< (\b 0) heigh of separator
            MinHeight,      ///< (\b 16) limit on the size of item content
            ActiveTextColor,///< (\b ColorMode(QPalette::HighlightedText)) color for active text and arrow
            TextColor,      ///< (\b ColorMode(QPalette::Text)) color for inactive text and arrow
            DisabledTextColor, ///< (\b ColorMode(QPalette::Text)) color for inactive + disabled text and arrow
            ActiveDisabledTextColor ///< (\b ColorMode(QPalette::Text)) color for active + disabled text and arrow
        };

        /**
         * Relevant Generic elements:
         * - @c Generic::Text the text of the menu item
         * - @c Generic::ArrowLeft @c Generic::ArrowRight arrows indicating a sub-menu
         *
         * @sa drawKStylePrimitive()
         */
        enum Primitive
        {
            CheckColumn, ///< Background of the checkmark/icon column
            CheckOn,     ///< The checkmark - checked
            CheckOff,    ///< The checkmark - not checked
            RadioOn,     ///< The checkmark of exclusive actions - selected
            RadioOff,    ///< The checkmark of exclusive actions - not selected
            CheckIcon,   ///< When @c CheckAlongsideIcon is disabled and item is checked: checkmark Indicator painted below the icon
            Separator,   //A separator item.
            ItemIndicator //Shows the active item
        };
    };


    /**
     * @brief Describes widgets like QScrollBar.
     *
     * @sa WT_ScrollBar
     */
    struct ScrollBar
    {
        /**
         * @note Dimensions of LayoutProperties are generally specified with respect
         *       to the vertical scrollbar. Of course, for horizontal ones they're flipped.
         *
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProp
        {
            DoubleTopButton,    ///< (\b 0) set to non-zero to have two buttons on top
            DoubleBotButton,    ///< (\b 1) set to non-zero to have two buttons on bottom
            SingleButtonHeight, ///< (\b 16) height of a single button
            DoubleButtonHeight, ///< (\b 32) height of a double button
            BarWidth,           ///< (\b 16) width of a scrollbar [sets QStyle::PM_ScrollBarExtent]
            MinimumSliderHeight,/**< (\b 0 ?) the minimum slider height
                                 * @note If the scrollbar is too small to accommodate
                                 *       this, this will not be enforced. */
            ArrowColor,         /**< (\b ColorMode(ColorMode::BWAutoContrastMode, QPalette::Button)) color mode of a button arrow
                                 * @sa ColorMode */
            ActiveArrowColor    /**< (\b ColorMode(ColorMode::BWAutoContrastMode, QPalette::ButtonText)) color mode of a pressed button arrow (?) */
        };

        /**
         * Relevant Generic elements:
         * - The @c Generic arrows
         *
         * [Groove Areas implement QStyle::CE_ScrollBarAddPage and QStyle::CE_ScrollBarSubPage]
         * [Sliders implement QStyle::CE_ScrollBarSlider]
         *
         * @sa drawKStylePrimitive()
         */
        enum Primitive
        {
            SingleButtonVert,   ///< used to draw a 1-button bevel, vertical
            SingleButtonHor,    ///< used to draw a 1-button bevel, horizontal
            DoubleButtonVert,           /**< Used to draw a 2-button bevel, vertical.
                                         * A DoubleButtonOption is passed to say which
                                         * button is pressed. */
            DoubleButtonHor,            /** @see DoubleButtonVert */
            GrooveAreaVertBottom,   ///< scrollbar groove area, vertical. An interesting flag is @c State_Sunken for pressed state
            GrooveAreaHorRight,    ///< scrollbar groove area, horizontal. Flags: @c State_Sunken for pressed state
            GrooveAreaVertTop,   ///< scrollbar groove area, vertical. An interesting flag is @c State_Sunken for pressed state
            GrooveAreaHorLeft,    ///< scrollbar groove area, horizontal. Flags: @c State_Sunken for pressed state
            SliderVert,       ///< scrollbar slider, vertical. Flags: @c State_On&&State_Sunken for pressed state
            SliderHor         ///< scrollbar slider, horizontal. Flags: @c State_On&&State_Sunken for pressed state
        };
    };

    /**
     * @brief Describes a tab bar.
     *
     * @sa WT_TabBar
     */
    struct TabBar
    {
        /**
         * Each tab is basically built hiearchically out of the following areas:
         * -# Content area, one of the following layouts:
         *     - Icon <- TextToIconSpace -> Text
         *     - Icon
         *     - Text
         * -# Bevel: @c LP_TabBar_TabContentsMargin outside of the content area
         * -# Focus indicator is placed @c LP_TabBar_TabFocusMargin inside the bevel
         *
         * @note The side tabs just have those rotated, bottom tabs have the margins reversed.
         *
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProp
        {
            TabContentsMargin,  ///< (\b 6) margin around the tab contents, used to size the tab
            TabFocusMargin     = TabContentsMargin + MarginInc,
                                /**< (\b 3) where the tab focus rect is placed, measured from the
                                 * tab sides (?) */
            TabTextToIconSpace = TabFocusMargin    + MarginInc,
                                /**< (\b 0 ?) space between icon and text if the tab contains both */
            TabOverlap,         /**< (\b 0) Amount of pixels tabs should overlap. The
                                 * paint rectangle will be extended to the left for
                                 * all tabs which are not at the beginning (accordingly
                                 * extended to the right in RightToLeft mode; extended
                                 * to the top for East/West tabs). */
            BaseHeight,        ///< (\b 2) the height of the tabBar's base, usually the frame width [sets QStyle::PM_TabBarBaseHeight] @todo is this used for things like I intended TabWidget::ContentsMargin for?
            BaseOverlap,       ///< (\b 2) the number of pixels the tabs overlap with the base (i.e. tabWidget frame) [sets QStyle::PM_TabBarBaseOverlap]
            ScrollButtonWidth  ///< (\b 10) buttons which are shown when there's not enough space for tabs (A ToolButton is used for this) [sets QStyle::PM_TabBarScrollButtonWidth]
        };

        /**
         * Relevant elements:
         * - @c Generic::Text for the TabBar labels
         * - @c Generic::FocusIndicator for focused tabs
         * - @c Generic::Icon for icons associated to tabs
         * - @c ToolButton::Panel paints the scroll button (when the tabs don't fit the tab bar)
         */
        enum Primitive
        {
            EastText,       /**< Special rotated text for east tabs. */
            WestText,       ///< @see EastText
            NorthTab,       ///< @todo say something about triangular shape etc.
            EastTab,
            WestTab,
            SouthTab,
            BaseFrame,      ///< [implements QStyle::PE_FrameTabBarBase]
            IndicatorTear   /**< painted in the left edge of a tabbar when the left
                             * tab is scrolled out
                             * [implements PE_IndicatorTabTear]
                             * @todo KStyle default implementation...? */
        };
    };

    /**
     * @brief Describes a tab widget (frame).
     *
     * Relevant elements:
     * - @c Generic::Frame for the frame/panel of the TabWidget
     *
     * @sa WT_TabWidget
     */
    struct TabWidget
    {
        /**
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProp
        {
            ContentsMargin,      /**< (\b 2) Width of the frame around a tab widget.
                               * Margins for a tabwidget with tab position 'North' are
                               * specified as expected. For other positions, the
                               * sides are rotated accordingly, e.g. the left margin
                               * of a 'West' tabwidget is the same as top for a 'North'
                               * tabwidget.
                               * [sets QStyle::SE_TabWidgetTabContents] */
            DummyProp = ContentsMargin+MarginInc
        };
    };

    /**
     * @brief Describes a slider, like QSlider.
     *
     * @sa WT_Slider
     */
    struct Slider
    {

        /**
         * @note The description applies to horizontal sliders.
         *
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProp
        {
            HandleThickness, ///< (\b 20) The height of a slider handle
            HandleLength     ///< (\b 16) The width of a slider handle [sets QStyle::PM_SliderLength]
        };

        /**
         * Relevant Generic elements:
         * - @c Generic::FocusIndicator indicating keyboard focus
         *
         * @sa drawKStylePrimitive()
         */
        enum Primitive
        {
            HandleVert,   ///< A vertical slider handle
            HandleHor,    ///< A horizontal slider handle
            GrooveVert,   ///< A vertical slider groove
            GrooveHor     ///< A horizontal slider groove
        };
    };


    /**
     * @brief Describes an expandable tree, e.g. in a QListView.
     */
    struct Tree
    {
        /**
         * For trees, all the control we provide here is to provide a cap on the size
         * of the expander widget, which is always square. There are 4 primitives to
         * implement: open and closed expander, and horizontal and vertical lines.
         * If you're using dots, it's suggested you use global brush alignment to
         * keep it all nicely aligned.
         *
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProp
        {
            MaxExpanderSize      /**< (\b 9) @note If you set MaxExpanderSize to a value less
                                  * than 9, designer will look funny. The value should also
                                  * be odd, or value - 1 will be used.
                                  */
        };

        /**
         * @sa drawKStylePrimitive()
         */
        enum Primitive
        {
            ExpanderClosed, ///< A closed tree expander, usually drawn as '+'. KStyle has a default implementation (Windows-like look).
            ExpanderOpen,   ///< An opened tree expander, usually drawn as '-' KStyle has a default implementation.
            HorizontalBranch, /**< A horizontal tree line.
                               * @note If you're using dots, it's suggested you
                               * use global brush alignment to keep it all nicely
                               * aligned. */
            VerticalBranch    /**< @see HorizontalBranch */
        };
    };

    /**
     * @brief Describes a widget like QSpinBox.
     */
    struct SpinBox
    {
        /**
         * @note The description applies to LTR (left to right) mode.
         *
         * -# @c FrameWidth and @c ButtonWidth are used to size
         * the contents area. To the EditField, @c FrameWidth
         * is added at the left, top, bottom while @c ButtonWidth is added
         * at the right.
         * -# The @c ButtonMargin is measured from the right side of the
         * EditField and the outside. Inside it, the up and down buttons are aligned with
         * spacing @c ButtonSpacing.
         * -# To make sure that both buttons are always of the same height, enable
         * @c SymmetricButtons.
         *
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProp
        {
            FrameWidth,        /**< (\b 1) Space reserved left, top, bottom of the SpinBox
                                * [sets QStyle::PM_SpinBoxFrameWidth] */
            ButtonWidth,       ///< (\b 16) Space reserved for the widget, right of the EditField
            ButtonMargin,      /**< (MainMargin \b 0, Right Top Bot \b 1)
                                * Where the up/down buttons are located, measured
                                * from right of the edit field and the top/right/bottom
                                * widget edges. */
            ButtonSpacing = ButtonMargin + MarginInc, ///< (\b 1) spacing between up/down buttons
            SymmetricButtons,  /**< (\b 0) Set to non-zero to make sure both buttons are
                                * always of the same height. To achieve this, the
                                * spacing of the buttons will be reduced by 1 if
                                * necessary to avoid rounding problems. Needs to be
                                * handled in your drawing code. */
            SupportFrameless,   /**< (\b 0) Set to non-zero to indicate that you are able to
                                * handle frame-less SpinBoxes. For a SpinBox with no
                                * frame, FrameWidth and Top/Bottom/Right ButtonMargin
                                * is ignored. */
            ContentsMargin
                                /**< (\b 5) space between the bevel and the spinbox contents
                                    */
        };

        /**
         * Relevant Generic elements:
         * - @c Generic::Frame for the area around text input field and buttons
         * - @c Generic::ArrowUp @c Generic::ArrowDown drawn on the buttons
         */
        enum Primitive
        {
            EditField,          /**< the text contents area, painted after Generic::Frame
                                 * @note This is respected only if the combobox is not
                                 * editable. */
            UpButton,           /**< Panel of the spinbox button which increases the value */
            DownButton,         /**< Panel of the spinbox button which decreases the value */
            ButtonArea,         /**< Can be used in addition or instead of
                                 * @c KPE_SpinBox_UpButton and @c KPE_SpinBox_DownButton.
                                 * The button area is painted before them. */
            PlusSymbol,         /**< Plus symbol painted on top of the up button,
                                 * centering primitive */
            MinusSymbol         /**< Minus symbol painted on top of the down button,
                                 * centering primitive */
        };
    };

    /**
     * @brief Describes a widget like QComboBox.
     */
    struct ComboBox
    {
        /**
         * @note The description applies to LTR (left to right) mode.
         *
         * -# @c FrameWidth and @c ButtonWidth are used to size
         * the contents area. To the EditField, @c FrameWidth
         * is added at the left, top, bottom while @c ButtonWidth is added
         * at the right.
         * -# The @c ButtonMargin is measured from the right side of the
         * EditField and the outside. Inside it, the button is aligned.
         * -# The @c FocusMargin is measured from the EditField rect.
         */
        enum LayoutProp
        {
            FrameWidth,         /**< (\b 1) @see SpinBox::FrameWidth */
            ButtonWidth,        /**< (\b 16) @see SpinBox::ButtonWidth */
            ButtonMargin,       /**< (MainMargin \b 0, Right Top Bot \b 1)
                                 * @see SpinBox::ButtonMargin */
            FocusMargin = ButtonMargin + MarginInc,
                                /**< (\b 1) Focus margin for ComboBoxes that aren't
                                 * editable, measured from the EditField rect */
            SupportFrameless = FocusMargin + MarginInc,
                                /**< (\b 0) @see LP_SpinBox_SupportFrameless same description
                                 * applies here */
            ContentsMargin
                                /**< (\b 5) space between the bevel and the combobox contents
                                    */
        };

        /**
        * Relevant Generic elements:
         * - @c Generic::Frame for the area around text input field and button
         * - @c Generic::ArrowDown drawn on the button
         * - @c Generic::FocusIndicator to indicate keyboard focus
         *
         * @sa drawKStylePrimitive()
         */
        enum Primitive
        {
            EditField,          /**< @see SpinBox::EditField */
            Button              /**< The button panel of the combobox */
        };
    };

    /**
     * @brief Describes a list header, like in QListView.
     *
     * @sa WT_Header
     */
    struct Header
    {
        /**
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProp
        {
            ContentsMargin,      /**< (\b 3) margin around contents used to size the header. */
            TextToIconSpace = ContentsMargin + MarginInc,
                                 /**< (\b 3) space that is allocated between icon and text
                                  * if both exist
                                  * [sets QStyle::PM_HeaderMargin] */
            MarkSize             /**< (\b 9) size of the sort indicator in a header
                                  * [sets QStyle::PM_HeaderMarkSize] */
        };

        /**
         * Relevant Generic elements:
         * - @c Generic::Text for the header text label
         * - @c Generic::ArrowUp @c Generic::ArrowDown to indicate the sorting of the column
         *
         * [the Sections implement QStyle::CE_HeaderSection]
         */
        enum Primitive
        {
            SectionHor, ///< header section, horizontal
            SectionVert ///< header section, vertical
        };
    };

    /**
     * @brief Describes a text edit widget like QLineEdit.
     *
     * The frame width of lineedits is determined using Generic::DefaultFrameWidth
     */
    struct LineEdit
    {
        /**
         * Relevant Generic elements:
         * - @c Generic::Frame paints a lineedit frame only [implements QStyle::PE_FrameLineEdit]
         *
         * @sa drawKStylePrimitive()
         */
        enum Primitive
        {
            Panel     ///< the panel for a QLineEdit (including frame...) [implements QStyle::PE_PanelLineEdit]
        };
    };

    /**
     * @brief Describes something like QGroupBox.
     *
     * Relevant Generic elements:
     * - @c Generic::Frame frame around the group box
     *
     * [the Frames implement QStyle::PE_FrameGroupBox]
     *
     * @sa Generic::DefaultFrameWidth
     *
     * @sa WT_GroupBox
     */
    struct GroupBox
    {
        /**
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProps
        {
            FrameWidth, /**< (\b 2) width of a groupbox frame */
            TextAlignTop, /**< (\b 0) set to non-zero, the title will be aligned
                          * above the groupbox frame, not vertically centered
                          * [sets QStyle::SH_GroupBox_TextLabelVerticalAlignment] */
            TitleTextColor ///< (\b ColorMode(QPalette::Text)) color of the title text label
        };
        /**
         * @sa drawKStylePrimitive()
         */
        enum Primitive
        {
            FlatFrame  /**< For groupboxes which are set to be 'flat' (usually
                        * a divider line from top left to top right). KStyle
                        * has a basic default implementation */
        };
    };

    /**
     * @brief Describes a status bar section.
     *
     * Relevant Generic elements:
     * - @c Generic::Frame paints the status bar section [implements QStyle::PE_FrameStatusBar]
     *
     * No LayoutProps for now.
     *
     * @sa WT_StatusBar
     */
    struct StatusBar
    {
        /**
        No LayoutProps for now.
         */
    };

    /**
     * @brief Describes a tool bar.
     *
     * @sa WT_ToolBar
     */
    struct ToolBar
    {
        /**
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProps
        {
            HandleExtent,       ///< (\b 6) the width(hor)/height(vert) of a ToolBar handle [sets QStyle::PM_ToolBarHandleExtent]
            SeparatorExtent,    ///< (\b 6) the width/height of a ToolBar separator [sets QStyle::PM_ToolBarSeparatorExtent]
            ExtensionExtent,    ///< (\b 10) the width/height of a ToolBar extender, when there is not enough room for toolbar buttons [sets PM_ToolBarExtensionExtent]
            FrameWidth,    /**< (\b 2) width of the frame around toolbars
                            * [sets QStyle::PM_ToolBarFrameWidth] */
            ItemMargin,         ///< (\b 1) [sets QStyle::PM_ToolBarItemMargin]
            ItemSpacing         ///< (\b 3) [sets QStyle::PM_ToolBarItemSpacing]
        };

        /**
         * [the Handles implement QStyle::PE_IndicatorToolBarHandle]
         * [the Panels implement QStyle::CE_ToolBar]
         *
         * @sa drawKStylePrimitive()
         */
        enum Primitive
        {
            HandleHor, ///< handle of a toolbar, horizontal
            HandleVert, ///< handle of a toolbar, vertical
            Separator, ///< [implements QStyle::PE_IndicatorToolBarSeparator]
            PanelHor,  ///< the actual toolbar, horizontal
            PanelVert  ///< the actual toolbar, vertical
        };
    };


    /**
     * @brief Describes a tab for a tool box, like QToolBox.
     *
     * @sa WT_ToolBoxTab
     */
    struct ToolBoxTab
    {
        /**
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProps
        {
            Margin  /**< (\b 0) used to specify the
                     * position of the tab contents, doesn't influence the tab size
                     * [sets QStyle::SE_ToolBoxTabContents] */
        };

        /**
         * @sa drawKStylePrimitive()
         */
        enum Primitive
        {
            Panel   /**< the panel of a toolbox tab, KStyles default implementation
                     * paints WT_ToolButton/ToolButton::Panel
                     * [implements CE_ToolBoxTab] */
        };
    };


    /**
     * @brief Describes widgets like QToolButton (usually inside a QToolBar).
     * The drawing of ToolButton's is much like that of PushButtons; however, in some cases the widget is configured
     * to not have a separate arrow area, but to incorporate the area inside the button itself. To handle this mode,
     * set InlineMenuIndicatorSize to the size of the arrow,
     * and the InlineMenuIndicatorXOff, and InlineMenuIndicatorYOff to offset from the bottom-right corner to place that
     * rectangle. If InlineMenuIndicatorSize isn't set, the arrow won't be drawn.
     *
     * @sa WT_ToolButton
     */
    struct ToolButton
    {
        /**
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProps
        {
            ContentsMargin,  /**< (\b 5) Margin reserved around the contents size of
                              * a toolbutton. Used to size the contents. */
            FocusMargin            = ContentsMargin + MarginInc,
                             /**< (\b 3) Where the focus rect will be drawn, measured
                              * from the widget sides */
            MenuIndicatorSize, /**< (\b 11) Size for the separate menu arrows on tool buttons
                                 * [sets QStyle::PM_MenuButtonIndicator wheen a toolbutton option is passed in] */
            InlineMenuIndicatorSize = FocusMargin + MarginInc, /**< (\b 0) Size of arrow when it's incorporated into
                                                                * the button directly. If it's 0, it will not be drawn
                                                                * @since 4.1 */
            InlineMenuIndicatorXOff,  /**< Horizontal offset off the bottom-right corner to place the arrow
                                       * if it's incorporated directly, and not in a separate area
                                       * @since 4.1 */
            InlineMenuIndicatorYOff   /**< Vertical offset off the bottom-right corner to place the arrow
                                       * if it's incorporated directly, and not in a separate area
                                       * @since 4.1 */
        };

        /**
         * Relevant Generic elements:
         * - @c Generic::ArrowDown indicating an associated sub-menu
         *
         * @todo Implement CE_ToolButtonLabel to have own Generic::Text, Generic::Icon,
         *       and LayoutProps PressedShiftHorizontal, PressedShiftVertical,
         *       TextToIconSpace, MenuIndicatorSize...
         *
         * @sa drawKStylePrimitive()
         */
        enum Primitive
        {
            Panel           /**< the toolbutton panel
                             * [implements QStyle::PE_PanelButtonTool] */
        };
    };


    /**
     * @brief Describes windows, like in QWorkspace.
     *
     * @todo SP_TitleBar* pixmaps
     *
     * @sa WT_Window
     */
    struct Window
    {
        /**
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProps
        {
            TitleTextColor, ///< (\b ColorMode(QPalette::HighlightedText)) color mode of the titlebar text
            TitleHeight, ///< (\b 20) height of the titlebar [sets QStyle::PM_TitleBarHeight]
            NoTitleFrame, /**< (\b 0) if set to non-zero, the frame primitive is not
                           * expected to paint around the titlebar area
                           * [sets QStyle::SH_TitleBar_NoBorder] */
            TitleMargin,  /**< (\b 2) margin around titlebar contents (buttons,
                           * text label), used to position them and determine the
                           * height of titlebar buttons, doesn't influence size */
            ButtonWidth = TitleMargin + MarginInc, ///< (\b 16) width of a titlebar button
            ButtonSpace,      ///< (\b 2) space between titlebar buttons
            ButtonToTextSpace ///< (\b 3) space between buttons and the title text
        };

        /**
         * Relevant Generic elements:
         * - @c Generic::Text paints the titlebar text label
         * - @c Generic::Frame indicating an associated sub-menu
         *
         * [titlebar elements implement CC_TitleBar]
         *
         * @sa drawKStylePrimitive()
         */
        enum Primitive
        {
            TitlePanel,  ///< whole titlebar panel/background, by KStyle default it's filled with plain highlight color
            ButtonMenu,     ///< system menu button, passes TitleButtonOption
            ButtonMin,      ///< minimize button, passes TitleButtonOption
            ButtonMax,      ///< maximize button, passes TitleButtonOption
            ButtonRestore,  ///< restore button, passes TitleButtonOption @todo split min/max restore?
            ButtonClose,    ///< close button, passes TitleButtonOption
            ButtonShade,    ///< shade button, passes TitleButtonOption
            ButtonUnshade,  ///< button to remove the shade state, passes TitleButtonOption
            ButtonHelp      ///< context help button, passes TitleButtonOption
        };
    };
//@}

    ///Interface for the style to configure various metrics that KStyle has customizable.
    void setWidgetLayoutProp(WidgetType widget, int metric, int value);

    /**
     * @brief Used to obtain information about KStyle layout properties and metrics.
     *
     * The default implementation returns values which are set
     * using setWidgetLayoutProp(), so normally it's not necessary to implement
     * it yourself.
     *
     * @note This method is not meant to be accessible from outside KStyle.
     *
     * @param widgetType the widget type context where the @p metric property belongs to
     * @param metric the value of this property is requested
     * @param opt Qt option parameters
     * @param w the actual widget this call is related to
     */
    virtual int widgetLayoutProp(WidgetType widgetType, int metric,
                                 const QStyleOption* opt = 0, const QWidget* w = 0) const;

    /**
     * @brief Draws primitives which are used inside KStyle.
     *
     * KStyle implements various elements of QStyle::ComplexControl
     * and QStyle::ControlElement for convenience. Usually complex drawing is
     * split into smaller pieces, which can be text, icons, or other KStyle primitives.
     * These are painted by this method.
     *
     * Common Qt option parameters are unpacked for convenience, and information
     * from KStyle are passed as a KStyleOption.
     *
     * @note This method is not meant to be accessible from outside KStyle.
     * @note You should make sure to use the @p r parameter for the rectangle,
     * since the QStyleOption is generally unaltered from the original request,
     * even if layout indicates a different painting rectangle.
     *
     * @param widgetType the widget context in which this call is happening in
     * @param primitive the primitive which should be called. Primitives from the Generic
     * struct are not directly coupled to the @p widgetType , other primitives are usually
     * defined in the struct corresponding to the widget type.
     * @param opt Qt option parameters
     * @param r parameter for the rectangle
     * @param pal the palette extracted from @p opt for convenience
     * @param flags state flags extracted from @p opt for convenience
     * @param p used to draw the primitive
     * @param widget the widget which is painted on
     * @param kOpt information passed from KStyle
     */
    virtual void drawKStylePrimitive(WidgetType widgetType, int primitive, 
                                     const QStyleOption* opt,
                                     const QRect &r, const QPalette &pal,
                                     State flags, QPainter* p,
                                     const QWidget* widget = 0,
                                     Option* kOpt    = 0) const;
private:
    ///Should we use a side text here?
    bool useSideText(const QStyleOptionProgressBar* opt)     const;
    int  sideTextWidth(const QStyleOptionProgressBar* pbOpt) const;

    ///Returns true if the tab is vertical
    bool isVerticalTab (const QStyleOptionTab* tbOpt) const;

    ///Returns true if the tab has reflected layout
    bool isReflectedTab(const QStyleOptionTab* tbOpt) const;

    enum Side
    {
        North,
        East,
        West,
        South
    };

    Side tabSide(const QStyleOptionTab* tbOpt) const;

    ///Returns the tab rectangle adjusted for the tab direction
    QRect marginAdjustedTab(const QStyleOptionTab* tbOpt, int property) const;

    ///Wrapper around visualRect for easier use
    QRect  handleRTL(const QStyleOption* opt, const QRect& subRect) const;
    QPoint handleRTL(const QStyleOption* opt, const QPoint& pos)    const;

    ///Storage for metrics/flags
    QVector<QVector<int> > metrics;
    
    ///Expands out the dimension to make sure it incorporates the margins
    QSize expandDim(const QSize& orig, WidgetType widget, int baseMarginMetric, const QStyleOption* opt, const QWidget* w, bool rotated = false) const;
    
    ///Calculates the contents rectangle by subtracting out the appropriate margins
    ///from the outside
    QRect insideMargin(const QRect &orig, WidgetType widget, int baseMarginMetric, const QStyleOption* opt, const QWidget* w) const;

    ///Internal subrect calculations, for e.g. scrollbar arrows,
    ///where we fake our output to get Qt to do what we want
    QRect internalSubControlRect (ComplexControl control, const QStyleOptionComplex* opt,
                                                    SubControl subControl, const QWidget* w) const;

    // fitt's law label support: QLabel focusing its buddy widget
    const QObject *clickedLabel;

    template<typename T>
    static T extractOptionHelper(T*);

public:
/** @name QStyle Methods
 * These are methods reimplemented from QStyle. Usually it's not necessary to
 * reimplement them yourself.
 *
 * Some of them are there for binary compatibility reasons only; all they do is to call
 * the implementation from QCommonStyle.
 */
//@{
    void drawControl      (ControlElement   elem, const QStyleOption* opt, QPainter* p, const QWidget* w) const;
    void drawPrimitive    (PrimitiveElement elem, const QStyleOption* opt, QPainter* p, const QWidget* w) const;
    int  pixelMetric      (PixelMetric    metric, const QStyleOption* opt = 0, const QWidget* w = 0) const;
    QRect subElementRect  (SubElement    subRect, const QStyleOption* opt, const QWidget* w) const;
    QSize sizeFromContents(ContentsType     type, const QStyleOption* opt,
                                                const QSize& contentsSize, const QWidget* w) const;
    int   styleHint       (StyleHint        hint, const QStyleOption* opt, const QWidget* w,
                                                               QStyleHintReturn* returnData) const;
    QRect subControlRect (ComplexControl control, const QStyleOptionComplex* opt,
                                                    SubControl subControl, const QWidget* w) const;
    SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex* opt,
                                             const QPoint& pt, const QWidget* w) const;
    void       drawComplexControl   (ComplexControl cc, const QStyleOptionComplex* opt,
                                             QPainter *p,      const QWidget* w) const;

    void polish(QWidget *);
    void unpolish(QWidget *);
    void polish(QApplication *);
    void unpolish(QApplication *);
    void polish(QPalette &);
    QRect itemTextRect(const QFontMetrics &fm, const QRect &r,
                           int flags, bool enabled,
                           const QString &text) const;
    QRect itemPixmapRect(const QRect &r, int flags, const QPixmap &pixmap) const;
    void drawItemText(QPainter *painter, const QRect &rect,
                              int flags, const QPalette &pal, bool enabled,
                              const QString &text, QPalette::ColorRole textRole = QPalette::NoRole) const;
    void drawItemPixmap(QPainter *painter, const QRect &rect,
                                int alignment, const QPixmap &pixmap) const;
    QPalette standardPalette() const;
    QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt,
                                   const QWidget *widget = 0) const; //### kde5 remove
    QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap,
                                   const QStyleOption *opt) const;
    bool eventFilter(QObject *, QEvent *);

protected Q_SLOTS:
    int layoutSpacingImplementation(QSizePolicy::ControlType control1,
                    QSizePolicy::ControlType control2, Qt::Orientation orientation,
                    const QStyleOption *option, const QWidget *widget) const;

    QIcon standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *option = 0,
                                     const QWidget *widget = 0) const;
//@}
private:
    KStylePrivate * const d;
};

template<typename T>
const char* kstyleName()
{ return "default"; }

/**
 * Template class which helps implementing the widget style plugin interface.
 *
 * You don't need to use this class directly. Use the following
 * macro (defined in kstyle.h) and you are done:
 * \code
 * K_EXPORT_STYLE("StyleName", StyleClassName)
 * \endcode
 */
template<typename T>
class KStyleFactory: public QStylePlugin
{
    QStringList keys() const
    {
        QStringList l;
        l << kstyleName<T>();
        return l;
    }
    
    QStyle* create(const QString& id)
    {
        QStringList names = keys();
        //check whether included in the keys
        if (names.contains(id, Qt::CaseInsensitive))
            return new T();

        return 0;
    }
};

// get the pointed-to type from a pointer
template<typename T>
T KStyle::extractOptionHelper(T*)
{
    return T();
}

template<typename T>
T KStyle::extractOption(Option* option)
{
    if (option) {
        if (dynamic_cast<T>(option))
            return static_cast<T>(option);
        // Ugly hacks for when RTLD_GLOBAL is not used (quite common with plugins, really)
        // and dynamic_cast fails.
        // This is still partially broken as it doesn't take into account subclasses.
        // ### KDE5 do this somehow differently
        if ( qstrcmp(typeid(*option).name(), typeid(extractOptionHelper(static_cast<T>(0))).name()) == 0 )
            return static_cast<T>(option);
    }

    //### warn if cast failed?

    //since T is a pointer type, need this to get to the static.
    return static_cast<T>(0)->defaultOption();
}

#define K_EXPORT_STYLE(name,type) template<> const char* kstyleName<type>() { return name; } \
    Q_EXPORT_PLUGIN(KStyleFactory<type>)

#endif
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
