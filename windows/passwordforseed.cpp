#include "passwordforseed.h"
#include "ui_passwordforseed.h"
#include <QMessageBox>
#include "../util/passwordanalyser.h"

namespace wnd {

PasswordForSeed::PasswordForSeed(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PasswordForSeed)
{
    ui->setupUi(this);

    util::PasswordAnalyser pa( ui->password1Edit->text() );
    ui->strengthLabel->setText(pa.getPasswordQualityStr());
}

PasswordForSeed::~PasswordForSeed()
{
    delete ui;
}

bool PasswordForSeed::validateData()
{
    QString pswd1 = ui->password1Edit->text();
    QString pswd2 = ui->password2Edit->text();

    if (pswd1!=pswd2) {
        QMessageBox::critical(this, "Password", "Password doesn't match confirm string. Please retype the password correctly");
        return false;
    }

    util::PasswordAnalyser pa(pswd1);
    if (! pa.isPasswordOK() ) {
        QMessageBox::critical(this, "Password", "Your password is not strong enough. Please input stronger password");
        return false;
    }

    return true;
}

void PasswordForSeed::on_password1Edit_textChanged(const QString &text)
{
    util::PasswordAnalyser pa(text);
    ui->strengthLabel->setText(pa.getPasswordQualityStr());
}

}
