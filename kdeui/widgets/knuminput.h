/* This file is part of the KDE libraries
 *  Copyright (c) 1997 Patrick Dowler <dowler@morgul.fsh.uvic.ca>
 *  Copyright (c) 2000 Dirk Mueller <mueller@kde.org>
 *  Copyright (c) 2002 Marc Mutz <mutz@kde.org>
 *
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
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef K_NUMINPUT_H
#define K_NUMINPUT_H

#include <kdeui_export.h>

#include <QtGui/QWidget>
#include <QtGui/QSpinBox>

class QSlider;
class QSpinBox;
class QValidator;

class KIntSpinBox;
class KNumInputPrivate;
class KLocalizedString;

/**
 * You need to inherit from this class if you want to implement K*NumInput
 * for a different variable type
 *
 */
class KDEUI_EXPORT KNumInput : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString label READ label WRITE setLabel )
public:
    /**
     * Default constructor
     * @param parent If parent is 0, the new widget becomes a top-level
     * window. If parent is another widget, this widget becomes a child
     * window inside parent. The new widget is deleted when its parent is deleted.
     */
    explicit KNumInput(QWidget* parent=0);

    /**
     * @param below A pointer to another KNumInput.
     * @param parent parent widget
     * \deprecated - use the version without the below parameter instead
     */
    KDE_CONSTRUCTOR_DEPRECATED KNumInput(QWidget *parent, KNumInput* below);
    
    /**
     * Destructor
     */
    ~KNumInput();

    /**
     * Sets the text and alignment of the main description label.
     *
     * @param label The text of the label.
     *              Use QString() to remove an existing one.
     *
     * @param a The alignment of the label (Qt::Alignment).
     *          Default is @p Qt:AlignLeft | @p Qt:AlignTop.
     *
     * The vertical alignment flags have special meaning with this
     * widget:
     *
     *     @li @p Qt:AlignTop     The label is placed above the edit/slider
     *     @li @p Qt:AlignVCenter The label is placed left beside the edit
     *     @li @p Qt:AlignBottom  The label is placed below the edit/slider
     *
     */
    virtual void setLabel(const QString & label, Qt::Alignment a = Qt::AlignLeft | Qt::AlignTop);

    /**
     * @return the text of the label.
     */
    QString label() const;

    /**
     * @return if the num input has a slider.
     */
    bool showSlider() const;

    /**
     * Sets the spacing of tickmarks for the slider.
     *
     * @param minor Minor tickmark separation.
     * @param major Major tickmark separation.
     */
    void setSteps(int minor, int major);

    /**
     * Returns a size which fits the contents of the control.
     *
     * @return the preferred size necessary to show the control
     */
    virtual QSize sizeHint() const;

protected:
    /**
     * @return the slider widget.
     * @internal
     */
    QSlider *slider() const;
      
    /**
     * Call this function whenever you change something in the geometry
     * of your KNumInput child.
     *
     */
    void layout(bool deep);

    /**
     * You need to overwrite this method and implement your layout
     * calculations there.
     *
     * See KIntNumInput::doLayout and KDoubleNumInput::doLayout implementation
     * for details.
     *
     */
    virtual void doLayout() = 0;

private:
    friend class KNumInputPrivate;
    KNumInputPrivate * const d;
    
    Q_DISABLE_COPY(KNumInput)
};

/* ------------------------------------------------------------------------ */

/**
 * @short An input widget for integer numbers, consisting of a spinbox and a slider.
 *
 * KIntNumInput combines a QSpinBox and optionally a QSlider
 * with a label to make an easy to use control for setting some integer
 * parameter. This is especially nice for configuration dialogs,
 * which can have many such combinated controls.
 *
 * The slider is created only when the user specifies a range
 * for the control using the setRange function or when the user
 * calls setSliderEnabled.
 *
 * A special feature of KIntNumInput, designed specifically for
 * the situation when there are several KIntNumInputs in a column,
 * is that you can specify what portion of the control is taken by the
 * QSpinBox (the remaining portion is used by the slider). This makes
 * it very simple to have all the sliders in a column be the same size.
 *
 * It uses KIntValidator validator class. KIntNumInput enforces the
 * value to be in the given range, and can display it in any base
 * between 2 and 36.
 *
 * \image html kintnuminput.png "KDE Int Number Input Spinbox"
 */

