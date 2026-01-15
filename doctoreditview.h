#ifndef DOCTOREDITVIEW_H
#define DOCTOREDITVIEW_H

#include <QWidget>
#include <QDataWidgetMapper>

// 匹配UI头文件的全大写类名
namespace Ui {
class DOCTOREDITVIEW; // 关键修改：从DoctorEditView改为DOCTOREDITVIEW
}

class DoctorEditView : public QWidget
{
    Q_OBJECT

public:
    explicit DoctorEditView(QWidget *parent = nullptr, int index = 0);
    ~DoctorEditView();

private slots:
    void on_pushButton_2_clicked(); // 保存按钮（对象名正确）
    void on_pushButton_clicked();  // 关键修改：取消按钮对象名是pushButton1

private:
    Ui::DOCTOREDITVIEW *ui; // 关键修改：匹配UI头文件的类名
    QDataWidgetMapper *dataMapper;

signals:
    void goPreviousView();
};

#endif // DOCTOREDITVIEW_H
