#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QDir>
#include "otherexec.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    OtherExec *p;
    QSettings *settings;

    void openConfigFile(const QString path);
    void applySettings();



    void showPic(QString picture);
    void playSound(QString sound);

    //set the tag after the toolbox tag
    void setOutputTagNum(int num);

    int line = 0;

    //slot when readyread
    void readLine(QString str);
    void resultReview();

    //exec an Clamav app
    void start(const QString appname, const QStringList args = {});

    void fileScan();
    void memoryScan();
    void updateDatabase();

private slots:
    void on_bowse_f_targ_clicked();
    void on_f_encounterVirusComboBox_currentIndexChanged(int index);
    void on_edit_clamd_config_btn_clicked();
    void on_edit_frsh_config_btn_clicked();
    void on_clamAVLineEdit_editingFinished();
    void on_toolBox_currentChanged(int index);
    void on_cancel_settings_btn_clicked();
    void on_stop_button_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
