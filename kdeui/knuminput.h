/*
 * knuminput.h
 *
 *  Copyright (c) 1997 Patrick Dowler <dowler@morgul.fsh.uvic.ca>
 *  Copyright (c) 1999 Dirk A. Mueller <mueller@kde.org>
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
public:
    KNumInput(QWidget* parent=0, const char* name=0);
    KNumInput(KNumInput* below, QWidget* parent=0, const char* name=0);
    ~KNumInput();

protected:
    KNumInput* m_prev, *m_next;
};

/* ------------------------------------------------------------------------ */

/**
 * KIntNumInput combines a @ref QSpinBox and optionally a @ref QSlider
 * with a label to make an easy to use control for setting some integer
 * parameter. This is especially nice for configuration dialogs,
 * which can have many such combinated controls.
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
 * @short Easy integer parameter entry, with spin and slider.
 * @version $Id$
 */

class KIntNumInput : public KNumInput
{
    Q_OBJECT
public:
    /**
     * Constructor
     *
     * @param label  main label for the control
     * @param lower  lower bound on range
     * @param upper  upper bound on range
     * @param step   step size for the QSlider
     * @param value  initial value for the control
     * @param units  the units for the control (can be empty or 0)
     * @param base   numeric base (default is 10)
     * @param slider whether a slider should be added (default: true)
     * @param parent parent QWidget
     * @param name   internal name for this widget
     */
    KIntNumInput(const QString& label, int lower, int upper, int step, int value,
                 const QString& units, int base = 10, bool slider = true,
                 QWidget *parent=0, const char *name=0);

    /**
     * overloaded constructor, provided for convenience
     *
     */
    KIntNumInput(int lower, int upper, int step, int value, QWidget* parent=0,
                 const QString& label = QString::null, const QString& units = QString::null,
                 bool slider = true, int base = 10, const char* name = 0);

    virtual ~KIntNumInput();

    /**
     * This method returns the minimum size necessary to display the
     * control. The minimum size is enough to show all the labels
     * in the current font (font change may invalidate the return value).
     *
     * @return the minimum size necessary to show the control
     */
    virtual QSize minimumSize() const;

    /**
     * Returns a size which fits the contents of the control.
     *
     * @return the preferred size necessary to show the control
     */
    virtual QSize sizeHint() const;

    /**
     * @return the current value
     */
    int value();

    /**
     * Sets the spacing of tockmarks for the slider.
     *
     * @param minor minor tickmark separation
     * @param major major tickmark separation
     */
    void setSteps(int minor, int major);

    /**
     * Sets the alignment of the main control label. The value label,
     * including the specified units, is always centered under the
     * slider.
     *
     * @param a one of AlignLeft, AlignCenter, AlignRight
     */
    void setLabelAlignment(int a);

    /**
     * Sets the fraction of the controls width taken by the SpinBox.
     * 100-frac is taken by the slider (if exists).
     *
     * @param frac fraction (1..100) of width taken by SpinBox
     *
     * default is 33 (33%)
     */
    void setSpinBoxSize(int frac);

    /**
     * Specifies that this widget may stretch horizontally, but is
     * fixed vertically (like QSpinBox itself)
     */
    QSizePolicy sizePolicy() const;

    /**
     * Sets the special value text. If set, the SpinBox will display
     * this text instead of the numeric value whenever the current
     * value is equal to minVal(). Typically this is used for indicating
     * that the choice has a special (default) meaning.
     */
    void setSpecialValueText(const QString& text);

public slots:
    /**
     * Sets the Widget enabled/disabled
     */
    void setEnabled(bool);

    /**
     * Sets the value of the control.
     */
    void setValue(int);

signals:
    void valueChanged(int);

protected:
    void init(const QString& label, int lower, int upper, int step, int val,
              const QString& units, int _base, bool use_slider);

    void resizeEvent ( QResizeEvent * );

    QLabel*      main_label;
    KIntSpinBox* spin;
    QSlider*     slider;
    QSize        spin_size;

    int label_align;
    int spin_frac;
    int int_value;
};


/* ------------------------------------------------------------------------ */

/**
 * KDoubleNumInput combines a @ref QSpinBox and optionally a @ref QSlider
 * with a label to make an easy to use control for setting some float
 * parameter. This is especially nice for configuration dialogs,
 * which can have many such combinated controls.
 *
 * A special feature of KDoubleNumInput, designed specifically for
 * the situation when there are several KDoubleNumInputs in a column,
 * is that you can specify what portion of the control is taken by the
 * QSpinBox (the remaining portion is used by the slider). This makes
 * it very simple to have all the sliders in a column be the same size.
 *
 * It uses KDoubleValidator validator class. KDoubleNumInput enforces the
 * value to be in the given range.
 */

