#ifndef OUTPUTDIALOG_H
#define OUTPUTDIALOG_H

#include <QDialog>

namespace Ui {
class OutputDialog;
}

class OutputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OutputDialog(QWidget *parent = nullptr);
    ~OutputDialog();

    void textAdd(const QString& str);
    void clear();

private:
    Ui::OutputDialog *ui;

signals:

};

#endif // OUTPUTDIALOG_H
