#include "qtsprat.h"
#include "ui_qtsprat.h"
#include <QtPrintSupport/QPrinter>
#include <QPrintPreviewDialog>
#include <QPrintDialog>
#include <QActionGroup>



browser_tbl_t browser_tbl[] =
{
    { "Internal", "0" },
    { "Firefox",  "1" },
    { "IE",       "2" },
    { "Chrome",   "3" },
    { "Safari",   "4" },
    { "Opera",    "5" },
};

size_t num_browsers = sizeof(browser_tbl) / sizeof(browser_tbl[0]);



qtsprat::qtsprat(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::qtsprat),
    m_flyout_dlg(NULL),
    m_sil_dlg(NULL),
    m_met_dlg(new met_range_Dialog(this)),
    m_cmd_chain_dlg(NULL),
    m_config_dlg(NULL),
    m_map_dlg(new map_dialog(this)),
    m_saf_maint_dlg(new saf_maint_dialog(this)),
    m_find_dlg(NULL),
    m_add_pseudo_dlg(NULL),
    m_segment_dlg(new segment_dialog(this)),
    m_find_add_dlg(new find_add_dialog(this)),
    m_network_dlg(new network_dialog(this)),
    m_console_app(NULL),
    m_spratdir(qgetenv("SPRATDIR")),
    m_project_path(m_spratdir),
    m_user_msf_path("C:/ProgramData/SPRAT"),
    m_project(""),
    m_browser("1"),
    m_msf(""),
    m_current_browser_file(""),
    m_font_size("2"),
    m_script(""),
    m_cmd_history(),
    m_hist_index(-1),
    m_watchdog(NULL),
    m_cmd(""),
    m_available_versions("")
{
    ui->setupUi(this);

    if (m_spratdir.isNull() || m_spratdir.isEmpty())
    {
        QMessageBox::warning( this, "qtsprat", "env var SPRATDIR not set - exiting", "OK", 0, 0 );
        exit(0);
    }

//    m_met_dlg = new met_range_Dialog(this);
    m_playback_dlg = new playback_Dialog(m_spratdir, this);
//    m_map_dlg = new map_dialog(this);
//    m_saf_maint_dlg = new saf_maint_dialog(this);
//    m_segment_dlg = new segment_dialog(this);
//    m_find_add_dlg = new find_add_dialog(this);
//    m_network_dlg = new network_dialog(this);

    // create some permanent widgets for the status bar
    QHBoxLayout *layout = new QHBoxLayout(ui->status_bar_progressBar);
    ui->progress_label->setText("");
    layout->addWidget(ui->progress_label);
    layout->setContentsMargins(0,0,0,0);

    ui->statusBar->addPermanentWidget(ui->status_bar_progressBar, 1);
    ui->statusBar->addPermanentWidget(ui->status_bar_label);

    // initialize the browser
    load_browser("file:///" + m_spratdir + "/html/welcome.html");

    // create the configuration dialog and wait for response
    m_config_dlg = new config_dialog(this);

    // load the preferences file
    load_rc();
    m_config_dlg->show();
    m_config_dlg->exec();

    if (!m_project.isEmpty())
    {
        this->raise();

        QString msg = "Do you want to load the previous project:\n" + m_project;
        if (0 == QMessageBox::question(this,
                                       "Load Initial Project",
                                       msg,
                                       "&Yes", "Cancel",
                                       QString::null, 0, 1))
        {
            // launch a build-specific version of the SPRAT executable
            QString build = m_project.mid(0, m_project.indexOf('_'));
            qDebug() << "build = " << build;

            QString cmd = build + "_sprat.exe";
            ui->messages_textEdit->append("starting process " + cmd);

            if (start_process(cmd, m_project_path + "/" + m_project))
            {
                ui->messages_textEdit->append("reading SPRAT MAP file - this may take a while...");
                QFile::remove(m_current_browser_file);
                send_to_process("quiet");
            }
            configure_gui();
        }
        else
        {
            m_project = "";
        }
    }

    // connect signals to slots
    connect(ui->cmd_lineEdit, SIGNAL(returnPressed()), this, SLOT(on_actionSubmit_triggered()));
    connect(ui->cmd_lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(cmd_changed()));
    connect(ui->browser_comboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(browser_changed(const QString&)));
    connect(ui->font_size_spinBox, SIGNAL(valueChanged(int)), this, SLOT(font_size_triggered()));

    // add the browser and font size selection to the tool bar
    ui->mainToolBar->addWidget(ui->browser_comboBox);
    ui->mainToolBar->addWidget(ui->font_size_spinBox);

    // set up the file that contains the URL to display
    m_current_browser_file = m_spratdir + "/current_browser_file.txt";
    QFile::remove(m_current_browser_file);

    configure_gui();

    // use a fixed width font for the message area
    ui->messages_textEdit->setFontFamily("Courier New");

    // create the find (search) dialog
    m_find_dlg = new find_dialog();
    connect(m_find_dlg, SIGNAL(find_fwd(const QString&)), this, SLOT(find_fwd(const QString&)));
    connect(m_find_dlg, SIGNAL(find_back(const QString&)), this, SLOT(find_back(const QString&)));

    // connect the "Add Pseudo" to the slot
    m_add_pseudo_dlg = new add_pseudo_dialog(this);
    connect(m_add_pseudo_dlg, SIGNAL(add(const QString&)), this, SLOT(add_pseudo(const QString&)));

    // stuff for the find/add dialog
    connect(m_find_add_dlg, SIGNAL(search_term(const QString&)), this, SLOT(find_add_search_term(const QString&)));
    connect(m_find_add_dlg, SIGNAL(add_cmd(const QString&)), this, SLOT(find_add_cmd(const QString&)));

    // setup a watchdog timer to make sure binscanner or sprat aren't waiting for unprompted input
    m_watchdog = new QTimer(this);
    connect(m_watchdog, SIGNAL(timeout()), this, SLOT(process_timer()));
    m_watchdog->start(TIMER_MSEC);

    build_available_version_file();
}


qtsprat::~qtsprat()
{
    delete ui;
}


void qtsprat::build_available_version_file()
{
    QString exe_path = m_config_dlg->exe_path();
    if (0 == exe_path.length())
    {
        // default exe path
        exe_path="C:/Program Files (x86)/NASA FSW Branch/SPRAT";
    }

    QDir dir(exe_path, "*_sprat.exe");

    QStringList file_list = dir.entryList();
    for (int ii = 0; ii< file_list.count(); ii++)
    {
        {
            // qDebug() << "found file " << file_list[ii];

            m_available_versions += file_list[ii];
            m_available_versions.remove("_sprat.exe");
            m_available_versions += "\n";
        }
    }
}


