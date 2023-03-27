#ifndef SEGMENT_DIALOG_H
#define SEGMENT_DIALOG_H

#include <QDialog>

namespace Ui {
class segment_dialog;
}

class segment_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit segment_dialog(QWidget *parent = 0);
    ~segment_dialog();

    QString selected_segment();
    void set_segment(QString seg);


private:
    Ui::segment_dialog* ui;
};

#endif // SEGMENT_DIALOG_H
