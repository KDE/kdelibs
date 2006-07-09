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

#include <qcommonstyle.h>
#include <qpalette.h>
#include <qstyleplugin.h>

class QStyleOptionProgressBar;
class QStyleOptionTab;

#include <kdelibs_export.h>

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

/*
 ### ### TODO:Where does visualRect fit in? Probably should be done already before calling drawKStylePrimitive?
 ...
All the  basic PE_Primitive calls are also broken down to KStylePrimitive calls by default, as follows:
 
 PE_FocusRect -> WT_Generic, Generic::FocusIndicator
 ### TODO, actually
*/
class KDEFX_EXPORT KStyle: public QCommonStyle
{
public:
    KStyle();
    
protected:
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
    QRect centerRect(QRect in, int w, int h) const
    {
        return QRect(in.x() + (in.width() - w)/2, in.y() + (in.height() - h)/2, w, h);
    }

    /**
     Return a size-dimension QRect centered inside the 'in' rectangle
    */
    QRect centerRect(QRect in, QSize size) const
    {
        return centerRect(in, size.width(), size.height());
    }
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
    class KDEFX_EXPORT ColorMode
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
        ColorMode(QPalette::ColorRole _role): mode(PaletteEntryMode), role(_role)
        {}

	/// Constructor with explicit color mode and palette roles.
        ColorMode(Mode _mode, QPalette::ColorRole _role): mode(_mode), role(_role)
        {}

        /// Represent as an int to store as a property
        operator int() const
        {
            return int(role) | int(mode);
        }

        /// Decode from an int.
        ColorMode(int encoded)
        {
            mode = (encoded & BWAutoContrastMode) ? BWAutoContrastMode : PaletteEntryMode;
            role = QPalette::ColorRole(encoded & (~BWAutoContrastMode));
        }