void qtsprat::on_actionOpen_Project_triggered()
{
    qDebug() << "m_spratdir = " << m_spratdir;

    // open file selection box
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "Open SPRAT Project (SAF)",
        m_project_path,
        QFileDialog::ShowDirsOnly);

    if (dir.isEmpty())
        return;

    // strip off just the basename of the SAF
    QFileInfo fi(dir);
    m_project_path = fi.absolutePath();
    m_project = fi.fileName();

    qDebug() << "m_project_path = " << m_project_path;
    qDebug() << "m_project = " << m_project;


    // create script_files folder if it doesn't exist
    QString script_dir = m_project_path + "/" + m_project + "/script_files";
    QDir sdir(script_dir);
    sdir.mkdir(".");

    // launch a build-specific version of the SPRAT executable
    QString build = m_project.mid(0, m_project.indexOf('_'));
    qDebug() << "build = " << build;

    QString cmd = build + "_sprat.exe";

    // kick off the sprat.exe process with the current project (SAF)
    ui->messages_textEdit->clear();
    ui->messages_textEdit->append("starting process " + cmd);
    if (start_process(cmd, m_project_path + "/" + m_project))
    {
        ui->messages_textEdit->append("reading SPRAT MAP file - this may take a while...");
        QFile::remove(m_current_browser_file);
        send_to_process("quiet");

        // configure the ARTEMIS timestamp display
        send_to_process("artemis off");
        ui->actionARTEMIS->setChecked(false);

        send_to_process("altadata off");
        ui->actionAltadata->setChecked(false);

        // select last saved browser and font selection
        send_to_process("browser " + m_browser);
        send_to_process("font_size " + m_font_size);
    }
    else
    {
        // failed to start the process...
        m_project = "";

        QString qmsg = "The SPRAT SAF folder must begin with\n"
                       "one of the following available FSW build strings:\n\n";

        qmsg += m_available_versions;

        qmsg.append("\nand end with either \"_fcas\" or \"_gras\"");

        QMessageBox::warning( this, "qtsprat", qmsg, "OK", 0, 0 );
    }

    // configure the sensitivity of all controls
    configure_gui();
    save_rc();
}


void qtsprat::on_actionRun_binscanner_triggered()
{
    qDebug("run binscanner");

    // open file selection box in $SPRATDIR
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "Run binscanner for Project (Format release_XXXX_fcas)",
        m_project_path,
        QFileDialog::ShowDirsOnly);

    if (dir.isEmpty())
        return;

    // strip off just the basename of the SAF
    QFileInfo fi(dir);
    m_project_path = fi.absolutePath();
    m_project = fi.fileName();

    qDebug() << "m_project_path = " << m_project_path;
    qDebug() << "m_project = " << m_project;

    ui->messages_textEdit->clear();

    erase_binscanner_products(dir);

    // kick off the binscanner.exe process on the selected project
    start_process("binscanner.exe", dir);

    // open a file browser to show binscanner activity in the project
    QDesktopServices::openUrl("file:///" + dir);
    load_browser("file:///" + dir);
}


void qtsprat::on_actionCurrent_Search_Results_triggered()
{
    show_in_browser(m_project_path + "/" + m_project + "/output_files/search_summary.htm");
}


void qtsprat::on_actionCurrent_Measurement_File_triggered()
{
    show_in_browser(m_project_path + "/" + m_project + "/output_files/current_msf_contents.htm");
}


void qtsprat::on_actionAll_Measurement_Files_triggered()
{
    show_in_browser(m_project_path + "/" + m_project + "/output_files/msf_list.htm");
}


void qtsprat::on_actionFlyout_Report_triggered()
{
    show_in_browser(m_project_path + "/" + m_project + "/output_files/flyout_summary.htm");
}


