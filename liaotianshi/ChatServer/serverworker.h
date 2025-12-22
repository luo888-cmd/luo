#ifndef SERVERWORKER_H
#define SERVERWORKER_H

#include <QObject>
#include <QTcpSocket>

class ServerWorker : public QObject
{
    Q_OBJECT
public:
    explicit ServerWorker(QObject *parent = nullptr);
    virtual bool setSocketDescriptor(qintptr socketDescriptor);
    // 新增：声明获取用户名的方法
    QString userName();
    // 新增：声明设置用户名的方法
    void setUserName(QString user);


signals:
    void logMessage(const QString &msg);
    void jsonReceived(ServerWorker *sender, const QJsonObject &docObj);
    void disconnectedFromClient();

private:
    QTcpSocket * m_serverSocket;
    QString m_userName;

public slots:
    void onReadyRead();
    void sendMessage(const QString &text, const QString &type = "message"); // 补充默认参数
    void sendJson(const QJsonObject &json);
};

#endif // SERVERWORKER_H