	/// Return the color corresponding to our role from the palette,
	/// automatically compensating for the contrast mode.
        QColor color(const QPalette& palette)
        {
            QColor palColor = palette.color(role);

            if (mode == BWAutoContrastMode)
                if (qGray(palColor.rgb()) > 128) //### CHECKME
                    return Qt::black;
                else
                    return Qt::white;
            else
                return palColor;
        }
    };

    
    /**
     Base for our own option classes. 
     The idea here is that Option is the main base, and all the
     public bases inherit off it indirectly using OptionBase,
     which helps implement the default handling
    
     When implementing the actual types, just implement the default ctor,
     filling in defaults, and you're set.
    */
    struct KDEFX_EXPORT Option
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
    struct KDEFX_EXPORT OptionBase: public BaseType
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
    static T extractOption(Option* option)
    {
        if (option && dynamic_cast<T>(option))
            return static_cast<T>(option);
        
        //### warn if cast failed?
        
        //since T is a pointer type, need this to get to the static.
        return static_cast<T>(0)->defaultOption();
    }

    /**
     Option representing the color of the thing to draw. Used for arrows, and for text
     (the latter actually uses TextOption)
    */
    struct KDEFX_EXPORT ColorOption: public OptionBase<ColorOption, Option>
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
    struct KDEFX_EXPORT IconOption: public OptionBase<IconOption, Option>
    {
        bool  active; ///< Is the icon active?
        QIcon icon;   ///< Icon drawn by this option

        IconOption(): active(false)
        {}
    };

    /**
     * Option for drawing double scrollbar buttons, indicating whether
     * a button should be drawn active or not.
     * @sa ScrollBar::Primitive
     */
    struct KDEFX_EXPORT DoubleButtonOption: public OptionBase<DoubleButtonOption, Option>
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
    
    
    ///Option representing text drawing info. For Generic::Text. 
    struct KDEFX_EXPORT TextOption: public OptionBase<TextOption, ColorOption>
    {
        Qt::Alignment        hAlign; ///< The horizontal alignment, default is Qt::AlignLeft
        QString              text;   ///< The text to draw
        
        TextOption()
        { init(); }

        /**
         * Convenience constructor.
         *
         * @param _text initializes the text string property
         */
        TextOption(const QString& _text): text(_text)
        { init(); }

        /**
         * Called by the constructor to set the default value of @c hAlign
         */
        void init()
        {
            hAlign = Qt::AlignLeft; //NOTE: Check BIDI?
        }
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
        WT_DockWidgetTitle, ///< @sa DockWidgetTitle
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
        WT_ToolBar,         ///< @sa ToolBar
        WT_ToolButton,      ///< @sa ToolButton
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
            DefaultFrameWidth   ///< The FrameWidth used by LineEdit, ... [sets QStyle::PM_DefaultFrameWidth]
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
            Frame,          ///< Frame around widget
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
         * @todo create diagrams to illustrate the meaning of the metrics...?
         *
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProp
        {
            ContentsMargin, ///< Space between the bevel and the button contents
            FocusMargin            = ContentsMargin + MarginInc, ///< Used to calculate the area of the focus indicator. Measured from the bevel.
            DefaultIndicatorMargin = FocusMargin    + MarginInc, ///< Default indicator between the very outside and the bevel. KStyle may reserve this for auto-default buttons, too, for consistency's sake. @todo related but unused: PM_ButtonDefaultIndicator
            PressedShiftHorizontal = DefaultIndicatorMargin + MarginInc, ///< Horizontal contents shift for pressed buttons [sets QStyle::PM_ButtonShiftHorizontal]
            PressedShiftVertical, ///< Vertical contents shift for pressed buttons [sets QStyle::PM_ButtonShiftVertical]
            MenuIndicatorSize, ///< Space inside the content area, which is allocated to the down arrow if there is a popup menu [sets QStyle::PM_MenuButtonIndicator]
            TextToIconSpace ///< Space between the icon and the text if both exist
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
     * Describes widgets like QSplitter.
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
            Size ///< Size of the splitter handle [sets QStyle::PM_SplitterWidth] @todo rename?
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
     * Describes widgets like QCheckBox.
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
            Size,               ///< Size of the checkbox [sets PM_IndicatorWidth, PM_IndicatorHeight]
            BoxTextSpace,       ///< Space to leave between checkbox and text
            NoLabelFocusMargin, /**< Rectangle to apply to the checkbox rectangle
                                 * to get where to paint the focus rectangle in
                                 * case of a labelless checkbox */
            FocusMargin = NoLabelFocusMargin + MarginInc ///< Margin around the checkbox contents reserved for the focus rect
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
     * Describes widgets like QRadioButton.
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
     * Describes the title of a dock widget.
     *
     * Relevant elements:
     * - @c Generic::Text the title text
     * - KStyle implements @c QStyle::CE_DockWidgetTitle to split it into KStyle primitives.
     */
    struct DockWidgetTitle
    {
        /**
         * @todo need to add a few more layout properties/pixel metric values for dock widgets...
         *
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProp
        {
            Margin ///<Margin for the title: note that this is a symmetric margin always (only MainMargin is respected)! [sets QStyle::PM_DockWidgetFrameWidth]
        };

        /**
         * @sa drawKStylePrimitive()
         */
        enum Primitive
        {
            Panel       ///< The panel/background of the title bar
        };
    };

    /**
     * Describes widgets like QProgressBar.
     *
     * @todo support for Qt > 4.1 orientation, bottomToTop, invertedAppearance properties!
     */
    struct ProgressBar
    {
        /**
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProp
        {
            GrooveMargin,        ///<Margin to allocate for the groove. Content area will be inside of it.
            SideText = GrooveMargin + MarginInc, ///<Set this to true to have the text positionned to the side
            SideTextSpace,       ///<Extra space besides that needed for text to allocate to side indicator (on both sides)
            Precision,           ///<The indicator size will always be a multiple of this (modulo busy indicator size clamping) [sets QStyle::PM_ProgressBarChunkWidth] @todo rename to ChunkWidth?
            BusyIndicatorSize,   ///<The busy indicator size, in percent of area size
            MaxBusyIndicatorSize ///<Size limit on the busy indicator size;
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
            Margin,                          ///< Margin rectangle for the contents.
            ItemSpacing = Margin + MarginInc ///< Space between items [sets QStyle::PM_MenuBarItemSpacing]
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
     * Describes MenuBar items.
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
            Margin,                    /**< Margin rectangle to allocate for any
                                        * bevel, etc. (Text will be drawn with
                                        * the inside rect) */
            Dummy = Margin + MarginInc //Paranoia about underlying type, TODO: ask maks if this should be done everywhere, giessl
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
     * Describes a menu.
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
            FrameWidth, ///< The width of the frame, note that this does not affect the layout.
            Margin,     ///< The margin of the menu @todo have a look at comments at PM_MenuHMargin...
            ScrollerHeight = Margin + MarginInc, ///< Height of a menu scroller [sets QStyle::PM_MenuScrollerHeight]
            TearOffHeight ///< Height of the TearOff area [sets QStyle::PM_MenuTearoffHeight]
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
     * Describes an item in a menu.
     *
     * Relevant elements:
     * - @c KPE_Generic_Text the text of the menu item
     * - @c KPE_Generic_ArrowLeft @c KPE_Generic_ArrowRight arrows indicating a sub-menu
     *
     * MenuItems are layouted like this:
     * -# There are two MenuItem modes, toggled by @c LP_MenuItem_CheckAlongsideIcon.
     *    Horizontal layout inside the items is as follow:
     *     - @c LP_MenuItem_CheckAlongsideIcon disabled:
     *          |icon/checkmark|IconSpace|text|AccelSpace|accel|ArrowSpace|ArrowWidth|
     *     - @c LP_MenuItem_CheckAlongsideIcon enabled:
     *          |checkmark|CheckSpace|icon|IconSpace|text|AccelSpace|accel|ArrowSpace|ArrowWidth|
     * -# The icon/checkmark column is at least @c LP_MenuItem_CheckWidth wide in all cases.
     * -# Then the margin is applied outside that.
     *
     * @note For the subprimitives the passed rect is their own.
     * @note Background is erased with WT_Menu/Menu::Background.
     */
    struct MenuItem
    {
        /**
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProp
        {
            Margin,       //Margin for each entry
            CheckAlongsideIcon = Margin + MarginInc, //Set to non-zero to have checkmarks painted separate from icons
            CheckWidth,                      //Size of the checkmark column (CheckAlongsideButton enabled)
            CheckSpace,                      //Space between the checkmark column and the icon column (CheckAlongsideButton enabled)
            IconWidth,                       //Minimum size of the icon column
            IconSpace,                       //Space between the icon column and text one
            AccelSpace,                      //Space between text and accel
            ArrowSpace,                      //Space to reserve for the menu arrow
            ArrowWidth,
            SeparatorHeight,                  //Heigh of separator
            MinHeight,                        //Limit on the size of item content
            ActiveTextColor,                  //Color for active text and arrow
            TextColor,                        //Color for inactive text and arrow
            DisabledTextColor,                //Color for inactive + disabled text and arrow
            ActiveDisabledTextColor           //Color for active + disabled text and arrow
        };

        /**
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
     * Describes widgets like QScrollBar.
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
            DoubleTopButton, //Set to non-zero to have two buttons on top
            DoubleBotButton, //Set to non-zero to have two buttons on bottom
            SingleButtonHeight,
            DoubleButtonHeight,
            BarWidth, ///< @todo currently not implemented @todo PM_ScrollBarExtent
            MinimumSliderHeight, //Note: if the scrollbar is too small to accommodate
                                 //this, this will not be enforced
            ArrowColor,
            ActiveArrowColor
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
            SingleButtonVert, //Used to draw a 1-button bevel, vertical
            SingleButtonHor,                                //Used to draw a 1-button bevel, horizontal
            DoubleButtonVert,           /**< Used to draw a 2-button bevel, vertical.
                                         * A DoubleButtonOption is passed to say which
                                         * button is pressed. */
            DoubleButtonHor,            /** @see DoubleButtonVert */
            GrooveAreaVert,   ///< scrollbar groove area, vertical. An interesting flag is @c State_Sunken for pressed state
            GrooveAreaHor,    ///< scrollbar groove area, horizontal. Flags: @c State_Sunken for pressed state
            SliderVert,       ///< scrollbar slider, vertical. Flags: @c State_On&&State_Sunken for pressed state
            SliderHor         ///< scrollbar slider, horizontal. Flags: @c State_On&&State_Sunken for pressed state
        };
    };

    /**
     * Describes a tab bar.
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
            TabContentsMargin,
            TabFocusMargin     = TabContentsMargin + MarginInc,
            TabTextToIconSpace = TabFocusMargin    + MarginInc,
            TabOverlap, // TODO: PM_TabBarTabOverlap seems to be completely ignored by qt styles/tabbar. remove if it doesn't get fixed.
            BaseHeight,        ///< the height of the tabBar's base, usually the frame width [sets QStyle::PM_TabBarBaseHeight]
            BaseOverlap,       ///< the number of pixels the tabs overlap with the base (i.e. tabWidget frame) [sets QStyle::PM_TabBarBaseOverlap]
            ScrollButtonWidth  ///< buttons which are shown when there's not enough space for tabs [sets QStyle::PM_TabBarScrollButtonWidth]
        };

        /**
         * Relevant elements:
         * - @c Generic::Text for the TabBar labels
         * - @c Generic::FocusIndicator for focussed tabs
         * - @c Generic::Icon for icons associated to tabs
         */
        enum Primitive
        {
            EastText,       /**< Special rotated text for east tabs.
                             * @todo implement it in KStyle...? */
            WestText,       ///< @see EastText
            NorthTab,       ///< @todo say something about triangular shape etc.
            EastTab,
            WestTab,
            SouthTab,
            BaseFrame,      ///< [implements QStyle::PE_FrameTabBarBase]
            ScrollButton    ///< [implements PE_IndicatorTabTear] @todo I think this is wrong, PE_IndicatorTabTear seems to be something different from its description, have a closer look (giessl).
        };
    };

    /**
     * Describes a tab widget (frame).
     *
     * Relevant elements:
     * - @c Generic::Frame for the frame/panel of the TabWidget
     */
    struct TabWidget
    {
        /**
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProp
        {
            FrameWidth      // TODO: this is ignored for now. See SE_TabWidgetTabContents comment.
        };
    };

    /**
     * Describes a slider, like QSlider.
     *
     * @todo Custom slider tickmarks?
     */
    struct Slider
    {

        /**
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProp
        {
            HandleThickness,                        // The height of a slider in horizontal direction
            HandleLength                            ///< The width of a slider in horizontal direction [sets QStyle::PM_SliderLength]
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
     * Describes an expandable tree, e.g. in a QListView.
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
            MaxExpanderSize      /**< @note If you set MaxExpanderSize to a value less
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
     * Describes a widget like QSpinBox.
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
            FrameWidth,        /**< Space reserved left, top, bottom of the SpinBox
                                * @todo PM_SpinBoxFrameWidth
                                * @todo merge FrameWidth, ButtonWidth
                                * to a margin, and note that only FrameMargin is
                                * reported with PM_SpinBoxFrameWidth? */
            ButtonWidth,       ///< Space reserved for the widget, right of the EditField
            ButtonMargin,      /**< Where the up/down buttons are located, measured
                                * from right of the edit field and the top/right/bottom
                                * widget edges. */
            ButtonSpacing = ButtonMargin + MarginInc, ///< spacing between up/down buttons
            SymmetricButtons,  /**< Set to non-zero to make sure both buttons are
                                * always of the same height. To achieve this, the
                                * spacing of the buttons will be reduced by 1 if
                                * necessary to avoid rounding problems. Needs to be
                                * handled in your drawing code. */
            SupportFrameless   /**< Set to non-zero to indicate that you are able to
                                * handle frame-less SpinBoxes. For a SpinBox with no
                                * frame, FrameWidth and Top/Bottom/Right ButtonMargin
                                * is ignored. */
        };

        /**
         * Relevant Generic elements:
         * - @c Generic::Frame for the area around text input field and buttons
         * - @c Generic::ArrowUp @c Generic::ArrowDown drawn on the buttons
         */
        enum Primitive
        {
            EditField,          /**< The text input area.
                                 * @todo I think this has only an effect for
                                 * disabled spinboxes, need to test. */
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
     * Describes a widget like QComboBox.
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
            FrameWidth,         /**< @see SpinBox::FrameWidth */
            ButtonWidth,        /**< @see SpinBox::ButtonWidth */
            ButtonMargin,       /**< @see SpinBox::ButtonMargin */
            FocusMargin = ButtonMargin + MarginInc,
                                /**< Focus margin for ComboBoxes that aren't
                                 * editable, measured from the EditField rect */
            SupportFrameless = FocusMargin + MarginInc
                                /**< @see LP_SpinBox_SupportFrameless same description
                                 * applies here */
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
     * Describes a list header, like in QListView.
     */
    struct Header
    {
        /**
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProp
        {
            ContentsMargin,      /**< margin around contents used to size the header. */
            TextToIconSpace = ContentsMargin + MarginInc,
                                 /**< space that is allocated between icon and text
                                  * if both exist
                                  * @todo PM_HeaderMargin is supposed to do the same */
            IndicatorSize        ///< [sets QStyle::PM_HeaderMarkSize] @todo rename to MarkSize?
        };

        /**
         * Relevant Generic elements:
         * - @c Generic::Text for the header text label
         * - @c Generic::ArrowUp @c Generic::ArrowDown to indicate the sorting of the column
         */
        enum Primitive
        {
            SectionHor, ///< @todo CE_HeaderSection @todo merge hor/vert?
            SectionVert
        };
    };

    /**
     * @brief Describes a text edit widget like QLineEdit.
     *
     * @todo Add property for the specific FrameWidth of a LineEdit?
     */
    struct LineEdit
    {
        /**
         * Relevant elements:
         * - @c Generic::Frame (?)
         *
         * @sa drawKStylePrimitive()
         */
        enum Primitive
        {
            Panel     ///< the panel for (usually disabled) lineedits [implements PE_PanelLineEdit]
        };
    };

    /**
     * @brief Describes something like QGroupBox.
     *
     * Relevant Generic elements:
     * - @c Generic::Frame
     *
     * No LayoutProps for now.
     * @sa Generic::FrameWidth
     *
     * @todo What about frame width etc.?
     */
    struct GroupBox
    {
        /**
        No LayoutProps for now.
        */
    };

    /**
     * @brief Describes a tool bar.
     */
    struct ToolBar
    {
        /**
         * @sa setWidgetLayoutProp()
         */
        enum LayoutProps
        {
            HandleExtent,       ///< the width(hor)/height(vert) of a ToolBar handle [sets QStyle::PM_ToolBarHandleExtent]
            SeparatorExtent,    ///< The width/height of a ToolBar separator @todo PM_ToolBarSeparatorExtent @todo not implemented!!
            ExtensionExtent,    ///< The width/height of a ToolBar extender, when there is not enough room for toolbar buttons @todo PM_ToolBarExtensionExtent @todo not implemented!!
            PanelFrameWidth,    ///< [sets QStyle::PM_ToolBarFrameWidth] @todo rename to FrameWidth?
            ItemMargin,         ///< [sets QStyle::PM_ToolBarItemMargin]
            ItemSpacing         ///< [sets QStyle::PM_ToolBarItemSpacing]
        };

        /**
         * @sa drawKStylePrimitive()
         */
        enum Primitive
        {
            Handle, ///< @todo handlehor, handlevert? [implements QStyle::PE_IndicatorToolBarHandle]
            Separator, ///< [implements QStyle::PE_IndicatorToolBarSeparator]
            Panel      ///< [implements QStyle::PE_PanelToolBar]
        };
    };


    /**
     * @brief Describes widgets like QToolButton (usually inside a QToolBar).
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
            ContentsMargin,  /**< Margin reserved around the contents size of
                              * a toolbutton. Used to size the contents. */
            FocusMargin            = ContentsMargin + MarginInc,
                             /**< Where the focus rect will be drawn, measured
                              * from the widget sides */
            DummyProp      = FocusMargin + MarginInc
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
//@}

    ///Interface for the style to configure various metrics that KStyle has customizable.
    void setWidgetLayoutProp(WidgetType widget, int metric, int value);

    /**
     * Used to obtain information about KStyle layout properties and metrics.
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
                                     QRect r, QPalette pal, State flags,
                                     QPainter* p, 
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
    QSize expandDim(QSize orig, WidgetType widget, int baseMarginMetric, const QStyleOption* opt, const QWidget* w) const;
    
    ///Calculates the contents rectangle by subtracting out the appropriate margins
    ///from the outside
    QRect insideMargin(QRect orig, WidgetType widget, int baseMarginMetric, const QStyleOption* opt, const QWidget* w) const;

    ///Internal subrect calculations, for e.g. scrollbar arrows,
    ///where we fake our output to get Qt to do what we want
    QRect internalSubControlRect (ComplexControl control, const QStyleOptionComplex* opt,
                                                    SubControl subControl, const QWidget* w) const;

public:
/** @name QStyle Methods
 * These are methods reimplemented from QStyle. Usually it's not necessary to
 * reimplement them yourself.
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
//@}
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

#define K_EXPORT_STYLE(name,type) template<> const char* kstyleName<type>() { return name; } \
    Q_EXPORT_PLUGIN(KStyleFactory<type>)

#endif
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
