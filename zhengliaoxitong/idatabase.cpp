#include "idatabase.h"
#include <QUuid>

void IDatabase::initDatabase()
{
    database=QSqlDatabase::addDatabase(
        "QSQLITE");//添加sql lite 数据块驱动
    QString aFile="D:/Qt/luo.db";
    database.setDatabaseName(aFile);//数据库名称


    if(!database.open()){//打开数据库
        qDebug()<<"failed to open database";
    }else
        qDebug()<<"open database is ok" << database.connectionName();
}

bool IDatabase::initPatientModel()
{
    patientTabModel = new QSqlTableModel(this,database);
    patientTabModel->setTable("patient");
    patientTabModel->setEditStrategy(
        QSqlTableModel::OnManualSubmit);//保存数据方式
    patientTabModel->setSort(patientTabModel->fieldIndex("name"),Qt::AscendingOrder);
    if(!(patientTabModel->select()))//查询数据
        return false;
    thePatientSelection = new QItemSelectionModel(patientTabModel);
    return true;
}

int IDatabase::addNewPatient()
{
    patientTabModel->insertRow(patientTabModel->rowCount(),
                               QModelIndex());//在末尾添加一条记录
    QModelIndex curIndex = patientTabModel->index(patientTabModel->rowCount()-1,
                                                  1);//创建最后一行的modelindex
    int curRecNo = curIndex.row();
    QSqlRecord curRec = patientTabModel->record(curRecNo);//获取当前记录
    curRec.setValue("CREATEDTIMESTAMP",QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    curRec.setValue("ID",QUuid::createUuid().toString(QUuid::WithoutBraces));
    patientTabModel->setRecord(curRecNo,curRec);
    return curIndex.row();
}

bool IDatabase::searchPaient(QString filter)
{
    patientTabModel->setFilter(filter);
    return patientTabModel->select();
}

bool IDatabase::deleteCurrentPatient()
{
    QModelIndex curIndex = thePatientSelection->currentIndex();
    if (!curIndex.isValid()) {
        return false;
    }
    // 先清除选择状态
    thePatientSelection->clearSelection();
    // 再删除行
    if (patientTabModel->removeRow(curIndex.row())) {
        if (patientTabModel->submitAll()) {
            patientTabModel->select(); // 刷新数据
            return true;
        } else {
            qDebug() << "删除提交失败：" << patientTabModel->lastError().text();
            patientTabModel->revertAll();
            return false;
        }
    }
    return false;
}

bool IDatabase::submitPatientEdit()
{
    return patientTabModel->submitAll();
}

void IDatabase::revertPatientEdit()
{
    patientTabModel->revertAll();
}

QString IDatabase::userLogin(QString userName, QString password)
{
    //return "loginOK";
    QSqlQuery query;
    query.prepare ("select username,password from User where username = :USER");
    query.bindValue(":USER",userName);
    query.exec();
    qDebug() << query.lastQuery() << query.first();

    if(query.first() && query.value("username").isValid()){
        QString passwd = query.value("password").toString();
        if(passwd == password)
        {
            qDebug() << "login ok";
            return "loginOK";
        } else {
            qDebug() << "wrong password";
            return "wrongPassword";
        }
    } else {
        qDebug() << "no such user";
        return "wrongUsername";
    }
}
IDatabase::IDatabase(QObject *parent): QObject(parent)
{
    initDatabase();
}
