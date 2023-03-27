#include "network_dialog.h"
#include "ui_network_dialog.h"

network_dialog::network_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::network_dialog)
{
    ui->setupUi(this);
}

network_dialog::~network_dialog()
{
    delete ui;
}

QString network_dialog::ctc_ip_addr()
{
    return ui->ctc_ip_lineEdit->text();
}

QString network_dialog::tlm_ip_addr()
{
    return ui->telem_ip_lineEdit->text();
}

QString network_dialog::tlm_port()
{
    return ui->telem_port_lineEdit->text();
}

QString network_dialog::cmd_port()
{
    return ui->cmd_port_lineEdit->text();
}
