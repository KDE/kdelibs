/*
 * knuminput.h
 *
 *  Copyright (c) 1997 Patrick Dowler <dowler@morgul.fsh.uvic.ca>
 *  Copyright (c) 1999 Dirk A. Mueller <dmuell@gmx.net>
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

/* --------------------------------------------------------------------------- */

class KIntSpinBox : public QSpinBox
{
    Q_OBJECT

public:

    KIntSpinBox(int lower, int upper, int step, int value, int base = 10,
                QWidget* parent = 0, const char* name = 0);
    
    virtual ~KIntSpinBox() {};

protected:
    virtual QString mapValueToText(int);
    virtual int mapTextToValue(bool*);

    int val_base;
};


/* --------------------------------------------------------------------------- */

/**
 * KIntNumInput combines a QSpinbox and optionally a QSlider
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
 * It uses KIntValidator validator class. KIntNumInput enforces the
 * value to be in the given range, and can display it in any base
 * between 2 and 36.
 */

class KIntNumInput : public QWidget
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
    
    virtual ~KIntNumInput();

    /**
     * This method returns the minimum size necessary to display the
     * control. The minimum size is enough to show all the labels
     * in the current font (font change may invalidate the return value).
     * 
     * @return the minimum size necessary to show the control
     */
    QSize minimumSize() const;
    
    /**
     * Sets the value of the control.
     */
    void setValue(int);
    
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
    void setSpecialValueText(const QString& text)
    {
        spin->setSpecialValueText(text); 
    };

    /**
     * Sets the Widget enabled/disabled
     */
    void setEnabled(bool);
    
signals:
    void valueChanged(int);

protected slots:
    void resetValueField(int);
    
protected:
    void resizeEvent ( QResizeEvent * );
  
    QLabel*      main_label;
    KIntSpinBox* spin;
    QSlider*     slider;
    
    int label_align;
    int spin_frac;
    int int_value;
};

#endif // K_NUMINPUT_H