bool qtsprat::start_process(QString proc_name, QString full_project_path)
{
    // note: proc_name needs to be in PATH
    full_project_path.replace("/", "\\");

    m_cmd = "\"";

    QString exe_path = m_config_dlg->exe_path();
    if (exe_path.length())
    {
        m_cmd += exe_path + "\\";
    }
    m_cmd += proc_name;
    m_cmd += "\"";

    if (full_project_path.size())
        m_cmd += " \"" + full_project_path +"\"";

    // apply command line options from config dialog
    if (m_cmd.contains("sprat.exe"))
    {
        if (!m_config_dlg->do_irs())
        {
            m_cmd += " bypass_irs";
        }

        if (!m_config_dlg->get_tlm_files())
        {
            m_cmd += " bypass_tlmfiles";
        }

        if (m_config_dlg->max_fp_digs())
        {
            m_cmd += " MAX_FP_DIGS";
        }

        m_cmd += " bypass_summary";
    }

    // select memory model
    if (m_cmd.contains("scanner"))
    {
        // see if noxml option
        if (m_config_dlg->imacs_tlm_only())
        {
            m_cmd += " noxml";
        }

        // see if gige only
        if (m_config_dlg->gige_only())
        {
            m_cmd += " gige";
        }

        // see if PCAP
        if (m_config_dlg->process_pcap())
        {
            m_cmd += " pcap";
        }

        // see if streaming data
        if (m_config_dlg->process_stream_data())
        {
            m_cmd += " stream";
            // m_segment_dlg->set_segment("MIDDLE");   // default to MIDDLE, can be overridden later
        }

        else if (m_config_dlg->skip_stream_data())
        {
            m_cmd += " skip_stream2bin";
            // m_segment_dlg->set_segment("MIDDLE");   // default to MIDDLE, can be overridden later
        }

        if (m_config_dlg->gige_only() || m_config_dlg->process_pcap())
        {
            QString net_options =
                    " ctc_addr " + m_network_dlg->ctc_ip_addr() +
                    " multicast_addr " + m_network_dlg->tlm_ip_addr() +
                    " tlm_port " + m_network_dlg->tlm_port() +
                    " cmd_port " + m_network_dlg->cmd_port();
            m_cmd += net_options;
        }

        // see if EFI/DFI
        if (m_config_dlg->process_efi())
        {
            m_cmd += " efi";
        }

        if (m_config_dlg->process_dfi())
        {
            m_cmd += " dfi";
        }

        // get selected memory model (if none specified, default is "auto"
        if (MEM_SMALL == m_config_dlg->mem_model())
        {
            m_cmd += " small_mode";
        }
        if (MEM_MEDIUM == m_config_dlg->mem_model())
        {
            m_cmd += " medium_mode";
        }
        else if (MEM_LARGE == m_config_dlg->mem_model())
        {
            m_cmd += " large_mode";
        }
        else if (MEM_HUGE == m_config_dlg->mem_model())
        {
            m_cmd += " huge_mode";
        }


        if (m_config_dlg->use_blackboard_file())
        {
            m_cmd += " blackboard";
        }

        if (m_config_dlg->debug())
        {
            m_cmd += " debug";
        }

        // get the selected segment (FIRST, MIDDLE, or FINAL) from the dialog
        m_segment_dlg->show();
        m_segment_dlg->exec();
        m_cmd += " " + m_segment_dlg->selected_segment();


        // ask about copying support_files
        QString fsw_ver = m_project.mid(0, m_project.indexOf('_'));

        QMessageBox msgBox;
        msgBox.setText("Do you wish to copy the support files for " + fsw_ver + " into " + m_project + "?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();

        if (QMessageBox::Yes == ret)
        {
            // open the config file
            QString spratdir = qgetenv( "SPRATDIR" ).constData();
            QString supp_file_path = spratdir + "/support_files/" + fsw_ver;

            copy_files(supp_file_path, full_project_path);
        }
    }

    qDebug() << "m_cmd = " << m_cmd;

    m_console_app = new QProcess(this);
    m_console_app->setProcessChannelMode(QProcess::MergedChannels);
    m_console_app->setWorkingDirectory(m_spratdir);

    connect(m_console_app, SIGNAL(readyReadStandardOutput()), this, SLOT(read_msg()));
    connect(m_console_app, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(proc_done()));

    ui->messages_textEdit->append("starting process " + m_cmd);

    m_console_app->start(m_cmd);
    if (false == m_console_app->waitForStarted(5000))
    {
        ui->messages_textEdit->append("failed to start " + m_cmd);
        m_project = "";
        configure_gui();

        QMessageBox::warning( this, "qtsprat", "failed to start " + m_cmd, "OK", 0, 0 );
        return false;
    }

    return true;
}


void qtsprat::proc_done()
{
    ui->messages_textEdit->append("process has finished");
    set_wait_cursor(false);
    m_watchdog->stop();

    m_console_app = NULL;

    ui->status_bar_progressBar->setValue(0);
    ui->progress_label->setText("");

    if (m_cmd.contains("scanner"))
    {
        QMessageBox msgBox;
        msgBox.setText("The scanner application has completed.");
        msgBox.setInformativeText("Do you want to open the " + m_project + " project in SPRAT?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);

        msgBox.show();
        msgBox.activateWindow();
        msgBox.raise();
        int ret = msgBox.exec();

        if (QMessageBox::Yes == ret)
        {
            // create script_files folder if it doesn't exist
            QString script_dir = m_project_path + "/" + m_project + "/script_files";
            QDir sdir(script_dir);
            sdir.mkdir(".");

            // launch a build-specific version of the SPRAT executable
            QString build = m_project.mid(0, m_project.indexOf('_'));
            qDebug() << "build = " << build;

            QString cmd = build + "_sprat.exe";

            // kick off the sprat.exe process with the current project (SAF)
            ui->messages_textEdit->clear();
            ui->messages_textEdit->append("starting process " + cmd);
            if (start_process(cmd, m_project_path + "/" + m_project))
            {
                ui->messages_textEdit->append("reading SPRAT MAP file - this may take a while...");
                QFile::remove(m_current_browser_file);
                send_to_process("quiet");

                // configure the ARTEMIS timestamp display
                send_to_process("artemis off");
                ui->actionARTEMIS->setChecked(false);
                send_to_process("altadata off");
                ui->actionAltadata->setChecked(false);

                // select last saved browser and font selection
                send_to_process("browser " + m_browser);
                send_to_process("font_size " + m_font_size);
            }
            else
            {
                m_project = "";
            }

            // configure the sensitivity of all controls
            configure_gui();
            save_rc();
        }
    }
}


void qtsprat::send_to_process(QString qstr)
{
    if (m_console_app)
    {
        // strip any trailing spaces
        while(qstr.endsWith(' '))
        {
            qstr = qstr.mid(0, qstr.size()-1);
        }

        qDebug() << "sending console command: " << qstr;

        QString cmd = qstr + "\n";
        m_console_app->write(cmd.toStdString().c_str(), cmd.size());

        // save the command in the history
        cmd.remove('\n');
        if (cmd.size())
        {
            m_cmd_history.append(cmd);
            m_hist_index = -1;
        }
    }
}


void qtsprat::read_msg()
{
    m_watchdog->start(TIMER_MSEC);

    QByteArray qmsg = m_console_app->readAllStandardOutput();
    QString tmpstr(qmsg);

//    qDebug() << "tmpstr = " << tmpstr;

    static QString qstr = "";
    qstr.append(tmpstr);

    // if this is not a full line, wait and keep appending
    // to avoid those annoying partial lines
    if (!qstr.endsWith("\n") &&
            !qstr.endsWith("\r") &&
            !qstr.endsWith("msf>") &&
            !qstr.contains("(Y/N)?") &&
            !qstr.contains("Press"))
    {
        // qDebug() << "string not complete - returning";
        return;
    }

//    qDebug() << "qstr = " << qstr;

    // split into single lines
    QStringList qlst = qstr.split("\r\n");
    qstr = "";

    foreach(QString line, qlst)
    {
        bool log_me = true;

        // ignore blank lines
        line = line.trimmed();
        if (line.length() == 0)
        {
            log_me = false;
        }

//        qDebug() << "line = " << line;

        // end of output from sprat.exe
        if (line.contains("msf>"))
        {
            line.replace("\nmsf>", "");
            set_wait_cursor(false);
            ui->status_bar_progressBar->setValue(0);
            ui->progress_label->setText("");

            if (m_cmd_chain_dlg)
            {
                m_cmd_chain_dlg->advance();
            }

            if (m_find_add_dlg && m_find_add_dlg->isVisible())
            {
                m_find_add_dlg->reset_cursor();
            }
        }

        if (line.contains("Script Processing Completed"))
        {
            // end of script - show the script output
            qDebug() << "end of script " << m_script;
        }

        // support for find/add dialog
        if (m_find_add_dlg &&  line.contains("hit #"))
        {
            m_find_add_dlg->add_hit(line);
        }

        // interactive prompt from binscanner.exe
        if (line.contains("Press any key", Qt::CaseInsensitive) ||
                line.contains("Press enter", Qt::CaseInsensitive))
        {
            ui->messages_textEdit->append(line);

            QMessageBox::information(this,
                                    "SPRAT Prompt",
                                    "Click OK to continue",
                                    "OK",
                                     QString::null, 0, 1 );
            send_to_process("\n");

            log_me = false;   // message has already been written to message area
            set_wait_cursor(false);
        }

        // update the progress bar
        if (line.contains("Generating Section"))
        {
            QString text = line.mid(line.indexOf("Generating Section"));
            text = text.mid(text.indexOf(':')+2);
            text = text.mid(0, text.indexOf("::")-1);
            ui->progress_label->setText(text);
        }

        if (line.contains("%complete"))
        {
            QString label = line.mid(line.indexOf('('));
            label = label.mid(0, label.indexOf(')')+1);

            QString val_str = line.mid(line.indexOf('=')+1);
            val_str = val_str.mid(0, val_str.indexOf('%'));
            int value = val_str.toInt();

            if (value < 100)
            {
                ui->status_bar_progressBar->setValue(value);
                ui->progress_label->setText(label);
            }
            else
            {
                ui->status_bar_progressBar->setValue(0);
                ui->progress_label->setText("");
            }

            // don't bother printing out the % complete messages, they're in the progress bar
            log_me = false;
        }

        // generate question dialog
        if (line.contains("(Y/N)?"))
        {
            if (0 == QMessageBox::question( this,
                                            "SPRAT Prompt",
                                            line,
                                            "&Yes", "No",
                                            QString::null, 0, 1 ))
            {
                send_to_process("Y");
            }
            else
            {
                send_to_process("N");
            }

            log_me = false;
        }

        // filter out unwanted text
        if (line.contains("+++"))
        {
            log_me = false;
        }
        if (line.contains("SPRAT SAF:"))
        {
            log_me = false;
        }

        if (log_me)
        {
            ui->messages_textEdit->append(line);
        }
    }

    check_browser_file();
}


void qtsprat::on_actionClose_Project_triggered()
{
    if (m_console_app)
    {
        m_console_app->close();
    }
    m_console_app = NULL;
    m_project = "";
    m_msf = "";
    configure_gui();

    load_browser("file:///" + m_spratdir + "/html/welcome.html");

    std::system("TASKKILL /IM qtfep.exe 2> NUL");
    std::system("TASKKILL /IM dem_pb.exe 2> NUL");
    std::system("TASKKILL /IM fswdisplay.exe 2> NUL");
    std::system("TASKKILL /IM cwatool.exe 2> NUL");
    std::system("TASKKILL /IM tlmtool.exe 2> NUL");
    std::system("TASKKILL /IM scripting.exe 2> NUL");
}


void qtsprat::on_actionExit_triggered()
{
    if (0 == QMessageBox::question( this,
                                    "Exit Confirmation - qtsprat",
                                    "Do you really want to exit?",
                                    "&Yes", "Cancel",
                                    QString::null, 0, 1 ))
    {
        if (m_console_app)
        {
            m_console_app->close();
        }

        save_rc();
        ::exit(0);
    }
}


void qtsprat::closeEvent(QCloseEvent* evt)
{
    on_actionExit_triggered();

    // if the exit function returns, ignore the event
    evt->ignore();
}


void qtsprat::on_actionAbout_triggered()
{
    QString build_info;
    build_info.sprintf( "Build Date: %s\nBuild Time: %s\n",
             __DATE__,
             __TIME__ );

    QMessageBox::about( this, "About qtsprat", build_info );
}


void qtsprat::on_actionUsers_Guide_triggered()
{
    show_in_browser(m_spratdir + "/help/SPRAT_Users_Guide.htm");
}



void qtsprat::show_in_browser(QString filename)
{
    QFile file(m_current_browser_file);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QString qstr = filename;

    file.write(qstr.toStdString().c_str(), qstr.size());
    file.close();

    check_browser_file();
}


void qtsprat::load_rc()
{
    qDebug() << "load_rc()";

    QFile file(QDir::homePath() + "/qtsprat.rc");
    qDebug() << "rc file = " << QDir::homePath() << "/qtsprat.rc";

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug("no rc file");
        return;
    }

    while(!file.atEnd())
    {
        QString line = file.readLine();
        line.remove('\n');
        QStringList list = line.split("=");

        if ( 0 == list.first().compare("BROWSER") )
        {
            m_browser = list.last();

            int browser_index = 0;     // default
            for (size_t ii = 0; ii < num_browsers; ii++)
            {
                if (m_browser == browser_tbl[ii].browser_num)
                {
                    browser_index = ii;
                    break;
                }
            }

            m_browser = browser_tbl[browser_index].browser_num;
            ui->browser_comboBox->setCurrentIndex(browser_index);

            qDebug() << "rc: found browser " << m_browser;
        }

        else if ( 0 == list.first().compare("FONT_SIZE") )
        {
            m_font_size = list.last();

            if (m_font_size.toInt() < ui->font_size_spinBox->minimum())
                m_font_size = QString::number(ui->font_size_spinBox->minimum());

            if (m_font_size.toInt() > ui->font_size_spinBox->maximum())
                m_font_size = QString::number(ui->font_size_spinBox->maximum());

            ui->font_size_spinBox->setValue(m_font_size.toInt());

            qDebug() << "rc: found font size " << m_font_size;
        }

        else if ( 0 == list.first().compare("PROJECT_PATH"))
        {
            m_project_path = list.last();
        }
        else if ( 0 == list.first().compare("PROJECT"))
        {
            QString proj = list.last();
            m_project = proj;
        }

        else if ( 0 == list.first().compare("NOXML"))
        {
            m_config_dlg->set_noxml(true);
        }

        else if ( 0 == list.first().compare("MEM_MODEL"))
        {
            QString model = list.last();

            if ("small" == model)
            {
                m_config_dlg->set_mem_model(MEM_SMALL);
            }
            else if ("medium" == model)
            {
                m_config_dlg->set_mem_model(MEM_MEDIUM);
            }
            else if ("large" == model)
            {
                m_config_dlg->set_mem_model(MEM_LARGE);
            }
            else if ("huge" == model)
            {
                m_config_dlg->set_mem_model(MEM_HUGE);
            }
            else
            {
                m_config_dlg->set_mem_model(MEM_AUTO);
            }
        }

        else if ( 0 == list.first().compare("IRS"))
        {
            m_config_dlg->set_irs(true);
        }

        else if ( 0 == list.first().compare("GIGE"))
        {
            m_config_dlg->set_gige_only(true);
        }

        else if ( 0 == list.first().compare("PCAP"))
        {
            m_config_dlg->set_pcap(true);
        }

        else if ( 0 == list.first().compare("STREAM"))
        {
            m_config_dlg->set_stream_data(true);
        }

        else if ( 0 == list.first().compare("SKIP_STREAM"))
        {
            m_config_dlg->set_skip_stream_data(true);
        }

        else if ( 0 == list.first().compare("PLOT_CMD"))
        {
            m_config_dlg->set_plot_cmd(list.last());
        }

        else if ( 0 == list.first().compare("BLACKBOARD"))
        {
            QString use_me = list.last();
            if ("1" == use_me)
            {
                m_config_dlg->set_blackboard(true);
            }
            else
            {
                m_config_dlg->set_blackboard(false);
            }
        }

        else if ( 0 == list.first().compare("EFI"))
        {
            QString use_me = list.last();
            if ("1" == use_me)
            {
                m_config_dlg->set_efi(true);
            }
            else
            {
                m_config_dlg->set_efi(false);
            }
        }

        else if ( 0 == list.first().compare("DFI"))
        {
            QString use_me = list.last();
            if ("1" == use_me)
            {
                m_config_dlg->set_dfi(true);
            }
            else
            {
                m_config_dlg->set_dfi(false);
            }
        }

        else if ( 0 == list.first().compare("MAX_FP_DIGS"))
        {
            QString use_me = list.last();
            if ("1" == use_me)
            {
                m_config_dlg->set_max_fp_digs(true);
            }
            else
            {
                m_config_dlg->set_max_fp_digs(false);
            }
        }

        else if ( 0 == list.first().compare("EXE_PATH"))
        {
            QString path = list.last();
            m_config_dlg->set_exe_path(path);
        }

        else if ( 0 == list.first().compare("BYPASS_TLM"))
        {
            m_config_dlg->set_bypass_tlmfiles();
        }
        else if ( 0 == list.first().compare("USER_MSF_PATH"))
        {
            m_user_msf_path = list.last();
            qDebug() << "m_user_msf_path: " <<  m_user_msf_path;
        }
    }

    file.close();
}



void qtsprat::save_rc()
{
    qDebug() << "save_rc";

    QFile file(QDir::homePath() + "/qtsprat.rc");

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug("rc file open error");
        return;
    }

    QTextStream out( &file );

    out << "BROWSER=" << m_browser << endl;
    out << "FONT_SIZE=" << m_font_size << endl;

    if (m_project_path != "")
        out << "PROJECT_PATH=" << m_project_path << endl;

    if (m_project != "")
        out << "PROJECT=" << m_project << endl;

    if (m_config_dlg->imacs_tlm_only())
    {
        out << "NOXML" << endl;
    }

    if (m_config_dlg->gige_only())
    {
        out << "GIGE" << endl;
    }

    if (m_config_dlg->process_pcap())
    {
        out << "PCAP" << endl;
    }

    if (m_config_dlg->process_stream_data())
    {
        out << "STREAM" << endl;
    }

    if (m_config_dlg->skip_stream_data())
    {
        out << "SKIP_STREAM" << endl;
    }

    if (MEM_SMALL == m_config_dlg->mem_model())
    {
        out << "MEM_MODEL=small" << endl;
    }
    else if (MEM_MEDIUM == m_config_dlg->mem_model())
    {
        out << "MEM_MODEL=medium" << endl;
    }
    else if (MEM_LARGE == m_config_dlg->mem_model())
    {
        out << "MEM_MODEL=large" << endl;
    }
    else if (MEM_HUGE == m_config_dlg->mem_model())
    {
        out << "MEM_MODEL=huge" << endl;
    }
    else
    {
        out << "MEM_MODEL=auto" << endl;
    }


    if (m_config_dlg->do_irs())
    {
        out << "IRS" << endl;
    }

    if (!m_config_dlg->get_tlm_files())
    {
        out << "BYPASS_TLM" << endl;
    }

    if (m_user_msf_path != "")
        out << "USER_MSF_PATH=" << m_user_msf_path << endl;



    out << "BLACKBOARD=" << m_config_dlg->use_blackboard_file() << endl;
    out << "EFI=" << m_config_dlg->process_efi() << endl;
    out << "DFI=" << m_config_dlg->process_dfi() << endl;
    out << "MAX_FP_DIGS=" << m_config_dlg->max_fp_digs() << endl;
    out << "EXE_PATH=" << m_config_dlg->exe_path() << endl;
    out << "PLOT_CMD=" << m_config_dlg->plot_cmd() << endl;

    file.close();
}


