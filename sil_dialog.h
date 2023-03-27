#ifndef SIL_DIALOG_H
#define SIL_DIALOG_H


#include <QDialog>
#include <QtWidgets>


#define MAX_COLS    3


struct sil_section_t
{
    QString     name;
    QCheckBox*  chkbox;
    bool        fcas;
    bool        gras;
};


namespace Ui {
class sil_Dialog;
}

class sil_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit sil_Dialog(QString dir, QWidget *parent = 0);
    ~sil_Dialog();

private slots:
    void on_buttonBox_accepted();
    void select_all();
    void select_none();

private:
    Ui::sil_Dialog *ui;
    QString m_dir;

    bool is_set_in_cfg_file(QString& section);

};

#endif // SIL_DIALOG_H
