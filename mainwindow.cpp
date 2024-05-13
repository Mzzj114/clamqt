#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QDebug>
#include <QPushButton>
#include <QSound>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    p = new OtherExec(this);

    //settings
    settings = new QSettings(QDir::current().filePath("settings.ini"),QSettings::IniFormat,this);
    settings->beginGroup("Path");
    ui->clamAVLineEdit->setText(settings->value("ClamAVDir").toString());
    settings->endGroup();
    on_clamAVLineEdit_editingFinished();

    //connections
    connect(p, &OtherExec::readyRead, this, &MainWindow::readLine);
    connect(p, &OtherExec::readyReadError, this, &MainWindow::readLine);
    connect(p, &OtherExec::exit, this, &MainWindow::resultReview);
    connect(ui->apply_settings_btn, &QPushButton::clicked, this, &MainWindow::applySettings);
    connect(ui->f_scan_btn, &QPushButton::clicked, this, &MainWindow::fileScan);
    connect(ui->m_scan_btn, &QPushButton::clicked, this, &MainWindow::memoryScan);
    connect(ui->update_database_btn, &QPushButton::clicked, this, &MainWindow::updateDatabase);


    ui->sum_browser->hide();
    ui->f_encounterVirusComboBox->setCurrentIndex(3);
    //showPic("check");
    playSound("scan");

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openConfigFile(const QString path)
{
    QFileInfo fileInfo(path);

    // 检查文件是否存在
    if (!fileInfo.exists()) {
        QMessageBox::warning(this, tr("File dosen't exist"), tr("The config file dose not exist in ") + path);
        return;
    }

    // 检查文件后缀是否为.conf
    if (fileInfo.suffix() != "conf") {
        QMessageBox::warning(this, tr("Wrong suffix"), tr("The suffix of the file is ") + fileInfo.suffix() + tr(", instead of .conf"));
        return;
    }

    // 使用系统默认的文本编辑器打开文件
    QUrl url = QUrl::fromLocalFile(path);
    if (!QDesktopServices::openUrl(url)) {
        QMessageBox::warning(this, tr("Can't open file"), tr("Error occurred when open file ") + path);
    }
}

void MainWindow::applySettings()
{
    ui->apply_settings_btn->setEnabled(false);
    ui->f_scan_btn->setEnabled(false);
    ui->m_scan_btn->setEnabled(false);
    ui->update_database_btn->setEnabled(false);

    settings->beginGroup("Path");
    settings->setValue("ClamAVDir",QDir(ui->clamAVLineEdit->text()).absolutePath());

    settings->endGroup();

    start("clamconf.exe");

}

/*
void MainWindow::showPic(QString picture)
{
    // 图片目录的路径
    QDir imgdir = QDir::current();
    imgdir.cd("img");

    QString fileName = picture+".png";


    // 构造图片文件的完整路径
    QString fullPath = imgdir.filePath(fileName);

    // 加载图片
    QPixmap pixmap(fullPath);
    if (!pixmap.isNull()) {
        pixmap.scaled(ui->result_label->size(), Qt::KeepAspectRatio);
        ui->result_label->setScaledContents(true);
        ui->result_label->setPixmap(pixmap);
    }
    else {}
}
*/

void MainWindow::playSound(QString sound)
{
    QDir sounddir = QDir::current();
    sounddir.cd("sound");

    QString fileName = sound+".wav";


    // 构造图片文件的完整路径
    QString fullPath = sounddir.filePath(fileName);
    QSound::play(fullPath);
}

void MainWindow::setOutputTagNum(int num)
{
    if (num==0)
        ui->toolBox->setItemText(3,tr("Output"));
    else
        ui->toolBox->setItemText(3,tr("Output(")+QString::fromStdString(std::to_string(num))+')');
}

void MainWindow::readLine(QString str)
{
    setOutputTagNum(++line);
    qDebug() << str;
    //checking
    if (str.count("Checking configuration files in ")>=1)
    {
        //showPic("check");
        qDebug() << "check";
        return;
    }

    //scaning
    int index = str.indexOf("----------- SCAN SUMMARY -----------");
    if (index != -1) {
        playSound("scan");
        qDebug() << "scan";

        QString afterSummary = str.mid(index);
        QString beforeSummary = str.left(index);


        qDebug() << afterSummary;
        ui->sum_browser->setText(ui->sum_browser->toPlainText()+afterSummary);
        ui->sum_browser->show();
        ui->detail_browser->setText(ui->detail_browser->toPlainText()+beforeSummary);
        return;
    }
    else
    {}


    //default
    ui->detail_browser->setText(ui->detail_browser->toPlainText()+'\n'+str);
}

