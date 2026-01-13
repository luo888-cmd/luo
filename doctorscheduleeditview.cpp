#include "doctorscheduleeditview.h"
#include "ui_doctorscheduleeditview.h"

DoctorScheduleEditView::DoctorScheduleEditView(QWidget *parent, int idx)
    : QWidget(parent)
    , ui(new Ui::DoctorScheduleEditView)
    , m_rowIndex(idx)
{
    ui->setupUi(this);
    this->setWindowTitle("排班添加/编辑");

    m_model = IDatabase::getInstance().doctorScheduleTabModel;
    m_mapper = new QDataWidgetMapper(this);
    m_mapper->setModel(m_model);
    m_mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);

    // ========== 组件与数据库字段绑定 完全不变 ==========
    m_mapper->addMapping(ui->dbEditSchID,        m_model->fieldIndex("ID"));
    m_mapper->addMapping(ui->dbEditDocID,       m_model->fieldIndex("DOCTOR_ID"));
    m_mapper->addMapping(ui->dbEditDocName,     m_model->fieldIndex("DOCTOR_NAME"));
    m_mapper->addMapping(ui->dateEditSchDate,   m_model->fieldIndex("SCHEDULE_DATE"));
    m_mapper->addMapping(ui->dbEditShiftType,   m_model->fieldIndex("SHIFT_TYPE"));
    m_mapper->addMapping(ui->spinBoxMaxNum,     m_model->fieldIndex("MAX_NUM"));
    m_mapper->addMapping(ui->spinBoxNowNum,     m_model->fieldIndex("NOW_NUM"));
    m_mapper->addMapping(ui->comboBoxStatus,    m_model->fieldIndex("STATUS"));

    initWidgetStatus();
    initData();
    // 在构造函数中添加
    qDebug() << "模型字段列表：";
    for(int i=0; i<m_model->columnCount(); i++) {
        qDebug() << m_model->headerData(i, Qt::Horizontal).toString();
    }
}

DoctorScheduleEditView::~DoctorScheduleEditView()
{
    delete ui;
}

// ========== 初始化组件状态 无修改 ==========
void DoctorScheduleEditView::initWidgetStatus()
{
    ui->dbEditSchID->setEnabled(false);
    ui->spinBoxNowNum->setEnabled(false);
    ui->comboBoxStatus->setEnabled(false);
    ui->comboBoxStatus->addItem("可预约", 0);
    ui->comboBoxStatus->addItem("已满员", 1);
    ui->spinBoxMaxNum->setMinimum(1);
    ui->dateEditSchDate->setDate(QDate::currentDate());
}

// ========== 初始化页面数据 ✅修复所有setValue/value报错 ==========
void DoctorScheduleEditView::initData()
{
    if(m_rowIndex == -1)
    {
        // 1. 强制提交模型原有脏数据，避免干扰新行
        m_model->submitAll();
        int newRow = m_model->rowCount();

        // 2. 插入新行并立即设置默认值（绕过mapper，直接给模型赋值）
        bool insertOk = m_model->insertRow(newRow);
        if(!insertOk) {
            qDebug() << "插入新行失败：" << m_model->lastError().text();
            QMessageBox::warning(this, "提示", "新增行初始化失败！");
            return;
        }

        // 3. 主动给模型字段赋值（关键：让模型识别到有可更新字段）
        m_model->setData(m_model->index(newRow, m_model->fieldIndex("DOCTOR_ID")), ""); // 先赋空，后续由用户填写
        m_model->setData(m_model->index(newRow, m_model->fieldIndex("DOCTOR_NAME")), "");
        m_model->setData(m_model->index(newRow, m_model->fieldIndex("SCHEDULE_DATE")), QDate::currentDate().toString("yyyy-MM-dd"));
        m_model->setData(m_model->index(newRow, m_model->fieldIndex("SHIFT_TYPE")), "");
        m_model->setData(m_model->index(newRow, m_model->fieldIndex("MAX_NUM")), 5);
        m_model->setData(m_model->index(newRow, m_model->fieldIndex("NOW_NUM")), 0);
        m_model->setData(m_model->index(newRow, m_model->fieldIndex("STATUS")), 0);

        // 4. 绑定mapper到新行，同步默认值到控件
        m_mapper->setCurrentIndex(newRow);
        ui->spinBoxNowNum->setValue(0);
        ui->spinBoxMaxNum->setValue(5);
        ui->comboBoxStatus->setCurrentIndex(0);
    }
    else
    {
        // 编辑逻辑不变
        m_mapper->setCurrentIndex(m_rowIndex);
        if(ui->spinBoxNowNum->value() >= ui->spinBoxMaxNum->value())
        {
            ui->comboBoxStatus->setCurrentIndex(1);
        }
        else
        {
            ui->comboBoxStatus->setCurrentIndex(0);
        }
    }
}

