#ifndef PLAYBACK_DIALOG_H
#define PLAYBACK_DIALOG_H

#include <QDialog>
#include <QtWidgets>


#define MAX_COLS    3


struct display_t
{
    QString     name;          // name on menu
    QString     display;       // filename of display
    int         udp_port;      // UDP port that the display listens to
    QCheckBox*  chkbox;        // checkbox widget
};


namespace Ui
{
class playback_Dialog;
}


class playback_Dialog : public QDialog
{
    Q_OBJECT


public:
    explicit playback_Dialog(QString spratdir, QWidget *parent = 0);
    ~playback_Dialog();

private slots:
    void on_buttonBox_accepted();
    void select_all();
    void select_none();

private:
    Ui::playback_Dialog *ui;
    QString m_spratdir;
    QString m_cfg_file;

    bool run_me(QString& section);
};

#endif // PLAYBACK_DIALOG_H
