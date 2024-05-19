#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#include <winsvc.h>

void MainWindow::platformInit()
{
    ui->onacc_page->hide();

    connect(ui->s_startservice_btn, &QAbstractButton::clicked, this, &MainWindow::startService);

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
