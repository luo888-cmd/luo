#include "doctorview.h"
#include "ui_doctorview.h"

DoctorView::DoctorView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DoctorView)
{
    ui->setupUi(this);
}

DoctorView::~DoctorView()
{
    delete ui;
}
