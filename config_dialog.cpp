#include "config_dialog.h"
#include "ui_config_dialog.h"
#include <QDir>
#include <QtWidgets>

config_dialog::config_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::config_dialog)
{
    ui->setupUi(this);
    ui->exe_path_lineEdit->setText(DEFAULT_STR);
}

config_dialog::~config_dialog()
{
    delete ui;
}

bool config_dialog::do_irs()
{
    return ui->irs_checkBox->isChecked();
}

bool config_dialog::gige_only()
{
    return ui->gige_checkBox->isChecked();
}

bool config_dialog::get_tlm_files()
{
    return ui->tlm_files_checkBox->isChecked();
}

bool config_dialog::imacs_tlm_only()
{
    return ui->imacs_tlm_checkBox->isChecked();
}

bool config_dialog::max_fp_digs()
{
    return ui->max_fp_digs_checkBox->isChecked();
}

bool config_dialog::process_pcap()
{
    return ui->pcap_checkBox->isChecked();
}

bool config_dialog::process_stream_data()
{
    return ui->stream_checkBox->isChecked();
}

bool config_dialog::skip_stream_data()
{
    return ui->skip_stream_checkBox->isChecked();
}

bool config_dialog::debug()
{
    return ui->debug_checkBox->isChecked();
}

mem_model_t config_dialog::mem_model()
{
    if (ui->small_radioButton->isChecked())
    {
        return MEM_SMALL;
    }

    if (ui->medium_radioButton->isChecked())
    {
        return MEM_MEDIUM;
    }

    if (ui->large_radioButton->isChecked())
    {
        return MEM_LARGE;
    }

    if (ui->huge_radioButton->isChecked())
    {
        return MEM_HUGE;
    }

    return MEM_AUTO;
}

bool config_dialog::use_blackboard_file()
{
    return ui->blackboard_checkBox->isChecked();
}

bool config_dialog::process_efi()
{
    return ui->efi_checkBox->isChecked();
}

bool config_dialog::process_dfi()
{
    return ui->dfi_checkBox->isChecked();
}

void config_dialog::on_exe_browse_pushButton_clicked()
{
    // open file selection box
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "Executable Path",
        ui->exe_path_lineEdit->text(),
        QFileDialog::ShowDirsOnly);

    if ((dir.isEmpty()) || (dir.isNull()) || (0 == dir.size()))
    {
        ui->exe_path_lineEdit->setText(DEFAULT_STR);
    }

    ui->exe_path_lineEdit->setText(dir);
}

QString config_dialog::exe_path()
{
    if (ui->exe_path_lineEdit->text() == DEFAULT_STR)
    {
        return "";
    }

    return ui->exe_path_lineEdit->text();
}

void config_dialog::set_noxml(bool checked)
{
    ui->imacs_tlm_checkBox->setChecked(checked);
}

void config_dialog::set_mem_model(mem_model_t model)
{
    if (MEM_SMALL == model)
    {
        ui->small_radioButton->setChecked(true);
    }
    else if (MEM_MEDIUM == model)
    {
        ui->medium_radioButton->setChecked(true);
    }
    else if (MEM_LARGE == model)
    {
        ui->large_radioButton->setChecked(true);
    }
    else if (MEM_HUGE == model)
    {
        ui->huge_radioButton->setChecked(true);
    }
    else
    {
        ui->auto_radioButton->setChecked(true);
    }
}

void config_dialog::set_irs(bool checked)
{
    ui->irs_checkBox->setChecked(checked);
}

void config_dialog::set_gige_only(bool checked)
{
    ui->gige_checkBox->setChecked(checked);
}

void config_dialog::set_pcap(bool checked)
{
    ui->pcap_checkBox->setChecked(checked);
}

void config_dialog::set_stream_data(bool checked)
{
    ui->stream_checkBox->setChecked(checked);
}

void config_dialog::set_skip_stream_data(bool checked)
{
    ui->skip_stream_checkBox->setChecked(checked);
}

void config_dialog::set_blackboard(bool use_me)
{
    ui->blackboard_checkBox->setChecked(use_me);
}

void config_dialog::set_efi(bool use_me)
{
    ui->efi_checkBox->setChecked(use_me);
}

void config_dialog::set_dfi(bool use_me)
{
    ui->dfi_checkBox->setChecked(use_me);
}

void config_dialog::set_max_fp_digs(bool use_me)
{
    qDebug() << "cfg_dlg: MAX_FP_DIGS" << use_me;
    ui->max_fp_digs_checkBox->setChecked(use_me);
}

void config_dialog::set_exe_path(QString path)
{
    ui->exe_path_lineEdit->setText(path);
}

void config_dialog::set_plot_cmd(QString plot_cmd)
{
    ui->plot_comboBox->setCurrentText(plot_cmd);
}

void config_dialog::set_bypass_tlmfiles()
{
    ui->tlm_files_checkBox->setChecked(false);
}

QString config_dialog::plot_cmd()
{
    if (ui->plot_comboBox->currentText() == "gnuplot")
        return "plot";

    return ui->plot_comboBox->currentText();     // currently only winplot
}

// PCAP, GIGE, and STREAMING data are mutually dependent - set checkboxes accordingly
void config_dialog::on_pcap_checkBox_clicked()
{
    if (ui->pcap_checkBox->isChecked())
    {
        ui->gige_checkBox->setChecked(true);
        ui->stream_checkBox->setChecked(false);
        ui->imacs_tlm_checkBox->setChecked(true);
    }
}

void config_dialog::on_stream_checkBox_clicked()
{
    if (ui->stream_checkBox->isChecked())
    {
        ui->pcap_checkBox->setChecked(false);
        ui->gige_checkBox->setChecked(false);
        ui->skip_stream_checkBox->setChecked(false);
    }
}

void config_dialog::on_skip_stream_checkBox_clicked()
{
    if (ui->skip_stream_checkBox->isChecked())
        ui->stream_checkBox->setChecked(false);
}

void config_dialog::on_gige_checkBox_clicked()
{
    if (ui->gige_checkBox->isChecked())
    {
        ui->pcap_checkBox->setChecked(false);
        ui->stream_checkBox->setChecked(false);
    }
}


