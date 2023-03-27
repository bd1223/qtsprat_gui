#ifndef FLYOUT_DIALOG_H
#define FLYOUT_DIALOG_H

#include <QDialog>
#include <QtWidgets>


#define MAX_COLS    3


struct flyout_section_t
{
    QString     name;
    QCheckBox*  chkbox;
    bool        fcas;
    bool        gras;
};


namespace Ui
{
class flyout_Dialog;
}


class flyout_Dialog : public QDialog
{
    Q_OBJECT


public:
    explicit flyout_Dialog(QString spratdir, QWidget *parent = 0);
    ~flyout_Dialog();

private slots:
    void on_buttonBox_accepted();
    void select_all();
    void select_none();

private:
    Ui::flyout_Dialog *ui;
    QString m_dir;

    bool is_set_in_cfg_file(QString& section);
};

#endif // FLYOUT_DIALOG_H
