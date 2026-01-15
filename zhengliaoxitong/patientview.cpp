#include "patientview.h"
#include "ui_patientview.h"
#include "idatabase.h"
#include <QMessageBox>  // 添加这行代码

PatientView::PatientView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PatientView)
{
    ui->setupUi(this);

    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setAlternatingRowColors(true);

    IDatabase &iDatabase = IDatabase::getInstance();
    if(iDatabase.initPatientModel()){
        ui->tableView->setModel(iDatabase.patientTabModel);
        ui->tableView->setSelectionModel(iDatabase.thePatientSelection);
    }
}

PatientView::~PatientView()
{
    delete ui;
}

void PatientView::on_btAdd_clicked()
{
    int currow = IDatabase::getInstance().addNewPatient();
    emit goPatientEditView(currow);
}


void PatientView::on_btSearch_clicked()
{
    QString filter = QString("name like '%%1%'").arg(ui->txtSearch->text());
    IDatabase::getInstance().searchPaient(filter);
}


void PatientView::on_btDelete_clicked()
{
    QModelIndex curIndex = IDatabase::getInstance().thePatientSelection->currentIndex();
    if (!curIndex.isValid()) {
        // 提示用户未选中行
        QMessageBox::warning(this, "提示", "请先选中要删除的患者");
        return;
    }
    IDatabase::getInstance().deleteCurrentPatient();
}


void PatientView::on_btEdit_clicked()
{
    QModelIndex curIdex =
        IDatabase::getInstance().thePatientSelection->currentIndex();//获取当前单元格索引
    emit goPatientEditView(curIdex.row());
}

