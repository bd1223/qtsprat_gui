#ifndef CONFIG_DIALOG_H
#define CONFIG_DIALOG_H

#include <QDialog>

namespace Ui
{
    class config_dialog;
}

enum mem_model_t
{
    MEM_SMALL,
    MEM_MEDIUM,
    MEM_LARGE,
    MEM_HUGE,
    MEM_AUTO
};

#define DEFAULT_STR "<default>"

class config_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit config_dialog(QWidget *parent = 0);
    ~config_dialog();

    bool imacs_tlm_only();
    bool do_irs();
    bool gige_only();
    bool get_tlm_files();
    mem_model_t mem_model();
    bool use_blackboard_file();
    bool process_efi();
    bool process_dfi();
    bool max_fp_digs();
    bool process_pcap();
    bool process_stream_data();
    bool skip_stream_data();
    bool debug();
    QString exe_path();
    QString plot_cmd();

    void set_noxml(bool checked);
    void set_mem_model(mem_model_t model);
    void set_irs(bool checked);
    void set_gige_only(bool checked);
    void set_pcap(bool checked);
    void set_stream_data(bool checked);
    void set_skip_stream_data(bool checked);
    void set_blackboard(bool use_me);
    void set_efi(bool use_me);
    void set_dfi(bool use_me);
    void set_max_fp_digs(bool use_me);
    void set_exe_path(QString exe_path);
    void set_plot_cmd(QString plot_cmd);
    void set_bypass_tlmfiles();


private slots:
    void on_exe_browse_pushButton_clicked();
    void on_pcap_checkBox_clicked();
    void on_stream_checkBox_clicked();
    void on_gige_checkBox_clicked();
    void on_skip_stream_checkBox_clicked();

private:
    Ui::config_dialog *ui;
};

#endif // CONFIG_DIALOG_H
