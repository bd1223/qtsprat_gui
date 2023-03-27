#ifndef QTSPRAT_H
#define QTSPRAT_H

#include <QMainWindow>
#include <QProcess>
#include <QtWidgets>
#include <QPrinter>
#include <QKeyEvent>

#include "flyout_dialog.h"
#include "sil_dialog.h"
#include "playback_dialog.h"
#include "met_range_dialog.h"
#include "cmd_chain_dialog.h"
#include "config_dialog.h"
#include "map_dialog.h"
#include "saf_maint_dialog.h"
#include "find_dialog.h"
#include "add_pseudo_dialog.h"
#include "segment_dialog.h"
#include "find_add_dialog.h"
#include "network_dialog.h"


// 5 minute watchdog timer
#define TIMER_MSEC 300000


namespace Ui
{
class qtsprat;
}


struct browser_tbl_t
{
    QString browser_str;
    QString browser_num;
};


class qtsprat : public QMainWindow
{
    Q_OBJECT

public:
    explicit qtsprat(QWidget *parent = 0);
    ~qtsprat();

private slots:
    void on_actionOpen_Project_triggered();
    void on_actionClose_Project_triggered();
    void on_actionLoad_Measurement_File_triggered();
    void on_actionPrint_triggered();
    void on_actionPrint_Preview_triggered();
    void on_actionExit_triggered();
    void on_actionCurrent_Search_Results_triggered();
    void on_actionAll_Measurement_Files_triggered();
    void on_actionFlyout_Report_triggered();
    void on_actionAbout_triggered();
    void on_actionRun_binscanner_triggered();
    void on_actionUsers_Guide_triggered();
    void on_actionFind_triggered();
    void on_actionTable_triggered();
    void on_actionSpreadsheet_triggered();
    void on_actionPlot_triggered();
    void on_actionFlyout_triggered();
    void on_actionPlayback_triggered();
    void on_actionSubmit_triggered();
    void on_actionBack_triggered();
    void on_actionForward_triggered();
    void on_actionReload_triggered();
    void on_actionZoom_In_triggered();
    void on_actionZoom_Out_triggered();
    void on_actionShow_SPRAT_Base_Directory_triggered();
    void on_actionRequirements_triggered();
    void on_actionEdit_Measurement_File_triggered();
    void on_actionSave_Measurement_File_triggered();
    void on_actionSave_Measurement_File_As_triggered();
    void on_actionCurrent_Measurement_File_triggered();
    void on_actionEdit_Script_File_triggered();
    void on_actionRun_Script_triggered();
    void on_actionDEM_Packing_Map_Browser_triggered();
    void on_actionRun_Command_Chain_triggered();
    void on_actionEdit_Command_Chain_triggered();
    void on_actionDScope_triggered();
    void on_actionStop_scope_triggered();
    void on_actionConfiguration_triggered();
    void on_actionMap_triggered();
    void on_actionSAF_Maintenance_triggered();
    void on_actionARTEMIS_triggered();
    void on_actionSIL_triggered();
    void on_actionSearch_triggered();
    void on_actionAltadata_triggered();
    void on_actionAdd_Pseudo_triggered();

    void keyPressEvent(QKeyEvent* evt);

    void get_next_cmd(QString& cmd);
    void proc_done();
    void read_msg();
    void browser_changed(const QString& browser);
    void cmd_changed();
    void print_slot(QPrinter* printer);
    void font_size_triggered();
    void configure_gui();
    void closeEvent(QCloseEvent* evt);
    void process_timer();
    void find_fwd(const QString& txt);
    void find_back(const QString& txt);
    void add_pseudo(const QString& txt);
    void find_add_search_term(const QString&);
    void find_add_cmd(const QString&);
    void build_available_version_file();

    void on_actionList_triggered();

    void on_actionWhat_s_New_triggered();

    void on_actionNetwork_triggered();

    void on_actionFlyout_Module_Info_triggered();

private:
    Ui::qtsprat *ui;
    flyout_Dialog* m_flyout_dlg;
    sil_Dialog* m_sil_dlg;
    playback_Dialog* m_playback_dlg;
    met_range_Dialog* m_met_dlg;
    cmd_chain_dialog* m_cmd_chain_dlg;
    config_dialog* m_config_dlg;
    map_dialog* m_map_dlg;
    saf_maint_dialog* m_saf_maint_dlg;
    find_dialog* m_find_dlg;
    add_pseudo_dialog* m_add_pseudo_dlg;
    segment_dialog* m_segment_dlg;
    find_add_dialog* m_find_add_dlg;
    network_dialog* m_network_dlg;
    QProcess* m_console_app;

    QString m_spratdir;
    QString m_project_path;
    QString m_user_msf_path;
    QString m_project;
    QString m_browser;
    QString m_msf;
    QString m_current_browser_file;
    QString m_font_size;
    QString m_script;
    QStringList m_cmd_history;
    int m_hist_index;
    QTimer* m_watchdog;
    QString m_cmd;    // current comamnd line for console app
    QString m_available_versions;

    bool start_process(QString proc_name, QString project);
    void send_to_process(QString q_str);
    void load_rc();
    void save_rc();
    void show_in_browser(QString file);
    void check_browser_file();
    void load_browser(QString addr);

    void set_wait_cursor(bool wait);
    void start_system_process(QString cmd);
    void erase_binscanner_products(QString path);
    bool copy_files(const QString &src, const QString &dest);

};

#endif // QTSPRAT_H