// 新增：字段校验辅助函数
bool DoctorScheduleEditView::validateFields()
{
    // 非空校验
    if(ui->dbEditDocID->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, "提示", "医生ID不能为空！");
        return false;
    }
    if(ui->dbEditDocName->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, "提示", "医生姓名不能为空！");
        return false;
    }
    if(ui->dbEditShiftType->currentText().trimmed().isEmpty())
    {
        QMessageBox::warning(this, "提示", "排班班次不能为空（如：上午/下午）！");
        return false;
    }
    // 日期校验
    if(ui->dateEditSchDate->date() < QDate::currentDate())
    {
        QMessageBox::warning(this, "提示", "排班日期不能早于当前日期！");
        return false;
    }
    return true;
}

// ========== 保存按钮 ✅零报错✅逻辑正确✅完整校验 ==========
void DoctorScheduleEditView::on_pushButton_2_clicked()
{
    if(!validateFields()) {
        return;
    }

    // 1. 自动校准状态（不变）
    int currentRow = m_mapper->currentIndex();
    int statusCol = m_model->fieldIndex("STATUS");
    if(ui->spinBoxNowNum->value() >= ui->spinBoxMaxNum->value())
    {
        m_model->setData(m_model->index(currentRow, statusCol), 1);
    }
    else
    {
        m_model->setData(m_model->index(currentRow, statusCol), 0);
    }

    // 2. 关键：先提交mapper，再手动同步控件值到模型（防止mapper同步遗漏）
    m_mapper->submit();
    // 兜底：强制将控件值写入模型（解决mapper同步失效）
    m_model->setData(m_model->index(currentRow, m_model->fieldIndex("DOCTOR_ID")), ui->dbEditDocID->text().trimmed());
    m_model->setData(m_model->index(currentRow, m_model->fieldIndex("DOCTOR_NAME")), ui->dbEditDocName->text().trimmed());
    m_model->setData(m_model->index(currentRow, m_model->fieldIndex("SCHEDULE_DATE")), ui->dateEditSchDate->date().toString("yyyy-MM-dd"));
    m_model->setData(m_model->index(currentRow, m_model->fieldIndex("SHIFT_TYPE")), ui->dbEditShiftType->currentText().trimmed());
    m_model->setData(m_model->index(currentRow, m_model->fieldIndex("MAX_NUM")), ui->spinBoxMaxNum->value());
    m_model->setData(m_model->index(currentRow, m_model->fieldIndex("NOW_NUM")), ui->spinBoxNowNum->value());

    // 3. 事务+强制提交（确保写入数据库）
    QSqlDatabase db = m_model->database();
    db.transaction();
    bool submitOk = m_model->submitAll();
    if(submitOk) {
        // 关键：提交后立即刷新模型，获取自增主键（SQLite必须）
        m_model->select();
        db.commit();
        QMessageBox::information(this, "成功", "排班信息保存成功！");
        emit goPreviousView();
    } else {
        db.rollback();
        qDebug() << "保存失败详情：" << m_model->lastError().text();
        QMessageBox::critical(this, "失败", "排班信息保存失败：" + m_model->lastError().text());
        m_mapper->revert();
        m_model->revertAll();
    }
}

void DoctorScheduleEditView::on_pushButton_clicked()
{
    m_mapper->revert();
    m_model->revertAll();
    emit goPreviousView();
}
