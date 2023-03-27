#include "find_dialog.h"
#include "ui_find_dialog.h"

find_dialog::find_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::find_dialog)
{
    ui->setupUi(this);
}

find_dialog::~find_dialog()
{
    delete ui;
}

void find_dialog::on_find_back_pushButton_clicked()
{
    emit find_back(ui->lineEdit->text());
}

void find_dialog::on_find_fwd_pushButton_clicked()
{
    emit find_fwd(ui->lineEdit->text());
}

void find_dialog::on_lineEdit_textChanged( /* const QString &arg1 */ )
{
    ui->not_found_Label->setVisible(false);
}

void find_dialog::set_text(QString txt)
{
    ui->lineEdit->setText(txt);
}

QString find_dialog::text()
{
    return ui->lineEdit->text();
}

void find_dialog::not_found(bool found)
{
    ui->not_found_Label->setVisible(found);
}

bool find_dialog::highlight_all()
{
    return ui->highlight_all_checkBox->isChecked();
}
