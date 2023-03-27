#include "saf_maint_dialog.h"
#include "ui_saf_maint_dialog.h"



saf_maint_dialog::saf_maint_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::saf_maint_dialog),
    m_console_app(NULL)
{
    ui->setupUi(this);

    ui->tableWidget->setColumnWidth(COL_SAF,    200);
    ui->tableWidget->setColumnWidth(COL_REC,    80);
    ui->tableWidget->setColumnWidth(COL_OUTPUT, 80);
    ui->tableWidget->setColumnWidth(COL_SPRAT,  80);
    ui->tableWidget->setColumnWidth(COL_TOTAL,  80);
    ui->tableWidget->setColumnWidth(COL_MOD,    150);
    ui->tableWidget->setColumnWidth(COL_MOTH,   70);

    // stretch first column
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    // draw a nice border around the column headers
    QHeaderView* header = ui->tableWidget->horizontalHeader();
    header->setFrameStyle(QFrame::Box | QFrame::Plain);
    header->setLineWidth(1);
    ui->tableWidget->setHorizontalHeader(header);

    ui->tableWidget->horizontalHeader()->setHighlightSections(false);

    connect(ui->tableWidget, SIGNAL(itemSelectionChanged()), this, SLOT(selection_changed()));

    configure_buttons();
}

saf_maint_dialog::~saf_maint_dialog()
{
    delete ui;
}

void saf_maint_dialog::on_browse_pushButton_clicked()
{
    qDebug() << "SAF Maintenance Browse";

    // open file selection box
    QString base_path = QFileDialog::getExistingDirectory(
        this,
        "SAF Maintenance - Select Base Path for SAF(s)",
        ui->base_path_lineEdit->text(),
        QFileDialog::ShowDirsOnly);

    if (base_path.isEmpty())
        return;

    // if the user selected a SAF, back up 1 directory...
    if ((base_path.endsWith("_fcas", Qt::CaseInsensitive)) ||
            (base_path.endsWith("_gras", Qt::CaseInsensitive)))
    {
        QDir dir(base_path);
        dir.cdUp();
        base_path = dir.absoluteFilePath("");
    }

    qDebug() << "base_path = " << base_path;

    ui->base_path_lineEdit->setText(base_path);
    populate_table();
}


void saf_maint_dialog::set_base_path(QString path)
{
    ui->base_path_lineEdit->setText(path);
    populate_table();
}


void saf_maint_dialog::populate_table()
{
    qDebug() << "populate table";

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    // find all SAF(s) in the base dir
    QDir base_dir(ui->base_path_lineEdit->text());
    base_dir.setFilter(QDir::Dirs);

    int row = 0;

    QFileInfoList list = base_dir.entryInfoList();
    for (int ii = 0; ii < list.size(); ++ii)
    {
        QFileInfo file_info = list.at(ii);

        // we only care about valid SAF paths...
        QString filename = file_info.fileName();
        if (file_info.isDir())
        {
            if ((filename.endsWith("_fcas", Qt::CaseInsensitive)) ||
                (filename.endsWith("_gras", Qt::CaseInsensitive)))
            {
                // add items to new row in table
                ui->tableWidget->insertRow( row );


                QStringList includes;
                QStringList excludes;

                includes.clear();
                excludes.clear();
                includes << ".bin" << ".gz";
                excludes << "binscanner" << "irs_";
                qint64 size_rec = dir_size(file_info.absoluteFilePath(), includes, excludes);

                qint64 size_output = dir_size(file_info.absoluteFilePath() + "/output_files");

                includes.clear();
                excludes.clear();
                includes << "binscanner" << ".msf" << ".csv" << ".zip" << ".dem" << ".xlsx" << ".cpp" << "irs_" << ".xml";
                excludes << "output_files";
                qint64 size_sprat = dir_size(file_info.absoluteFilePath(), includes, excludes);

                qint64 size_total = dir_size(file_info.absoluteFilePath());
                QString last_mod = dir_mod_time(file_info.absoluteFilePath()).toString("MMM dd yy  hh:mm:ss");

                QString mothballed = is_mothballed(file_info.absoluteFilePath()) ? "YES" : "NO";
                QString clean      = is_clean(file_info.absoluteFilePath()) ? "YES" : "NO";

                QTableWidgetItem* item = new QTableWidgetItem(filename);
                ui->tableWidget->setItem(row, COL_SAF, item);

                item = new QTableWidgetItem(QString::number(size_rec));
                item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                ui->tableWidget->setItem(row, COL_REC, item);

                item = new QTableWidgetItem(QString::number(size_output));
                item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                ui->tableWidget->setItem(row, COL_OUTPUT, item);

                item = new QTableWidgetItem(QString::number(size_sprat));
                item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                ui->tableWidget->setItem(row, COL_SPRAT, item);

                item = new QTableWidgetItem(QString::number(size_total));
                item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                ui->tableWidget->setItem(row, COL_TOTAL, item);

                item = new QTableWidgetItem(last_mod);
                item->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
                ui->tableWidget->setItem(row, COL_MOD, item);

                item = new QTableWidgetItem(mothballed);
                item->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
                ui->tableWidget->setItem(row, COL_MOTH, item);

                item = new QTableWidgetItem(clean);
                item->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
                ui->tableWidget->setItem(row, COL_CLEAN, item);

                ++row;
            }
        }
    }

    configure_buttons();
}