void MainWindow::resultReview()
{
    ui->statusbar->showMessage("finish",5000);


    ui->apply_settings_btn->setEnabled(true);
    ui->f_scan_btn->setEnabled(true);
    ui->m_scan_btn->setEnabled(true);
    ui->update_database_btn->setEnabled(true);
}

void MainWindow::start(const QString appname, const QStringList args)
{
    settings->beginGroup("Path");
    p->start(QDir(settings->value("ClamAVDir").toString()).filePath(appname),args);
    settings->endGroup();

    ui->detail_browser->clear();
    ui->sum_browser->clear();
    ui->sum_browser->hide();
}

void MainWindow::fileScan()
{
    ui->apply_settings_btn->setEnabled(false);
    ui->f_scan_btn->setEnabled(false);
    ui->m_scan_btn->setEnabled(false);
    ui->update_database_btn->setEnabled(false);

    QString command = "clamscan";
    QStringList args = {};
    if (ui->f_R_checkB->isChecked()) args.append("--recursive");
    if (ui->f_daemon_checkB->isChecked()) command = "clamdscan";
    if (ui->f_encounterVirusComboBox->currentIndex()==1) args.append("--remove");
    else if (ui->f_encounterVirusComboBox->currentIndex()==0) args.append("--move="+ui->moveCopyToLineEdit->text());
    else if (ui->f_encounterVirusComboBox->currentIndex()==2) args.append("--copy="+ui->moveCopyToLineEdit->text());

    for (int i=0;i<ui->files_to_scan_listWidget->count();i++)
        args.append(ui->files_to_scan_listWidget->item(i)->text());

    start(command,args);
}

void MainWindow::memoryScan()
{
    ui->apply_settings_btn->setEnabled(false);
    ui->f_scan_btn->setEnabled(false);
    ui->m_scan_btn->setEnabled(false);
    ui->update_database_btn->setEnabled(false);

    QString command = "clamscan";
    QStringList args = {"--memory"};
    if (ui->m_daemon_checkB->isChecked()) command = "clamdscan";
    if (ui->m_encounterVirusComboBox->currentIndex()==0) ;
    else if (ui->m_encounterVirusComboBox->currentIndex()==1) args.append("--kill");
    else if (ui->m_encounterVirusComboBox->currentIndex()==1) args.append("--unload");

    start(command,args);
}

void MainWindow::updateDatabase()
{
    start("freshclam");
    ui->toolBox->setCurrentIndex(3);
}


void MainWindow::on_bowse_f_targ_clicked()
{
    QStringList files = QFileDialog::getOpenFileNames(this,tr("Choose files to scan"));
    foreach (auto file, files) {
        ui->files_to_scan_listWidget->addItem(QDir::toNativeSeparators(file));
    }

}


void MainWindow::on_f_encounterVirusComboBox_currentIndexChanged(int index)
{
    /*
    index:
    0 Move  1 Remove   2 Copy   3 None

    */
    if (index == 1 || index == 3) //choose "Remove"
    {
        ui->bowse_movecopy_btn->setEnabled(false);
        ui->moveCopyToLineEdit->setEnabled(false);
    }
    else
    {
        ui->bowse_movecopy_btn->setEnabled(true);
        ui->moveCopyToLineEdit->setEnabled(true);
    }
}


void MainWindow::on_edit_clamd_config_btn_clicked()
{
    openConfigFile(ui->clamd_confpath_lineEdit->text());
}


void MainWindow::on_edit_frsh_config_btn_clicked()
{
    openConfigFile(ui->frsh_confpath_lineEdit->text());
}


void MainWindow::on_clamAVLineEdit_editingFinished()
{
    QString path1 = QDir(ui->clamAVLineEdit->text()).filePath("clamd.conf");
    QString path2 = QDir(ui->clamAVLineEdit->text()).filePath("freshclam.conf");

    ui->clamd_confpath_lineEdit->setText(path1);
    ui->frsh_confpath_lineEdit->setText(path2);
}


void MainWindow::on_toolBox_currentChanged(int index)
{
    if (index == 3)
    {
        line = 0;
        setOutputTagNum(line);
    }
}


void MainWindow::on_cancel_settings_btn_clicked()
{
    ui->toolBox->setCurrentIndex(0);
}


void MainWindow::on_stop_button_clicked()
{
    p->stop();

    ui->apply_settings_btn->setEnabled(true);
    ui->f_scan_btn->setEnabled(true);
    ui->m_scan_btn->setEnabled(true);
    ui->update_database_btn->setEnabled(true);
}

