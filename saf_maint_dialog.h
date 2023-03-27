#ifndef SAF_MAINT_DIALOG_H
#define SAF_MAINT_DIALOG_H

#include <QDialog>
#include <QtWidgets>

namespace Ui {
class saf_maint_dialog;
}


enum
{
    COL_SAF    = 0,
    COL_REC    = 1,
    COL_OUTPUT = 2,
    COL_SPRAT  = 3,
    COL_TOTAL  = 4,
    COL_MOD    = 5,
    COL_CLEAN  = 6,
    COL_MOTH   = 7
};



class saf_maint_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit saf_maint_dialog(QWidget *parent = 0);
    ~saf_maint_dialog();
    void set_base_path(QString path);

private slots:
    void on_browse_pushButton_clicked();
    void on_close_pushButton_clicked();
    void on_clean_pushButton_clicked();
    void on_mothball_pushButton_clicked();
    void on_sel_all_pushButton_clicked();
    void on_sel_none_pushButton_clicked();
    void proc_done();
    void read_msg();
    void selection_changed();

private:
    Ui::saf_maint_dialog *ui;
    QString m_base_path;
    QProcess* m_console_app;

    // member functions
    void populate_table();
    qint64 dir_size(QString path, QStringList include_filter, QStringList exclude_filter);
    qint64 dir_size(QString path);
    QDateTime dir_mod_time(QString path);
    bool is_clean(QString path);
    bool is_mothballed(QString path);
    bool start_process(QString proc_name, QString full_project_path, QStringList options);
    void configure_buttons();
    void run_binscanner(QStringList options);


};

#endif // SAF_MAINT_DIALOG_H
