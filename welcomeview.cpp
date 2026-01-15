#include "welcomeview.h"
#include "ui_welcomeview.h"
#include "netmanager.h"  // 必须引入网络模块头文件
#include <QDateTime>     // 增量备份需要用到时间类

WelcomeView::WelcomeView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WelcomeView)
{
    qDebug()<<"create WelcomeView";
                ui->setupUi(this);
    // ❗ 把pushButton_DrugSync改成你UI里「药品数据同步」按钮的真实objectName
    connect(ui->btnDrugSync, &QPushButton::clicked, this, &WelcomeView::slot_drugSyncClicked);

    // 绑定「数据远程备份」按钮 → slot_dataBackupClicked槽函数
    // ❗ 把pushButton_DataBackup改成你UI里「数据远程备份」按钮的真实objectName
    connect(ui->btnDataBackup, &QPushButton::clicked, this, &WelcomeView::slot_dataBackupClicked);
}

WelcomeView::~WelcomeView()
{
    qDebug()<<"destroy WelcomeView";
    delete ui;
}

void WelcomeView::on_btDoctor_clicked()
{
    emit goDoctorView();
}


void WelcomeView::on_Department_clicked()
{
    emit goDepartmentView();
}


void WelcomeView::on_btPatient_clicked()
{
    emit goPatientView();
}

void WelcomeView::on_btVisit_clicked()
{
    emit goVisitView();
}

// ========== 新增：实现药品与库存按钮点击槽函数 ==========
void WelcomeView::on_btDrugStock_clicked()
{
    qDebug() << "[WelcomeView] 点击药品与库存按钮，发射信号";
    emit goDrugStockView(); // 现在能识别goDrugStockView信号了
}

// WelcomeView.cpp 里的预约排班按钮点击槽函数
void WelcomeView::on_btDoctorSchedule_clicked()
{
    emit goDoctorScheduleView();
}

// ========== 药品数据同步槽函数 ==========
void WelcomeView::slot_drugSyncClicked()
{
    QMessageBox::information(this, "提示", "开始同步最新药品/诊断信息...");
    // 替换成你的远程API地址（测试用）
    QString apiUrl = "http://192.168.1.100:8080/api/getMedicalData";
    // 调用HTTP同步功能
    NetManager::getInstance().syncDrugAndDiagnosisData(apiUrl);
}

// ========== 数据远程备份槽函数 ==========
void WelcomeView::slot_dataBackupClicked()
{
    // 1. 连接TCP备份服务器
    QString serverIp = "192.168.1.9";   // 替换成你的服务器IP
    quint16 serverPort = 8888;            // 替换成你的服务器端口
    bool isConnected = NetManager::getInstance().connectTcpBackupServer(serverIp, serverPort, 5000);

    if (!isConnected) {
        QMessageBox::critical(this, "错误", "连接备份服务器失败！");
        return;
    }

    // 2. 执行全量备份（含断点续传+MD5校验）
    bool isBackupOk = NetManager::getInstance().tcpFullBackupAllData();

    // 3. 断开服务器
    NetManager::getInstance().disconnectTcpBackupServer();

    // 4. 提示结果
    if (isBackupOk) {
        QMessageBox::information(this, "成功", "✅ 数据远程备份完成！");
    } else {
        QMessageBox::critical(this, "失败", "❌ 数据远程备份失败！");
    }
}
