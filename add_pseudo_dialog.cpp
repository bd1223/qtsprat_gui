#include "add_pseudo_dialog.h"
#include "ui_add_pseudo_dialog.h"



add_pseudo_dialog::add_pseudo_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::add_pseudo_dialog)
{
    ui->setupUi(this);
    ui->buttonBox->addButton(ui->add_pushButton, QDialogButtonBox::ActionRole);
}

add_pseudo_dialog::~add_pseudo_dialog()
{
    delete ui;
}

void add_pseudo_dialog::on_add_pushButton_clicked()
{
    for (int ii = 0; ii < ui->listWidget->count(); ii++)
    {
        if (ui->listWidget->item(ii)->isSelected())
        {
            emit add(ui->listWidget->item(ii)->text());
        }
    }
}
