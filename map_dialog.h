#ifndef MAP_DIALOG_H
#define MAP_DIALOG_H

#include <QDialog>

namespace Ui {
class map_dialog;
}

class map_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit map_dialog(QWidget *parent = 0);
    ~map_dialog();

    QString start_met();
    QString stop_met();
    QString dem();
    bool simple();
    bool timeline();

    QString qualifiers();

private slots:
    void entire_mission_toggled();
    void times_changed();

private:
    Ui::map_dialog *ui;
};

#endif // MAP_DIALOG_H
