#include "segment_dialog.h"
#include "ui_segment_dialog.h"

segment_dialog::segment_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::segment_dialog)
{
    ui->setupUi(this);
}

segment_dialog::~segment_dialog()
{
    delete ui;
}

QString segment_dialog::selected_segment()
{
    if (ui->initial_radioButton->isChecked())
        return "INITIAL_SEGMENT";

    if (ui->middle_radioButton->isChecked())
        return "MIDDLE_SEGMENT";

    if (ui->final_radioButton->isChecked())
        return "FINAL_SEGMENT";

    return "";
}

void segment_dialog::set_segment(QString seg)
{
    if ("MIDDLE" == seg)
        ui->middle_radioButton->setChecked(true);
    else if ("FINAL" == seg)
        ui->final_radioButton->setChecked(true);
    else
        ui->initial_radioButton->setChecked(true);
}

