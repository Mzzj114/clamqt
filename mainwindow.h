#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QDir>
#include "otherexec.h"
#include "outputdialog.h"

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

    void platformInit();
    bool isServiceExists(const QString& serviceName);
    bool isServiceRunning(const QString &serviceName);

    enum states{
        ScaningFile,
        ScaningMemory,
        Updating,
        CheckingConf,
        Available,
    }state = states::Available;

    void openConfigFile(const QString path);
    void applySettings();
    void startService();


    void showPic(QString picture);
    void playSound(QString sound);

    //slot when readyread
    void readLine(QString str);
    void resultReview();

    //exec an Clamav app
    void start(const QString appname, const QStringList args = {});

    void fileScan();
    void memoryScan();
    void updateDatabase();
    void readFileScan(const QString& str);
    void readMemoryScan(const QString& str);
    void readUpdate(const QString& str);
    void readCheckingConf(const QString& str);


private slots:
    void on_bowse_f_targ_clicked();
    void on_f_encounterVirusComboBox_currentIndexChanged(int index);
    void on_edit_clamd_config_btn_clicked();
    void on_edit_frsh_config_btn_clicked();
    void on_clamAVLineEdit_editingFinished();
    //void on_toolBox_currentChanged(int index);
    void on_cancel_settings_btn_clicked();
    void on_stop_button_clicked();

private:
    Ui::MainWindow *ui;
    OutputDialog *outputdialog;
};
#endif // MAINWINDOW_H
