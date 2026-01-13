#ifndef NETMANAGER_H
#define NETMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDateTime>
#include <QTimer>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QFile>
#include <QMap>
#include "idatabase.h"

// 网络模块核心类：独立封装 无业务耦合 可全局调用
// 实现：HTTP同步药品/诊断信息、TCP远程数据备份(全量+增量+断点续传+校验)
class NetManager : public QObject
{
    Q_OBJECT
public:
    // 单例模式，和IDatabase保持一致的设计风格
    static NetManager& getInstance();
    ~NetManager();

    // ================= HTTP 核心接口 - 药品/诊断参考信息同步 =================
    // 发送GET请求同步远程药品目录+诊断参考信息 同步到本地DRUG_STOCK/诊断表
    void syncDrugAndDiagnosisData(const QString& httpApiUrl);

    // ================= TCP 核心接口 - 远程数据备份(客户端) =================
    // 1. 连接TCP备份服务器
    bool connectTcpBackupServer(const QString& serverIp, quint16 serverPort, int timeoutMs = 5000);
    // 2. 断开TCP连接
    void disconnectTcpBackupServer();
    // 3. TCP全量备份：备份【病人+就诊+药品】全部核心数据
    bool tcpFullBackupAllData();
    // 4. TCP增量备份：备份【上次备份后新增/修改】的核心数据
    bool tcpIncrementBackupData(const QDateTime& lastBackupTime);

signals:
    // 同步/备份结果通知（可绑定页面刷新UI）
    void httpSyncResult(bool ok, QString msg);
    void tcpBackupResult(bool ok, QString msg);

private slots:
    // HTTP响应/错误处理槽函数
    void slot_httpReplyFinished();
    void slot_httpReplyError(QNetworkReply::NetworkError error);
    void slot_httpTimeOut();
    // TCP状态/数据/错误处理槽函数
    void slot_tcpSocketStateChanged(QAbstractSocket::SocketState state);
    void slot_tcpReadyRead();
    void slot_tcpSocketError(QAbstractSocket::SocketError error);

private:
    // 私有构造：单例模式 禁止外部实例化
    explicit NetManager(QObject *parent = nullptr);
    NetManager(const NetManager&) = delete;
    NetManager& operator=(const NetManager&) = delete;

    // 私有工具函数
    // 解析HTTP返回的JSON药品/诊断数据 写入本地数据库
    bool parseHttpJsonData(const QByteArray& jsonData);
    // 封装数据库数据为TCP传输数据包【自定义协议：4字节长度 + 32字节MD5 + JSON数据】
    QByteArray packTcpData(const QList<QJsonObject>& dataList);
    // MD5校验数据完整性
    QString getMd5CheckSum(const QByteArray& data);
    // 断点续传：读取/保存传输进度
    qint64 getLastUploadProgress(const QString& taskId);
    void saveUploadProgress(const QString& taskId, qint64 progress);

private:
    // HTTP核心对象
    QNetworkAccessManager*  m_httpManager;
    QNetworkReply*          m_httpReply;
    QTimer*                 m_httpTimeOutTimer;
    // TCP核心对象
    QTcpSocket*             m_tcpSocket;
    // 临时数据缓存
    QByteArray              m_tcpSendBuffer;
    QMap<QString, qint64>   m_uploadProgressMap;
};

#endif // NETMANAGER_H
