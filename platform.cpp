#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>

//as for default, all the platform-specialized widgets are shown

#ifdef Q_OS_WIN
#include <windows.h>
#include <winsvc.h>
#include <QMessageBox>

void MainWindow::platformInit()
{

    connect(ui->s_startservice_btn, &QAbstractButton::clicked, this, &MainWindow::startService);
    connect(ui->link_btn_mscan, &QPushButton::clicked, this, [=]{ui->stackedWidget->setCurrentIndex(2);});
    connect(ui->m_scan_btn, &QPushButton::clicked, this, &MainWindow::memoryScan);

    settings->beginGroup("ClamD");
    ui->s_autostart_checkB->setChecked(settings->value("AutoStartService").toBool());
    settings->endGroup();

    if (ui->s_autostart_checkB->isChecked())
        system("net start clamd");

    if (!isServiceExists("clamd"))
    {
        ui->s_uninstallservice_btn->show();
        ui->s_startservice_btn->setText("Start service");
    }
    else if (isServiceRunning("clamd"))
    {
        ui->s_startservice_btn->setText("Stop service");
    }
    else
    {
        ui->s_startservice_btn->setText("Start service");
    }


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

void MainWindow::readMemoryScan(const QString &str)
{
    int index = str.indexOf("----------- SCAN SUMMARY -----------");
    if (index != -1) {
        //playSound("scan");
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


bool MainWindow::isServiceExists(const QString& serviceName) {
    SC_HANDLE hSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE);
    if (hSCManager == nullptr) {
        qDebug() << "OpenSCManager failed: " << GetLastError();
        return false;
    }

    ENUM_SERVICE_STATUSW ess;
    DWORD bytesNeeded = 0;
    DWORD servicesReturned = 0;
    DWORD resumeHandle = 0;

    // 第一次调用EnumServicesStatus来获取所需的缓冲区大小
    if (!EnumServicesStatusW(hSCManager, SERVICE_WIN32, SERVICE_STATE_ALL, &ess, 0, &bytesNeeded, &servicesReturned, &resumeHandle)) {
        if (GetLastError() != ERROR_MORE_DATA) {
            qDebug() << "EnumServicesStatusW failed (first call): " << GetLastError();
            CloseServiceHandle(hSCManager);
            return false;
        }
    }

    // 分配足够的内存来存储服务列表
    LPENUM_SERVICE_STATUSW pServices = (LPENUM_SERVICE_STATUSW)malloc(bytesNeeded);
    if (pServices == nullptr) {
        qDebug() << "Memory allocation failed";
        CloseServiceHandle(hSCManager);
        return false;
    }

    // 再次调用EnumServicesStatus来填充服务列表
    if (!EnumServicesStatusW(hSCManager, SERVICE_WIN32, SERVICE_STATE_ALL, pServices, bytesNeeded, &bytesNeeded, &servicesReturned, &resumeHandle)) {
        qDebug() << "EnumServicesStatusW failed (second call): " << GetLastError();
        free(pServices);
        CloseServiceHandle(hSCManager);
        return false;
    }

    // 检查每个服务的名称
    bool found = false;
    for (DWORD i = 0; i < servicesReturned; ++i) {
        if (wcscmp(pServices[i].lpServiceName, serviceName.toStdWString().c_str()) == 0) {
            found = true;
            break;
        }
    }

    // 清理并关闭句柄
    free(pServices);
    CloseServiceHandle(hSCManager);
    return found;
}

bool MainWindow::isServiceRunning(const QString& serviceName) {
    SC_HANDLE hSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE);
    if (hSCManager == nullptr) {
        qDebug() << "OpenSCManager failed:" << GetLastError();
        return false;
    }

    SC_HANDLE hService = OpenServiceW(hSCManager, reinterpret_cast<LPCWSTR>(serviceName.utf16()), SERVICE_QUERY_STATUS);
    if (hService == nullptr) {
        qDebug() << "OpenService failed:" << GetLastError();
        CloseServiceHandle(hSCManager);
        return false;
    }

    SERVICE_STATUS serviceStatus;
    if (!QueryServiceStatus(hService, &serviceStatus)) {
        qDebug() << "QueryServiceStatus failed:" << GetLastError();
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return false;
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);

    return serviceStatus.dwCurrentState == SERVICE_RUNNING;
}

void MainWindow::startService()
{
    if (!isServiceExists("clamd"))
    {
        start("clamd",{"--install-service"});
        ui->s_uninstallservice_btn->show();
        ui->s_startservice_btn->setText("Start service");
    }
    else if (isServiceRunning("clamd"))
    {
        system("net stop clamd");
        ui->s_startservice_btn->setText("Start service");
    }
    else
    {
        system("net start clamd");
        ui->s_startservice_btn->setText("Stop service");
    }
}
#endif  //Q_OS_WIN
