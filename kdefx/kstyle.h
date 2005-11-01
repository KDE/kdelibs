/**
 * KStyle for KDE4
 * Copyright (C) 2004-2005 Maksim Orlovich <maksim@kde.org>
 * Copyright (C) 2005      Sandro Giessl <sandro@giessl.com>
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
     Retyrn a size-dimension QRect centered inside the 'in' rectangle
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

        ColorMode(QPalette::ColorRole _role): mode(PaletteEntryMode), role(_role)
        {}

        ColorMode(Mode _mode, QPalette::ColorRole _role): mode(_mode), role(_role)
        {}

        ///Represent as an int to store as a property
        operator int() const
        {
            return int(role) | int(mode);
        }

        ///Decode from an int.
        ColorMode(int encoded)
        {
            mode = (encoded & BWAutoContrastMode) ? BWAutoContrastMode : PaletteEntryMode;
            role = QPalette::ColorRole(encoded & (~BWAutoContrastMode));
        }

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
        bool  active;
        QIcon icon;

        IconOption(): active(false)
        {}
    };

    struct KDEFX_EXPORT DoubleButtonOption: public OptionBase<DoubleButtonOption, Option>
    {
        enum ActiveButton
        {
            None,
            Top,
            Left,
            Right,
            Bottom
        };

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
        WT_PushButton,    ///Push button and similar
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
     setWidgetLayoutProp(WT_SomeWidget, SomeMargin + Top, 2);
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
       
    ///Basic primitives, which may be used with everything
    struct Generic
    {
        enum LayoutProp
        {
            DefaultFrameWidth   // The FrameWidth used by LineEdit, ... TODO: find a better place for the layoutProp
        };

        enum Primitive
        {
            Bevel = 0xFFFF,
            Text,        //Passes in TextOption
            Icon,        //Passes in IconOption
            FocusIndicator,
            Frame,
            ArrowUp,    //Note: the arrows are centering primitives
            ArrowDown,
            ArrowRight,
            ArrowLeft
        };
    };
    
    ///Metrics, primitives, etc., relevant for rendering buttons
    struct PushButton
    {
        /*
         Push button. These are structured as follows:
         
         1. Between the very outside and the bevel is the default indicator 
         area, controlled by the DefaultIndicatorMargin. KStyle may reserve
         this for auto-default buttons, too, for consistency's sake.
         
         2. From the bevel, the content and focus rect margins are measured.
         Only the content margin is used to size the content area.
         
         Inside the content area, MenuIndicatorSize is allocated to the down
         arrow if there is a popup menu.
         
         TextToIconSpace is allocated between icon and text if both exist
         
         Relevant generic primitives:
            Bevel, Text, FocusIndicator, ArrowDown
         
         Note that some primitives here may be called without a QStyleOptionButton. 
        */
        enum LayoutProp
        {
            ContentsMargin,
            FocusMargin            = ContentsMargin + MarginInc, 
            DefaultIndicatorMargin = FocusMargin    + MarginInc,
            PressedShiftHorizontal = DefaultIndicatorMargin + MarginInc,
            PressedShiftVertical,
            MenuIndicatorSize,
            TextToIconSpace
        };
        
        enum Primitive
        {
            DefaultButtonBevel
        };
    };

    /**
      For splitters, Bevel is used; the Size metric specifies the size
    */        
    struct Splitter
    {
        
        enum LayoutProp
        {
            Size //Size of the splitter
        };
    };
    
    /**
     From default primitives, Text and FocusIndicator are used
     */
    struct CheckBox
    {
        enum LayoutProp
        {
            Size,               //Size of the checkbox
            BoxTextSpace,       //Space to leave between checkbox and text
            NoLabelFocusMargin, //Rectangle to apply to the checkbox rectangle to get where to
                                //paint the focus rectangle in case of a labelless checkbox
            FocusMargin = NoLabelFocusMargin + MarginInc
        };
        
        enum Primitive
        {
            CheckOff,
            CheckOn,
            CheckTriState
        };
    };

    /**
    From default primitives, Text and FocusIndicator are used
     */
    struct RadioButton
    {
        //See CheckBox for description of the metrics
        enum LayoutProp
        {
            Size,
            BoxTextSpace,
            FocusMargin
        };
        
        enum Primitive
        {
            RadioOff,
            RadioOn
        };
    };
    
    /**
     From default primitives, Text, Bevel are used
    */
    struct DockWidgetTitle
    {
        enum LayoutProp
        {
            Margin //Margin for the title: note that this is a symmetric margin always!
        };
    };
    
    /**
     From default primitives, Text, Bevel are used
    */
    struct ProgressBar
    {
        enum LayoutProp
        {
            GrooveMargin,        //Margin to allocate for the groove. Content area will be inside of it.
            SideText = GrooveMargin + MarginInc, //Set this to true to have the text positionned to the side
            SideTextSpace,       //Extra space besides that needed for text to allocate to side indicator (on both sides)
            Precision,           //The indicator size will always be a multiple of this (modulo busy indicator size clamping)
            BusyIndicatorSize,   //The busy indicator size, in percent of area size
            MaxBusyIndicatorSize //Size limit on the busy indicator size;
            
        };
    
        enum Primitive
        {
            Indicator,
            BusyIndicator
        };
    };

    /**
     From default primitives, Bevel is used
    */
    struct MenuBar
    {
        enum LayoutProp
        {
            Margin,         //Margin rectangle for the contents.
            ItemSpacing = Margin + MarginInc //Space between items
        };
    };

    /**
     From default primitives, Text & Bevel are used
    */
    struct MenuBarItem
    {
        enum LayoutProp
        {
            Margin,                    //Margin rectangle to allocate for any bevel, etc. (Text will be drawn with the inside rect)
            Dummy = Margin + MarginInc //Paranoia about underlying type
        };
    };

    /**
     From default primitives, Frame is used
    */
    struct Menu
    {
        enum LayoutProp
        {
            FrameWidth, //The width of the frame, note that this does not affect the layout.
            Margin,     //The margin of the menu
            ScrollerHeight = Margin + MarginInc,
            TearOffHeight
        };

        enum Primitive
        {
            Background, //Menu and menu item background
            TearOff,
            Scroller
        };
    };

    /**
     Note: bg is erased with WT_Menu/Menu::Background.
     The Generic::Text, Generic::ArrowLeft/Right primitives
     are also used

     There are two MenuItem modes, toggled by CheckAlongsideButton.
     Horizontal layout inside the items is as follow:
     - CheckAlongsideButton disabled:

                  |icon/checkmark|IconSpace|text|AccelSpace|accel|ArrowSpace|ArrowWidth|

     - Layout with CheckAlongsideButton enabled:

       |checkmark|CheckSpace|icon|IconSpace|text|AccelSpace|accel|ArrowSpace|ArrowWidth|

     The icon/checkmark column is at least CheckWidth wide in all cases.

     Then the margin is applied outside that

     Note that for the subprimitives the passed rect is their
     own, 
    */
    struct MenuItem
    {
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

        enum Primitive
        {
            CheckColumn, //Background of the checkmark/icon column
            CheckOn,     //The checkmark - on
            CheckOff,    //The checkmark - off
            RadioOn,     //The checkmark of exclusive actions - on
            RadioOff,    //The checkmark of exclusive actions - off
            CheckIcon,   //Checked mark, painted below the icon when LayoutProp CheckAlongsideButton is disabled.
            Separator,   //A separator item.
            ItemIndicator //Shows the active item
        };
    };

    /**
        Generic primitives used: the arrows
    */
    struct ScrollBar
    {
        /**
          Note: dimensions are generally specified with respect to the vertical scrollbar.
          Of course, for horizontal ones they're flipped
        */
        enum LayoutProp
        {
            DoubleTopButton, //Set to non-zero to have two buttons on top
            DoubleBotButton, //Set to non-zero to have two buttons on bottom
            SingleButtonHeight,
            DoubleButtonHeight,
            BarWidth,
            MinimumSliderHeight, //Note: if the scrollbar is too small to accomodate
                                 //this, this will not be enforced
            ArrowColor,
            ActiveArrowColor
        };

        enum Primitive
        {
            //Note: when drawing the double-buttons, you need to check
            //the active subcontrol inside the QStyleOption, to determine
            //which half is active.
            SingleButtonVert, //Used to draw a 1-button bevel, vertical
            SingleButtonHor,                                //Used to draw a 1-button bevel, horizontal
            DoubleButtonVert,                               //Used to draw a 2-button bevel, vertical
            DoubleButtonHor,                                //Used to draw a 2-button bevel, horizontal
                                                            //The above 2 are passed a DoubleButtonOption,
                                                            // to say which button is pressed
            GrooveAreaVert,                                 //### is this enough, or also provide split version?
            GrooveAreaHor,
            SliderVert,
            SliderHor
        };
    };

    struct TabBar
    {
        /**
         Each tab is basically built hiearchically out of the following areas:

         Content area:
            Icon <- TextToIconSpace -> Text
            -or- Icon -or- Text
         Bevel:
            ContentsMargin outside of the content area
         Focus indicator is placed FocusMargin inside the bevel

         The side tabs just have those rotated, bottom tabs have
         the margins reversed
        */
        enum LayoutProp
        {
            TabContentsMargin,
            TabFocusMargin     = TabContentsMargin + MarginInc,
            TabTextToIconSpace = TabFocusMargin    + MarginInc,
            TabOverlap, // TODO: PM_TabBarTabOverlap seems to be completely ignored by qt styles/tabbar. remove if it doesn't get fixed.
            BaseHeight,        // The height of the tabBar's base. usually the frame width.
            BaseOverlap,       // The number of pixels the tabs overlap with the base (i.e. tabWidget frame).
            ScrollButtonWidth  // Buttons which are shown when there's not enough space for tabs.
        };
    
        /**
         From generic primitives, Text, FocusIndicator, Icon are also used
        */
        enum Primitive
        {
            EastText, //Special rotated text for east tabs.
            WestText, //Special rotated text for west tabs.
            NorthTab,
            EastTab,
            WestTab,
            SouthTab,
            BaseFrame,
            ScrollButton
        };
    };

    struct TabWidget
    {
        /**

        */
        enum LayoutProp
        {
            FrameWidth      // TODO: this is ignored for now. See SE_TabWidgetTabContents comment.
        };

        /**
         From generic primitives, Frame is used
        */
    };

    struct Slider
    {
        // TODO: custom slider tickmarks?

        enum LayoutProp
        {
            HandleThickness,                        // The height of a slider in horizontal direction
            HandleLength                            // The width of a slider in horizontal direction
        };

        /**
         From generic primitives, FocusIndicator is used
        */
        enum Primitive
        {
            HandleVert,   // A vertical slider handle
            HandleHor,    // A horizontal slider handle
            GrooveVert,   // A vertical slider groove
            GrooveHor     // A horizontal slider groove
        };
    };

    /**
     For trees, all the control we provide here is to provide a cap on the size
     of the expander widget, which is always square. There are 4 primitives to
     implement: open and closed expander, and horizontal and vertical lines.
     If you're using dots, it's suggested you use global brush alignment to
     keep it all nicely aligned.

     The default implementation has a Windows-like look

     Note: if you set MaxExpanderSize to a value less than 9, designer will
     look funny. The value should also be odd, or value - 1 will be used.
    */
    struct Tree
    {
        enum LayoutProp
        {
            MaxExpanderSize
        };
    
        enum Primitive
        {
            ExpanderClosed,
            ExpanderOpen,
            HorizontalBranch,
            VerticalBranch
        };
    };

    struct SpinBox
    {
        /**
         The description applies to LTR mode.

         FrameWidth and ButtonWidth are used to size of the contents area. To
         the EditField, FrameWidth is added at the left, top, bottom while
         ButtonWidth is added at the right.

         The ButtonMargin is measured from the right side of the EditField and
         the outside. Inside it, the up and down buttons are aligned with
         spacing ButtonSpacing.

         To make sure that both buttons are always of the same height, 
        */
        enum LayoutProp
        {
            FrameWidth,
            ButtonWidth,
            ButtonMargin,
            ButtonSpacing = ButtonMargin + MarginInc,
            SymmetricButtons,  // Set to non-zero to make sure both buttons are always of the same
                               // height. To achieve this, the spacing of the buttons will be reduced
                               // by 1 if necessary to avoid rounding problems. Needs to be handled
                               // in your drawing code.
            SupportFrameless   // Set to non-zero to indicate that you are able to handle frame-less
                               // SpinBoxes. For a SpinBox with no frame, FrameWidth and
                               // Top/Bottom/Right ButtonMargin is ignored.
        };

        /**
         From generic primitives, Frame, ArrowUp and ArrowDown are also used
        */
        enum Primitive
        {
            EditField,
            UpButton,
            DownButton,
            PlusSymbol,        // SymbolPlus and SymbolMinus are centering primitives
            MinusSymbol,
            ButtonArea         // Are including both of the buttons, painted before them
        };
    };

    struct ComboBox
    {
        /**
        The description applies to LTR mode.

        FrameWidth and ButtonWidth are used to size of the contents area. To
        the EditField, FrameWidth is added at the left, top, bottom while
        ButtonWidth is added at the right.

        The ButtonMargin is measured from the right side of the EditField and
        the outside. Inside it, the button is aligned.

        The FocusMargin is measured from the EditField rect.

        To make sure that both buttons are always of the same height,
         */
        enum LayoutProp
        {
            FrameWidth,
            ButtonWidth,
            ButtonMargin,
            FocusMargin = ButtonMargin + MarginInc,    // Focus margin for ComboBoxes that aren't editable
            SupportFrameless = FocusMargin + MarginInc // Set to non-zero to indicate that you are able to handle frame-less
                               // ComboBoxes. For a ComboBox with no frame, FrameWidth and
                               // Top/Bottom/Right ButtonMargin is ignored.
        };

        /**
        From generic primitives, Frame, ArrowDown and FocusIndicator are also used
         */
        enum Primitive
        {
            EditField,
            Button
        };
    };

    struct Header
    {
        enum LayoutProp
        {
            ContentsMargin,                                // used to size the contents.
            TextToIconSpace = ContentsMargin + MarginInc,  // space that is allocated between icon and text if both exist
            IndicatorSize
        };

        /**
        From generic primitives, text, ArrowUp, ArrowDown and FocusIndicator are also used
         */
        enum Primitive
        {
            SectionHor,
            SectionVert
        };
    };

    struct LineEdit
    {
        /**
        No LayoutProps for now.
        TODO: Add FrameWidth property...
        */

        /**
        From generic primitives, Frame is used
         */
        enum Primitive
        {
            Panel     // The panel for (usually disabled) lineedits.
        };
    };

    struct GroupBox
    {
        /**
        No LayoutProps for now.
        */

        /**
        From generic primitives, Frame is used
         */
    };

    struct ToolBar
    {
        enum LayoutProps
        {
            HandleExtent,       // The width(hor)/height(vert) of a ToolBar handle
            SeparatorExtent,    // The width/height of a ToolBar separator
            ExtensionExtent,    // The width/height of a ToolBar extender, when there is not enough room for toolbar buttons
            PanelFrameWidth,
            ItemMargin,
            ItemSpacing
        };

        enum Primitive
        {
            Handle, // TODO: handlehor, handlevert?
            Separator,
            Panel
        };
    };

    struct ToolButton
    {
        /**
        ContentsMargin is used to size the contents.
        */
        enum LayoutProps
        {
            ContentsMargin,
            FocusMargin            = ContentsMargin + MarginInc,
            DummyProp      = FocusMargin + MarginInc
        };

        // TODO: implement CE_ToolButtonLabel to have own Generic::Text,
        //        Generic::Icon, and LayoutProps PressedShiftHorizontal,
        //        PressedShiftVertical, TextToIconSpace, MenuIndicatorSize...

        /**
        From generic primitives, Bevel, ArrowDown are used.
        */
    };

    ///Interface for the style to configure various metrics that KStyle has customizable.
    void setWidgetLayoutProp(WidgetType widget, int metric, int value);
    
    /**
     This is called to draw things, with the common Qt option parameters unpacked for convenience, and information
     from KStyle passed as a KStyleOption.
      Note that you should make sure to use the r parameter for the rectangle,
     since the QStyleOption is generally unaltered from the original request, even if layout indicates
     a different painting rectangle
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
    
    int widgetLayoutProp(WidgetType widget, int metric) const;
    
    ///Expands out the dimension to make sure it incorporates the margins
    QSize expandDim(QSize orig, WidgetType widget, int baseMarginMetric) const;
    
    ///Calculates the contents rectangle by subtracting out the appropriate margins
    ///from the outside
    QRect insideMargin(QRect orig, WidgetType widget, int baseMarginMetric) const;

    ///Internal subrect calculations, for e.g. scrollbar arrows,
    ///where we fake our output to get Qt to do what we want
    QRect internalSubControlRect (ComplexControl control, const QStyleOptionComplex* opt,
                                                    SubControl subControl, const QWidget* w) const;

public:
    /*
     The methods below implement the QStyle interface
    */
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
