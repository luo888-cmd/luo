#include "netmanager.h"
#include <QSqlRecord>   //修复 QSqlRecord 不完整类型 必加
#include <QAbstractSocket> //修复QTcpSocket状态枚举必加
#include <QDate>        //修复 QDate::currentDate() 必加
#include <QUuid>        //你的工程用到了UUID，这里补充

// 单例实现
NetManager &NetManager::getInstance()
{
    static NetManager instance;
    return instance;
}

NetManager::NetManager(QObject *parent)
    : QObject(parent)
    , m_httpManager(new QNetworkAccessManager(this))
    , m_httpReply(nullptr)
    , m_httpTimeOutTimer(new QTimer(this))
    , m_tcpSocket(new QTcpSocket(this))
{
    // HTTP超时定时器配置 30秒
    m_httpTimeOutTimer->setSingleShot(true);
    m_httpTimeOutTimer->setInterval(30000);
    connect(m_httpTimeOutTimer, SIGNAL(timeout()), this, SLOT(slot_httpTimeOut()));

    // TCP信号槽绑定
    connect(m_tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(slot_tcpSocketStateChanged(QAbstractSocket::SocketState)));
    connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(slot_tcpReadyRead()));
    connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slot_tcpSocketError(QAbstractSocket::SocketError)));
}

NetManager::~NetManager()
{
    // ==========修复1: 低版本Qt 替换 isConnected()==========
    if(m_tcpSocket->state() == QAbstractSocket::ConnectedState){
        disconnectTcpBackupServer();
    }
    if(m_httpReply){
        m_httpReply->abort();
        m_httpReply->deleteLater();
        m_httpReply = nullptr;
    }
    m_httpTimeOutTimer->stop();
}

