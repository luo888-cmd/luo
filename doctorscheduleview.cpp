#include "doctorscheduleview.h"
#include "ui_doctorscheduleview.h"
#include "idatabase.h"
#include <QMessageBox>
#include <QSqlError>
#include <QDateTime>
#include <QUuid>
#include <QSqlTableModel>

DoctorScheduleView::DoctorScheduleView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DoctorScheduleView)
{
    ui->setupUi(this);

    // 表格样式（不变）
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setAlternatingRowColors(true);

    // 初始化数据库模型
    IDatabase &iDatabase = IDatabase::getInstance();
    iDatabase.initDoctorScheduleTabModel();

    // ========== 修复2：初始化 m_model（绑定数据库单例模型） ==========
    m_model = iDatabase.doctorScheduleTabModel; // 关键：m_model 指向数据库模型
    if (m_model) { // 统一用 m_model，不再混用 iDatabase.doctorScheduleTabModel
        ui->tableView->setModel(m_model);
        QItemSelectionModel *scheduleSelection = new QItemSelectionModel(m_model);
        ui->tableView->setSelectionModel(scheduleSelection);
    }
}

DoctorScheduleView::~DoctorScheduleView()
{
    delete ui;
}

// 满员判断（不变，仅把 schModel 替换为 m_model 更统一，可选）
bool DoctorScheduleView::isScheduleFull(int row)
{
    // IDatabase &iDatabase = IDatabase::getInstance(); // 可删除，直接用 m_model
    // QSqlTableModel *model = iDatabase.doctorScheduleTabModel; // 替换为 m_model
    QSqlTableModel *model = m_model; // 统一模型，避免混淆
    int nowNum = model->index(row, model->fieldIndex("NOW_NUM")).data().toInt();
    int maxNum = model->index(row, model->fieldIndex("MAX_NUM")).data().toInt();
    int status = model->index(row, model->fieldIndex("STATUS")).data().toInt();
    if(status == 1 || nowNum >= maxNum)
    {
        return true;
    }
    return false;
}

// 添加排班（不变）
void DoctorScheduleView::on_btAdd_clicked()
{
    qDebug() << "排班添加按钮被点击";
    int currow = IDatabase::getInstance().addNewDoctorSchedule();
    emit goDoctorScheduleEditView(currow);
}

// 搜索排班（修复：统一用 m_model）
void DoctorScheduleView::on_btSearch_clicked()
{
    QString keyword = ui->txtSearch->text().trimmed();
    QString filter = QString("DOCTOR_NAME like '%%1%' OR SCHEDULE_DATE like '%%1%' OR SHIFT_TYPE like '%%1%'").arg(keyword);
    // IDatabase &iDatabase = IDatabase::getInstance(); // 可删除
    // iDatabase.doctorScheduleTabModel->setFilter(filter); // 替换为 m_model
    // iDatabase.doctorScheduleTabModel->select();
    m_model->setFilter(filter); // 统一用 m_model
    m_model->select();
}

// 删除排班（修复：统一用 m_model）
void DoctorScheduleView::on_btDelete_clicked()
{
    QModelIndex curIndex = ui->tableView->selectionModel()->currentIndex();
    if (!curIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选中要删除的排班");
        return;
    }
    // IDatabase &iDatabase = IDatabase::getInstance(); // 可删除
    if (m_model->removeRow(curIndex.row())) { // 替换为 m_model
        if (IDatabase::getInstance().submitScheduleEdit()) {
            m_model->select(); // 统一用 m_model
            QMessageBox::information(this, "提示", "排班信息删除成功");
        } else {
            QMessageBox::critical(this, "错误", "删除失败：" + m_model->lastError().text());
            IDatabase::getInstance().revertScheduleEdit();
        }
    }
}

// 编辑排班（不变）
void DoctorScheduleView::on_btEdit_clicked()
{
    QModelIndex curIndex = ui->tableView->selectionModel()->currentIndex();
    if (!curIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选中要编辑的排班");
        return;
    }
    emit goDoctorScheduleEditView(curIndex.row());
}