void qtsprat::browser_changed(const QString& browser)
{
    qDebug() << "browser changed " << browser;

    m_browser = "0";

    // translate browser string to browser number
    for (size_t ii = 0; ii < num_browsers; ii++)
    {
        if (browser == browser_tbl[ii].browser_str)
        {
            m_browser = browser_tbl[ii].browser_num;
            break;
        }
    }

    qDebug() << "m_browser = " << m_browser;
    send_to_process("browser " + m_browser);

    save_rc();
    return;
}


void qtsprat::on_actionFind_triggered()
{
    // ui->cmd_lineEdit->setText("find ");

    m_find_add_dlg->show();
    m_find_add_dlg->clear();
}


void qtsprat::find_add_cmd(const QString& cmd)
{
    send_to_process(cmd);
}


void qtsprat::on_actionTable_triggered()
{
    m_met_dlg->set_label("Generate HTML Table:");

    m_met_dlg->show();
    if (m_met_dlg->exec() == QDialog::Accepted)
    {
        m_met_dlg->hide();

        QString sprat_cmd = "table " + m_met_dlg->start_met() + " " + m_met_dlg->stop_met();
        send_to_process(sprat_cmd);
    }
}


void qtsprat::on_actionSpreadsheet_triggered()
{
    m_met_dlg->set_label("Generate CSV File:");

    m_met_dlg->show();
    if (m_met_dlg->exec() == QDialog::Accepted)
    {
        m_met_dlg->hide();

        QString sprat_cmd = "csv " + m_met_dlg->start_met() + " " + m_met_dlg->stop_met();
        send_to_process(sprat_cmd);
    }
}


