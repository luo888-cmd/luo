#ifndef DOCTORSCHEDULEVIEW_H
#define DOCTORSCHEDULEVIEW_H

#include <QWidget>
#include <QSqlTableModel>

namespace Ui {
class DoctorScheduleView;
}

class DoctorScheduleView : public QWidget
{
    Q_OBJECT

public:
    explicit DoctorScheduleView(QWidget *parent = nullptr);
    ~DoctorScheduleView();

    // ========== 修复1：槽函数必须放在 private slots 区域 ==========
private slots:
    void on_btAdd_clicked();      // 添加排班
    void on_btSearch_clicked();   // 搜索排班
    void on_btDelete_clicked();   // 删除排班
    void on_btEdit_clicked();     // 编辑排班
    void on_btReserve_clicked();  // 预约按钮槽函数
    void onGoPreviousView();      // ✅移到 slots 区域（原在 signals 里，是核心错误）

signals:
    void goDoctorScheduleEditView(int idx); // 跳转到排班编辑页
    void goReserveSuccess();                // 预约成功的信号(可选)
    // ❌ 删除：onGoPreviousView(); （从signals里移除，它是槽函数不是信号）

private:
    Ui::DoctorScheduleView *ui;
    bool isScheduleFull(int row);          // 满员判断方法
    QSqlTableModel *m_model;               // 排班列表模型
};

#endif // DOCTORSCHEDULEVIEW_H