class KDEUI_EXPORT KIntNumInput : public KNumInput
{
    Q_OBJECT
    Q_PROPERTY( int value READ value WRITE setValue USER true )
    Q_PROPERTY( int minimum READ minimum WRITE setMinimum )
    Q_PROPERTY( int maximum READ maximum WRITE setMaximum )
    Q_PROPERTY( int singleStep READ singleStep WRITE setSingleStep )
    Q_PROPERTY( int referencePoint READ referencePoint WRITE setReferencePoint )
    Q_PROPERTY( double relativeValue READ relativeValue WRITE setRelativeValue )
    Q_PROPERTY( QString suffix READ suffix WRITE setSuffix )
    Q_PROPERTY( QString prefix READ prefix WRITE setPrefix )
    Q_PROPERTY( QString specialValueText READ specialValueText WRITE setSpecialValueText )
    Q_PROPERTY( bool sliderEnabled READ showSlider WRITE setSliderEnabled )

public:
    /**
     * Constructs an input control for integer values
     * with base 10 and initial value 0.
     */
    explicit KIntNumInput(QWidget *parent=0);
    /**
     * Constructor
     * It constructs a QSpinBox that allows the input of integer numbers
     * in the range of -INT_MAX to +INT_MAX. To set a descriptive label,
     * use setLabel(). To enforce the value being in a range and optionally to
     * attach a slider to it, use setRange().
     *
     * @param value  initial value for the control
     * @param base   numeric base used for display
     * @param parent parent QWidget
     */
    explicit KIntNumInput(int value, QWidget *parent=0,int base = 10);

    /**
     * Constructor
     *
     * the difference to the one above is the "below" parameter. It tells
     * this instance that it is visually put below some other KNumInput widget.
     * Note that these two KNumInput's need not to have the same parent widget
     * or be in the same layout group.
     * The effect is that it'll adjust its layout in correspondence
     * with the layout of the other KNumInput's (you can build an arbitrary long
     * chain).
     *
     * @param below  append KIntNumInput to the KNumInput chain
     * @param value  initial value for the control
     * @param base   numeric base used for display
     * @param parent parent QWidget
     * 
     * \deprecated use the version without the below parameter instead.
     */
    KDE_CONSTRUCTOR_DEPRECATED KIntNumInput(KNumInput* below, int value, QWidget *parent, int base = 10);

    /**
     * Destructor
     *
     *
     */
    virtual ~KIntNumInput();

    /**
     * @return the current value.
     */
    int value() const;

    /**
     * @return the curent value in units of the referencePoint.
     */
    double relativeValue() const;

    /**
     * @return the current reference point
     */
    int referencePoint() const;

    /**
     * @return the suffix displayed behind the value.
     * @see setSuffix()
     */
    QString suffix() const;
    /**
     * @return the prefix displayed in front of the value.
     * @see setPrefix()
     */
    QString prefix() const;
    /**
     * @return the string displayed for a special value.
     * @see setSpecialValueText()
     */
    QString specialValueText() const;

    /**
     * Sets the allowed input range and the step size for the slider and the
     * spin box.
     *
     * @param min  minimum value
     * @param max  maximum value
     * @param step step size
     */
    void setRange(int min, int max, int singleStep=1);

    /**
     * @deprecated Use the other setRange function and setSliderEnabled instead
     */
    KDE_DEPRECATED void setRange(int min, int max, int singleStep, bool slider);

    /**
      * @param enabled Show the slider
      * @default enabled
      */
    void setSliderEnabled(bool enabled=true);

    /**
     * Sets the minimum value.
     */
    void setMinimum(int min);
    /**
     * @return the minimum value.
     */
    int minimum() const;
    /**
     * Sets the maximum value.
     */
    void setMaximum(int max);
    /**
     * @return the maximum value.
     */
    int maximum() const;

    /**
     * @return the step of the spin box
     */
    int singleStep() const;

    /**
     * @return the step of the spin box
     */
    void setSingleStep(int step);

    /**
     * Sets the special value text. If set, the SpinBox will display
     * this text instead of the numeric value whenever the current
     * value is equal to minVal(). Typically this is used for indicating
     * that the choice has a special (default) meaning.
     */
    void setSpecialValueText(const QString& text);

    virtual void setLabel(const QString & label, Qt::Alignment a = Qt::AlignLeft | Qt::AlignTop);