void qtsprat::on_actionPlot_triggered()
{
    m_met_dlg->set_label("Plot Range:");

    m_met_dlg->show();
    if (m_met_dlg->exec() == QDialog::Accepted)
    {
        m_met_dlg->hide();

        QString sprat_cmd = m_config_dlg->plot_cmd() + " " + m_met_dlg->start_met() + " " + m_met_dlg->stop_met();
        send_to_process(sprat_cmd);
    }
}


void qtsprat::on_actionFlyout_triggered()
{
    QFile::remove(m_current_browser_file);

    send_to_process("r 0");     // deselect all reports to start

    // create the flyout report dialog
    if (m_flyout_dlg)
    {
        delete m_flyout_dlg;
        m_flyout_dlg = NULL;
    }

    m_flyout_dlg = new flyout_Dialog(m_project_path + "/" + m_project);

    // select desired sections of flyout report to run
    m_flyout_dlg->show();
    if (m_flyout_dlg->exec() == QDialog::Accepted)
    {
        m_flyout_dlg->hide();
        send_to_process("r 1\n");

        // select last saved browser and font selection
        send_to_process("browser " + m_browser);
        send_to_process("font_size " + m_font_size);
        send_to_process("artemis off");              // turn off ARTEMIS timestamps for all auto-generated tables
        send_to_process("altadata off");             // turn off Altadata timestamps for all auto-generated tables
        send_to_process("r");
    }
}


