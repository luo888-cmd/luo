#include "visitview.h"
#include "ui_visitview.h"
#include "idatabase.h" // 数据库单例类

VisitView::VisitView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VisitView)
{
    ui->setupUi(this);

    // ========== 表格样式初始化（和PatientView/科室/医生页面完全一致） ==========
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows); // 选中整行
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection); // 单选
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);   // 禁止直接编辑
    ui->tableView->setAlternatingRowColors(true);                       // 隔行变色

    // ========== 加载就诊记录表格模型 ==========
    IDatabase &iDatabase = IDatabase::getInstance();
    // 初始化就诊记录模型（需确保IDatabase中实现了initVisitModel方法）
    if(iDatabase.initVisitModel()){
        // 设置表格模型和选择模型（和PatientView逻辑一致）
        ui->tableView->setModel(iDatabase.visitTabModel);
        ui->tableView->setSelectionModel(iDatabase.theVisitSelection);
    }
}

VisitView::~VisitView()
{
    delete ui;
}

// 新增就诊记录：调用数据库添加空行，发射跳转信号到编辑页
void VisitView::on_btAdd_clicked()
{
    // 调用IDatabase添加新就诊记录，返回新行索引（需确保IDatabase实现addNewVisit）
    int currow = IDatabase::getInstance().addNewVisit();
    // 发射信号跳转到就诊编辑页（和PatientView的add逻辑一致）
    emit goVisitEditView(currow);
}

// 搜索就诊记录：按名称/关键词模糊查询（参考PatientView的搜索逻辑）
void VisitView::on_btSearch_clicked()
{
    // 构建模糊查询条件（匹配全大写的PATIENT_NAME字段）
    QString filter = QString("PATIENT_NAME like '%%1%'").arg(ui->txtSearch->text());
    // 调用IDatabase的搜索方法（需确保IDatabase实现searchVisit）
    IDatabase::getInstance().searchVisit(filter);
}

// 删除就诊记录：校验选中行，调用数据库删除
void VisitView::on_btDelete_clicked()
{
    // 获取当前选中行的索引（需确保IDatabase定义theVisitSelection）
    QModelIndex curIndex = IDatabase::getInstance().theVisitSelection->currentIndex();
    if (!curIndex.isValid()) {
        // 未选中行时提示（和PatientView的删除提示一致）
        QMessageBox::warning(this, "提示", "请先选中要删除的就诊记录");
        return;
    }
    // 调用IDatabase删除当前记录（需确保IDatabase实现deleteCurrentVisit）
    IDatabase::getInstance().deleteCurrentVisit();
}

// 编辑就诊记录：获取选中行索引，发射跳转信号到编辑页
void VisitView::on_btEdit_clicked()
{
    // 获取当前选中行的索引
    QModelIndex curIndex = IDatabase::getInstance().theVisitSelection->currentIndex();
    if (!curIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选中要编辑的就诊记录");
        return;
    }
    // 发射信号跳转到编辑页，传递行索引（和PatientView的edit逻辑一致）
    emit goVisitEditView(curIndex.row());
}