    /**
     * This method returns the minimum size necessary to display the
     * control. The minimum size is enough to show all the labels
     * in the current font (font change may invalidate the return value).
     *
     * @return the minimum size necessary to show the control
     */
    virtual QSize minimumSizeHint() const;
    
public Q_SLOTS:
    /**
     * Sets the value of the control.
     */
    void setValue(int);

    /**
     * Sets the value in units of the referencePoint
     */
    void setRelativeValue(double);

    /**
     * Sets the reference point for relativeValue.
     */
    void setReferencePoint(int);

    /**
     * Sets the suffix to @p suffix.
     * Use QString() to disable this feature.
     * Formatting has to be provided (e.g. a space separator between the
     * prepended @p value and the suffix's text has to be provided
     * as the first character in the suffix).
     *
     * @see QSpinBox::setSuffix(), #setPrefix()
     */
    void setSuffix(const QString &suffix);

    /**
     * Sets the suffix to @p suffix.
     * Use this to add a plural-aware suffix, e.g. by using ki18np("singular", "plural").
     *
     * @since 4.3
     */
    void setSuffix(const KLocalizedString &suffix);

    /**
     * Sets the prefix to @p prefix.
     * Use QString() to disable this feature.
     * Formatting has to be provided (see above).
     *
     * @see QSpinBox::setPrefix(), #setSuffix()
     */
    void setPrefix(const QString &prefix);

    /**
     * sets focus to the edit widget and marks all text in if mark == true
     *
     */
    void setEditFocus( bool mark = true );

Q_SIGNALS:
    /**
     * Emitted every time the value changes (by calling setValue() or
     * by user interaction).
     */
    void valueChanged(int);

    /**
     * Emitted whenever valueChanged is. Contains the change
     * relative to the referencePoint.
     */
    void relativeValueChanged(double);

private Q_SLOTS:
    void spinValueChanged(int);
    void slotEmitRelativeValueChanged(int);

protected:
    /**
     * @return the spin box widget.
     * @internal
     */
    QSpinBox *spinBox() const;
      
    virtual void doLayout();
    void resizeEvent ( QResizeEvent * );

private:
    void init(int value, int _base);

private:
    class KIntNumInputPrivate;
    friend class KIntNumInputPrivate;
    KIntNumInputPrivate * const d;
    
    Q_DISABLE_COPY(KIntNumInput)
};


/* ------------------------------------------------------------------------ */

class KDoubleLine;

/**
 * @short An input control for real numbers, consisting of a spinbox and a slider.
 *
 * KDoubleNumInput combines a QSpinBox and optionally a QSlider
 * with a label to make an easy to use control for setting some float
 * parameter. This is especially nice for configuration dialogs,
 * which can have many such combinated controls.
 *
 * The slider is created only when the user specifies a range
 * for the control using the setRange function with the slider
 * parameter set to "true".
 *
 * A special feature of KDoubleNumInput, designed specifically for
 * the situation when there are several instances in a column,
 * is that you can specify what portion of the control is taken by the
 * QSpinBox (the remaining portion is used by the slider). This makes
 * it very simple to have all the sliders in a column be the same size.
 *
 * \image html kdoublenuminput.png "KDE Double Number Input Spinbox"
 *
 * @see KIntNumInput
 */

class KDEUI_EXPORT KDoubleNumInput : public KNumInput
{
    Q_OBJECT
    Q_PROPERTY( double value READ value WRITE setValue USER true )
    Q_PROPERTY( double minimum READ minimum WRITE setMinimum )
    Q_PROPERTY( double maximum READ maximum WRITE setMaximum )
    Q_PROPERTY( double singleStep READ singleStep WRITE setSingleStep )
    Q_PROPERTY( QString suffix READ suffix WRITE setSuffix )
    Q_PROPERTY( QString prefix READ prefix WRITE setPrefix )
    Q_PROPERTY( QString specialValueText READ specialValueText WRITE setSpecialValueText )
    Q_PROPERTY( int decimals READ decimals WRITE setDecimals )
    Q_PROPERTY( double referencePoint READ referencePoint WRITE setReferencePoint )
    Q_PROPERTY( double relativeValue READ relativeValue  WRITE setRelativeValue )
    Q_PROPERTY( bool sliderEnabled READ showSlider WRITE setSliderEnabled )
    Q_PROPERTY( double exponentRatio READ exponentRatio WRITE setExponentRatio )

public:
    /**
     * Constructs an input control for double values
     * with initial value 0.00.
     */
    explicit KDoubleNumInput(QWidget *parent = 0);

