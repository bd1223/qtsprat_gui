#ifndef NETWORK_DIALOG_H
#define NETWORK_DIALOG_H

#include <QDialog>

namespace Ui {
class network_dialog;
}

class network_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit network_dialog(QWidget *parent = 0);
    ~network_dialog();

    QString ctc_ip_addr();
    QString tlm_ip_addr();
    QString tlm_port();
    QString cmd_port();

private:
    Ui::network_dialog *ui;
};

#endif // NETWORK_DIALOG_H
