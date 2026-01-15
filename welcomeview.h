#ifndef WELCOMEVIEW_H
#define WELCOMEVIEW_H

#include <QWidget>
#include "netmanager.h"
#include <QMessageBox>

namespace Ui {
class WelcomeView;
}

class WelcomeView : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomeView(QWidget *parent = nullptr);
    ~WelcomeView();

private slots:
    void on_btDoctor_clicked();

    void on_Department_clicked();

    void on_btPatient_clicked();

    // 新增：“就诊记录与处方”按钮的槽函数（根据UI中按钮的对象名调整，比如btVisit）
    void on_btVisit_clicked();

    void on_btDrugStock_clicked();

    void on_btDoctorSchedule_clicked();

    // ========== 新增：普通槽函数（去掉on_前缀，避免自动绑定冲突） ==========
    void slot_drugSyncClicked();       // 药品数据同步
    void slot_dataBackupClicked();     // 数据远程备份



signals:

    // 新增：跳转到就诊记录页面的信号
    void goVisitView();
    void goDepartmentView();
    void goDoctorView();
    void goPatientView();
    void goDrugStockView(); // 必须声明在signals下
    void goDoctorScheduleView();

private:
    Ui::WelcomeView *ui;
};

#endif // WELCOMEVIEW_H
