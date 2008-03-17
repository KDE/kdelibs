#ifndef _RATING_PAINTER_TEST_WIDGET_H_
#define _RATING_PAINTER_TEST_WIDGET_H_

#include <QtGui/QWidget>

class QComboBox;
class QCheckBox;
class KRatingWidget;
class QSpinBox;
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
    QSpinBox* m_spinMaxRating;
    QSpinBox* m_spinRating;
    QSpinBox* m_spinSpacing;

    QComboBox* m_comboAlignmentH;
    QComboBox* m_comboAlignmentV;
    QCheckBox* m_checkHalfSteps;
    QCheckBox* m_checkEnalbed;

    QPushButton* m_buttonIcon;

    KRatingWidget* m_ratingWidget;
};

#endif
