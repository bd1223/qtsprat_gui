#include "playback_dialog.h"
#include "ui_playback_dialog.h"


display_t playback_displays[] =
{
    // name             filename         port   checkbox widget
    { "FC1 HS",         "fc1_hs",        15001, NULL },
    { "FC2 HS",         "fc2_hs",        15002, NULL },
    { "FC3 HS",         "fc3_hs",        15003, NULL },
    { "C/W/Aborts",     "cwatool",       0,     NULL },     // separate app
    { "EPS",            "eps",           15016, NULL },
    { "FCRM",           "fcrm_status",   15010, NULL },
    { "Scripting",      "scripting",     0,     NULL },     // separate app
    { "GNC",            "gnc",           15005, NULL },
    { "GNC Plots",      "gnc_plots",     15025, NULL },
    { "MPS",            "mps",           15006, NULL },
    { "1553",           "m1553",         15017, NULL },
    { "OMD",            "omd",           15011, NULL },
    { "PFD",            "pfd",           0,     NULL },     // separate app
    { "Safing Events",  "safing_events", 15015, NULL },
    { "Scripting",      "scripting",     0,     NULL },     // separate app
    { "SSM Events",     "ssm_events",    15008, NULL },
    { "Summary",        "summary",       16000, NULL },
    { "Throttle",       "throttle",      15018, NULL },
    { "TVC Plots",      "tvc_plots",     15007, NULL },
    { "Telemetry Tool", "tlmtool",       0,     NULL }      // separate app
};

const size_t num_playback_displays = sizeof(playback_displays) / sizeof(playback_displays[0]);


playback_Dialog::playback_Dialog(QString spratdir, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::playback_Dialog),
    m_spratdir(spratdir),
    m_cfg_file(m_spratdir + "/playback_displays.cfg")
{
    ui->setupUi(this);

    // populate the grid in the playback displays table with the available display names
    size_t row = 0;
    size_t col = 0;

    for (size_t display = 0; display < num_playback_displays; display++)
    {
        row = display / MAX_COLS;
        col = display % MAX_COLS;

        QCheckBox* chkbox = new QCheckBox(playback_displays[display].name);
        playback_displays[display].chkbox = chkbox;
        chkbox->setChecked(run_me(playback_displays[display].name));
        ui->playback_gridLayout->addWidget(chkbox, row, col);
    }

    connect(ui->all_pushButton, SIGNAL(clicked(bool)), this, SLOT(select_all()));
    connect(ui->none_pushButton, SIGNAL(clicked(bool)), this, SLOT(select_none()));
}


playback_Dialog::~playback_Dialog()
{
    delete ui;
}


// write list of display filenames to run to a file
void playback_Dialog::on_buttonBox_accepted()
{
    QFile file(m_cfg_file);

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    for (size_t ii = 0; ii < num_playback_displays; ii++)
    {
        if (playback_displays[ii].chkbox->isChecked())
        {
            QString qstr = playback_displays[ii].display;

            if (QString::number(playback_displays[ii].udp_port) != 0)
            {
                qstr += ", ";
                qstr += QString::number(playback_displays[ii].udp_port);
                qstr += "\n";
            }

            file.write(qstr.toStdString().c_str());
        }
    }

    file.close();
}


void playback_Dialog::select_all()
{
    for (size_t ii = 0; ii < num_playback_displays; ii++)
    {
        playback_displays[ii].chkbox->setChecked(true);
    }
}


void playback_Dialog::select_none()
{
    for (size_t ii = 0; ii < num_playback_displays; ii++)
    {
        playback_displays[ii].chkbox->setChecked(false);
    }
}


bool playback_Dialog::run_me(QString& display)
{

    bool rtn = false;

    // set the status of the checkboxes according to the config file
    QFile file(m_cfg_file);

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    while(!file.atEnd())
    {
        QString line = file.readLine();
        line.remove('\n');
        QStringList list = line.split("=");

        if (0 == list.first().compare(display))
        {
            QString value = list.last();
            if (value.contains("YES"))
            {
                rtn = true;
            }
        }
    }

    file.close();

    return rtn;
}

