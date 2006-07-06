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
 * Makes style coding more convenient and allows to style KDE specific widgets.
 *
 * KStyle strives to ease style layout handling. Based on information obtained
 * from widgetLayoutProperty(), it implements many elements
 * of QStyle::subElementRect(), QStyle::sizeFromContents() (TODO: and
 * QStyle::pixelMetric()?).
 *
 * KStyle implements various elements of QStyle::drawComplexControl()
 * and QStyle::drawControl() for convenience. Usually complex drawing is
 * split into smaller pieces, which can be text, icons, or other KStyle primitives.
 * Drawing of these primitives is done in drawKStylePrimitive(). During this
 * 'composition' of primitives, metrics from widgetLayoutProperty() are
 * respected as well.
 *
 * \par PushButton
 * Draw widgets like QPushButton.
 *
 * Relevant elements:
 * - @c KPE_Generic_Text the button's text
 * - @c KPE_Generic_FocusIndicator indicating that the button has keyboard focus
 * - @c KPE_Generic_ArrowDown indicating that the button has a popup menu associated to it
 * - @c QStyle::PE_PanelButtonCommand Panel of a button, e.g. a QPushButton
 * - @c QStyle::PE_FrameDefaultButton Frame indicating default button. The normal
 *      button panel is drawn inside/above it.
 * - LayoutProperties prefixed with LP_PushButton_
 * - @c WT_PushButton
 * - KStyle implements @c QStyle::CE_PushButtonBevel to compose the above primitives.
 *   Styles usually don't need to implement this.
 *
 * @note some KStyle primitives (KPE_*) here may be called without a QStyleOptionButton. @todo: check if this is actually the case...
 *
 * The layout of a PushButton is structured as follows:
 * -# Between the very outside and the bevel is the default indicator
 *    area, controlled by the @c LP_PushButton_DefaultIndicatorMargin .
 * -# From the bevel, the content and focus rect margins are measured.
 *    Only the content margin is used to size the content area.
 * -# Inside the content area, @c LP_PushButton_MenuIndicatorSize is allocated to the down
 *    arrow if there is a popup menu.
 * -# @c LP_PushButton_TextToIconSpace is allocated between icon and text if both exist
 *
 * @todo create diagrams to illustrate the meaning of the metrics...?
 *
 *
 * \par Splitter
 * Draw widgets like QSplitter.
 *
 * Relevant elements:
 * - @c QStyle::CE_Splitter draws the splitter handle. Interesting flags:
 *     - State_Enabled&&State_MouseOver for mouseOver
 *     - State_Horizontal for orientation
 * - LayoutProperties prefixed with LP_Splitter_
 *
 *
 * \par CheckBox
 * Draw widgets like QCheckBox.
 *
 * Relevant elements:
 * - @c KPE_Generic_Text the CheckBox label alongside the CheckBox
 * - @c KPE_Generic_FocusIndicator the focus indicator. Usually drawn around the
 *      text label. If no label exists, it is drawn around the CheckBox.
 * - @c QStyle::PE_IndicatorCheckBox Check state indicator. Interesting
 *      flags:
 *     - State_NoChange for tristate (neither off nor on),
 *     - else if State_On for checked,
 *     - else not checked
 * - LayoutProperties prefixed with LP_CheckBox_
 * - KStyle implements @c QStyle::CE_CheckBox to compose a CheckBox, using the
 *     mentioned primitives. Styles usually don't need to implement this.
 *
 * @todo code example for the PE_IndicatorCheckBox switch...
 * @todo shouldn't PE_IndicatorCheckBox only render the check-mark and the
 *       checkbox bevel should be painted in CE_CheckBox...?!
 *
 *
 * \par RadioButton
 * Draw widgets like QRadioButton.
 *
 * Relevant elements:
 * - @c KPE_Generic_Text the RadioButton label alongside the RadioButton
 * - @c KPE_Generic_FocusIndicator the keyboard focus indicator
 * - @c QStyle::PE_IndicatorRadioButton Radio button state indicator. Interesting
 *      flags:
 *     - State_On for checked,
 *     - else not checked
 * - LayoutProperties prefixed with LP_RadioButton_
 * - KStyle implements @c QStyle::CE_RadioButton to compose a RadioButton, using the
 *   mentioned primitives. Styles usually don't need to implement this.
 *
 * @todo code example for the PE_IndicatorRadioButton flag switch...
 *
 *
 * \par DockWidgetTitle
 * Draw the title of a dock widget.
 *
 * Relevant elements:
 * - @c KPE_Generic_Text the title text
 * - LayoutProperties prefixed with LP_DockWidgetTitle_
 * - KStylePrimitiveElements prefixed with KPE_DockWidgetTitle_
 * - KStyle implements @c QStyle::CE_DockWidgetTitle to split it into KStyle primitives.
 *
 * @todo need to add a few more layout properties/pixel metric values for dock widgets...
 *
 * @todo Create doxygen modules for each widget for some detailed information...?
 *
 * \par Generic Primitives
 * Basic primitive drawing operations. The only KStylePrimitiveElements which
 * are intended to be used in every WidgetType combination.
 *
 * Relevant elements:
 * - KStylePrimitiveElements prefixed with KPE_Generic_
 *
 * @note the arrows are centering primitives
 * @todo explain what a centering primitive is
 *
 *
 * \par ProgressBar
 * Draw widgets like QProgressBar.
 *
 * Relevant elements:
 * - @c KPE_Generic_Text the progress label
 * - @c QStyle::CE_ProgressBarGroove The groove where progress indicator
 *      is drawn in
 * - LayoutProperties prefixed with LP_ProgressBar_
 * - KStylePrimitiveElements prefixed with KPE_ProgressBar_
 *
 * @todo support for Qt > 4.1 orientation, bottomToTop, invertedAppearance properties!
 *
 *
 * \par MenuBar
 * Draw widgets like QMenuBar.
 *
 * Relevant elements:
 * - @c QStyle::CE_MenuBarEmptyArea Empty area of a menu bar, e.g. background
 *      color or menubar separators (?)...
 * - LayoutProperties prefixed with LP_MenuBar_
 *
 *
 * \par MenuBarItem
 * Draw MenuBar items.
 *
 * Relevant elements:
 * - @c KPE_Generic_Text text appearing as menubar entry
 * - LayoutProperties prefixed with LP_MenuBarItem_
 * - KStylePrimitiveElements prefixed with KPE_MenuBarItem_
 *
 *
 * \par Menu
 * Draw a menu.
 *
 * Relevant elements:
 * - @c KPE_Generic_Frame frame around the menu panel
 * - @c QStyle::CE_MenuTearoff Paints the area where a menu can be teared off
 * - @c QStyle::CE_MenuScroller Scrolling areas in a QMenu
 * - LayoutProperties prefixed with LP_Menu_
 * - KStylePrimitiveElements prefixed with KPE_Menu_
 *
 *
 * \par MenuItem
 * Draw an item in a menu.
 *
 * Relevant elements:
 * - @c KPE_Generic_Text the text of the menu item
 * - @c KPE_Generic_ArrowLeft @c KPE_Generic_ArrowRight arrows indicating a sub-menu
 * - LayoutProperties prefixed with LP_MenuItem_
 * - KStylePrimitiveElements prefixed with KPE_MenuItem_
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
 *
 *
 * \par ScrollBar
 * Draw widgets like QScrollBar.
 *
 * Relevant elements:
 * - The @c KPE_Generic_ arrows
 * - @c QStyle::CE_ScrollBarSlider Interesting flags:
 *     - @c State_Horizontal for orientation,
 *     - @c State_Sunken for pressed state
 * - @c CE_ScrollBarAddPage @c CE_ScrollBarSubPage The scrollbar groove area.
 *      Interesting flags:
 *     - @c State_Horizontal for scrollbar orientation,
 *     - @c State_On&&State_Sunken for pressed state
 * - LayoutProperties prefixed with LP_ScrollBar_
 * - KStylePrimitiveElements prefixed with KPE_ScrollBar_
 *
 * @note Dimensions of LayoutProperties are generally specified with respect
 *       to the vertical scrollbar. Of course, for horizontal ones they're flipped.
 *
 *
 * \par TabBar
 * Draw a tab bar.
 *
 * Relevant elements:
 * - @c KPE_Generic_Text for the TabBar labels
 * - @c KPE_Generic_FocusIndicator for focussed tabs
 * - @c KPE_Generic_Icon for icons associated to tabs
 * - LayoutProperties prefixed with LP_TabBar_
 * - KStylePrimitiveElements prefixed with KPE_TabBar_
 *
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
 *
 * \par TabWidget
 * Draw a tab widget (frame).
 *
 * Relevant elements:
 * - @c KPE_Generic_Frame for the frame/panel of the TabWidget
 * - LayoutProperties prefixed with LP_TabWidget_
 *
 *
 * \par Slider
 * Draw a slider, like QSlider.
 *
 * Relevant elements:
 * - @c KPE_Generic_FocusIndicator indicating keyboard focus
 * - LayoutProperties prefixed with LP_Slider_
 * - KStylePrimitiveElements prefixed with KPE_Slider_
 *
 * @todo Custom slider tickmarks?
 *
 *
 * \par Tree
 * Draw an expandable tree, e.g. in a QListView.
 *
 * Relevant elements:
 * - LayoutProperties prefixed with LP_Tree_
 * - KStylePrimitiveElements prefixed with KPE_Tree_
 *
 * For trees, all the control we provide here is to provide a cap on the size
 * of the expander widget, which is always square. There are 4 primitives to
 * implement: open and closed expander, and horizontal and vertical lines.
 * If you're using dots, it's suggested you use global brush alignment to
 * keep it all nicely aligned.
 *
 * The default implementation has a Windows-like look.
 *
 * @note If you set MaxExpanderSize to a value less than 9, designer will
 *       look funny. The value should also be odd, or value - 1 will be used.
 *
 *
 * \par SpinBox,
 * Draw a widget like QSpinBox.
 *
 * Relevant elements:
 * - @c KPE_Generic_Frame for the area around text input field and buttons
 * - @c KPE_Generic_ArrowUp @c KPE_Generic_ArrowDown drawn on the buttons
 * - LayoutProperties prefixed with LP_SpinBox_
 * - KStylePrimitiveElements prefixed with KPE_SpinBox_
 *
 * @note The description applies to LTR (left to right) mode.
 *
 * -# @c LP_SpinBox_FrameWidth and @c LP_SpinBox_ButtonWidth are used to size
 * the contents area. To the EditField, @c LP_SpinBox_FrameWidth
 * is added at the left, top, bottom while @c LP_SpinBox_ButtonWidth is added
 * at the right.
 * -# The @c LP_SpinBox_ButtonMargin is measured from the right side of the
 * EditField and the outside. Inside it, the up and down buttons are aligned with
 * spacing ButtonSpacing.
 * -# To make sure that both buttons are always of the same height, enable
 * @c LP_SpinBox_SymmetricButtons.
 *
 *
 * \par ComboBox
 * Draw a widget like QComboBox.
 *
 * Relevant elements:
 * - @c KPE_Generic_Frame for the area around text input field and button
 * - @c KPE_Generic_ArrowDown drawn on the button
 * - @c KPE_Generic_FocusIndicator to indicate keyboard focus
 * - LayoutProperties prefixed with LP_ComboBox_
 * - KStylePrimitiveElements prefixed with KPE_ComboBox_
 *
 * @note The description applies to LTR (left to right) mode.
 *
 * -# @c LP_ComboBox_FrameWidth and @c LP_ComboBox_ButtonWidth are used to size
 * the contents area. To the EditField, @c LP_ComboBox_FrameWidth
 * is added at the left, top, bottom while @c LP_ComboBox_ButtonWidth is added
 * at the right.
 * -# The @c LP_ComboBox_ButtonMargin is measured from the right side of the
 * EditField and the outside. Inside it, the button is aligned.
 * -# The FocusMargin is measured from the EditField rect.
 *
 *
 * \par Header
 * Draw a list header, like in QListView.
 *
 * Relevant elements:
 * - @c KPE_Generic_Text for the header text label
 * - @c KPE_Generic_ArrowUp @c KPE_Generic_ArrowDown to indicate the sorting of the column
 * - @c KPE_Generic_FocusIndicator for header drag&drop? (TODO: I'm not sure)
 * - LayoutProperties prefixed with LP_Header_
 * - KStylePrimitiveElements prefixed with KPE_Header_
 *
 * \par LineEdit
 * Draw a text edit widget like QLineEdit.
 *
 * Relevant elements:
 * - @c KPE_Generic_Frame (?)
 * - KStylePrimitiveElements prefixed with KPE_LineEdit_ (Panel)
 *
 * @todo Add property for the specific FrameWidth of a LineEdit?
 * @todo Use the PE_ elements directly instead of KPE_...
 *
 *
 * \par GroupBox
 * Draw something like QGroupBox.
 *
 * Relevant elements:
 * - @c KPE_Generic_Frame
 *
 * @todo What about frame width etc.?
 * @todo Use PE_FrameGroupBox instead...!
 *
 *
 * \par ToolBar
 * Draw a tool bar.
 *
 * Relevant elements:
 * - LayoutProperties prefixed with LP_ToolBar_
 * - KStylePrimitiveElements prefixed with KPE_ToolBar_
 *
 * @todo Use PE_IndicatorToolBarHandle, PE_IndicatorToolBarSeparator,
 *       KPE_ToolBar_Separator instead...!
 *
 *
 * \par ToolButton
 * Draw widgets like QToolButton (usually inside a QToolBar).
 *
 * Relevant elements:
 * - @c KPE_Generic_ArrowDown indicating an associated sub-menu
 * - @c QStyle::PE_PanelButtonTool Panel of a tool button
 *
 * @c LP_ToolButton_ContentsMargin is used to size the contents.
 *
 * @todo Implement CE_ToolButtonLabel to have own Generic::Text, Generic::Icon,
 *       and LayoutProps PressedShiftHorizontal, PressedShiftVertical,
 *       TextToIconSpace, MenuIndicatorSize...
 *
 * \par Maintainer: Sandro Giessl (giessl\@kde.org)
 */

// TODO: From 'Qt4 Themes' discussion on kde-devel
// - Remi Villatel: extend QStyle enums for KColorButton, KColorCombo, KKeyButton, split PE_HeaderSection into KPopupTitle, PopupMenuTitle, TaskContainer)
// - RV: KLineEdit "plays with its colors" - related to KStyle?
// - RV: KMulitTabBarTab ... does it need support from KStyle (instead of manual button rotation etc.)? Or is it dumped already?
// - RV: KTabCtl draws itself mimicking QDrawShadeThingies
// - RV: fixed colors (e.g. Konqueror, KToolbarButton label text ->KPE_ToolbarButton?): To prevent hacks like "preventing any PaletteChange()"... mor related to KDE4 color schemes... I guess
// - Luciano Montanaro: Many apps assume fixed border size; make pixelMetric() used pervasively: Performance and Documentation disadvantage of current KStyle
// - LM: User interface guidelines... related to KStyle?
// - e.g. drawFancyPE() in kdeui for KDE widgets: check "qobject_cast<KStyle*>(style())", or fallback drawing...
// TODO: implement standardIcon().. and what about standardPalette()?
// TODO: maybe the arrow in CE_PushButtonLabel should be painted in CE_PushButtonBevel like QCommonStyle
// TODO: have a look at generic::text... really necessary or is sth like drawItemText() enough?
// TODO: couldn't the option classes be based on QStyleOption?

/*
 ### ### TODO:Where does visualRect fit in? Probably should be done already before calling drawKStylePrimitive?
 ...
All the  basic PE_Primitive calls are also broken down to KStylePrimitive calls by default, as follows:
 
 PE_FocusRect -> WT_Generic, Generic::FocusIndicator
 ### TODO, actually
 
 Note that those not mentionned here are not redirected
*/
class KDEFX_EXPORT KStyle: public QCommonStyle
{
public:
    KStyle();
    
protected:
    ///BEGIN Helper methods

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

    ///END Helper methods

    ///BEGIN Representation of options passed when drawing things

    /**
     A representation for colors. This marshals to and from integers
     to be able to be stored as a widget layout property.
    */
    class KDEFX_EXPORT ColorMode
    {
    public:
        /**
         KStyle understands two kinds of colors:
         1. Palette entries. This means the item must be painted with a specific
            color role from the palette
         2. Auto-selected black or white, dependent on the brightness of a certain
            color role from the palette
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
        // TODO: To bring it in line with the ScrollBar properties:
        // Only None,Top,Bottom for specification and assume a flip
        // for horizontal scrollbars ?? giessl
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

        DoubleButtonOption(ActiveButton ab): activeButton(ab)
        {}
    };
    
    
    ///Option representing text drawing info. For Generic::Text. 
    struct KDEFX_EXPORT TextOption: public OptionBase<TextOption, ColorOption>
    {
        Qt::Alignment        hAlign; //The horizontal alignment
        QString              text;   //The text to draw
        
        TextOption()
        { init(); }

        TextOption(const QString& _text): text(_text)
        { init(); }

        void init()
        {
            hAlign = Qt::AlignLeft; //NOTE: Check BIDI?
        }
    };

    ///END options

    /**
     This enum is used to represent KStyle's concept of
     a widget, and to associate drawing requests and metrics
     with it. The generic value is used for primitives and metrics
     that are common between many widgets
    */
    enum WidgetType
    {
        WT_Generic,       
        WT_PushButton,      ///< Like QPushButton
        WT_Splitter,      
        WT_CheckBox,
        WT_RadioButton,
        WT_DockWidgetTitle,
        WT_ProgressBar,
        WT_MenuBar,
        WT_MenuBarItem,
        WT_Menu,
        WT_MenuItem,
        WT_ScrollBar,
        WT_TabBar,
        WT_TabWidget,
        WT_Slider,
        WT_Tree,
        WT_SpinBox,
        WT_ComboBox,
        WT_Header,
        WT_LineEdit,
        WT_GroupBox,
        WT_ToolBar,
        WT_ToolButton,
        WT_Limit = 0xFFFF ///For enum extensibility
    };

                
    /**
     These constants describe how to access various fields of a margin property.
     For example, to set an additional top margin of 2 pixels, use
     setWidgetLayoutProp(WT_SomeWidget, SomeMargin + Top, 2); // TODO: update example to new API
    */
    enum MarginOffsets
    {
        MainMargin,
        Top,
        Bot,
        Left,
        Right,
        MarginInc
    };

    /**
     * Properties which are used by KStyle to adjust widget layout etc.
     *
     * @sa widgetLayoutProp()
     */
    enum LayoutProperties
    {
        LP_Generic_DefaultFrameWidth, ///< The FrameWidth used by LineEdit, ... TODO: find a better place for the layoutProp... TODO: simple use QStyle pixelMetric()?

        LP_PushButton_ContentsMargin, ///< Space between the bevel and the button contents
        LP_PushButton_FocusMargin = LP_PushButton_ContentsMargin + MarginInc, ///< Used to calculate the area of the focus indicator. Measured from the bevel.
        LP_PushButton_DefaultIndicatorMargin = LP_PushButton_FocusMargin    + MarginInc, ///< Default indicator between the very outside and the bevel. KStyle may reserve this for auto-default buttons, too, for consistency's sake.
        LP_PushButton_PressedShiftHorizontal = LP_PushButton_DefaultIndicatorMargin + MarginInc, ///< Horizontal contents shift for pressed buttons
        LP_PushButton_PressedShiftVertical, ///< Vertical contents shift for pressed buttons
        LP_PushButton_MenuIndicatorSize, ///< Space inside the content area, which is allocated to the down arrow if there is a popup menu
        LP_PushButton_TextToIconSpace, ///< Space between the icon and the text if both exist

        LP_Splitter_Size, ///< Size of the splitter handle

        LP_CheckBox_Size,               ///< Size of the checkbox
        LP_CheckBox_BoxTextSpace,       ///< Space to leave between checkbox and text
        LP_CheckBox_NoLabelFocusMargin, ///< Rectangle to apply to the checkbox rectangle to get where to
                                ///< paint the focus rectangle in case of a labelless checkbox
        LP_CheckBox_FocusMargin = LP_CheckBox_NoLabelFocusMargin + MarginInc,

        LP_RadioButton_Size = LP_CheckBox_FocusMargin + MarginInc,
        LP_RadioButton_BoxTextSpace,

    /** @defgroup group3 The Third Group
         *  This is the third group
     */

        /**
         * @ingroup group3
         * This is some element for focus margin... TODO: fix description.
         */
        LP_RadioButton_FocusMargin,

        LP_DockWidgetTitle_Margin = LP_RadioButton_FocusMargin + MarginInc, ///<Margin for the title: note that this is a symmetric margin always!

        LP_ProgressBar_GrooveMargin = LP_DockWidgetTitle_Margin + MarginInc,        ///<Margin to allocate for the groove. Content area will be inside of it.
        LP_ProgressBar_SideText = LP_ProgressBar_GrooveMargin + MarginInc, ///<Set this to true to have the text positionned to the side
        LP_ProgressBar_SideTextSpace,       ///<Extra space besides that needed for text to allocate to side indicator (on both sides)
        LP_ProgressBar_Precision,           ///<The indicator size will always be a multiple of this (modulo busy indicator size clamping)
        LP_ProgressBar_BusyIndicatorSize,   ///<The busy indicator size, in percent of area size
        LP_ProgressBar_MaxBusyIndicatorSize, ///<Size limit on the busy indicator size;


        LP_MenuBar_Margin,         //Margin rectangle for the contents.
        LP_MenuBar_ItemSpacing = LP_MenuBar_Margin + MarginInc, //Space between items

        LP_MenuBarItem_Margin,                    //Margin rectangle to allocate for any bevel, etc. (Text will be drawn with the inside rect)

        LP_Menu_FrameWidth = LP_MenuBarItem_Margin + MarginInc, ///< The width of the frame, note that this does not affect the layout.
        LP_Menu_Margin,     ///< The margin of the menu
        LP_Menu_ScrollerHeight = LP_Menu_Margin + MarginInc,
        LP_Menu_TearOffHeight,

        LP_MenuItem_Margin,       //Margin for each entry
        LP_MenuItem_CheckAlongsideIcon = LP_MenuItem_Margin + MarginInc, //Set to non-zero to have checkmarks painted separate from icons
        LP_MenuItem_CheckWidth,                      //Size of the checkmark column (CheckAlongsideButton enabled)
        LP_MenuItem_CheckSpace,                      //Space between the checkmark column and the icon column (CheckAlongsideButton enabled)
        LP_MenuItem_IconWidth,                       //Minimum size of the icon column
        LP_MenuItem_IconSpace,                       //Space between the icon column and text one
        LP_MenuItem_AccelSpace,                      //Space between text and accel
        LP_MenuItem_ArrowSpace,                      //Space to reserve for the menu arrow
        LP_MenuItem_ArrowWidth,
        LP_MenuItem_SeparatorHeight,                  //Heigh of separator
        LP_MenuItem_MinHeight,                        //Limit on the size of item content
        LP_MenuItem_ActiveTextColor,                  //Color for active text and arrow
        LP_MenuItem_TextColor,                        //Color for inactive text and arrow
        LP_MenuItem_DisabledTextColor,                //Color for inactive + disabled text and arrow
        LP_MenuItem_ActiveDisabledTextColor,           //Color for active + disabled text and arrow

        LP_ScrollBar_DoubleTopButton, //Set to non-zero to have two buttons on top
        LP_ScrollBar_DoubleBotButton, //Set to non-zero to have two buttons on bottom
        LP_ScrollBar_SingleButtonHeight,
        LP_ScrollBar_DoubleButtonHeight,
        LP_ScrollBar_BarWidth,
        LP_ScrollBar_MinimumSliderHeight, //Note: if the scrollbar is too small to accommodate
                                 //this, this will not be enforced
        LP_ScrollBar_ArrowColor,
        LP_ScrollBar_ActiveArrowColor,

        LP_TabBar_TabContentsMargin,
        LP_TabBar_TabFocusMargin     = LP_TabBar_TabContentsMargin + MarginInc,
        LP_TabBar_TabTextToIconSpace = LP_TabBar_TabFocusMargin    + MarginInc,
        LP_TabBar_TabOverlap, // TODO: PM_TabBarTabOverlap seems to be completely ignored by qt styles/tabbar. remove if it doesn't get fixed.
        LP_TabBar_BaseHeight,        // The height of the tabBar's base. usually the frame width.
        LP_TabBar_BaseOverlap,       // The number of pixels the tabs overlap with the base (i.e. tabWidget frame).
        LP_TabBar_ScrollButtonWidth,  // Buttons which are shown when there's not enough space for tabs.

        LP_TabWidget_FrameWidth,  // TODO: this is ignored for now. See SE_TabWidgetTabContents comment.

        LP_Slider_HandleThickness,                        // The height of a slider in horizontal direction
        LP_Slider_HandleLength,                            // The width of a slider in horizontal direction

        LP_Tree_MaxExpanderSize,

        LP_SpinBox_FrameWidth,
        LP_SpinBox_ButtonWidth,
        LP_SpinBox_ButtonMargin,
        LP_SpinBox_ButtonSpacing = LP_SpinBox_ButtonMargin + MarginInc,
        LP_SpinBox_SymmetricButtons,  // Set to non-zero to make sure both buttons are always of the same
                               // height. To achieve this, the spacing of the buttons will be reduced
                               // by 1 if necessary to avoid rounding problems. Needs to be handled
                               // in your drawing code.
        LP_SpinBox_SupportFrameless,   // Set to non-zero to indicate that you are able to handle frame-less
                               // SpinBoxes. For a SpinBox with no frame, FrameWidth and
                               // Top/Bottom/Right ButtonMargin is ignored.

        LP_ComboBox_FrameWidth,
        LP_ComboBox_ButtonWidth,
        LP_ComboBox_ButtonMargin,
        LP_ComboBox_FocusMargin = LP_ComboBox_ButtonMargin + MarginInc,    // Focus margin for ComboBoxes that aren't editable
        LP_ComboBox_SupportFrameless = LP_ComboBox_FocusMargin + MarginInc, // Set to non-zero to indicate that you are able to handle frame-less
                               // ComboBoxes. For a ComboBox with no frame, FrameWidth and
                               // Top/Bottom/Right ButtonMargin is ignored.

        LP_Header_ContentsMargin,                                // used to size the contents.
        LP_Header_TextToIconSpace = LP_Header_ContentsMargin + MarginInc,  // space that is allocated between icon and text if both exist
        LP_Header_IndicatorSize,

        LP_ToolBar_HandleExtent,       // The width(hor)/height(vert) of a ToolBar handle
        LP_ToolBar_SeparatorExtent,    // The width/height of a ToolBar separator
        LP_ToolBar_ExtensionExtent,    // The width/height of a ToolBar extender, when there is not enough room for toolbar buttons
        LP_ToolBar_PanelFrameWidth,
        LP_ToolBar_ItemMargin,
        LP_ToolBar_ItemSpacing,

        LP_ToolButton_ContentsMargin,
        LP_ToolButton_FocusMargin = LP_ToolButton_ContentsMargin + MarginInc,
        LP_ToolButton_Dummy       = LP_ToolButton_FocusMargin + MarginInc,

        LP_Limit = 0xFFFF ///For enum extensibility...
    };

    /**
     * Primitives which are used by KStyle to paint QStyle elements.
     *
     * @sa drawKStylePrimitive()
     */
    enum KStylePrimitiveElements
    {
        KPE_Generic_Text,  ///< Passes in TextOption
        KPE_Generic_Icon,           ///< Passes in IconOption
        KPE_Generic_FocusIndicator, ///< Indication that this widget has focus
        KPE_Generic_Frame,          ///< Frame around widget
        KPE_Generic_ArrowUp,        ///< Up arrow (pointing up)
        KPE_Generic_ArrowDown,      ///< Down arrow
        KPE_Generic_ArrowRight,     ///< Right arrow
        KPE_Generic_ArrowLeft,       ///< Left arrow

        KPE_DockWidgetTitle_Panel,  ///< The panel/background of the title bar

        KPE_ProgressBar_Indicator,
        KPE_ProgressBar_BusyIndicator,

        KPE_MenuBarItem_Panel,      ///< The panel/background of a menubar item. Interesting flags: State_Selected && State_HasFocus for mouseOver, State_Sunken for pressed state.

        KPE_Menu_Background, //Menu and menu item background

        KPE_MenuItem_CheckColumn, //Background of the checkmark/icon column
        KPE_MenuItem_CheckOn,     //The checkmark - on
        KPE_MenuItem_CheckOff,    //The checkmark - off
        KPE_MenuItem_RadioOn,     //The checkmark of exclusive actions - on
        KPE_MenuItem_RadioOff,    //The checkmark of exclusive actions - off
        KPE_MenuItem_CheckIcon,   //Checked mark, painted below the icon when LayoutProp CheckAlongsideButton is disabled.
        KPE_MenuItem_Separator,   //A separator item.
        KPE_MenuItem_ItemIndicator, //Shows the active item

            //Note: when drawing the double-buttons, you need to check
            //the active subcontrol inside the QStyleOption, to determine
            //which half is active.
        KPE_ScrollBar_SingleButtonVert, //Used to draw a 1-button bevel, vertical
        KPE_ScrollBar_SingleButtonHor,                                //Used to draw a 1-button bevel, horizontal
        KPE_ScrollBar_DoubleButtonVert,                               //Used to draw a 2-button bevel, vertical
        KPE_ScrollBar_DoubleButtonHor,                                //Used to draw a 2-button bevel, horizontal
                                                            //The above 2 are passed a DoubleButtonOption,
                                                            // to say which button is pressed

        KPE_TabBar_EastText, //Special rotated text for east tabs.
        KPE_TabBar_WestText, //Special rotated text for west tabs.
        KPE_TabBar_NorthTab,
        KPE_TabBar_EastTab,
        KPE_TabBar_WestTab,
        KPE_TabBar_SouthTab,
        KPE_TabBar_BaseFrame,
        KPE_TabBar_ScrollButton,

        KPE_Slider_HandleVert,   // A vertical slider handle
        KPE_Slider_HandleHor,    // A horizontal slider handle
        KPE_Slider_GrooveVert,   // A vertical slider groove
        KPE_Slider_GrooveHor,     // A horizontal slider groove

        KPE_Tree_ExpanderClosed,
        KPE_Tree_ExpanderOpen,
        KPE_Tree_HorizontalBranch,
        KPE_Tree_VerticalBranch,

        KPE_SpinBox_EditField,
        KPE_SpinBox_UpButton,
        KPE_SpinBox_DownButton,
        KPE_SpinBox_PlusSymbol,        // SymbolPlus and SymbolMinus are centering primitives
        KPE_SpinBox_MinusSymbol,
        KPE_SpinBox_ButtonArea,         // Are including both of the buttons, painted before them

        KPE_ComboBox_EditField,
        KPE_ComboBox_Button,

        KPE_Header_SectionHor,
        KPE_Header_SectionVert,

        KPE_LineEdit_Panel,     // The panel for (usually disabled) lineedits.

        KPE_ToolBar_Handle, // TODO: handlehor, handlevert?
        KPE_ToolBar_Separator,
        KPE_ToolBar_Panel,

        

        KPE_Limit = 0xFFFF ///For enum extensibility...
    };


    /**
     * @brief Draws primitives which are used inside KStyle.
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

    /**
     * Used to obtain information about KStyle layout properties and metrics.
     *
     * @note This method is not meant to be accessible from outside KStyle.
     *
     * @param metric the value of this property is requested
     * @param opt Qt option parameters. TODO: remove? almost not provided ATM
     * @param widget the actual widget this call is related to. TODO: remove? almost not provided ATM
     */
    virtual int widgetLayoutProp(int metric, const QStyleOption* opt = 0, const QWidget* w = 0) const;
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
    QSize expandDim(QSize orig, int baseMarginMetric, const QStyleOption* opt = 0, const QWidget* w = 0) const;
    
    ///Calculates the contents rectangle by subtracting out the appropriate margins
    ///from the outside
    QRect insideMargin(QRect orig, int baseMarginMetric, const QStyleOption* opt = 0, const QWidget* w = 0) const;

    ///Internal subrect calculations, for e.g. scrollbar arrows,
    ///where we fake our output to get Qt to do what we want
    QRect internalSubControlRect (ComplexControl control, const QStyleOptionComplex* opt,
                                                    SubControl subControl, const QWidget* w) const;

public:
    /*
     The methods below implement the QStyle interface
    */
    /** Reimplemented from QStyle */
    void drawControl      (ControlElement   elem, const QStyleOption* opt, QPainter* p, const QWidget* w) const;
    
    /** Reimplemented from QStyle */
    void drawPrimitive    (PrimitiveElement elem, const QStyleOption* opt, QPainter* p, const QWidget* w) const;
    
    /** Reimplemented from QStyle */
    int  pixelMetric      (PixelMetric    metric, const QStyleOption* opt = 0, const QWidget* w = 0) const;
    
    /** Reimplemented from QStyle */
    QRect subElementRect  (SubElement    subRect, const QStyleOption* opt, const QWidget* w) const;
    
    /** Reimplemented from QStyle */
    QSize sizeFromContents(ContentsType     type, const QStyleOption* opt,
                                                const QSize& contentsSize, const QWidget* w) const;
    
    /** Reimplemented from QStyle */
    int   styleHint       (StyleHint        hint, const QStyleOption* opt, const QWidget* w,
                                                               QStyleHintReturn* returnData) const;
                                                               
    /** Reimplemented from QStyle */
    QRect subControlRect (ComplexControl control, const QStyleOptionComplex* opt,
                                                    SubControl subControl, const QWidget* w) const;

    /** Reimplemented from QStyle */
    SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex* opt,
                                             const QPoint& pt, const QWidget* w) const;

    /** Reimplemented from QStyle */
    void       drawComplexControl   (ComplexControl cc, const QStyleOptionComplex* opt,
                                             QPainter *p,      const QWidget* w) const;
};

template<typename T>
const char* kstyleName()
{ return "default"; }

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
