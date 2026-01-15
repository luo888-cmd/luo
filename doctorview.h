#ifndef DOCTORVIEW_H
#define DOCTORVIEW_H

#include <QWidget>

namespace Ui {
class DoctorView;  // 类名从PatientView改为DoctorView
}

class DoctorView : public QWidget
{
    Q_OBJECT

public:
    explicit DoctorView(QWidget *parent = nullptr);
    ~DoctorView();

private slots:
    void on_btAdd_clicked();      // 按钮槽函数名不变（组件名一致）
    void on_btSearch_clicked();
    void on_btDelete_clicked();
    void on_btEdit_clicked();

signals:
    void goDoctorEditView(int idx);  // 信号名从患者改为医生

private:
    Ui::DoctorView *ui;  // UI类名同步修改
};

#endif // DOCTORVIEW_H