// ====================== 【HTTP 实现】药品/诊断信息同步 ======================
void NetManager::syncDrugAndDiagnosisData(const QString &httpApiUrl)
{
    if(m_httpReply){
        m_httpReply->abort();
        m_httpReply->deleteLater();
        m_httpReply = nullptr;
    }

    QNetworkRequest request;
    request.setUrl(QUrl(httpApiUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json;charset=utf-8");
    // ==========修复2: 删除低版本Qt没有的 ConnectTimeoutAttribute/RequestTimeoutAttribute==========

    m_httpReply = m_httpManager->get(request);
    connect(m_httpReply, SIGNAL(finished()), this, SLOT(slot_httpReplyFinished()));
    connect(m_httpReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slot_httpReplyError(QNetworkReply::NetworkError)));
    // 启动HTTP超时定时器 替代原生超时属性
    m_httpTimeOutTimer->start();
}

void NetManager::slot_httpReplyFinished()
{
    m_httpTimeOutTimer->stop(); //关闭超时定时器
    if(!m_httpReply) return;

    int statusCode = m_httpReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray jsonData = m_httpReply->readAll();

    if(statusCode == 200){
        bool isOk = parseHttpJsonData(jsonData);
        if(isOk){
            QMessageBox::information(nullptr, "同步成功", "✅ 药品目录及诊断参考信息已同步至本地！");
            emit httpSyncResult(true, "同步成功");
        }else{
            QMessageBox::critical(nullptr, "同步失败", "❌ 数据格式错误，解析失败！");
            emit httpSyncResult(false, "数据解析失败");
        }
    }else{
        QString errMsg = QString("❌ HTTP请求失败，状态码：%1").arg(statusCode);
        QMessageBox::critical(nullptr, "同步失败", errMsg);
        emit httpSyncResult(false, errMsg);
    }

    m_httpReply->deleteLater();
    m_httpReply = nullptr;
}

void NetManager::slot_httpReplyError(QNetworkReply::NetworkError error)
{
    m_httpTimeOutTimer->stop();
    if(!m_httpReply) return;
    QString errMsg = QString("❌ 网络异常：%1").arg(m_httpReply->errorString());
    QMessageBox::critical(nullptr, "同步失败", errMsg);
    emit httpSyncResult(false, errMsg);
    m_httpReply->deleteLater();
    m_httpReply = nullptr;
}

// HTTP请求超时处理槽函数
void NetManager::slot_httpTimeOut()
{
    if(m_httpReply){
        m_httpReply->abort();
        QMessageBox::critical(nullptr, "同步超时", "❌ 请求超时，请检查网络连接！");
        emit httpSyncResult(false, "请求超时");
    }
}

// 解析远程JSON数据并写入本地数据库【完美适配你的DRUG_STOCK表】
bool NetManager::parseHttpJsonData(const QByteArray &jsonData)
{
    QJsonParseError jsonErr;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &jsonErr);
    if(jsonErr.error != QJsonParseError::NoError){
        qDebug() << "JSON解析错误：" << jsonErr.errorString();
        return false;
    }

    QJsonObject rootObj = doc.object();
    QJsonArray drugArray = rootObj["drug_list"].toArray();    // 远程药品列表
    QJsonArray diagArray = rootObj["diagnosis_list"].toArray();//远程诊断参考信息

    IDatabase& db = IDatabase::getInstance();
    if(!db.initDrugStockModel()) return false;

    // 开启事务 保证数据一致性
    db.drugStockTabModel->database().transaction();
    // 清空旧药品数据，同步最新数据
    db.drugStockTabModel->removeRows(0, db.drugStockTabModel->rowCount());

    // 写入药品数据到本地DRUG_STOCK表
    foreach (const QJsonValue& val, drugArray) {
        QJsonObject drugObj = val.toObject();
        int newRow = db.drugStockTabModel->rowCount();
        db.drugStockTabModel->insertRow(newRow);
        db.drugStockTabModel->setData(db.drugStockTabModel->index(newRow, db.drugStockTabModel->fieldIndex("ID")), QUuid::createUuid().toString(QUuid::WithoutBraces));
        db.drugStockTabModel->setData(db.drugStockTabModel->index(newRow, db.drugStockTabModel->fieldIndex("DRUG_NAME")), drugObj["drug_name"].toString());
        db.drugStockTabModel->setData(db.drugStockTabModel->index(newRow, db.drugStockTabModel->fieldIndex("SPECIFICATION")), drugObj["spec"].toString());
        db.drugStockTabModel->setData(db.drugStockTabModel->index(newRow, db.drugStockTabModel->fieldIndex("STOCK_NUM")), drugObj["stock_num"].toInt());
        db.drugStockTabModel->setData(db.drugStockTabModel->index(newRow, db.drugStockTabModel->fieldIndex("UNIT")), drugObj["unit"].toString());
        db.drugStockTabModel->setData(db.drugStockTabModel->index(newRow, db.drugStockTabModel->fieldIndex("PRICE")), drugObj["price"].toDouble());
        db.drugStockTabModel->setData(db.drugStockTabModel->index(newRow, db.drugStockTabModel->fieldIndex("UPDATE_TIME")), QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    }

    if(db.drugStockTabModel->database().commit()){
        db.drugStockTabModel->submitAll();
        return true;
    }else{
        db.drugStockTabModel->database().rollback();
        return false;
    }
}

// ====================== 【TCP 实现】远程数据备份(客户端) ======================
bool NetManager::connectTcpBackupServer(const QString &serverIp, quint16 serverPort, int timeoutMs)
{
    // ==========修复3: 低版本Qt 替换 isConnected()==========
    if(m_tcpSocket->state() == QAbstractSocket::ConnectedState) return true;

    m_tcpSocket->connectToHost(serverIp, serverPort);
    if(!m_tcpSocket->waitForConnected(timeoutMs)){
        QString errMsg = QString("❌ 连接备份服务器失败：%1").arg(m_tcpSocket->errorString());
        QMessageBox::critical(nullptr, "连接失败", errMsg);
        return false;
    }
    QMessageBox::information(nullptr, "连接成功", "✅ 已成功连接远程备份服务器！");
    return true;
}

void NetManager::disconnectTcpBackupServer()
{
    if(m_tcpSocket->state() == QAbstractSocket::ConnectedState){
        m_tcpSocket->disconnectFromHost();
        m_tcpSocket->waitForDisconnected(3000);
        QMessageBox::information(nullptr, "断开连接", "✅ 已断开与备份服务器的连接！");
    }
}

// TCP全量备份：病人+就诊+药品 全部核心数据
bool NetManager::tcpFullBackupAllData()
{
    // ==========修复4: 低版本Qt 替换 isConnected()==========
    if(m_tcpSocket->state() != QAbstractSocket::ConnectedState){
        QMessageBox::warning(nullptr, "备份失败", "❌ 请先连接备份服务器！");
        return false;
    }

    IDatabase& db = IDatabase::getInstance();
    QList<QJsonObject> dataList;

    // 1. 读取所有病人数据
    if(db.initPatientModel()){
        for(int i=0; i<db.patientTabModel->rowCount(); i++){
            QJsonObject obj;
            obj["type"] = "patient";
            obj["ID"] = db.patientTabModel->record(i).value("ID").toString();
            obj["NAME"] = db.patientTabModel->record(i).value("NAME").toString();
            obj["ID_CARD"] = db.patientTabModel->record(i).value("ID_CARD").toString();
            obj["HEIGHT"] = db.patientTabModel->record(i).value("HEIGHT").toInt();
            obj["MOBILEPHONE"] = db.patientTabModel->record(i).value("MOBILEPHONE").toString();
            obj["DOB"] = db.patientTabModel->record(i).value("DOB").toString();
            obj["SEX"] = db.patientTabModel->record(i).value("SEX").toString();
            dataList.append(obj);
        }
    }

    // 2. 读取所有就诊记录
    if(db.initVisitModel()){
        for(int i=0; i<db.visitTabModel->rowCount(); i++){
            QJsonObject obj;
            obj["type"] = "visit";
            obj["ID"] = db.visitTabModel->record(i).value("ID").toString();
            obj["PATIENT_NAME"] = db.visitTabModel->record(i).value("PATIENT_NAME").toString();
            obj["DOCTOR_NAME"] = db.visitTabModel->record(i).value("DOCTOR_NAME").toString();
            obj["SYMPTOMS"] = db.visitTabModel->record(i).value("SYMPTOMS").toString();
            obj["PRESCRIPTION"] = db.visitTabModel->record(i).value("PRESCRIPTION").toString();
            obj["VISIT_TIME"] = db.visitTabModel->record(i).value("VISIT_TIME").toString();
            dataList.append(obj);
        }
    }

    // 3. 打包数据并发送
    QByteArray sendData = packTcpData(dataList);
    // ==========修复5: QDateTime::currentDate → QDate::currentDate==========
    QString taskId = "full_backup_" + QDate::currentDate().toString("yyyyMMdd");
    qint64 lastProg = getLastUploadProgress(taskId);

    // 断点续传：从上次失败的位置开始发送
    if(lastProg >0 && lastProg < sendData.size()){
        sendData = sendData.mid(lastProg);
    }

    // 分块发送 防止大数据包卡顿
    const qint64 blockSize = 1024*1024; // 1MB/块
    qint64 sent = 0;
    while(sent < sendData.size()){
        qint64 writeSize = qMin(blockSize, sendData.size()-sent);
        qint64 realWrite = m_tcpSocket->write(sendData.mid(sent, writeSize));
        if(realWrite <=0){
            saveUploadProgress(taskId, lastProg+sent);
            QMessageBox::warning(nullptr, "备份中断", QString("❌ 备份失败，已保存断点：%1字节").arg(lastProg+sent));
            return false;
        }
        sent += realWrite;
        m_tcpSocket->waitForBytesWritten(100);
        saveUploadProgress(taskId, lastProg+sent);
    }

    saveUploadProgress(taskId, 0); // 发送完成 清空断点
    QMessageBox::information(nullptr, "备份成功", QString("✅ 全量备份完成，共发送 %1 字节数据").arg(sent));
    emit tcpBackupResult(true, "全量备份成功");
    return true;
}

// TCP增量备份：仅备份上次备份后新增/修改的数据
bool NetManager::tcpIncrementBackupData(const QDateTime &lastBackupTime)
{
    // ==========修复6: 低版本Qt 替换 isConnected()==========
    if(m_tcpSocket->state() != QAbstractSocket::ConnectedState){
        QMessageBox::warning(nullptr, "备份失败", "❌ 请先连接备份服务器！");
        return false;
    }

    IDatabase& db = IDatabase::getInstance();
    QList<QJsonObject> dataList;

    // 仅读取 上次备份后 新增的病人/就诊数据
    if(db.initPatientModel()){
        db.patientTabModel->setFilter(QString("CREATEDTIMESTAMP > '%1'").arg(lastBackupTime.toString("yyyy-MM-dd")));
        db.patientTabModel->select();
        for(int i=0; i<db.patientTabModel->rowCount(); i++){
            QJsonObject obj;
            obj["type"] = "patient";
            obj["ID"] = db.patientTabModel->record(i).value("ID").toString();
            obj["NAME"] = db.patientTabModel->record(i).value("NAME").toString();
            dataList.append(obj);
        }
    }

    if(db.initVisitModel()){
        db.visitTabModel->setFilter(QString("VISIT_TIME > '%1'").arg(lastBackupTime.toString("yyyy-MM-dd HH:mm:ss")));
        db.visitTabModel->select();
        for(int i=0; i<db.visitTabModel->rowCount(); i++){
            QJsonObject obj;
            obj["type"] = "visit";
            obj["ID"] = db.visitTabModel->record(i).value("ID").toString();
            obj["PATIENT_NAME"] = db.visitTabModel->record(i).value("PATIENT_NAME").toString();
            dataList.append(obj);
        }
    }

    if(dataList.isEmpty()){
        QMessageBox::information(nullptr, "增量备份", "✅ 暂无新增/修改数据，无需备份！");
        emit tcpBackupResult(true, "无增量数据");
        return true;
    }

    QByteArray sendData = packTcpData(dataList);
    qint64 sent = m_tcpSocket->write(sendData);
    if(sent <=0){
        QMessageBox::critical(nullptr, "备份失败", "❌ 增量数据发送失败！");
        return false;
    }
    m_tcpSocket->waitForBytesWritten(3000);
    QMessageBox::information(nullptr, "备份成功", QString("✅ 增量备份完成，共发送 %1 字节数据").arg(sent));
    emit tcpBackupResult(true, "增量备份成功");
    return true;
}

// TCP数据包打包：自定义协议 【4字节数据长度 + 32字节MD5校验码 + JSON数据】
QByteArray NetManager::packTcpData(const QList<QJsonObject> &dataList)
{
    QJsonArray jsonArr;
    foreach (const QJsonObject& obj, dataList) {
        jsonArr.append(obj);
    }
    QByteArray jsonData = QJsonDocument(jsonArr).toJson(QJsonDocument::Compact);
    QString md5 = getMd5CheckSum(jsonData);

    QByteArray packData;
    qint32 dataLen = jsonData.size();
    packData.append((const char*)&dataLen, 4);  // 写入数据长度
    packData.append(md5.toUtf8());              // 写入MD5校验码
    packData.append(jsonData);                  // 写入实际数据

    return packData;
}

// MD5校验：保证数据传输完整性
QString NetManager::getMd5CheckSum(const QByteArray &data)
{
    return QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();
}

// 读取断点续传进度
qint64 NetManager::getLastUploadProgress(const QString &taskId)
{
    QFile file("./backup_progress.ini");
    if(!file.open(QIODevice::ReadOnly)) return 0;
    QJsonObject obj = QJsonDocument::fromJson(file.readAll()).object();
    file.close();
    return obj[taskId].toInt(0);
}

// 保存断点续传进度
void NetManager::saveUploadProgress(const QString &taskId, qint64 progress)
{
    QFile file("./backup_progress.ini");
    if(!file.open(QIODevice::WriteOnly)) return;
    QJsonObject obj;
    if(progress ==0) obj.remove(taskId);
    else obj[taskId] = progress;
    file.write(QJsonDocument(obj).toJson());
    file.close();
}

// TCP槽函数实现
void NetManager::slot_tcpSocketStateChanged(QAbstractSocket::SocketState state)
{
    switch (state) {
    case QAbstractSocket::ConnectedState: qDebug()<<"TCP已连接"; break;
    // ==========修复7: DisconnectedState → UnconnectedState==========
    case QAbstractSocket::UnconnectedState: qDebug()<<"TCP已断开"; break;
    case QAbstractSocket::ConnectingState: qDebug()<<"TCP正在连接"; break;
    default: break;
    }
}

void NetManager::slot_tcpReadyRead()
{
    QByteArray resp = m_tcpSocket->readAll();
    qDebug()<<"服务器响应："<<resp;
    if(resp.contains("MD5_CHECK_OK")){
        QMessageBox::information(nullptr, "校验成功", "✅ 数据传输完整，服务端校验通过！");
    }
}

void NetManager::slot_tcpSocketError(QAbstractSocket::SocketError error)
{
    QString errMsg = QString("TCP错误：%1").arg(m_tcpSocket->errorString());
    qDebug()<<errMsg;
    QMessageBox::critical(nullptr, "TCP错误", errMsg);
    emit tcpBackupResult(false, errMsg);
}
