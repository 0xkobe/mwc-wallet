#include "windows/k_accounttransfer_w.h"
#include "ui_k_accounttransfer_w.h"
#include "../control/messagebox.h"
#include "../state/k_AccountTransfer.h"
#include "sendcoinsparamsdialog.h"
#include "send2_online_w.h"

namespace wnd {

AccountTransfer::AccountTransfer(QWidget *parent, state::AccountTransfer * _state) :
    core::NavWnd(parent, _state->getStateMachine(), _state->getAppContext()),
    ui(new Ui::AccountTransfer),
    state(_state)
{
    ui->setupUi(this);
    ui->progress->initLoader(false);

    updateAccounts();
}

AccountTransfer::~AccountTransfer()
{
    delete ui;
}


void AccountTransfer::updateAccounts() {
    accountInfo = state->getWalletBalance();

    ui->accountFromCB->clear();
    ui->accountToCB->clear();

    int idx = 0;
    for (auto & info : accountInfo) {
        QString accountStr = info.getLongAccountName();
        ui->accountFromCB->addItem(accountStr, QVariant(idx) );
        ui->accountToCB->addItem(accountStr, QVariant(idx) );

        idx++;
    }
    ui->accountFromCB->setCurrentIndex(-1);
    ui->accountToCB->setCurrentIndex(-1);
}


void AccountTransfer::showTransferResults(bool ok, QString errMsg) {

    ui->progress->hide();

    if (ok) {
        control::MessageBox::message(this, "Success", "Your funds was successfully transferred");
        // reset state
        ui->amountEdit->setText("");
        updateAccounts();
    }
    else {
        control::MessageBox::message(this, "Transfer failure", "Funds transfer request was failed.\n" + errMsg);
    }
}


void AccountTransfer::on_allAmountButton_clicked()
{
    ui->amountEdit->setText("All");
}

void AccountTransfer::on_settingsButton_clicked()
{
    core::SendCoinsParams  params = state->getSendCoinsParams();

    SendCoinsParamsDialog dlg(this, params);
    if (dlg.exec() == QDialog::Accepted) {
        state->updateSendCoinsParams( dlg.getSendCoinsParams() );
    }
}

void AccountTransfer::on_transferButton_clicked()
{
    auto fromDt = ui->accountFromCB->currentData();
    auto toDt = ui->accountToCB->currentData();
    if ( !fromDt.isValid() || !toDt.isValid() ) {
        control::MessageBox::message(this, "Incorrect Input", "Please select pair of accounts to transfer coins.");
        return;
    }

    int fromI = fromDt.toInt();
    int toI = toDt.toInt();

    if (fromI == toI || fromI<0 || toI<0 || fromI>accountInfo.size() || toI>accountInfo.size()) {
        control::MessageBox::message(this, "Incorrect Input", "Please select pair of different accounts to transfer coins.");
        return;
    }

    QString sendAmount = ui->amountEdit->text();

    QPair<bool, int64_t> mwcAmount;
    if (sendAmount != "All") {
        mwcAmount = util::one2nano(ui->amountEdit->text());
        if (!mwcAmount.first) {
            control::MessageBox::message(this, "Incorrect Input", "Please specify correct number of MWC to send");
            ui->amountEdit->setFocus();
            return;
        }
    }
    else { // All
        mwcAmount = QPair<bool, int64_t>(true, -1);
    }

    auto & acc = accountInfo[fromI];
    if ( mwcAmount.second > acc.currentlySpendable ) {

        QString msg2print = generateAmountErrorMsg( mwcAmount.second, acc, state->getSendCoinsParams() );

        control::MessageBox::message(this, "Incorrect Input",
                                     msg2print );
        ui->amountEdit->setFocus();
        return;
    }


    ui->progress->show();
    state->transferFunds( accountInfo[fromI], accountInfo[toI] , mwcAmount.second );
}

void wnd::AccountTransfer::on_backButton_clicked()
{
    state->goBack();
}

}
