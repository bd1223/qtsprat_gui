#ifndef MET_RANGE_DIALOG_H
#define MET_RANGE_DIALOG_H

#include <QDialog>

namespace Ui {
class met_range_Dialog;
}

class met_range_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit met_range_Dialog(QWidget *parent = 0);
    ~met_range_Dialog();

    void set_label(QString lbl);
    QString start_met();
    QString stop_met();
    QString qualifiers();

private slots:
    void entire_mission_toggled();
    void times_changed();

private:
    Ui::met_range_Dialog *ui;
};

#endif // MET_RANGE_DIALOG_H
