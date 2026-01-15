#include "welcomeview.h"
#include "ui_welcomeview.h"

WelcomeView::WelcomeView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WelcomeView)
{
    qDebug()<<"create WelcomeView";
                ui->setupUi(this);
}

WelcomeView::~WelcomeView()
{
    qDebug()<<"destroy WelcomeView";
    delete ui;
}

void WelcomeView::on_btDoctor_clicked()
{
    emit goDoctorView();
}


void WelcomeView::on_Department_clicked()
{
    emit goDepartmentView();
}


void WelcomeView::on_btPatient_clicked()
{
    emit goPatientView();
}

