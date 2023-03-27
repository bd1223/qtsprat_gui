#ifndef FIND_ADD_DIALOG_H
#define FIND_ADD_DIALOG_H

#include <QDialog>
#include <QtWidgets>


namespace Ui {
class find_add_dialog;
}


enum
{
    AF_COL_HIT   = 0,
    AF_COL_UTID  = 1,
    AF_COL_CUI   = 2,
    AF_COL_TYPE  = 3,
    AF_COL_CID   = 4,
    AF_COL_DESCR = 5
};



class find_add_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit find_add_dialog(QWidget *parent = 0);
    ~find_add_dialog();

    void clear();
    void add_hit(const QString& hit);
    void reset_cursor();

signals:
    void search_term(const QString& txt);
    void add_cmd(const QString& txt);

private slots:
    void on_find_pushButton_clicked();

    void on_buttonBox_accepted();

private:
    Ui::find_add_dialog *ui;
};

#endif // FIND_ADD_DIALOG_H
