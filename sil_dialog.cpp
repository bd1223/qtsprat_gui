#include "sil_dialog.h"
#include "ui_sil_dialog.h"


sil_section_t sil_sections[] =
{
    // name                    checkbox widget  FCAS   GRAS
    { "CAPUC",                 NULL,            true,  true  },
    { "CCSE",                  NULL,            true,  true  },
    { "CSEC",                  NULL,            true,  true  },
    { "CTC",                   NULL,            true,  true  },
    { "DACU",                  NULL,            true,  true  },
    { "FC",                    NULL,            true,  true  },
    { "LCCs",                  NULL,            true,  true  },
    { "MPCV",                  NULL,            true,  true  },
    { "PDCU",                  NULL,            true,  true  },
    { "RGA",                   NULL,            true,  true  },
    { "RINU",                  NULL,            true,  true  },
    { "SRB",                   NULL,            true,  true  },
    { "TAC",                   NULL,            true,  true  },
    { "PAD SAFE",              NULL,            true,  true  },
    { "GR Checks",             NULL,            true,  true  }
};

const size_t num_sil_sections = sizeof(sil_sections) / sizeof(sil_sections[0]);


sil_Dialog::sil_Dialog(QString dir, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::sil_Dialog),
    m_dir(dir)
{
    ui->setupUi(this);

    // populate the grid in the SIL sections table with the available section names
    size_t row = 0;
    size_t col = 0;

    QString project = (dir.endsWith("fcas", Qt::CaseInsensitive)) ? "fcas" : "gras";

    for (size_t section = 0; section < num_sil_sections; section++)
    {
        sil_sections[section].chkbox = NULL;

        // skip GRAS-only sections
        if ((project == "fcas") && (!sil_sections[section].fcas))
            continue;

        // skip FCAS-only sections
        if ((project == "gras") && (!sil_sections[section].gras))
            continue;

        QCheckBox* chkbox = new QCheckBox(sil_sections[section].name);
        sil_sections[section].chkbox = chkbox;

        chkbox->setChecked(is_set_in_cfg_file(sil_sections[section].name));
        ui->sil_gridLayout->addWidget(chkbox, row, col);

        if (++col >= MAX_COLS)
        {
            col = 0;
            ++row;
        }
    }

    connect(ui->all_pushButton, SIGNAL(clicked(bool)), this, SLOT(select_all()));
    connect(ui->none_pushButton, SIGNAL(clicked(bool)), this, SLOT(select_none()));
}

sil_Dialog::~sil_Dialog()
{
    delete ui;
}

void sil_Dialog::on_buttonBox_accepted()
{
    QString cfg_file = m_dir + "/sil_report.cfg";
    QFile file(cfg_file);

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    for (size_t ii = 0; ii < num_sil_sections; ii++)
    {
        QString qstr = sil_sections[ii].name + "=";

        if ((sil_sections[ii].chkbox) && (sil_sections[ii].chkbox->isChecked()))
            qstr += "YES\n";
        else
            qstr += "NO\n";

        file.write(qstr.toStdString().c_str(), qstr.size());
    }

    file.close();
}


void sil_Dialog::select_all()
{
    for (size_t ii = 0; ii < num_sil_sections; ii++)
    {
        if (sil_sections[ii].chkbox)
            sil_sections[ii].chkbox->setChecked(true);
    }
}


void sil_Dialog::select_none()
{
    for (size_t ii = 0; ii < num_sil_sections; ii++)
    {
        if (sil_sections[ii].chkbox)
            sil_sections[ii].chkbox->setChecked(false);
    }
}


bool sil_Dialog::is_set_in_cfg_file(QString& section)
{

    bool rtn = true;

    // set the status of the checkboxes according to the config file
    QString cfg_file = m_dir + "/sil_report.cfg";
    QFile file(cfg_file);

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "error opening SIL report config file " << cfg_file;
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