// compute the disk usage (in MB) of the files in a directory (recursive)
qint64 saf_maint_dialog::dir_size(QString path, QStringList include_filter, QStringList exclude_filter)
{
    QDir dir(path);
    qint64 size = 0;

    QFileInfoList list = dir.entryInfoList();
    for (int ii = 0; ii < list.size(); ++ii)
    {
        QFileInfo file_info = list.at(ii);
        if (file_info.fileName().startsWith("."))
            continue;

        // exclusion filter
        if (!exclude_filter.isEmpty())
        {
            bool skip = false;
            for (int ii = 0; ii < exclude_filter.size(); ++ii)
            {
                QString exclude = exclude_filter.at(ii);
                if (file_info.fileName().contains(exclude))
                {
                    skip = true;
                }
            }

            if (skip)
                continue;
        }

        // inclusion filter
        if (!include_filter.isEmpty())
        {
            bool skip = true;
            for (int ii = 0; ii < include_filter.size(); ++ii)
            {
                QString include = include_filter.at(ii);
                if (file_info.fileName().contains(include))
                {
                    skip = false;
                }
            }

            if (skip)
                continue;
        }

        if (file_info.isDir())
        {
            size += dir_size(file_info.absoluteFilePath(), include_filter, exclude_filter);
        }
        else
        {
            size += file_info.size();
        }
    }

    return size / 1000000;
}


// (overloaded function with no filters)
qint64 saf_maint_dialog::dir_size(QString path)
{
    QStringList empty_filter;
    empty_filter.empty();
    return dir_size(path, empty_filter, empty_filter);
}


// return the most recent mod time of any file in the specified directory (recursive)
QDateTime saf_maint_dialog::dir_mod_time(QString path)
{
    QDir dir(path);
    QDateTime last_mod;
    QDateTime test_mod;

    QFileInfoList list = dir.entryInfoList();
    for (int ii = 0; ii < list.size(); ++ii)
    {
        QFileInfo file_info = list.at(ii);
        if (file_info.fileName().startsWith("."))
            continue;

        if (file_info.isDir())
        {
            test_mod = dir_mod_time(file_info.absoluteFilePath());
        }
        else
        {
            test_mod = file_info.lastModified();
        }

        if (test_mod > last_mod)
        {
            last_mod = test_mod;
        }
    }

    return last_mod;
}


// returns true if the SAF has been mothballed
bool saf_maint_dialog::is_mothballed(QString path)
{
    QDir dir(path);

    QFileInfoList list = dir.entryInfoList();
    for (int ii = 0; ii < list.size(); ++ii)
    {
        QFileInfo file_info = list.at(ii);
        if (file_info.fileName().startsWith("."))
            continue;

        if (0 == (file_info.fileName().compare("binaries.7z", Qt::CaseInsensitive)))
            return true;
    }

    return false;
}


// returns true if the SAF is "clean"
bool saf_maint_dialog::is_clean(QString path)
{
    QDir dir(path);

    QFileInfoList list = dir.entryInfoList();
    for (int ii = 0; ii < list.size(); ++ii)
    {
        QFileInfo file_info = list.at(ii);
        if (file_info.fileName().startsWith("."))
            continue;

        if (0 == (file_info.fileName().compare("output_files", Qt::CaseInsensitive)))
            return false;
    }

    return true;
}


void saf_maint_dialog::on_close_pushButton_clicked()
{
    this->hide();
}


void saf_maint_dialog::on_clean_pushButton_clicked()
{
    QStringList options;
    options.clear();
    options << "clean";

    run_binscanner(options);
}


void saf_maint_dialog::on_mothball_pushButton_clicked()
{
    QStringList options;
    options.clear();
    options << "mothball";

    run_binscanner(options);
}


