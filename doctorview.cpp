#include "doctorview.h"
#include "ui_doctorview.h"  // 注意：若UI文件仍叫patientview.ui，需改为ui_patientview.h
#include "idatabase.h"
#include <QMessageBox>
#include <QSqlError>  // 新增：包含QSqlError的完整定义

DoctorView::DoctorView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DoctorView)
{
    ui->setupUi(this);

    // 表格样式（组件名不变，无需修改）
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setAlternatingRowColors(true);

    // ========== 核心修改：初始化医生表模型 ==========
    IDatabase &iDatabase = IDatabase::getInstance();
    iDatabase.initDoctorTabModel();  // 替换为医生模型初始化接口
    if (iDatabase.doctorTabModel) {  // 替换为医生表模型
        ui->tableView->setModel(iDatabase.doctorTabModel);
        // 【注意】需在IDatabase中补充医生表的选择模型（theDoctorSelection）
        // 临时方案：手动创建选择模型（建议后续在IDatabase中补充）
        QItemSelectionModel *theDoctorSelection = new QItemSelectionModel(iDatabase.doctorTabModel);
        ui->tableView->setSelectionModel(theDoctorSelection);
    }
}

DoctorView::~DoctorView()
{
    delete ui;
}

void DoctorView::on_btAdd_clicked()
{
    qDebug() << "医生添加按钮被点击"; // 调试输出，确认槽函数执行
    // 替换-1为实际的新增行索引
    int currow = IDatabase::getInstance().addNewDoctor();
    emit goDoctorEditView(currow); // 发射信号，传递新行索引
}
// ========== 【核心修改-重点】无QRegExp版本 姓名+科室ID双条件查询 ==========
void DoctorView::on_btSearch_clicked()
{
    IDatabase &iDatabase = IDatabase::getInstance();
    if(nullptr == iDatabase.doctorTabModel)
    {
        QMessageBox::warning(this, "提示", "数据表模型初始化失败，无法查询");
        return;
    }

    // 获取搜索框内容并去除首尾空格，避免空格影响查询
    QString searchText = ui->txtSearch->text().trimmed();
    QString filterSql; // 最终拼接的查询过滤条件

    // ✅ 【Qt全版本兼容】判断是否为纯数字(科室ID)，替代QRegExp的最优方案
    bool isNumber = false;
    searchText.toInt(&isNumber); // 核心：toInt转换成功 → isNumber=true(纯数字)

    if(searchText.isEmpty())
    {
        // 情况1：输入框为空 → 查询全部医生数据
        filterSql = "";
    }
    else if(isNumber)
    {
        // 情况2：输入的是【纯数字】→ 按【科室ID】精准查询
        filterSql = QString("DEPARTMENT_ID = %1").arg(searchText);
    }
    else
    {
        // 情况3：输入的是【文字/数字+文字】→ 按【医生姓名】模糊查询
        filterSql = QString("NAME like '%%1%'").arg(searchText);
    }

    // 执行过滤查询并刷新表格
    iDatabase.doctorTabModel->setFilter(filterSql);
    bool isOk = iDatabase.doctorTabModel->select();
    if(!isOk)
    {
        QMessageBox::critical(this, "查询失败", "查询异常："+iDatabase.doctorTabModel->lastError().text());
    }
}

void DoctorView::on_btDelete_clicked()
{
    // ========== 核心修改：删除医生（需在IDatabase中补充deleteCurrentDoctor函数） ==========
    IDatabase &iDatabase = IDatabase::getInstance();
    // 临时：先获取表格当前选中索引（建议后续在IDatabase中补充theDoctorSelection）
    QModelIndex curIndex = ui->tableView->selectionModel()->currentIndex();
    if (!curIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选中要删除的医生");
        return;
    }
    // 临时删除逻辑（建议后续移到IDatabase的deleteCurrentDoctor函数中）
    if (iDatabase.doctorTabModel->removeRow(curIndex.row())) {
        if (iDatabase.submitDoctorEdit()) {
            iDatabase.doctorTabModel->select();
            QMessageBox::information(this, "提示", "医生信息删除成功");
        } else {
            QMessageBox::critical(this, "错误", "删除失败：" + iDatabase.doctorTabModel->lastError().text());
            iDatabase.revertDoctorEdit();
        }
    }
}

void DoctorView::on_btEdit_clicked()
{
    // ========== 核心修改：编辑医生 ==========
    // 临时：从表格获取选中索引（建议后续在IDatabase中补充theDoctorSelection）
    QModelIndex curIndex = ui->tableView->selectionModel()->currentIndex();
    if (!curIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选中要编辑的医生");
        return;
    }
    emit goDoctorEditView(curIndex.row());  // 信号名改为医生编辑页面
}