    /**
     * Constructor
     *
     * @param lower lower boundary value
     * @param upper upper boundary value
     * @param value  initial value for the control
     * @param singleStep   step size to use for up/down arrow clicks
     * @param precision number of digits after the decimal point
     * @param parent parent QWidget
     */
    KDoubleNumInput(double lower, double upper, double value, QWidget *parent=0,double singleStep=0.01,
		    int precision=2);

    /**
     * destructor
     */
    virtual ~KDoubleNumInput();


    /**
     * Constructor
     *
     * the difference here is the "below" parameter. It tells this
     * instance that it is visually put below some other KNumInput
     * widget.  Note that these two KNumInput's need not to have the
     * same parent widget or be in the same layout group.  The effect
     * is that it'll adjust its layout in correspondence with the
     * layout of the other KNumInput's (you can build an arbitrary long
     * chain).
     *
     * @param below  append KDoubleNumInput to the KDoubleNumInput chain
     * @param lower lower boundary value
     * @param upper upper boundary value
     * @param value  initial value for the control
     * @param singleStep   step size to use for up/down arrow clicks
     * @param precision number of digits after the decimal point
     * @param parent parent QWidget
     *
     * \deprecated use the version without below instead
     */
    KDE_CONSTRUCTOR_DEPRECATED KDoubleNumInput(KNumInput* below,
		    double lower, double upper, double value, QWidget *parent=0,double singleStep=0.02,
		    int precision=2);

    /**
     * @return the current value.
     */
    double value() const;

    /**
     * @return the suffix.
     * @see setSuffix()
     */
    QString suffix() const;

    /**
     * @return the prefix.
     * @see setPrefix()
     */
    QString prefix() const;

    /**
     * @return number of decimals.
     * @see setDecimals()
     */
    int decimals() const;

    /**
     * @return the string displayed for a special value.
     * @see setSpecialValueText()
     */
    QString specialValueText() const;

     /**
     * @param min  minimum value
     * @param max  maximum value
     * @param singleStep step size for the QSlider
     * @param slider whether the slider is created or not
     */
    void setRange(double min, double max, double singleStep=1, bool slider=true);

    /**
      * @param enabled Show the slider
      * @default enabled
      */
    void setSliderEnabled(bool enabled);

    /**
     * Sets the minimum value.
     */
    void setMinimum(double min);
    /**
     * @return the minimum value.
     */
    double minimum() const;
    /**
     * Sets the maximum value.
     */
    void setMaximum(double max);
    /**
     * @return the maximum value.
     */
    double maximum() const;

    /**
     * @return the step of the spin box
     */
    double singleStep() const;

    /**
     * @return the step of the spin box
     */
    void setSingleStep(double singleStep);

    /**
     * Specifies the number of digits to use.
     */
    void setDecimals(int decimals);

    KDE_DEPRECATED void setPrecision(int precision) { setDecimals(precision); }

    /**
     * @return the reference point for relativeValue calculation
     */
    double referencePoint() const;

    /**
     * @return the current value in units of referencePoint.
     */
    double relativeValue() const;

    /**
     * Sets the special value text. If set, the spin box will display
     * this text instead of the numeric value whenever the current
     * value is equal to minVal(). Typically this is used for indicating
     * that the choice has a special (default) meaning.
     */
    void setSpecialValueText(const QString& text);

    virtual void setLabel(const QString & label, Qt::Alignment a = Qt::AlignLeft | Qt::AlignTop);
    virtual QSize minimumSizeHint() const;

    /**
     * @return the value of the exponent use to map the slider to the
     *         spin box.
     */
    double exponentRatio() const;

    /**
     * @param dbl the value of the exponent use to map the slider to the
     *         spin box (dbl need to be strictly positive).
     */
    void setExponentRatio(double dbl);
public Q_SLOTS:
    /**
     * Sets the value of the control.
     */
    void setValue(double);

    /**
     * Sets the value in units of referencePoint.
     */
    void setRelativeValue(double);