void saf_maint_dialog::run_binscanner(QStringList options)
{
    this->setCursor(Qt::WaitCursor);
    QApplication::processEvents();

    ui->textEdit->clear();

    for (int ii = 0; ii < ui->tableWidget->rowCount(); ii++)
    {
        if (ui->tableWidget->item(ii, COL_SAF)->isSelected())
        {
            QString filename = ui->tableWidget->item(ii, COL_SAF)->text();
            QString full_path = ui->base_path_lineEdit->text() + "\\" + filename;

            start_process("binscanner.exe", full_path, options);

            // wait for process to finish before continuing to next SAF
            while(m_console_app)
            {
                QApplication::processEvents();
            }

            // update the row currently being processed
            QStringList includes;
            QStringList excludes;
            includes.clear();
            excludes.clear();

            includes << ".bin" << ".gz";
            excludes << "binscanner" << "irs_";
            qint64 size_rec = dir_size(full_path, includes, excludes);
            qint64 size_output = dir_size(full_path + "/output_files");

            includes.clear();
            excludes.clear();
            includes << "binscanner" << ".msf" << ".csv" << ".zip" << ".dem" << ".xlsx" << ".cpp" << "irs_" << ".xml";
            excludes << "output_files";
            qint64 size_sprat = dir_size(full_path, includes, excludes);
            qint64 size_total = dir_size(full_path);

            QString last_mod = dir_mod_time(full_path).toString("MMM dd yy  hh:mm:ss");

            QString mothballed = is_mothballed(full_path) ? "YES" : "NO";
            QString clean      = is_clean(full_path) ? "YES" : "NO";

            ui->tableWidget->item(ii, COL_REC)->setText(QString::number(size_rec));
            ui->tableWidget->item(ii, COL_OUTPUT)->setText(QString::number(size_output));
            ui->tableWidget->item(ii, COL_SPRAT)->setText(QString::number(size_sprat));
            ui->tableWidget->item(ii, COL_TOTAL)->setText(QString::number(size_total));
            ui->tableWidget->item(ii, COL_MOD)->setText(last_mod);
            ui->tableWidget->item(ii, COL_MOTH)->setText(mothballed);
            ui->tableWidget->item(ii, COL_CLEAN)->setText(clean);
        }
    }

    ui->tableWidget->clearSelection();
    populate_table();

    this->setCursor(Qt::ArrowCursor);
}


bool saf_maint_dialog::start_process(QString proc_name, QString project_path, QStringList options)
{
    QDir dir(project_path);
    QString dirname = dir.dirName();

    qDebug() << "project_path = " << project_path << ", dirname = " << dirname;


    // note: proc_name needs to be in PATH
    project_path.replace("/", "\\");

    QString cmd = proc_name + " " + dirname;

    for (int ii = 0; ii < options.size(); ii++)
    {
        QString option = options.at(ii);
        cmd += " ";
        cmd += option;
    }

    qDebug() << "cmd = " << cmd;

    m_console_app = new QProcess(this);
    m_console_app->setProcessChannelMode(QProcess::MergedChannels);
    m_console_app->setWorkingDirectory(ui->base_path_lineEdit->text());

    connect(m_console_app, SIGNAL(readyReadStandardOutput()), this, SLOT(read_msg()));
    connect(m_console_app, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(proc_done()));

    ui->textEdit->append("running command: " + cmd);

    m_console_app->start(cmd);
    if (false == m_console_app->waitForStarted(5000))
    {
        ui->textEdit->append("failed to start " + cmd);
        QMessageBox::warning( this, "qtsprat", "failed to start " + cmd, "OK", 0, 0 );
        return false;
    }

    return true;
}


void saf_maint_dialog::proc_done()
{
    ui->textEdit->append("process has finished");
    m_console_app = NULL;

    configure_buttons();
}


void saf_maint_dialog::read_msg()
{
    QByteArray qmsg = m_console_app->readAllStandardOutput();
    QString tmpstr(qmsg);

    qDebug() << "tmpstr = " << tmpstr;

    static QString qstr = "";
    qstr.append(tmpstr);

    // if this is not a full line, wait and keep appending
    // to avoid those annoying partial lines
    if (!qstr.endsWith("\n") && !qstr.endsWith("\r"))
        return;

    qstr = qstr.trimmed();

    // ignore blank lines
    if (qstr.length() == 0)
        return;

    ui->textEdit->append(qstr);
    qstr = "";
}

void saf_maint_dialog::on_sel_all_pushButton_clicked()
{
    ui->tableWidget->selectAll();
}

void saf_maint_dialog::on_sel_none_pushButton_clicked()
{
    ui->tableWidget->clearSelection();
}


void saf_maint_dialog::configure_buttons()
{
    ui->clean_pushButton->setEnabled(false);
    ui->mothball_pushButton->setEnabled(false);

    for (int ii = 0; ii < ui->tableWidget->rowCount(); ii++)
    {
        if (ui->tableWidget->item(ii, COL_SAF)->isSelected())
        {
            ui->clean_pushButton->setEnabled(true);
            ui->mothball_pushButton->setEnabled(true);
        }
    }

    ui->tableWidget->setFocus();
}


void saf_maint_dialog::selection_changed()
{
    qDebug() << "selection changed";
    configure_buttons();
}