void qtsprat::on_actionPlayback_triggered()
{
    QFile::remove(m_current_browser_file);

    send_to_process("r 0");     // deselect all reports to start

    // select desired displays to run
    m_playback_dlg->show();
    if (m_playback_dlg->exec() == QDialog::Accepted)
    {
        m_playback_dlg->hide();
        send_to_process("r 2\n");

        // select last saved browser and font selection
        send_to_process("browser " + m_browser);
        send_to_process("font_size " + m_font_size);
        send_to_process("r");
    }
}


void qtsprat::on_actionPrint_Preview_triggered()
{
    QApplication::processEvents();

    QPrinter printer(QPrinter::HighResolution);
//    printer.setOrientation(QPrinter::Landscape);
//    printer.setPageMargins(0.25, 0.25, 0.25, 0.25, QPrinter::Inch);

    QPrintPreviewDialog preview(&printer, this);
    preview.setWindowFlags(Qt::Window);

    connect(&preview, SIGNAL(paintRequested(QPrinter*)),
            this, SLOT(print_slot(QPrinter*)));

    preview.exec();
}


void qtsprat::on_actionPrint_triggered()
{
    QPrinter printer(QPrinter::PrinterResolution);
//    printer.setPageMargins(0.25, 0.25, 0.25, 0.25, QPrinter::Inch);
//    printer.setPageSize(QPrinter::A4);
//    printer.setFullPage(true);

    QPrintDialog* print_dialog = new QPrintDialog(&printer);
    if (print_dialog->exec() != QDialog::Accepted)
    {
        qDebug("print dialog failed");
        return;
    }

    // create document for printing
    qDebug("calling print_slot()");
    print_slot(&printer);
}


// creates document and draws onto printer
void qtsprat::print_slot(QPrinter* printer)
{
    qDebug("print_slot");

    if (printer)
    {
        // print the webview
        qDebug("print the webview");
        ui->webView->print(printer);
    }
}


void qtsprat::cmd_changed()
{
    ui->cmd_lineEdit->setFocus(Qt::OtherFocusReason);
}


void qtsprat::on_actionSubmit_triggered()
{
    send_to_process(ui->cmd_lineEdit->text());
    ui->cmd_lineEdit->setText("");
}


void qtsprat::on_actionLoad_Measurement_File_triggered()
{
    QString filename = QFileDialog::getOpenFileName(
        this,
        "Open Measurement File",
        m_user_msf_path,
        "MSF Files (*.msf)");

    if (filename.isEmpty())
        return;

    QFileInfo fi(filename);
    m_user_msf_path = fi.absolutePath();
    m_msf = fi.fileName();

    filename.replace("/", "\\");            // windows cmd path needs backslashes, not forward slashes - jdc

    send_to_process("load " + filename);

    configure_gui();
}


void qtsprat::on_actionSave_Measurement_File_triggered()
{
    QString filename = m_user_msf_path + "/" + m_msf;
    filename.replace("/", "\\");            // windows cmd path needs backslashes, not forward slashes - jdc

    send_to_process("save " +  filename);
}


void qtsprat::on_actionSave_Measurement_File_As_triggered()
{

    QString filename = QFileDialog::getSaveFileName(
                this,
                "Save Measurement File",
                m_user_msf_path,
                "*.msf",
                0,
                QFileDialog::DontConfirmOverwrite);

    if ( filename.isNull() )
    {
        return;
    }

    QFileInfo fi(filename);
    m_user_msf_path = fi.absolutePath();
    m_msf = fi.baseName();

    QString save_path = fi.absoluteFilePath();
    save_path.replace("/", "\\");

    send_to_process("save " + save_path);
}


void qtsprat::configure_gui()
{
    // configure the pulldown menus

    // configure File menu
    ui->actionOpen_Project->setEnabled((m_project.isEmpty()));
    ui->actionClose_Project->setEnabled((!m_project.isEmpty()));
    ui->actionLoad_Measurement_File->setEnabled(!m_project.isEmpty());
    ui->actionSave_Measurement_File->setEnabled(!m_project.isEmpty());
    ui->actionSave_Measurement_File_As->setEnabled(!m_project.isEmpty());

    // configure Edit menu
    ui->actionEdit_Measurement_File->setEnabled(!m_msf.isEmpty());

    // configure View menu
    ui->actionCurrent_Search_Results->setEnabled(!m_project.isEmpty());
    ui->actionAll_Measurement_Files->setEnabled(!m_project.isEmpty());

    // configure all menu buttons and pushbuttons
    ui->actionFind->setEnabled(!m_project.isEmpty());
    ui->actionList->setEnabled(!m_project.isEmpty());
    ui->actionTable->setEnabled(!m_project.isEmpty());
    ui->actionSpreadsheet->setEnabled(!m_project.isEmpty());
    ui->actionDScope->setEnabled(!m_project.isEmpty());
    ui->actionStop_scope->setEnabled(!m_project.isEmpty());
    ui->actionPlot->setEnabled(!m_project.isEmpty());
    ui->actionMap->setEnabled(!m_project.isEmpty());

    // ui->cmd_lineEdit->setEnabled(m_console_app != NULL);
    ui->cmd_lineEdit->setEnabled(true);

    ui->actionRun_binscanner->setEnabled(m_project.isEmpty());

    ui->actionEdit_Script_File->setEnabled(!m_project.isEmpty());
    ui->actionRun_Script->setEnabled(!m_project.isEmpty());
    ui->actionFlyout->setEnabled(!m_project.isEmpty());
    ui->actionSIL->setEnabled(!m_project.isEmpty());
    ui->actionPlayback->setEnabled(!m_project.isEmpty());
    ui->actionARTEMIS->setEnabled(!m_project.isEmpty());
    ui->actionAltadata->setEnabled(!m_project.isEmpty());
    ui->actionAdd_Pseudo->setEnabled((!m_project.isEmpty()));

    // update the status bar
    ui->status_bar_label->setText("Project: " + m_project +
                               " | MSF: " + m_msf);

    this->setWindowTitle("SPRAT - " + m_project);
}


// see if the current_browser_file has changed
void qtsprat::check_browser_file()
{
    static QDateTime prev_timetag;
    QFileInfo fi(m_current_browser_file);

    // if the file doesn't exist, return
    if (!fi.exists())
    {
        load_browser("file:///" + m_spratdir + "/html/welcome.html");
        return;
    }

    // if the file hasn't been modified since the last time, return
    if (fi.lastModified() == prev_timetag)
        return;

    prev_timetag = fi.lastModified();

    QFile file(m_current_browser_file);

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->messages_textEdit->append("qtsprat: error opening " + m_current_browser_file);
        load_browser("file:///" + m_spratdir + "/html/welcome.html");

        qDebug() << "error opening current browser file " << m_current_browser_file;

        return;
    }

    QString qstr = file.readLine();
    file.close();

    qstr.remove('\n');
    qstr.remove('\t');
    qstr.remove(' ');
    qstr.replace('\\', '/');

    qstr.replace("file:///", "");

    fi.setFile(qstr);
    if (!fi.exists())
    {
        ui->messages_textEdit->append("qtsprat: error opening " + qstr);
        load_browser("file:///" + m_spratdir + "/html/error.html");

        qDebug() << "error opening URL file " << qstr;

        return;
    }

    QString url = "file:///" + qstr;
    ui->webView->load(QUrl(url));

    if (!ui->webView->url().isValid())
    {
        ui->messages_textEdit->append("URL " + url + " is invalid");
        load_browser("file:///" + m_spratdir + "/html/error.html");
    }
}


