#include "met_range_dialog.h"
#include "ui_met_range_dialog.h"


met_range_Dialog::met_range_Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::met_range_Dialog)
{
    ui->setupUi(this);
    ui->start_met_lineEdit->setValidator( new QDoubleValidator(-600.0, 800.0, 3, this) );
    ui->stop_met_lineEdit->setValidator( new QDoubleValidator(-600.0, 800.0, 3, this) );

    connect(ui->start_met_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(times_changed()));
    connect(ui->stop_met_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(times_changed()));
    connect(ui->entire_mission_checkBox, SIGNAL(toggled(bool)), this, SLOT(entire_mission_toggled()));
}


met_range_Dialog::~met_range_Dialog()
{
    delete ui;
}


void met_range_Dialog::set_label(QString lbl)
{
    ui->met_range_Label->setText(lbl);

    // show qualifiers if Data Scope or Data Range commands
    if (lbl.contains("Data"))
    {
        ui->qualifiers_lineEdit->setVisible(true);
        ui->qualifiers_label->setVisible(true);
    }
    else
    {
        ui->qualifiers_lineEdit->setVisible(false);
        ui->qualifiers_label->setVisible(false);
    }
}


QString met_range_Dialog::start_met()
{
    return ui->start_met_lineEdit->text();
}

QString met_range_Dialog::stop_met()
{
    return ui->stop_met_lineEdit->text();
}

QString met_range_Dialog::qualifiers()
{
    return ui->qualifiers_lineEdit->text();
}

void met_range_Dialog::entire_mission_toggled()
{
    if (ui->entire_mission_checkBox->isChecked())
    {
        ui->start_met_lineEdit->setText("");
        ui->stop_met_lineEdit->setText("");
    }
}

void met_range_Dialog::times_changed()
{
    if (ui->start_met_lineEdit->text().size() || ui->stop_met_lineEdit->text().size())
    {
        ui->entire_mission_checkBox->setChecked(false);
    }
    else
    {
        ui->entire_mission_checkBox->setChecked(true);
    }
}
