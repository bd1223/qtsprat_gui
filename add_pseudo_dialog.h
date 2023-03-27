#ifndef ADD_PSEUDO_DIALOG_H
#define ADD_PSEUDO_DIALOG_H

#include <QDialog>
#include <QtWidgets>


namespace Ui {
class add_pseudo_dialog;
}

class add_pseudo_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit add_pseudo_dialog(QWidget *parent = 0);
    ~add_pseudo_dialog();

signals:
    void add(const QString& txt);

private slots:
    void on_add_pushButton_clicked();

private:
    Ui::add_pseudo_dialog *ui;


};

#endif // ADD_PSEUDO_DIALOG_H