// 预约按钮（修复：统一用 m_model，可选）
void DoctorScheduleView::on_btReserve_clicked()
{
    QModelIndex curIndex = ui->tableView->selectionModel()->currentIndex();
    if (!curIndex.isValid()) {
        QMessageBox::warning(this, "预约提示", "请先选中要预约的医生排班！");
        return;
    }
    int selectRow = curIndex.row();

    if(isScheduleFull(selectRow))
    {
        QMessageBox::information(this, "预约提示", "该医生此班次的预约名额已满，无法预约！");
        return;
    }

    // IDatabase &iDatabase = IDatabase::getInstance(); // 可删除
    // QSqlTableModel *schModel = iDatabase.doctorScheduleTabModel; // 替换为 m_model
    QSqlTableModel *schModel = m_model; // 统一模型
    QString schID = schModel->index(selectRow, schModel->fieldIndex("ID")).data().toString();
    int docID = schModel->index(selectRow, schModel->fieldIndex("DOCTOR_ID")).data().toInt();
    QString docName = schModel->index(selectRow, schModel->fieldIndex("DOCTOR_NAME")).data().toString();
    QString schDate = schModel->index(selectRow, schModel->fieldIndex("SCHEDULE_DATE")).data().toString();
    QString shiftType = schModel->index(selectRow, schModel->fieldIndex("SHIFT_TYPE")).data().toString();
    int nowNum = schModel->index(selectRow, schModel->fieldIndex("NOW_NUM")).data().toInt();

    int patID = 1;
    QString patName = "张三";

    QSqlTableModel *resModel = IDatabase::getInstance().reservationTabModel;
    int newRow = resModel->rowCount();
    resModel->insertRow(newRow);
    resModel->setData(resModel->index(newRow, resModel->fieldIndex("ID")), QUuid::createUuid().toString(QUuid::WithoutBraces));
    resModel->setData(resModel->index(newRow, resModel->fieldIndex("PATIENT_ID")), patID);
    resModel->setData(resModel->index(newRow, resModel->fieldIndex("PATIENT_NAME")), patName);
    resModel->setData(resModel->index(newRow, resModel->fieldIndex("DOCTOR_ID")), docID);
    resModel->setData(resModel->index(newRow, resModel->fieldIndex("DOCTOR_NAME")), docName);
    resModel->setData(resModel->index(newRow, resModel->fieldIndex("SCHEDULE_ID")), schID);
    resModel->setData(resModel->index(newRow, resModel->fieldIndex("SCHEDULE_DATE")), schDate);
    resModel->setData(resModel->index(newRow, resModel->fieldIndex("SHIFT_TYPE")), shiftType);
    resModel->setData(resModel->index(newRow, resModel->fieldIndex("RESERVE_TIME")), QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    resModel->setData(resModel->index(newRow, resModel->fieldIndex("STATUS")), 0);

    schModel->setData(schModel->index(selectRow, schModel->fieldIndex("NOW_NUM")), nowNum + 1);
    if(nowNum +1 >= schModel->index(selectRow, schModel->fieldIndex("MAX_NUM")).data().toInt())
    {
        schModel->setData(schModel->index(selectRow, schModel->fieldIndex("STATUS")), 1);
    }

    if(resModel->submitAll() && schModel->submitAll())
    {
        QMessageBox::information(this, "预约成功", "恭喜！已成功预约【"+docName+"】医生 "+schDate+" "+shiftType+" 班次！");
        schModel->select();
        resModel->select();
    }
    else
    {
        QMessageBox::critical(this, "预约失败", "预约出错："+resModel->lastError().text());
        resModel->revertAll();
        schModel->revertAll();
    }
}

// 刷新列表槽函数（现在 m_model 已初始化，无编译报错）
void DoctorScheduleView::onGoPreviousView()
{
    m_model->select(); // 现在 m_model 指向数据库模型，可正常调用
    ui->tableView->reset();
    ui->tableView->setModel(m_model);
}
