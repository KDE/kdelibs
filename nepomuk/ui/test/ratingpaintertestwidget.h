#ifndef _RATING_PAINTER_TEST_WIDGET_H_
#define _RATING_PAINTER_TEST_WIDGET_H_

#include <QtGui/QWidget>

class QComboBox;
class QCheckBox;
class KRatingWidget;
class KIntSpinBox;
class QPushButton;

class RatingPainterTestWidget : public QWidget
{
    Q_OBJECT

public:
    RatingPainterTestWidget();
    ~RatingPainterTestWidget();

private Q_SLOTS:
    void slotChangeIcon();
    void slotAlignmentChanged();

private:
    KIntSpinBox* m_spinMaxRating;
    KIntSpinBox* m_spinRating;
    KIntSpinBox* m_spinSpacing;

    QComboBox* m_comboAlignmentH;
    QComboBox* m_comboAlignmentV;
    QCheckBox* m_checkHalfSteps;
    QCheckBox* m_checkEnalbed;

    QPushButton* m_buttonIcon;

    KRatingWidget* m_ratingWidget;
};

#endif