class KDoubleNumInput : public KNumInput
{
    Q_OBJECT
public:
    /**
     * Constructor
     *
     * @param label  main label for the control
     * @param lower  lower bound on range
     * @param upper  upper bound on range
     * @param step   step size for the QSlider
     * @param value  initial value for the control
     * @param units  the units for the control (can be empty or 0)
     * @param slider whether a slider should be added (default: true)
     * @param format how to display the value (sprintf() format string)
     * @param parent parent QWidget
     * @param name   internal name for this widget
     */
    KDoubleNumInput(double value, QWidget *parent=0, const char *name=0);

    /**
     * adds below after other KDoubleNumInput
     *
     **/

    KDoubleNumInput(KNumInput* below, double value, QWidget* parent=0, const char* name=0);

    /**
     * This method returns the minimum size necessary to display the
     * control. The minimum size is enough to show all the labels
     * in the current font (font change may invalidate the return value).
     *
     * @return the minimum size necessary to show the control
     */
    virtual QSize minimumSize() const;

    /**
     * Returns a size which fits the contents of the control.
     *
     * @return the preferred size necessary to show the control
     */
    virtual QSize sizeHint() const;

    /**
     * Sets the value of the control.
     */
    void setValue(double);

    /**
     * @return the current value
     */
    double value();

    /**
     * Sets the alignment of the main control label. The value label,
     * including the specified units, is always centered under the
     * slider.
     *
     * @param label the text of the label (QString::null to remove an existing one)
     * @param a one of AlignLeft, AlignHCenter, AlignRight and
     *          AlignTop, AlignVCenter, AlignBottom.
     *          default is AlignHCenter | AlignTop.
     */
    void setLabel(QString label, int a = AlignHCenter | AlignTop);

    /**
     * @param lower  lower bound on range
     * @param upper  upper bound on range
     * @param step   step size for the QSlider
     */
    void setRange(double lower, double upper, double step, bool slider=true);

    /**
     * sets the Suffix
     * @param suffix the suffix that should be used. QString::null to disable
     */
    void setSuffix(QString suffix);

    /**
     * sets the Prefix
     * @param prefix the prefix that should be used. QString::null to disable
     */
    void setPrefix(QString prefix);

    /**
     * the Format string that should be used to display the double value.
     *
     * @param format uses the same format as QString::sprintf().
     */
    void setFormat(const char* format);


    /**
     * Specifies that this widget may stretch horizontally, but is
     * fixed vertically (like QSpinBox itself)
     */
    QSizePolicy sizePolicy() const;

    /**
     * Sets the special value text. If set, the SpinBox will display
     * this text instead of the numeric value whenever the current
     * value is equal to minVal(). Typically this is used for indicating
     * that the choice has a special (default) meaning.
     */
    void setSpecialValueText(const QString& text);

signals:
    void valueChanged(double);

protected slots:
    void sliderMoved(int);

protected:
    void init(double value);
    void doLayout();

    void resizeEvent ( QResizeEvent * );
    void resetEditBox();

    QLabel*      main_label;
    QLineEdit*   edit;
    QSlider*     m_slider;
    QLayout*     layout;

    int m_alignment;
    bool m_range;
    double    m_value, m_lower, m_upper, m_step;
    QString   m_units, m_specialvalue, m_prefix, m_suffix, m_format;

    QSize     m_sizeEdit, m_sizeSlider, m_sizeLabel;

    QWidget* m_prev, *m_next;
};


/* ------------------------------------------------------------------------ */

/**
 *  An integer inputline with scrollbar and slider.
 *
 *  The class provides an easy interface to use other
 *  numeric systems then the decimal.
 *
 *  @short An integer inputline with scrollbar and slider.
 */
class KIntSpinBox : public QSpinBox
{
    Q_OBJECT

public:

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
    virtual ~KIntSpinBox() {};

protected:

    /**
     *  Overloaded to make use of the base given in the constructor.
     */
    virtual QString mapValueToText(int);

    /**
     *  Overloaded to make use of the base given in the constructor.
     */
    virtual int mapTextToValue(bool*);

    int val_base;
};


/* --------------------------------------------------------------------------- */


#endif // K_NUMINPUT_H
