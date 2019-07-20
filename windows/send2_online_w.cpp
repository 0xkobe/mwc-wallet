#include "send2_online_w.h"
#include "ui_send2_online.h"
#include "sendcoinsparamsdialog.h"
#include "../control/messagebox.h"
#include "../util/address.h"
#include "../state/send2_Online.h"
#include "../state/timeoutlock.h"

namespace wnd {

QString generateAmountErrorMsg( int64_t mwcAmount, const wallet::AccountInfo & acc, const core::SendCoinsParams & sendParams ) {
        QString msg2print = "You are trying to send " + util::nano2one(mwcAmount) + " wmc, but you only have " +
                            util::nano2one(acc.currentlySpendable) + " spendable wmc.";
        if (acc.awaitingConfirmation > 0)
            msg2print += " " + util::nano2one(acc.awaitingConfirmation) + " coins are awaiting confirmation.";

        if (acc.lockedByPrevTransaction > 0)
            msg2print += " " + util::nano2one(acc.lockedByPrevTransaction) + " coins are locked.";

        if (acc.awaitingConfirmation > 0 || acc.lockedByPrevTransaction > 0) {
            if (sendParams.inputConfirmationNumber != 1) {
                if (sendParams.inputConfirmationNumber < 0)
                    msg2print += " You can modify settings to spend wmc with less than 10 confirmations (wallet default value).";
                else
                    msg2print += " You can modify settings to spend wmc with less than " +
                                 QString::number(sendParams.inputConfirmationNumber) + " confirmations.";
            }
        }
        return msg2print;
}

SendOnline::SendOnline(QWidget *parent, state::SendOnline * _state ) :
    core::NavWnd(parent, _state->getContext() ),
    ui(new Ui::SendOnline),
    state(_state)
{
    ui->setupUi(this);

    ui->progress->initLoader(false);

    // inti accounts
    accountInfo = state->getWalletBalance();
    QString selectedAccount = state->getCurrentAccountName();
    contacts    = state->getContacts();

    int selectedAccIdx = 0;
    int idx=0;
    for (auto & info : accountInfo) {
        if (info.accountName == selectedAccount)
            selectedAccIdx = idx;

        ui->accountComboBox->addItem( info.getLongAccountName(), QVariant(idx++) );
    }
    ui->accountComboBox->setCurrentIndex(selectedAccIdx);
}

SendOnline::~SendOnline()
{
    state->deleteWnd(this);
    delete ui;
}



void SendOnline::on_contactsButton_clicked()
{
    state::TimeoutLockObject to( state );

    // Get the contacts
    control::MessageBox::message(this, "Not implemented", "This functionality is not implemented yet");

/*    SelectContactDlg dlg(this, contacts);
    if (dlg.exec() == QDialog::Accepted) {
        wallet::WalletContact selectedContact = dlg.getSelectedContact();
        ui->sendEdit->setText( selectedContact.address );
    }*/

}

void SendOnline::on_allAmountButton_clicked()
{
    ui->amountEdit->setText("All");//  util::nano2one( accountInfo[accountIdx].currentlySpendable ) );
}

void SendOnline::on_settingsButton2_clicked()
{
    state::TimeoutLockObject to( state );

    core::SendCoinsParams  params = state->getSendCoinsParams();

    SendCoinsParamsDialog dlg(this, params);
    if (dlg.exec() == QDialog::Accepted) {
        state->updateSendCoinsParams( dlg.getSendCoinsParams() );
    }

}

void SendOnline::on_sendButton_clicked()
{
    state::TimeoutLockObject to( state );

    auto dt = ui->accountComboBox->currentData();
    if (!dt.isValid())
        return;

    int accountIdx = dt.toInt();
    wallet::AccountInfo acc = accountInfo[accountIdx];

    QString sendAmount = ui->amountEdit->text().trimmed();

    QPair<bool, int64_t> mwcAmount;
    if (sendAmount != "All") {
        mwcAmount = util::one2nano(ui->amountEdit->text().trimmed());
        if (!mwcAmount.first) {
            control::MessageBox::message(this, "Incorrect Input", "Please specify correct number of MWC to send");
            ui->amountEdit->setFocus();
            return;
        }
    }
    else { // All
        mwcAmount = QPair<bool, int64_t>(true, -1);
    }

    QString sendTo = ui->sendEdit->text().trimmed();
    if ( sendTo.size()>0 && sendTo[0] == '@' )
        sendTo = sendTo.right(sendTo.size()-1);

    {
        QPair<bool, QString> valRes = util::validateMwc713Str(sendTo);
        if (!valRes.first) {
            control::MessageBox::message(this, "Incorrect Input", valRes.second);
            ui->sendEdit->setFocus();
            return;
        }
    }


    if (sendTo.size() == 0 ) {
        control::MessageBox::message(this, "Incorrect Input",
                                     "Please specify an address of send your MWC" );
        ui->sendEdit->setFocus();
        return;
    }

    // init expected to be fixed, so no need to disable the message
    if ( mwcAmount.second > acc.currentlySpendable ) {

        QString msg2print = generateAmountErrorMsg( mwcAmount.second, acc, state->getSendCoinsParams() );

        control::MessageBox::message(this, "Incorrect Input",
                                     msg2print );
        ui->amountEdit->setFocus();
        return;
    }


    // Check the address. Try contacts first
    QString address;

    for ( auto & cnt : contacts ) {
        if (cnt.name == sendTo) {
            address = "@" + cnt.name;
            break;
        }
    }
    if (address.isEmpty()) {
        for ( auto & cnt : contacts ) {
            if (cnt.name.compare(sendTo,  Qt::CaseSensitivity::CaseInsensitive)) {
                address = "@" + cnt.name;
                break;
            }
        }
    }


    if (address.isEmpty())
        address = sendTo;

    // Let's  verify address first
    if (address[0]!='@') {
        QPair< bool, util::ADDRESS_TYPE > res = util::verifyAddress(address);
        if ( !res.first ) {
            control::MessageBox::message(this, "Incorrect Input",
                                         "Please specify correct address to send your MWC" );
            ui->sendEdit->setFocus();
            return;
        }
    }

    QString description = ui->descriptionEdit->toPlainText().trimmed();

    {
        QPair<bool, QString> valRes = util::validateMwc713Str(description);
        if (!valRes.first) {
            control::MessageBox::message(this, "Incorrect Input", valRes.second);
            ui->descriptionEdit->setFocus();
            return;
        }
    }

    // Ask for confirmation

    if ( control::MessageBox::RETURN_CODE::BTN2 != control::MessageBox::question(this,"Confirn Send request",
                                  "You are sending " + util::nano2one(mwcAmount.second) + " mwc to address " + address, "Decline", "Confirm", false, true ) )
        return;

    ui->progress->show();

    state->sendMwc( accountInfo[accountIdx], address, mwcAmount.second, description );
}

void SendOnline::sendRespond( bool success, const QStringList & errors ) {
    state::TimeoutLockObject to( state );

    ui->progress->hide();

    if (success) {
        control::MessageBox::message(this, "Success", "Your mwc was successfully sent to recipient");
        ui->sendEdit->setText("");
        ui->amountEdit->setText("");
        ui->descriptionEdit->setText("");
        return;
    }

    QString errMsg = util::formatErrorMessages(errors);

    if (errMsg.isEmpty())
        errMsg = "Your send request was failed by some reasons";
    else
        errMsg = "Your send request was failed:\n" + errMsg;

    control::MessageBox::message( this, "Send request was failed", errMsg );
}


}


