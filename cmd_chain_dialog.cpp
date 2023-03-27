#include "cmd_chain_dialog.h"
#include "ui_cmd_chain_dialog.h"

#include <QtWidgets>


cmd_chain_dialog::cmd_chain_dialog(QString& filename, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::cmd_chain_dialog),
    m_paused(false)
{
    ui->setupUi(this);

    // hide the "?" button in the title bar
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // open the command chain file
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning( this,
                              "Command Chain",
                              tr( "error reading file %s:\n%2" )
                              .arg( filename )
                              .arg( file.errorString() ) );
        return;
    }

    QTextStream in(&file);
    int num_items = 0;
    while(!in.atEnd())
    {
        QString qstr = in.readLine();
        ui->listWidget->addItem(qstr);

        if (0 == qstr.compare("wait", Qt::CaseInsensitive))
        {
            ui->listWidget->item(num_items)->setForeground(Qt::red);
        }

        num_items++;
    }
    file.close();

    // select the first command in the list
    ui->listWidget->setCurrentRow(0);

    m_paused = true;

    QFileInfo finfo(filename);
    setWindowTitle(finfo.fileName());
    show();
    configure_gui();
}


cmd_chain_dialog::~cmd_chain_dialog()
{
    delete ui;
}


void cmd_chain_dialog::on_pause_pushButton_clicked()
{
    m_paused = true;
    configure_gui();
}


void cmd_chain_dialog::on_start_pushButton_clicked()
{
    m_paused = false;
    on_step_pushButton_clicked();
}


void cmd_chain_dialog::on_step_pushButton_clicked()
{
    int current_row = ui->listWidget->currentRow();
    if ((current_row >= 0) && (current_row < (ui->listWidget->count())))
    {
        QString cmd = ui->listWidget->item(current_row)->text();

        if (0 == cmd.compare("wait", Qt::CaseInsensitive))
        {
            ui->status_label->setText("Idle");
            m_paused = true;
            ui->listWidget->setCurrentRow(current_row+1);
        }
        else
        {
            ui->status_label->setText("Running");
            ui->listWidget->setCurrentRow(current_row+1);

            next_cmd(cmd);    // emit signal
        }
    }
    else
    {
        ui->status_label->setText("Idle");
        m_paused = true;
    }

    configure_gui();
}


void cmd_chain_dialog::configure_gui()
{
    ui->start_pushButton->setEnabled(m_paused);
    ui->step_pushButton->setEnabled(m_paused);
    ui->pause_pushButton->setEnabled(!m_paused);

    if ((m_paused) && (ui->status_label->text() != "Idle"))
    {
        ui->status_label->setText("Paused");
    }
}


void cmd_chain_dialog::advance()
{
    if (!m_paused)
    {
        on_step_pushButton_clicked();
    }
}
