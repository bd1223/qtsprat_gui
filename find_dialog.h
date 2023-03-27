#ifndef FIND_DIALOG_H
#define FIND_DIALOG_H

#include <QDialog>
#include <QtWidgets>


namespace Ui {
class find_dialog;
}

class find_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit find_dialog(QWidget *parent = 0);
    ~find_dialog();

    QString text();
    void set_text(QString txt);
    void not_found(bool found);
    bool highlight_all();

signals:
    void find_fwd(const QString& txt);
    void find_back(const QString& txt);

private slots:
    void on_find_back_pushButton_clicked();
    void on_find_fwd_pushButton_clicked();
    void on_lineEdit_textChanged( /* const QString &arg1 */ );

private:
    Ui::find_dialog *ui;
};

#endif // FIND_DIALOG_H
