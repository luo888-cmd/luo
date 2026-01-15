#include "doctoreditview.h"
#include "ui_doctoreditview.h" // UI头文件正确（生成的就是这个）
#include <QSqlTableModel>
#include "idatabase.h"

DoctorEditView::DoctorEditView(QWidget *parent, int index)
    : QWidget(parent)
    , ui(new Ui::DOCTOREDITVIEW) // 关键修改：实例化正确的UI类
{
    ui->setupUi(this); // 现在能正确调用setupUi，不会报不完整类型

    // 初始化数据映射器
    dataMapper = new QDataWidgetMapper();
    QSqlTableModel *doctorTabModel = IDatabase::getInstance().doctorTabModel;
    dataMapper->setModel(doctorTabModel);
    dataMapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);

    // 严格匹配UI头文件中的组件名（和截图一致，无需修改）
    dataMapper->addMapping(ui->dbEditID,        doctorTabModel->fieldIndex("ID"));
    dataMapper->addMapping(ui->dbEditName,      doctorTabModel->fieldIndex("NAME"));
    dataMapper->addMapping(ui->dbComboSex,      doctorTabModel->fieldIndex("SEX"));
    dataMapper->addMapping(ui->lineEdit_3,      doctorTabModel->fieldIndex("TITLE"));
    dataMapper->addMapping(ui->lineEdit_4,      doctorTabModel->fieldIndex("DEPARTMENT_ID"));
    dataMapper->addMapping(ui->dbEditMobile,    doctorTabModel->fieldIndex("MOBILE"));
    dataMapper->addMapping(ui->dbCreatedTimeStamp, doctorTabModel->fieldIndex("CREATEDTIME"));

    dataMapper->setCurrentIndex(index);
}

DoctorEditView::~DoctorEditView()
{
    delete ui;
}

// 保存按钮：提交医生表修改
void DoctorEditView::on_pushButton_2_clicked()
{
    IDatabase::getInstance().submitDoctorEdit();
    emit goPreviousView();
}

// 取消按钮：回滚医生表修改（槽函数名匹配pushButton1）
void DoctorEditView::on_pushButton_clicked()
{
    IDatabase::getInstance().revertDoctorEdit();
    emit goPreviousView();
}
