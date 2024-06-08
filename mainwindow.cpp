#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QMessageBox>
#include <QDebug>
#include <QPushButton>
#include <QScrollBar>
#include <QSound>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    p = new OtherExec(this);
    outputdialog = new OutputDialog(this);
    trayIcon = new QSystemTrayIcon(this);

    //settings
    settings = new QSettings(QDir::current().filePath("settings.ini"),QSettings::IniFormat,this);
    settings->beginGroup("Path");
    ui->clamAVLineEdit->setText(settings->value("ClamAVDir").toString());
    settings->endGroup();

    on_clamAVLineEdit_editingFinished();

    //sys tray icon
    trayIcon->setIcon(QIcon(":/resource/img/scan.png"));
    trayIcon->setToolTip("clamqt");

    QMenu *trayMenu = new QMenu(this);
    trayMenu->addAction(tr("Open Clamqt"), this, [=]{
        this->show();
    });
    trayMenu->addAction(tr("exit"), this, [=]{
        this->close();
    });
    trayIcon->setContextMenu(trayMenu);

    trayIcon->show();

    //connections
    connect(p, &OtherExec::readyRead, this, &MainWindow::readLine);
    connect(p, &OtherExec::readyReadError, this, [=](QString str){
        QMessageBox::warning(this,"Warning",str);
    });
    connect(p, &OtherExec::exit, this, &MainWindow::resultReview);

    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::OnSystemTrayClicked);

    connect(ui->apply_settings_btn, &QPushButton::clicked, this, &MainWindow::applySettings);
    connect(ui->f_scan_btn, &QPushButton::clicked, this, &MainWindow::fileScan);
    connect(ui->m_scan_btn, &QPushButton::clicked, this, &MainWindow::memoryScan);
    connect(ui->update_database_btn, &QPushButton::clicked, this, &MainWindow::updateDatabase);
    connect(ui->f_status_listWidget->verticalScrollBar(), &QScrollBar::valueChanged, ui->files_to_scan_listWidget->verticalScrollBar(), &QScrollBar::setValue);
    connect(ui->m_status_listWidget->verticalScrollBar(), &QScrollBar::valueChanged, ui->m_to_scan_listWidget->verticalScrollBar(), &QScrollBar::setValue);

    //ui
    ui->f_encounterVirusComboBox->setCurrentIndex(3);
    outputdialog->hide();
    ui->headerWidget->setWidgetToMove(this);
    //ui->tableWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    //showPic("check");
    playSound("scan");

    platformInit();

}

MainWindow::~MainWindow()
{
#ifdef Q_OS_WIN
    if (ui->s_autostart_checkB->isChecked())
        system("net stop clamd");
#endif
    delete ui;
}

void MainWindow::OnSystemTrayClicked(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::ActivationReason::Trigger)
    {
        this->show();
    }
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
    state = states::CheckingConf;
    ui->apply_settings_btn->setEnabled(false);
    ui->f_scan_btn->setEnabled(false);
    ui->m_scan_btn->setEnabled(false);
    ui->update_database_btn->setEnabled(false);

    settings->beginGroup("Path");
    settings->setValue("ClamAVDir",QDir(ui->clamAVLineEdit->text()).absolutePath());
    settings->endGroup();

    settings->beginGroup("ClamD");
    settings->setValue("AutoStartService",ui->s_autostart_checkB->isChecked());
    settings->endGroup();

    start("clamconf");

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


void MainWindow::readLine(QString str)
{
    switch (state) {
    case states::ScaningFile:
        readFileScan(str);
        break;
    case states::ScaningMemory:
        readMemoryScan(str);
        break;
    case states::Updating:
        readUpdate(str);
        break;
    case states::CheckingConf:
        readCheckingConf(str);
        break;
    default:
        outputdialog->textAdd(str);
        break;
    }






}

void MainWindow::resultReview()
{
    state = states::Available;
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

    outputdialog->clear();
}

void MainWindow::fileScan()
{
    state = states::ScaningFile;

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
    state = states::ScaningMemory;

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
    state = states::Updating;

    start("freshclam");
    ui->toolBox->setCurrentIndex(3);
}

