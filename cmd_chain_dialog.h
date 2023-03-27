#ifndef CMD_CHAIN_DIALOG_H
#define CMD_CHAIN_DIALOG_H

#include <QDialog>

namespace Ui {
class cmd_chain_dialog;
}

class cmd_chain_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit cmd_chain_dialog(QString& filename, QWidget *parent = 0);
    ~cmd_chain_dialog();
    void advance();

signals:
    void next_cmd(QString& cmd);

private slots:
    void on_pause_pushButton_clicked();
    void on_start_pushButton_clicked();
    void on_step_pushButton_clicked();

private:
    Ui::cmd_chain_dialog *ui;
    bool m_paused;

    void configure_gui();
};

#endif // CMD_CHAIN_DIALOG_H
