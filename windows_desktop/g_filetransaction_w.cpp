// Copyright 2019 The MWC Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "g_filetransaction_w.h"
#include "ui_g_filetransaction.h"
#include "../control_desktop/messagebox.h"
#include "../dialogs_desktop/g_sendconfirmationdlg.h"
#include "../util_desktop/timeoutlock.h"
#include "../bridge/wnd/g_filetransaction_b.h"
#include "../bridge/wallet_b.h"
#include "../bridge/config_b.h"
#include "../bridge/util_b.h"
#include "../core/global.h"

namespace wnd {

FileTransaction::FileTransaction(QWidget *parent,
                                 QString callerId,
                                 const QString & _fileNameOrSlatepack, const util::FileTransactionInfo & transInfo,
                                 int nodeHeight,
                                 QString transactionType, QString processButtonName) :
        core::NavWnd(parent),
        ui(new Ui::FileTransaction),
        fileNameOrSlatepack(_fileNameOrSlatepack)
{
    ui->setupUi(this);
    txUuid = transInfo.transactionId;

    fileTransaction = new bridge::FileTransaction(this);
    fileTransaction->setCallerId(callerId);
    wallet = new bridge::Wallet(this);
    config = new bridge::Config(this);
    util = new bridge::Util(this);

    QObject::connect( fileTransaction, &bridge::FileTransaction::sgnHideProgress,
                      this, &FileTransaction::onSgnHideProgress, Qt::QueuedConnection);

    ui->progress->initLoader(false);

    ui->transactionType->setText(transactionType);
    ui->processButton->setText(processButtonName);

    if (transactionType.contains("Receive", Qt::CaseInsensitive)) {
        ui->receiverLabel->setText("<b>Sender Address:</b>");
    }

    if (transInfo.amount_fee_not_defined)
        ui->mwcLabel->setText( "-" );
    else
        ui->mwcLabel->setText( util::nano2one( transInfo.amount ) + " MWC" );

    ui->transactionIdLabel->setText(txUuid);
    ui->lockHeightLabel->setText( transInfo.lock_height>nodeHeight ? util::longLong2Str(transInfo.lock_height) : "-" );
    ui->receiverAddressLabel->setText(transInfo.fromAddress.isEmpty() ? "-" : transInfo.fromAddress);
    ui->message->setText( transInfo.message );

    if (!fileTransaction->needResultTxFileName()) {
        QSize rc = ui->resultLocationFrame->frameSize();
        ui->resultLocationFrame->hide();

        QRect cbRc = ui->cancelButton->frameGeometry();
        cbRc.moveTop( cbRc.top() - rc.height() );
        ui->cancelButton->setGeometry( cbRc );

        cbRc = ui->processButton->frameGeometry();
        cbRc.moveTop( cbRc.top() - rc.height() );
        ui->processButton->setGeometry( cbRc );

        ui->primaryFrame->setMinimumHeight( ui->primaryFrame->minimumHeight() - rc.height() );
    }
    else
    { // set default file name if possible
        if (!fileNameOrSlatepack.startsWith("BEGINSLATE")) {
            // it is a filename
            QString resFN = fileNameOrSlatepack;
            if (resFN.endsWith(".response"))
                resFN = resFN.left(resFN.length() - int(strlen(".response")));

            resFN += ".mwctx";
            ui->resultingTxFileName->setText(resFN);
        }
    }
}

FileTransaction::~FileTransaction() {
    delete ui;
}

void FileTransaction::on_cancelButton_clicked() {
    fileTransaction->ftBack();
}

void FileTransaction::on_processButton_clicked()
{
    util::TimeoutLockObject to( "FileTransaction" );

    QString resTxFN;
    if ( fileTransaction->needResultTxFileName() ) {
        resTxFN = ui->resultingTxFileName->text();
        if (resTxFN.isEmpty()) {
            control::MessageBox::messageText( this, "Input value", "Please specify the file name for the resulting transaction." );
            ui->resultingTxFileName->setFocus();
            return;
        }

        QPair <bool, QString> res = util::validateMwc713Str(resTxFN);
        if (!res.first) {
            control::MessageBox::messageText( this, "Input", res.second );
            ui->resultingTxFileName->setFocus();
            return;
        }
    }
    else {
        // Check if node healthy first
        if (!fileTransaction->isNodeHealthy()) {
            control::MessageBox::messageText(this, "Unable to finalize", "Your MWC Node, that wallet connected to, is not ready to finalize transactions.\n"
                                                                         "MWC Node need to be connected to few peers and finish blocks synchronization process");
            return;
        }
    }
    QString walletPasswordHash = wallet->getPasswordHash();
    if (!walletPasswordHash.isEmpty()) {
        if (mwc::isFinalize()) {
            dlg::SendConfirmationDlg confirmDlg(this, "Confirm Finalize Request",
                                            "You are finalizing transaction for " + ui->mwcLabel->text(),
                                            1.0, walletPasswordHash );
            if (confirmDlg.exec() != QDialog::Accepted)
                return;
        }
    }
    ui->progress->show();

    if (fileNameOrSlatepack.startsWith("BEGINSLATE")) {
        fileTransaction->ftContinueSlatepack( fileNameOrSlatepack, txUuid, resTxFN, config->isFluffSet() );
    }
    else
    { // file
        fileTransaction->ftContinue( fileNameOrSlatepack, resTxFN, config->isFluffSet() );
    }


}

void FileTransaction::onSgnHideProgress() {
    ui->progress->hide();
}

void FileTransaction::on_resultTransFileNameSelect_clicked()
{

    QString fileName = util->getSaveFileName("Resulting MWC transaction",
                                             "resultTx",
                                             "MWC transaction (*.mwctx)",
                                             ".mwctx");

    if (fileName.isEmpty())
        return;

    ui->resultingTxFileName->setText(fileName);
}

}


