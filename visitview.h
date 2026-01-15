#ifndef VISITVIEW_H
#define VISITVIEW_H

#include <QWidget>
#include <QMessageBox> // 用于提示框

namespace Ui {
class VisitView;
}

class VisitView : public QWidget
{
    Q_OBJECT

public:
    explicit VisitView(QWidget *parent = nullptr);
    ~VisitView();

private slots:
    // 对应UI中的按钮（组件名和PatientView一致）
    void on_btAdd_clicked();      // 新增就诊记录
    void on_btSearch_clicked();   // 搜索就诊记录
    void on_btDelete_clicked();   // 删除就诊记录
    void on_btEdit_clicked();     // 编辑就诊记录

signals:
    // 跳转到就诊记录编辑页的信号（和PatientView的goPatientEditView逻辑一致）
    void goVisitEditView(int idx);

private:
    Ui::VisitView *ui;
};

#endif // VISITVIEW_H
