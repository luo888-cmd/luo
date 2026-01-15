#ifndef IDATABASE_H
#define IDATABASE_H
#include <QObject>
#include <Qtsql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDataWidgetMapper>

class IDatabase :public QObject
{
    Q_OBJECT
public:
    static IDatabase &getInstance()
    {
        static IDatabase instance;
        return instance;
    }

    QString userLogin(QString userName,QString password);

private:
    explicit IDatabase(QObject *parent = nullptr);
    IDatabase(IDatabase const&)          = delete;
    void operator=(IDatabase const &)= delete;
    QSqlDatabase database;

    void initDatabase();

signals:

public:

    bool initPatientModel();
    int addNewPatient();
    bool searchPaient(QString filter);
    bool deleteCurrentPatient();
    bool submitPatientEdit();
    void revertPatientEdit();

    QSqlTableModel *patientTabModel;
    QItemSelectionModel *thePatientSelection;

};
#endif // IDATABASE H
