/*
 * kslidercontrol.h
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

#ifndef K_SLIDER_CONTROL_H
#define K_SLIDER_CONTROL_H

#include <qwidget.h>
#include <qslider.h>

class QLabel;
class QSize;
class QSpinBox;


/**
 * KSliderControl combines a QSlider and a pair of labels to make an
 * easy to use control for setting some integer parameter. This is
 * especially nice for configuration dialogs, which can have many
 * such combination controls.
 *
 * A special feature of KSliderControl, designed specifically for
 * the situation when there are several KSliderControls in a column,
 * is that you can specify what portion of the control is taken by the
 * main label (the remaining portion is used by the slider). This makes
 * it very simple to have all the sliders in a column be the same size.
 * 
 */

class KSliderControl: public QWidget
{
    Q_OBJECT
public:
    /**
     * Constructor
     *
     * @param label main label for the control
     * @param lower lower bound on range
     * @param upper upper bound on range
     * @param step  step size for the QSlider
     * @param value initial value for the control
     * @param units the units for the control (can be QString::null or empty)
     * @param parent parent QWidget
     * @param name internal name for this widget
     */
    KSliderControl(const QString& label, 
                   const int lower, const int upper,
                   const int step, const int value,
                   const QString& units, 
                   QWidget *parent=0, const char *name=0);
    
    ~KSliderControl();

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
     * Sets the fraction of the controls width taken by the main label.
     * 1-frac is taken by the slider and value label.
     *
     * @param frac fraction (1..100) of width taken by main label
     */
    void setLabelSize(int frac);

signals:
    /**
     * Is emitted when user has changed value
     */
    void valueChanged(int);
        
protected slots:
   void resetValueField(int);
    
protected:

   void computeMinimumSize();
   void resizeEvent ( QResizeEvent * );
  
   QLabel *main_label;
   QSlider *slider;
   QString unit_str;
   QSize qs;
   QSpinBox *spinbox;
   
   int h_spacing, v_spacing;
   int label_align;
   int spin_frac;
   
   int slider_value;
};


inline void KSliderControl::setSteps(int minor, int major)
{
    slider->setSteps( minor, major );
}


inline void KSliderControl::setValue(int val)
{
    slider->setValue(val);
}

inline int KSliderControl::value()
{
    return slider_value;
}

inline void KSliderControl::setLabelAlignment(int a)
{
    label_align = a;
}

inline void KSliderControl::setLabelSize(int frac)
{
    spin_frac = QMIN(QMAX(frac,1),100);
    computeMinimumSize();
}


inline QSize KSliderControl::minimumSize() const
{
    return qs;
}


#endif // K_SLIDER_CONTROL_H
