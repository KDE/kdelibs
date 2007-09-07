#ifndef RICHPAGE_H
#define RICHPAGE_H

#include <kprintdialogpage.h>

class QSpinBox;
class QComboBox;

#ifdef __GNUC__
#warning rename class or remove from global namespace
#endif
class RichPage : public KPrintDialogPage
{
public:
    RichPage(QWidget *parent = 0);
    ~RichPage();

    void setOptions(const QMap<QString, QString>& options);
    void getOptions(QMap<QString, QString>& options, bool incldef = false);

private:
    QSpinBox *margin_;
    QComboBox *fontname_;
    QSpinBox *fontsize_;
};

#endif