void qtsprat::font_size_triggered()
{
    m_font_size = QString::number(ui->font_size_spinBox->value());
    send_to_process("font_size " + m_font_size);
    save_rc();
}


void qtsprat::on_actionBack_triggered()
{
    ui->webView->back();
}


void qtsprat::on_actionForward_triggered()
{
    ui->webView->forward();
}


void qtsprat::on_actionReload_triggered()
{
    ui->webView->reload();
}


void qtsprat::on_actionZoom_In_triggered()
{
    ui->webView->setZoomFactor(ui->webView->zoomFactor() + 0.1);
}


void qtsprat::on_actionZoom_Out_triggered()
{
    ui->webView->setZoomFactor(ui->webView->zoomFactor() - 0.1);
}


void qtsprat::set_wait_cursor(bool wait)
{
    if (wait)
        QApplication::setOverrideCursor(Qt::WaitCursor);
    else
    {
        // override cursors can be stacked, returns 0 if stack empty
        while(QApplication::overrideCursor())
        {
            QApplication::restoreOverrideCursor();   // pop
        }
    }

    QApplication::processEvents();
}


void qtsprat::on_actionShow_SPRAT_Base_Directory_triggered()
{
    QFileInfo* fi = new QFileInfo(m_spratdir);
    QDesktopServices::openUrl("file:///" + fi->absoluteFilePath());
}


void qtsprat::erase_binscanner_products(QString path)
{
    QDir dir(path);
    dir.setNameFilters(QStringList() << "binscanner.*" << "*.csv" << "*.map" << "*.dem" << "*.txt");
    dir.setFilter(QDir::Files);
    foreach(QString dirFile, dir.entryList())
    {
        if ((!dirFile.startsWith("CmdVerifier")) &&
            (!dirFile.startsWith("atcs")))
        {
            dir.remove(dirFile);
        }
    }
}


void qtsprat::on_actionRequirements_triggered()
{
    show_in_browser(m_spratdir + "/html/required.html");
}

void qtsprat::on_actionEdit_Measurement_File_triggered()
{
    QString file = m_user_msf_path + "/" + m_msf;
    file.replace("/", "\\");            // windows cmd path needs backslashes, not forward slashes - jdc

    if (!file.contains(".msf"))
        file.append(".msf");

    start_system_process("notepad " + file);
}

void qtsprat::on_actionEdit_Script_File_triggered()
{
    QString filename = QFileDialog::getOpenFileName(
        this,
        "Edit Script File",
        m_project_path + "/" + m_project + "/script_files",
        "Script Files (*.txt)");

    if (filename.isEmpty())
        return;

    start_system_process("notepad " + filename);
}

void qtsprat::on_actionRun_Script_triggered()
{
    QString filename = QFileDialog::getOpenFileName(
        this,
        "Edit Script File",
        m_project_path + "/" + m_project + "/script_files",
        "Script Files (*.txt)");

    if (filename.isEmpty())
        return;

    QFileInfo fi(filename);
    m_script = fi.fileName();

    if (m_script.startsWith("b"))
    {
        send_to_process("batch " + m_script);
    }
    else
    {
        send_to_process("script " + m_script);
    }
}


/**
 * @brief qtsprat::load_browser
 * @param addr QString containing URL to load
 */
void qtsprat::load_browser(QString addr)
{
    ui->webView->load(QUrl(addr));
}


/**
 * @brief qtsprat::on_actionDEM_Packing_Map_Browser_triggered
 */
void qtsprat::on_actionDEM_Packing_Map_Browser_triggered()
{
    start_system_process("dem_browser.exe");
}


/**
 * @brief qtsprat::start_system_process
 */
void qtsprat::start_system_process(QString cmd)
{
    // windows system() barfs if you use forward slashes in paths
    cmd.replace("/", "\\");

    qDebug() << "system: " << cmd;

    QProcess* proc = new QProcess(this);

    ui->messages_textEdit->append("starting " + cmd);
    proc->start(cmd);
    if (false == proc->waitForStarted(5000))
    {
        ui->messages_textEdit->append("failed to start " + cmd);
    }
}



/**
 * @brief qtsprat::keyPressEvent handle up/down arrows for command history
 * @param evt
 */
void qtsprat::keyPressEvent(QKeyEvent* evt)
{
    if (0 == m_cmd_history.size())
        return;

    switch (evt->key())
    {
    case Qt::Key_Up :
    {
        if (m_hist_index < 0)
        {
            m_hist_index = m_cmd_history.size() - 1;
        }
        else
        {
            --m_hist_index;
        }

        if (m_hist_index < 0)
            m_hist_index = 0;

        ui->cmd_lineEdit->setText(m_cmd_history.at(m_hist_index));
        break;
    }

    case Qt::Key_Down :
    {
        if (m_hist_index < 0)
        {
            m_hist_index = m_cmd_history.size() - 1;
        }
        else
        {
            ++m_hist_index;
        }

        if (m_hist_index >= m_cmd_history.size())
            m_hist_index = m_cmd_history.size() -1;

        ui->cmd_lineEdit->setText(m_cmd_history.at(m_hist_index));
        break;
    }
    }
}


/**
 * @brief qtsprat::on_actionRun_Command_Chain_triggered
 */
void qtsprat::on_actionRun_Command_Chain_triggered()
{
    QString filename = QFileDialog::getOpenFileName(
        this,
        "Run Command Chain",
        m_spratdir + "/support_files/command_chains",
        "Command Chains (*.chn)");

    if (filename.isEmpty())
        return;

    qDebug() << "command chain " << filename;

    m_cmd_chain_dlg = new cmd_chain_dialog(filename);
    connect(m_cmd_chain_dlg, SIGNAL(next_cmd(QString&)), this, SLOT(get_next_cmd(QString&)));
}


/**
 * @brief qtsprat::get_next_cmd
 * @param cmd
 */
void qtsprat::get_next_cmd(QString& cmd)
{
    qDebug() << "get_next_cmd() " << cmd;
    send_to_process(cmd);
}


/**
 * @brief qtsprat::on_actionEdit_Command_Chain_triggered
 */