void MainWindow::readFileScan(const QString &str)
{
    //文件扫描时的读取，这里的问题是结果可能是分段发的，就算有SCAN SUMMARY也可能不在这一段
    int index = str.indexOf("----------- SCAN SUMMARY -----------");
    if (index != -1) {
        playSound("scan");
        qDebug() << "scan";

        QString afterSummary = str.mid(index);
        QString beforeSummary = str.left(index);


        //ui->detail_browser->setText(ui->detail_browser->toPlainText()+beforeSummary);

        QMessageBox::information(this,tr("Scan conclude"),afterSummary);

        readFileScan(beforeSummary);

    }
    else
    {
        //人工智能写的，读取输出并更改两个listwidget
        foreach (const QString &each_result, str.split('\n'))
        {
            qDebug() << "spliting \\n, we got: " << each_result;
            QStringList file_and_status = each_result.split(": ", Qt::SkipEmptyParts);

            if (file_and_status.size() >= 2) // 确保我们有足够的部分
            {
                qDebug() << "spliting : , we got: " << file_and_status;

                // 查找文件项，注意findItems可能返回空列表
                QList<QListWidgetItem*> items = ui->files_to_scan_listWidget->findItems(file_and_status.at(0), Qt::MatchContains);
                if (!items.isEmpty()) // 确保找到了至少一个项
                {
                    QListWidgetItem *item = items.at(0); // 假设我们只关心第一个匹配项
                    int index = ui->files_to_scan_listWidget->row(item);

                    // 确保f_status_listWidget的行数和files_to_scan_listWidget一样多
                    // 或者确保我们不会超出f_status_listWidget的范围
                    if (index >= 0 && index < ui->f_status_listWidget->count())
                    {
                        QListWidgetItem *statusItem = ui->f_status_listWidget->item(index); // 使用item而不是itemAt
                        if (statusItem) // 确保statusItem不是nullptr
                        {
                            statusItem->setText(file_and_status.at(1));
                        }
                    }
                    else
                    {
                        qDebug() << "Index out of range for f_status_listWidget";
                    }
                }
                else
                {
                    qDebug() << "No matching item found in files_to_scan_listWidget";
                }
            }
            else
            {
                qDebug() << "Not enough parts in the split string";
            }
        }

    }
}

void MainWindow::readMemoryScan(const QString &str)
{
    int index = str.indexOf("----------- SCAN SUMMARY -----------");
    if (index != -1) {
        playSound("scan");
        qDebug() << "mscan";

        QString afterSummary = str.mid(index);
        QString beforeSummary = str.left(index);


        //ui->detail_browser->setText(ui->detail_browser->toPlainText()+beforeSummary);

        QMessageBox::information(this,tr("Memory Scan conclude"),afterSummary);

        readFileScan(beforeSummary);

    }
    else
    {
        //人工智能写的，读取输出并更改两个listwidget
        foreach (const QString &each_result, str.split('\n'))
        {
            qDebug() << "spliting \\n, we got: " << each_result;
            QStringList file_and_status = each_result.split(": ", Qt::SkipEmptyParts);

            if (file_and_status.size() >= 2) // 确保我们有足够的部分
            {
                qDebug() << "spliting : , we got: " << file_and_status;

                ui->m_to_scan_listWidget->addItem(file_and_status.at(0));
                ui->m_status_listWidget->addItem(file_and_status.at(1));
            }
            else
            {
                qDebug() << "Not enough parts in the split string";
            }
        }

    }
}

void MainWindow::readUpdate(const QString &str)
{
    outputdialog->textAdd(str);
}

void MainWindow::readCheckingConf(const QString &str)
{
    //checking
    if (str.count("Checking configuration files in ")>=1)
    {
        //showPic("check");
        qDebug() << "check";
        return;
    }
}


void MainWindow::on_bowse_f_targ_clicked()
{
    QStringList files = QFileDialog::getOpenFileNames(this,tr("Choose files to scan"));
    foreach (auto file, files) {
        ui->files_to_scan_listWidget->addItem(QDir::toNativeSeparators(file));
        ui->f_status_listWidget->addItem(tr("Unknown"));
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


void MainWindow::on_bowse_d_targ_clicked()
{
    QString dic = QFileDialog::getExistingDirectory(this,tr("Choose files to scan"));

    ui->files_to_scan_listWidget->addItem(QDir::toNativeSeparators(dic));
    ui->f_status_listWidget->addItem(tr("Unknown"));
}


void MainWindow::on_close_btn_clicked()
{
    this->hide();
}

