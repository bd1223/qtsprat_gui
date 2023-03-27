#include "find_add_dialog.h"
#include "ui_find_add_dialog.h"

find_add_dialog::find_add_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::find_add_dialog)
{
    ui->setupUi(this);

    ui->tableWidget->setColumnWidth(AF_COL_HIT,  40);
    ui->tableWidget->setColumnWidth(AF_COL_UTID, 240);
    ui->tableWidget->setColumnWidth(AF_COL_CUI,  130);
    ui->tableWidget->setColumnWidth(AF_COL_TYPE, 60);
    ui->tableWidget->setColumnWidth(AF_COL_CID,  80);

    // change the label on the Yes button to "Add"
    ui->buttonBox->button(QDialogButtonBox::Yes)->setText("Add");
}

find_add_dialog::~find_add_dialog()
{
    delete ui;
}

// emit a signal with the search term
void find_add_dialog::on_find_pushButton_clicked()
{
    QString qstr = ui->include_lineEdit->text();
    QString excl = ui->exclude_lineEdit->text();
    excl = excl.simplified();

    if (excl.size())
        qstr = qstr + " ! " + excl;

    emit search_term(qstr);

    clear();
}

void find_add_dialog::clear()
{
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
}

// add a hit from the find command to the console app
void find_add_dialog::add_hit(const QString& instr)
{
    this->setCursor(Qt::WaitCursor);

    QString qstr = instr;

    qstr.remove("hit #");
    qstr.remove("cnt ::", Qt::CaseInsensitive);

    int index = qstr.indexOf(':');

    if (-1 == index)
    {
        qDebug() << "find_add: ':' not found";
        return;
    }

    QString hitnum = qstr.mid(0, index);
    qstr.remove(0, index+2);

    QStringList qstr_list = qstr.split("::", QString::SkipEmptyParts);
    if (qstr_list.size() < 5)
    {
        qDebug() << "find_add: qstr_list.size() = " << qstr_list.size();
        return;
    }

    QString utid = qstr_list.at(0);
    QString cui = qstr_list.at(1);
    QString type = qstr_list.at(2);
    QString cid = qstr_list.at(3);
    QString descr = qstr_list.at(4);

    // if there are units, there is one more token in the string
    if (descr.contains("CID="))
    {
        cid = qstr_list.at(4);

        if (qstr_list.size() > 5)
            descr = qstr_list.at(5);
        else
            descr = "";
    }

    cid.remove("(CID=");
    cid.remove(")");
    cid = cid.simplified();

    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    ui->tableWidget->setRowHeight(row, 20);

    QTableWidgetItem* item = new QTableWidgetItem(hitnum);
    ui->tableWidget->setItem(row, AF_COL_HIT, item);

    item = new QTableWidgetItem(utid);
    ui->tableWidget->setItem(row, AF_COL_UTID, item);

    item = new QTableWidgetItem(cui);
    ui->tableWidget->setItem(row, AF_COL_CUI, item);

    item = new QTableWidgetItem(type);
    ui->tableWidget->setItem(row, AF_COL_TYPE, item);

    item = new QTableWidgetItem(cid);
    ui->tableWidget->setItem(row, AF_COL_CID, item);

    item = new QTableWidgetItem(descr);
    ui->tableWidget->setItem(row, AF_COL_DESCR, item);


}

void find_add_dialog::on_buttonBox_accepted()
{
    QString cmd = "add ";

    for (int row = 0; row < ui->tableWidget->rowCount(); row++)
    {
        if (ui->tableWidget->item(row, AF_COL_UTID)->isSelected())
        {
            cmd += ui->tableWidget->item(row, AF_COL_HIT)->text();
            cmd += ",";
        }
    }

    // remove the last ","
    if (cmd.endsWith(','))
    {
        cmd.chop(1);
    }

    qDebug() << "cmd = " << cmd;
    emit add_cmd(cmd);
}

void find_add_dialog::reset_cursor()
{
    this->setCursor(Qt::ArrowCursor);
}
