/*
 * knuminput.h
 *
 *  Copyright (c) 1997 Patrick Dowler <dowler@morgul.fsh.uvic.ca>
 *  Copyright (c) 2000 Dirk A. Mueller <mueller@kde.org>
 *
 *  Requires the Qt widget libraries, available at no cost at
 *  http://www.troll.no/
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#ifndef K_NUMINPUT_H
#define K_NUMINPUT_H

#include <qwidget.h>
#include <qspinbox.h>

class QLabel;
class QSlider;
class QLineEdit;
class QLayout;

class KIntSpinBox;

/* ------------------------------------------------------------------------ */

/**
 * You need to inherit from this class if you want to implement K*NumInput
 * for a different variable type
 *
 */
class KNumInput : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString label READ label WRITE setLabel )
public:
    /**
     * Default constructor
     *
     */
    KNumInput(QWidget* parent=0, const char* name=0);

    /**
     * @param below A pointer to another KNumInput.
     *
     */
    KNumInput(KNumInput* below, QWidget* parent=0, const char* name=0);
    ~KNumInput();

    /**
     * Sets the text and alignment of the main description label.
     *
     * @param label The text of the label.
     *              Use QString::null to remove an existing one.
     *
     * @param a one of @p AlignLeft, @p AlignHCenter, YAlignRight and
     *          @p AlignTop, @p AlignVCenter, @p AlignBottom.
     *          default is @p AlignLeft | @p AlignTop.
     *
     * The vertical alignment flags have special meaning with this
     * widget:
     *
     *     @li @p AlignTop     The label is placed above the edit/slider
     *     @li @p AlignVCenter The label is placed left beside the edit
     *     @li @p AlignBottom  The label is placed below the edit/slider
     *
     */
    virtual void setLabel(const QString & label, int a = AlignLeft | AlignTop);

    /**
     * @return the text of the label.
     */
    QString label() const;

    /**
     * @return if the num input has a slider.
     */
    bool showSlider() const { return m_slider; }

    /**
     * Sets the spacing of tickmarks for the slider.
     *
     * @param minor Minor tickmark separation.
     * @param major Major tickmark separation.
     */
    void setSteps(int minor, int major);

    /**
     * Specifies that this widget may stretch horizontally, but is
     * fixed vertically (like @ref QSpinBox itself).
     */
    QSizePolicy sizePolicy() const;

    /**
     * Returns a size which fits the contents of the control.
     *
     * @return the preferred size necessary to show the control
     */
    virtual QSize sizeHint() const;

protected:
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

    KNumInput* m_prev, *m_next;
    int m_colw1, m_colw2;

    QLabel*  m_label;
    QSlider* m_slider;
    QSize    m_sizeSlider, m_sizeLabel;

    int      m_alignment;

private:
    void init();

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KNumInputPrivate;
    KNumInputPrivate *d;
};

/* ------------------------------------------------------------------------ */

/**
 * KIntNumInput combines a @ref QSpinBox and optionally a @ref QSlider
 * with a label to make an easy to use control for setting some integer
 * parameter. This is especially nice for configuration dialogs,
 * which can have many such combinated controls.
 *
 * The slider is created only when the user specifies a range
 * for the control using the setRange function with the slider
 * parameter set to "true".
 *
 * A special feature of KIntNumInput, designed specifically for
 * the situation when there are several KIntNumInputs in a column,
 * is that you can specify what portion of the control is taken by the
 * QSpinBox (the remaining portion is used by the slider). This makes
 * it very simple to have all the sliders in a column be the same size.
 *
 * It uses @ref KIntValidator validator class. KIntNumInput enforces the
 * value to be in the given range, and can display it in any base
 * between 2 and 36.
 *
 * @short An input widget for integer numbers, consisting of a spinbox and a slider.
 * @version $Id$
 */

class KIntNumInput : public KNumInput
{
    Q_OBJECT
    Q_PROPERTY( int value READ value WRITE setValue )
    Q_PROPERTY( int minValue READ minValue WRITE setMinValue )
    Q_PROPERTY( int maxValue READ maxValue WRITE setMaxValue )
    Q_PROPERTY( QString suffix READ suffix WRITE setSuffix )
    Q_PROPERTY( QString prefix READ prefix WRITE setPrefix )
    Q_PROPERTY( QString specialValueText READ specialValueText WRITE setSpecialValueText )

public:
    /**
     * Constructs an input control for integer values
     * with base 10 and initial value 0.
     */
    KIntNumInput(QWidget *parent=0, const char *name=0);
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
     * @param name   internal name for this widget
     */
    KIntNumInput(int value, QWidget* parent=0, int base = 10, const char *name=0);

