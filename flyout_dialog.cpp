#include "flyout_dialog.h"
#include "ui_flyout_dialog.h"


flyout_section_t flyout_sections[] =
{
    // name                    checkbox widget  FCAS   GRAS
    { "1750a Validity",        NULL,            true,  true  },
    { "Aborts and Warnings",   NULL,            true,  true  },
    { "ALS",                   NULL,            true,  true  },
    { "Ascent Tgt Update",     NULL,            true,  false },
    { "Attitude Hold",         NULL,            true,  false },
    { "Auto-Generated Files",  NULL,            true,  true  },
    { "Cmd Response Timing",   NULL,            true,  true  },
    { "Cmd Verifiers",         NULL,            true,  true  },
    { "CPU Usage",             NULL,            true,  true  },
    { "CS Disposal",           NULL,            true,  false },
    { "CSE Performance",       NULL,            true,  true  },
    { "CTC Commanding",        NULL,            true,  true  },
    { "Cyclic CSTVC Cmds",     NULL,            true,  false },
    { "Discrete Control",      NULL,            true,  true  },
    { "DQI-Engines",           NULL,            true,  true  },
    { "DQI-MPS",               NULL,            true,  true  },
    { "DQI-CSTVC",             NULL,            true,  true  },
    { "DQI-Misc",              NULL,            true,  true  },
    { "DQI-SRB",               NULL,            true,  false },
    { "FC-CTC Time Sync",      NULL,            true,  true  },
    { "FCRM",                  NULL,            true,  true  },
    { "FCOG Drift",            NULL,            true,  true  },
    { "Green Run Control",     NULL,            false, true  },
    { "M1553 TBM Timing",      NULL,            true,  true  },
    { "M1553 Validity",        NULL,            true,  true  },
    { "File Transfer",         NULL,            true,  true  },
    { "First Motion",          NULL,            true,  false },
    { "FSW Errors",            NULL,            true,  true  },
    { "ICPS",                  NULL,            true,  false },
    { "LCCs",                  NULL,            true,  true  },
    { "Ullage Control",        NULL,            true,  true  },
    { "MECO Analysis",         NULL,            true,  false },
    { "MECO Initiation",       NULL,            true,  false },
    { "Mode Transitions",      NULL,            true,  true  },
    { "MPS",                   NULL,            true,  true  },
//    { "NTP",                   NULL,            true,  true  },
    { "OMD",                   NULL,            true,  true  },
    { "Open Loop Guidance",    NULL,            true,  false },
//    { "PAR Verification",      NULL,            true,  true  },
    { "RGA",                   NULL,            true,  true  },
    { "RINU",                  NULL,            true,  true  },
//    { "RINU Quaternion Check", NULL,            true,  true  },
    { "Safing Events",         NULL,            true,  true  },
    { "SRB/CSTVC Cmd Timing",  NULL,            true,  false },
    { "SRB HPU Startup",       NULL,            true,  false },
    { "SRB Ignition",          NULL,            true,  false },
    { "SRB Separation",        NULL,            true,  false },
    { "SRB TVC",               NULL,            true,  false },
    { "SSM Events",            NULL,            true,  true  },
    { "Throttle Control",      NULL,            true,  true  }
};

const size_t num_flyout_sections = sizeof(flyout_sections) / sizeof(flyout_sections[0]);


flyout_Dialog::flyout_Dialog(QString dir, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::flyout_Dialog),
    m_dir(dir)
{
    ui->setupUi(this);

    // populate the grid in the flyout sections table with the available section names
    size_t row = 0;
    size_t col = 0;

    QString project = (dir.endsWith("fcas", Qt::CaseInsensitive)) ? "fcas" : "gras";

    for (size_t section = 0; section < num_flyout_sections; section++)
    {
        flyout_sections[section].chkbox = NULL;

        // skip GRAS-only sections
        if ((project == "fcas") && (!flyout_sections[section].fcas))
            continue;

        // skip FCAS-only sections
        if ((project == "gras") && (!flyout_sections[section].gras))
            continue;

        QCheckBox* chkbox = new QCheckBox(flyout_sections[section].name);
        flyout_sections[section].chkbox = chkbox;

        chkbox->setChecked(is_set_in_cfg_file(flyout_sections[section].name));
        ui->flyout_gridLayout->addWidget(chkbox, row, col);

        if (++col >= MAX_COLS)
        {
            col = 0;
            ++row;
        }

        // disable FC-CTC Time Sync module by default until more mature
        if (chkbox->text() == "FC-CTC Time Sync")
        {
            chkbox->setChecked(false);
        }
    }

    connect(ui->all_pushButton, SIGNAL(clicked(bool)), this, SLOT(select_all()));
    connect(ui->none_pushButton, SIGNAL(clicked(bool)), this, SLOT(select_none()));
}


flyout_Dialog::~flyout_Dialog()
{
    delete ui;
}


void flyout_Dialog::on_buttonBox_accepted()
{
    QString cfg_file = m_dir + "/flyout_report.cfg";
    QFile file(cfg_file);

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    for (size_t ii = 0; ii < num_flyout_sections; ii++)
    {
        QString qstr = flyout_sections[ii].name + "=";

        if ((flyout_sections[ii].chkbox) && (flyout_sections[ii].chkbox->isChecked()))
            qstr += "YES\n";
        else
            qstr += "NO\n";

        file.write(qstr.toStdString().c_str(), qstr.size());
    }

    file.close();
}


void flyout_Dialog::select_all()
{
    for (size_t ii = 0; ii < num_flyout_sections; ii++)
    {
        if (flyout_sections[ii].chkbox)
            flyout_sections[ii].chkbox->setChecked(true);
    }
}


void flyout_Dialog::select_none()
{
    for (size_t ii = 0; ii < num_flyout_sections; ii++)
    {
        if (flyout_sections[ii].chkbox)
            flyout_sections[ii].chkbox->setChecked(false);
    }
}


bool flyout_Dialog::is_set_in_cfg_file(QString& section)
{

    bool rtn = true;

    // set the status of the checkboxes according to the config file
    QString cfg_file = m_dir + "/flyout_report.cfg";
    QFile file(cfg_file);

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "error opening flyout report config file " << cfg_file;
        return true;
    }

    while(!file.atEnd())
    {
        QString line = file.readLine();
        line.remove('\n');
        QStringList list = line.split("=");

        if (0 == list.first().compare(section))
        {
            QString value = list.last();
            if (value.contains("NO"))
            {
                rtn = false;
            }
        }
    }

    file.close();

    return rtn;
}