    /**
     * Sets the reference Point to @p ref. It @p ref == 0, emitting of
     * relativeValueChanged is blocked and relativeValue
     * just returns 0.
     */
    void setReferencePoint(double ref);

    /**
     * Sets the suffix to be displayed to @p suffix. Use QString() to disable
     * this feature. Note that the suffix is attached to the value without any
     * spacing. So if you prefer to display a space separator, set suffix
     * to something like " cm".
     * @see setSuffix()
     */
    void setSuffix(const QString &suffix);

    /**
     * Sets the prefix to be displayed to @p prefix. Use QString() to disable
     * this feature. Note that the prefix is attached to the value without any
     * spacing.
     * @see setPrefix()
     */
    void setPrefix(const QString &prefix);

Q_SIGNALS:
    /**
     * Emitted every time the value changes (by calling setValue() or
     * by user interaction).
     */
    void valueChanged(double);
    /**
     * This is an overloaded member function, provided for
     * convenience. It essentially behaves like the above function.
     *
     * Contains the value in units of referencePoint.
     */
    void relativeValueChanged(double);

private Q_SLOTS:
    void sliderMoved(int);
    void spinBoxChanged(double);
    void slotEmitRelativeValueChanged(double);

protected:
    virtual void doLayout();
    void resizeEvent ( QResizeEvent * );

    friend class KDoubleLine;
private:
    void init(double value, double lower, double upper,
    double singleStep, int precision);
    double mapSliderToSpin(int) const;
    void updateLegacyMembers();

private:
    class KDoubleNumInputPrivate;
    friend class KDoubleNumInputPrivate;
    KDoubleNumInputPrivate * const d;

    Q_DISABLE_COPY(KDoubleNumInput)
};


/* ------------------------------------------------------------------------ */

/**
 *  @short A QSpinBox with support for arbitrary base numbers.
 *
 *  A QSpinBox with support for arbitrary base numbers
 *  (e.g. hexadecimal).
 *
 *  The class provides an easy interface to use other
 *  numeric systems than the decimal.
 *
 * \image html kintspinbox.png "KDE Integer Input Spinboxes with hexadecimal and binary input"
 */
class KDEUI_EXPORT KIntSpinBox : public QSpinBox
{
    Q_OBJECT
    Q_PROPERTY( int base READ base WRITE setBase )

public:

    /**
     *  Constructor.
     *
     *  Constructs a widget with an integer inputline with a little scrollbar
     *  and a slider, with minimal value 0, maximal value 99, step 1, base 10
     *  and initial value 0.
     */
    explicit KIntSpinBox( QWidget *parent = 0 );

    /**
     *  Constructor.
     *
     *  Constructs a widget with an integer inputline with a little scrollbar
     *  and a slider.
     *
     *  @param lower  The lowest valid value.
     *  @param upper  The greatest valid value.
     *  @param singleStep   The step size of the scrollbar.
     *  @param value  The actual value.
     *  @param base   The base of the used number system.
     *  @param parent The parent of the widget.
     */
    KIntSpinBox(int lower, int upper, int singleStep, int value, QWidget *parent,int base = 10);

    /**
     *  Destructor.
     */
    virtual ~KIntSpinBox();

    /**
     * Sets the base in which the numbers in the spin box are represented.
     */
    void setBase(int base);
    /**
     * @return the base in which numbers in the spin box are represented.
     */
    int base() const;
    /**
     * sets focus and optionally marks all text
     *
     */
    void setEditFocus(bool mark);

    /**
     * Sets the suffix to @p suffix.
     * Use this to add a plural-aware suffix, e.g. by using ki18np("singular", "plural").
     *
     * @since 4.3
     */
    void setSuffix(const KLocalizedString &suffix);

    using QSpinBox::setSuffix;

protected:

    /**
     *  Overloaded the method in QSpinBox
     *  to make use of the base given in the constructor.
     */
    virtual QString textFromValue(int) const;

    /**
     *  Overloaded the method in QSpinBox
     *  to make use of the base given in the constructor.
     */
    virtual int valueFromText(const QString &text) const;

private:
    class KIntSpinBoxPrivate;
    friend class KIntSpinBoxPrivate;
    KIntSpinBoxPrivate *const d;
    
    Q_DISABLE_COPY(KIntSpinBox)
    Q_PRIVATE_SLOT(d, void updateSuffix(int))
};

#endif // K_NUMINPUT_H
