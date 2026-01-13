#ifndef DOCTORSCHEDULEEDITVIEW_H
#define DOCTORSCHEDULEEDITVIEW_H

#include <QWidget>
#include <QSqlTableModel>
#include <QDataWidgetMapper>
#include <QMessageBox>
#include <QDateTime>
#include <QUuid>
#include "idatabase.h"
#include <QSqlError>  //✅ 新增：必须加这个头文件，解决QSqlError报错

namespace Ui {
class DoctorScheduleEditView;
}

class DoctorScheduleEditView : public QWidget
{
    Q_OBJECT

public:
    // 重载构造函数：支持【新增排班】(idx=-1) 和 【编辑排班】(idx>=0)，和你项目其他编辑页一致
    explicit DoctorScheduleEditView(QWidget *parent = nullptr, int idx = -1);

    ~DoctorScheduleEditView();

signals:
    // 返回上一页的信号，和你项目所有编辑页一致，绑定MasterView的goPreviousView
    void goPreviousView();

private slots:
    // 保存按钮点击槽函数：新增/修改排班数据入库
    void on_pushButton_2_clicked();
    // 取消按钮点击槽函数：放弃操作，返回列表页
    void on_pushButton_clicked();

private:
    Ui::DoctorScheduleEditView *ui;
    int m_rowIndex;          // 当前编辑的行索引，-1表示新增
    QSqlTableModel *m_model; // 排班表模型
    QDataWidgetMapper *m_mapper; // 数据映射器：自动绑定组件和数据库字段，核心！

    // 初始化页面数据：新增时赋默认值，编辑时回显数据
    void initData();
    // 初始化组件状态：设置只读/可编辑，下拉框赋值
    void initWidgetStatus();
    // 新增：校验字段值是否有效（辅助函数）
    bool validateFields();
};

#endif // DOCTORSCHEDULEEDITVIEW_H
