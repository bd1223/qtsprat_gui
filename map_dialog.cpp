#include "map_dialog.h"
#include "ui_map_dialog.h"

#define MET_MIN -600
#define MET_MAX 1000

map_dialog::map_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::map_dialog)
{
    ui->setupUi(this);
    ui->start_met_lineEdit->setValidator( new QDoubleValidator(MET_MIN, MET_MAX, 3, this) );
    ui->stop_met_lineEdit->setValidator( new QDoubleValidator(MET_MIN, MET_MAX, 3, this) );

    connect(ui->start_met_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(times_changed()));
    connect(ui->stop_met_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(times_changed()));
    connect(ui->entire_mission_checkBox, SIGNAL(toggled(bool)), this, SLOT(entire_mission_toggled()));
}

map_dialog::~map_dialog()
{
    delete ui;
}

QString map_dialog::start_met()
{
    return ui->start_met_lineEdit->text();
}

QString map_dialog::stop_met()
{
    return ui->stop_met_lineEdit->text();
}

QString map_dialog::dem()
{
    return ui->dem_lineEdit->text();
}

bool map_dialog::simple()
{
    return ui->simple_radioButton->isChecked();
}

bool map_dialog::timeline()
{
    return ui->timeline_radioButton->isChecked();
}

QString map_dialog::qualifiers()
{
    return ui->qualifiers_lineEdit->text();
}

void map_dialog::entire_mission_toggled()
{
    if (ui->entire_mission_checkBox->isChecked())
    {
        ui->start_met_lineEdit->setText(QString::number(MET_MIN));
        ui->stop_met_lineEdit->setText(QString::number(MET_MAX));
    }
}

void map_dialog::times_changed()
{
    if (ui->start_met_lineEdit->text() != "-600" || ui->stop_met_lineEdit->text() != "1000")
    {
        ui->entire_mission_checkBox->setChecked(false);
    }
    else
    {
        ui->entire_mission_checkBox->setChecked(true);
    }
}