    /**
     * Constructor
     *
     * the difference to the one above is the "below" parameter. It tells
     * this instance that it is visually put below some other KNumInput widget.
     * Note that these two KNumInput's need not to have the same parent widget
     * or be in the same layout group.
     * The effect is that it'll adjust it's layout in correspondence
     * with the layout of the other KNumInput's (you can build an arbitary long
     * chain).
     *
     * @param below  append KIntNumInput to the KNumInput chain
     * @param value  initial value for the control
     * @param base   numeric base used for display
     * @param parent parent QWidget
     * @param name   internal name for this widget
     */
    KIntNumInput(KNumInput* below, int value, QWidget* parent=0, int base = 10, const char *name=0);

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
     * @return the suffix displayed behind the value.
     * @see #setSuffix()
     */
    QString suffix() const;
    /**
     * @return the prefix displayed in front of the value.
     * @see #setPrefix()
     */
    QString prefix() const;
    /**
     * @return the string displayed for a special value.
     * @see #setSpecialValueText()
     */
    QString specialValueText() const;

    /**
     * @param min  minimum value
     * @param max  maximum value
     * @param step step size for the QSlider
     */
    void setRange(int min, int max, int step=1, bool slider=true);
    /**
     * Sets the minimum value.
     */
    void setMinValue(int min);
    /**
     * @return the minimum value.
     */
    int minValue() const;
    /**
     * Sets the maximum value.
     */
    void setMaxValue(int max);
    /**
     * @return the maximum value.
     */
    int maxValue() const;

    /**
     * Sets the special value text. If set, the SpinBox will display
     * this text instead of the numeric value whenever the current
     * value is equal to minVal(). Typically this is used for indicating
     * that the choice has a special (default) meaning.
     */
    void setSpecialValueText(const QString& text);

    /**
     * @reimplemented
     */
    virtual void setLabel(const QString & label, int a = AlignLeft | AlignTop);

    /**
     * This method returns the minimum size necessary to display the
     * control. The minimum size is enough to show all the labels
     * in the current font (font change may invalidate the return value).
     *
     * @return the minimum size necessary to show the control
     */
    virtual QSize minimumSizeHint() const;

public slots:
    /**
     * Sets the value of the control.
     */
    void setValue(int);

    /**
     * Sets the suffix to @p suffix.
     * Use QString::null to disable this feature.
     * Formatting has to be provided (e.g. a space separator between the
     * prepended @p value and the suffix's text has to be provided
     * as the first character in the suffix).
     *
     * @see QSpinBox::setSuffix(), #setPrefix()
     */
    void setSuffix(const QString &suffix);

    /**
     * Sets the prefix to @p prefix.
     * Use QString::null to disable this feature.
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

signals:
    /**
     * Emitted every time the value changes (by calling @ref setValue() or
     * by user interaction).
     */
    void valueChanged(int);

private slots:
    void spinValueChanged(int);

protected:
    /**
     * @reimplemented
     */
    virtual void doLayout();
    /**
     * @reimplemented
     */
    void resizeEvent ( QResizeEvent * );

    KIntSpinBox* m_spin;
    QSize        m_sizeSpin;

private:
    void init(int value, int _base);

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KIntNumInputPrivate;
    KIntNumInputPrivate *d;
};


/* ------------------------------------------------------------------------ */

class KDoubleLine;

/**
 * KDoubleNumInput combines a @ref QSpinBox and optionally a @ref QSlider
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
 * It uses @ref KFloatValidator validator class. KDoubleNumInput enforces the
 * value to be in the given range.
 *
 * @see KIntNumInput
 * @short An input control for real numbers, consisting of a spinbox and a slider.
 */
class KDoubleNumInput : public KNumInput
{
    Q_OBJECT
    Q_PROPERTY( double value READ value WRITE setValue )
    Q_PROPERTY( double minValue READ minValue WRITE setMinValue )
    Q_PROPERTY( double maxValue READ maxValue WRITE setMaxValue )
    Q_PROPERTY( QString suffix READ suffix WRITE setSuffix )
    Q_PROPERTY( QString prefix READ prefix WRITE setPrefix )
    Q_PROPERTY( QString specialValueText READ specialValueText WRITE setSpecialValueText )

public:
    /**
     * Constructs an input control for double values
     * with initial value 0.0.
     */
    KDoubleNumInput(QWidget *parent=0, const char *name=0);
    /**
     * Constructor
     *
     * @param value  initial value for the control
     * @param parent parent QWidget
     * @param name   internal name for this widget
     */
    KDoubleNumInput(double value, QWidget *parent=0, const char *name=0);

    /**
     * destructor
     */
    virtual ~KDoubleNumInput();