void qtsprat::on_actionEdit_Command_Chain_triggered()
{
    QString filename = QFileDialog::getOpenFileName(
        this,
        "Edit Command Chain",
        m_project_path + "/" + m_project + "/command_chains",
        "Command Chains (*.chn)");

    if (filename.isEmpty())
        return;

    start_system_process("notepad " + filename);
}


/**
 * @brief qtsprat::process_timer
 */
void qtsprat::process_timer()
{
    if (NULL == m_console_app)
        return;

    qDebug("timer expired");
    send_to_process("\n");
}


void qtsprat::on_actionDScope_triggered()
{
    m_met_dlg->set_label("Data Scope:");

    m_met_dlg->show();
    if (m_met_dlg->exec() == QDialog::Accepted)
    {
        m_met_dlg->hide();

        QString sprat_cmd = "dscope " + m_met_dlg->start_met() + " " + m_met_dlg->stop_met();
        if (!m_met_dlg->qualifiers().isEmpty())
        {
            sprat_cmd += " ";
            sprat_cmd += m_met_dlg->qualifiers();
        }

        send_to_process(sprat_cmd);
    }
}

void qtsprat::on_actionStop_scope_triggered()
{
    qDebug("stop dscope");

    if (0 == QMessageBox::question( this,
                                    "Stop DataScope Confirmation",
                                    "Are you sure you want to abort the DataScope operation?",
                                    "&Yes", "Cancel",
                                    QString::null, 0, 1 ))
    {
        // create file that the dscope command checks for existence
        QFile file(m_spratdir + "stop_scope.txt");
        if(file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            file.write("stop dscope");
            file.close();
        }
    }
}



void qtsprat::on_actionConfiguration_triggered()
{
    m_config_dlg->show();
}

void qtsprat::on_actionMap_triggered()
{
    m_map_dlg->show();
    if (m_map_dlg->exec() == QDialog::Accepted)
    {
        QString sprat_cmd = "map " + m_map_dlg->start_met() + " " + m_map_dlg->stop_met();

        if (!m_map_dlg->dem().isEmpty())
        {
            // this should handle any reasonable delimiters between DEM content IDs
            QStringList dem_list = m_map_dlg->dem().split(QRegExp("\\W+"), QString::SkipEmptyParts);

            foreach(const QString &dem, dem_list)
            {
                sprat_cmd += " dem";
                sprat_cmd += dem;
            }
        }

        if (m_map_dlg->simple())
        {
            sprat_cmd += " simple";
        }

        if (m_map_dlg->timeline())
        {
            sprat_cmd += " timeline";
        }

        if (!m_map_dlg->qualifiers().isEmpty())
        {
            sprat_cmd += " ";
            sprat_cmd += m_met_dlg->qualifiers();
        }

        qDebug() << "sprat_cmd = " << sprat_cmd;
        send_to_process(sprat_cmd);
    }
}


void qtsprat::on_actionSAF_Maintenance_triggered()
{
    m_saf_maint_dlg->set_base_path(m_project_path);
    m_saf_maint_dlg->show();
    m_saf_maint_dlg->exec();

    return;
}


void qtsprat::on_actionARTEMIS_triggered()
{
    if (ui->actionARTEMIS->isChecked())
    {
        send_to_process("artemis on");
    }
    else
    {
        send_to_process("artemis off");
    }
}

void qtsprat::on_actionSIL_triggered()
{
    QFile::remove(m_current_browser_file);

    send_to_process("r 0");     // deselect all reports to start

    // create the flyout report dialog
    if (m_sil_dlg)
    {
        delete m_sil_dlg;
        m_sil_dlg = NULL;
    }

    m_sil_dlg = new sil_Dialog(m_project_path + "/" + m_project);

    // select desired sections of SIL integration report to run
    m_sil_dlg->show();
    if (m_sil_dlg->exec() == QDialog::Accepted)
    {
        m_sil_dlg->hide();
        send_to_process("r 5\n");

        // select last saved browser and font selection
        send_to_process("browser " + m_browser);
        send_to_process("font_size " + m_font_size);
        send_to_process("artemis off");              // turn off ARTEMIS timestamps for all auto-generated tables
        send_to_process("r");
    }
}


bool qtsprat::copy_files(const QString &src, const QString &dest)
{
    ui->messages_textEdit->append("copying files from " + src + " to " + dest);

    QDir dir(src);
    QDir dirdest(dest);

    if(!dir.isReadable())
        return false;

    QFileInfoList entries = dir.entryInfoList();
    for(QList<QFileInfo>::iterator it = entries.begin(); it!=entries.end(); ++it)
    {
        QFileInfo &finfo = *it;

        if(finfo.fileName() == "." || finfo.fileName() == "..")
            continue;

        if(finfo.isFile() && finfo.isReadable())
        {
            QFile file(finfo.filePath());
            file.remove(dirdest.absoluteFilePath(finfo.fileName()));
            file.copy(dirdest.absoluteFilePath(finfo.fileName()));
        }

        else
        {
            return false;
        }
    }
    return true;
}

void qtsprat::on_actionSearch_triggered()
{
    qDebug() << "search";

    m_find_dlg->not_found(false);
    m_find_dlg->show();
}


void qtsprat::find_fwd(const QString& txt)
{
    QWebPage::FindFlags flags = 0;
    if (m_find_dlg->highlight_all())
    {
        flags |= QWebPage::HighlightAllOccurrences;
    }

    ui->webView->findText(txt, flags);
}


void qtsprat::find_back(const QString& txt)
{
    QWebPage::FindFlags flags = QWebPage::FindBackward;
    if (m_find_dlg->highlight_all())
    {
        flags |= QWebPage::HighlightAllOccurrences;
    }

    ui->webView->findText(txt, flags);
}


void qtsprat::on_actionAltadata_triggered()
{
    if (ui->actionAltadata->isChecked())
    {
        send_to_process("altadata on");
    }
    else
    {
        send_to_process("altadata off");
    }
}

void qtsprat::on_actionAdd_Pseudo_triggered()
{
    m_add_pseudo_dlg->show();
}


void qtsprat::add_pseudo(const QString& txt)
{
    send_to_process("add " + txt);
}


void qtsprat::find_add_search_term(const QString& txt)
{
    qDebug() << "find/add search term: " << txt;
    send_to_process("find " + txt);
}


void qtsprat::on_actionList_triggered()
{
    send_to_process("list");
}

void qtsprat::on_actionWhat_s_New_triggered()
{
    qDebug() << "what's new";
    show_in_browser(m_spratdir + "/msi_log.txt");
}

void qtsprat::on_actionNetwork_triggered()
{
    m_network_dlg->show();
}

void qtsprat::on_actionFlyout_Module_Info_triggered()
{
    qDebug() << "flyout module info";
    show_in_browser(m_spratdir + "/help/flyout_module_info/index.html");
}