    /**
     * Constructor
     *
     * put it below other KNumInput
     *
     * @param  below
     * @param  value  initial value for the control
     * @param  parent parent QWidget
     * @param  name   internal name for this widget
     **/
    KDoubleNumInput(KNumInput* below, double value, QWidget* parent=0, const char* name=0);

    /**
     * @return the current value.
     */
    double value() const;
    /**
     * @return the suffix.
     * @see #setSuffix()
     */
    QString suffix() const;
    /**
     * @return the prefix.
     * @see #setPrefix()
     */
    QString prefix() const;
    /**
     * @return the precision.
     * @see #setPrecision()
     */
    int precision() const;
    /**
     * @return the string displayed for a special value.
     * @see #setSpecialValueText()
     */
    QString specialValueText() const { return m_specialvalue; }

     /**
     * @param min  minimum value
     * @param max  maximum value
     * @param step step size for the QSlider
     */
    void setRange(double min, double max, double step=1, bool slider=true);
    /**
     * Sets the minimum value.
     */
    void setMinValue(double min);
    /**
     * @return the minimum value.
     */
    double minValue() const;
    /**
     * Sets the maximum value.
     */
    void setMaxValue(double max);
    /**
     * @return the maximum value.
     */
    double maxValue() const;

    /**
     * Specifies the number of digits to use.
     */
    void setPrecision(int precision);

    /**
     * Sets the special value text. If set, the spin box will display
     * this text instead of the numeric value whenever the current
     * value is equal to @ref #minVal(). Typically this is used for indicating
     * that the choice has a special (default) meaning.
     */
    void setSpecialValueText(const QString& text);

    /**
     * @reimplemented
     */
    virtual void setLabel(const QString & label, int a = AlignLeft | AlignTop);
    /**
     * @reimplemented
     */
    virtual QSize minimumSizeHint() const;
    /**
     * @reimplemented
     */
    virtual bool eventFilter(QObject*, QEvent*);

public slots:
    /**
     * Sets the value of the control.
     */
    void setValue(double);

    /**
     * Sets the suffix to be displayed to @p suffix. Use QString::null to disable
     * this feature. Note that the suffix is attached to the value without any
     * spacing. So if you prefer to display a space separator, set suffix
     * to something like " cm".
     * @see #setSuffix()
     */
    void setSuffix(const QString &suffix);

    /**
     * Sets the prefix to be displayed to @p prefix. Use QString::null to disable
     * this feature. Note that the prefix is attached to the value without any
     * spacing.
     * @see #setPrefix()
     */
    void setPrefix(const QString &prefix);

signals:
    /**
     * Emitted every time the value changes (by calling @ref setValue() or
     * by user interaction).
     */
    void valueChanged(double);

private slots:
    void sliderMoved(int);

protected:

    /**
     * @reimplemented
     */
    virtual void doLayout();
    /**
     * @reimplemented
     */
    void resizeEvent ( QResizeEvent * );
    virtual void resetEditBox();

    KDoubleLine*   edit;

    bool     m_range;
    double   m_lower, m_upper, m_step;

    QSize    m_sizeEdit;

    friend class KDoubleLine;

private:
    void init(double value);
    QString  m_specialvalue, m_prefix, m_suffix;
    double   m_value;
    short    m_precision;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KDoubleNumInputPrivate;
    KDoubleNumInputPrivate *d;
};


/* ------------------------------------------------------------------------ */

/**
 *  An integer inputline with scrollbar and slider.
 *
 *  The class provides an easy interface to use other
 *  numeric systems then the decimal.
 *
 *  @short A spin box widget for non-decimal numbers.
 */
class KIntSpinBox : public QSpinBox
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
    KIntSpinBox( QWidget *parent=0, const char *name=0);

    /**
     *  Constructor.
     *
     *  Constructs a widget with an integer inputline with a little scrollbar
     *  and a slider.
     *
     *  @param lower  The lowest valid value.
     *  @param upper  The greatest valid value.
     *  @param step   The step size of the scrollbar.
     *  @param value  The actual value.
     *  @param base   The base of the used number system.
     *  @param parent The parent of the widget.
     *  @param name   The Name of the widget.
     */
    KIntSpinBox(int lower, int upper, int step, int value, int base = 10,
                QWidget* parent = 0, const char* name = 0);

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

protected:

    /**
     *  Overloaded the method in QSpinBox
     *  to make use of the base given in the constructor.
     */
    virtual QString mapValueToText(int);

    /**
     *  Overloaded the method in QSpinBox
     *  to make use of the base given in the constructor.
     */
    virtual int mapTextToValue(bool*);

private:
    int val_base;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KIntSpinBoxPrivate;
    KIntSpinBoxPrivate *d;
};


/* --------------------------------------------------------------------------- */


#endif // K_NUMINPUT_H
