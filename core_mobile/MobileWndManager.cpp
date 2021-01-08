// Copyright 2020 The MWC Developers
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

#include "MobileWndManager.h"
#include <QCoreApplication>
#include <QThread>
#include <QQmlApplicationEngine>
#include <QMessageBox>
#include "../state/state.h"
#include <QJsonDocument>
#include <QDebug>

namespace core {

// NOTE: all  xxxxDlg methids must be modal. In mobile conetext it is mean that we should wait for closure and return back the result.
void MobileWndManager::init(QQmlApplicationEngine * _engine) {
    engine = _engine;
    Q_ASSERT(engine);

    engine->load(QUrl(QStringLiteral("qrc:/windows_mobile/main.qml")));
    mainWindow = engine->rootObjects().first();
    mainWindow->setProperty("currentState", 0);
}


void MobileWndManager::messageTextDlg( QString title, QString message, double widthScale) {
    if (engine== nullptr) {
        Q_ASSERT(false); // early crash, not much what we can do. May be do some logs (message is an error description)?
        return;
    }
    Q_UNUSED(widthScale)
    if (mainWindow) {
        QVariant retValue;
        QMetaObject::invokeMethod(mainWindow, "openMessageTextDlg", Q_RETURN_ARG(QVariant, retValue), Q_ARG(QVariant, title), Q_ARG(QVariant, message));
    } else {
        QMessageBox::critical(nullptr, title,message);
    }
}

void MobileWndManager::messageHtmlDlg( QString title, QString message, double widthScale)  {
    Q_UNUSED(title)
    Q_UNUSED(message)
    Q_UNUSED(widthScale)

    Q_ASSERT(false); // implement me
}

// Two button box
WndManager::RETURN_CODE MobileWndManager::questionTextDlg( QString title, QString message, QString btn1, QString btn2,
                                                           QString btn1Tooltip, QString btn2Tooltip,
                                                           bool default1, bool default2, double widthScale, int *ttl_blocks) {
    Q_UNUSED(btn1Tooltip) // Mobile doesn't have any tooltips
    Q_UNUSED(btn2Tooltip)
    Q_UNUSED(widthScale)
    Q_UNUSED(default1)
    Q_UNUSED(default2)

    if(mainWindow) {
        mainWindow->setProperty("questionTextDlgResponse", -1);
        QVariant retValue;
        QMetaObject::invokeMethod(mainWindow, "openQuestionTextDlg", Q_RETURN_ARG(QVariant, retValue), Q_ARG(QVariant, title), Q_ARG(QVariant, message), Q_ARG(QVariant, btn1), Q_ARG(QVariant, btn2), Q_ARG(QVariant, ""), Q_ARG(QVariant, 0), Q_ARG(QVariant, *ttl_blocks));
        while(mainWindow->property("questionTextDlgResponse") == -1) {
            QCoreApplication::processEvents();
            QThread::usleep(50);
        }
        if (mainWindow->property("questionTextDlgResponse") == 1) {
//            if (ttl_blocks != nullptr) {
//                *ttl_blocks = mainWindow->property("ttl_blocks").toInt();
//            }
            return WndManager::RETURN_CODE::BTN2;
        } else
            return WndManager::RETURN_CODE::BTN1;
    } else {
        if ( QMessageBox::Yes == QMessageBox::question(nullptr, title, message) )
            return WndManager::RETURN_CODE::BTN2;
        else
            return WndManager::RETURN_CODE::BTN1;
    }
}

WndManager::RETURN_CODE MobileWndManager::questionHTMLDlg( QString title, QString message, QString btn1, QString btn2,
                                                           QString btn1Tooltip, QString btn2Tooltip,
                                                           bool default1, bool default2, double widthScale )  {
    Q_UNUSED(btn1Tooltip) // Mobile doesn't have any tooltips
    Q_UNUSED(btn2Tooltip)
    Q_UNUSED(title)
    Q_UNUSED(message)
    Q_UNUSED(widthScale)
    Q_UNUSED(default1)
    Q_UNUSED(default2)
    Q_UNUSED(btn1)
    Q_UNUSED(btn2)

    Q_ASSERT(false); // implement me
    return WndManager::RETURN_CODE::BTN1;
}

// Password accepted as a HASH. EMpty String mean that no password is set.
// After return, passwordHash value will have input raw Password value. So it can be user for wallet
WndManager::RETURN_CODE MobileWndManager::questionTextDlg( QString title, QString message, QString btn1, QString btn2,
                                                           QString btn1Tooltip, QString btn2Tooltip,
                                                           bool default1, bool default2, double widthScale, QString & passwordHash, WndManager::RETURN_CODE blockButton, int *ttl_blocks)  {
    Q_UNUSED(btn1Tooltip) // Mobile doesn't have any tooltips
    Q_UNUSED(btn2Tooltip)
    Q_UNUSED(default1)
    Q_UNUSED(default2)
    Q_UNUSED(widthScale)

    mainWindow->setProperty("questionTextDlgResponse", -1);
    QVariant retValue;
    QMetaObject::invokeMethod(mainWindow, "openQuestionTextDlg", Q_RETURN_ARG(QVariant, retValue), Q_ARG(QVariant, title), Q_ARG(QVariant, message), Q_ARG(QVariant, btn1), Q_ARG(QVariant, btn2), Q_ARG(QVariant, blockButton == WndManager::RETURN_CODE::BTN1 ? 0 : 1), Q_ARG(QVariant, *ttl_blocks), Q_ARG(QVariant, passwordHash));
    while(mainWindow->property("questionTextDlgResponse") == -1) {
        QCoreApplication::processEvents();
        QThread::usleep(50);
    }
    if (mainWindow->property("questionTextDlgResponse") == 1) {
//        if (ttl_blocks != nullptr) {
//            *ttl_blocks = mainWindow->property("ttl_blocks").toInt();
//        }
        return WndManager::RETURN_CODE::BTN2;
    } else
        return WndManager::RETURN_CODE::BTN1;
}

// QFileDialog::getSaveFileName call
QString MobileWndManager::getSaveFileName(const QString &caption, const QString &dir, const QString &filter) {
    Q_UNUSED(caption)
    Q_UNUSED(dir)
    Q_UNUSED(filter)

    QDateTime now;
    QString fileName = "/mnt/user/0/primary/" + now.currentDateTime().toString("MMMM-d-yyyy-hh-mm");
    return fileName;
}

// QFileDialog::getLoadFileName call
// Mobile migth never need that.
QString MobileWndManager::getOpenFileName(const QString &caption, const QString &dir, const QString &filter) {
    Q_ASSERT(false); // implement me
}


// Ask for confirmation
bool MobileWndManager::sendConfirmationDlg( QString title, QString message, double widthScale, QString passwordHash ) {
    Q_UNUSED(widthScale)

    if(mainWindow) {
        mainWindow->setProperty("sendConformationDlgResponse", -1);
        QVariant retValue;
        QMetaObject::invokeMethod(mainWindow, "openSendConfirmationDlg", Q_RETURN_ARG(QVariant, retValue), Q_ARG(QVariant, title), Q_ARG(QVariant, message), Q_ARG(QVariant, passwordHash));
        while(mainWindow->property("sendConformationDlgResponse") == -1) {
            QCoreApplication::processEvents();
            QThread::usleep(50);
        }
        return mainWindow->property("sendConformationDlgResponse") == 1;
    }
    return false;
}

// Stopping wallet message
void MobileWndManager::showWalletStoppingMessage(int taskTimeout) {
    Q_UNUSED(taskTimeout)

    Q_ASSERT(false); // implement me
}

void MobileWndManager::hideWalletStoppingMessage() {
    Q_ASSERT(false); // implement me
}

//---------------- Pages ------------------------
void MobileWndManager::pageInitFirstTime() {
    Q_ASSERT(false); // implement me
}
void MobileWndManager::pageInputPassword(QString pageTitle, bool lockMode) {
    Q_UNUSED(pageTitle)
    Q_UNUSED(lockMode)

    mainWindow->setProperty("currentState", state::STATE::INPUT_PASSWORD);
}
void MobileWndManager::pageInitAccount(QString path, bool restoredFromSeed) {
    Q_UNUSED(path) // mobile doesn't need it
    Q_UNUSED(restoredFromSeed)  // mobile doesn't need it
    Q_ASSERT(false); // implement me
}
void MobileWndManager::pageEnterSeed() {
    QJsonObject obj;
    obj["currentStep"] = 3;
    QVariant retValue;
    QMetaObject::invokeMethod(mainWindow, "updateInitParams", Q_RETURN_ARG(QVariant, retValue), Q_ARG(QVariant, QJsonDocument(obj).toJson(QJsonDocument::Compact)));
}
void MobileWndManager::pageNewSeed(QString pageTitle, QVector<QString> seed, bool hideSubmitButton) {
    Q_UNUSED(pageTitle)

    QJsonObject obj;
    QString strSeed = "";
    for (int i = 0; i < seed.length() ; i++) {
        strSeed += seed.at(i) + " ";
    }
    obj["currentStep"] = 1;
    obj["seed"] = strSeed;
    obj["hideSubmitButton"] = hideSubmitButton;
    QVariant retValue;
    QMetaObject::invokeMethod(mainWindow, "updateInitParams", Q_RETURN_ARG(QVariant, retValue), Q_ARG(QVariant, QJsonDocument(obj).toJson(QJsonDocument::Compact)));
}
void MobileWndManager::pageNewSeedTest(int wordIndex) {
    QJsonObject obj;
    obj["currentStep"] = 2;
    obj["wordIndex"] = wordIndex;
    QVariant retValue;
    QMetaObject::invokeMethod(mainWindow, "updateInitParams", Q_RETURN_ARG(QVariant, retValue), Q_ARG(QVariant, QJsonDocument(obj).toJson(QJsonDocument::Compact)));
}
void MobileWndManager::pageProgressWnd(QString pageTitle, QString callerId, QString header, QString msgProgress, QString msgPlus, bool cancellable ) {
    Q_UNUSED(pageTitle)
    Q_UNUSED(header)
    Q_UNUSED(msgPlus)
    Q_UNUSED(cancellable)

    QJsonObject obj;
    obj["callerId"] = callerId;
    obj["msgProgress"] = msgProgress;
    QVariant retValue;
    QMetaObject::invokeMethod(mainWindow, "updateInitParams", Q_RETURN_ARG(QVariant, retValue), Q_ARG(QVariant, QJsonDocument(obj).toJson(QJsonDocument::Compact)));
}
void MobileWndManager::pageOutputs() {
    Q_ASSERT(false); // implement me
}
void MobileWndManager::pageFileTransactionReceive(QString pageTitle,
                                     const QString & fileName, const util::FileTransactionInfo & transInfo,
                                     int nodeHeight,
                                     QString transactionType, QString processButtonName) {
    Q_UNUSED(pageTitle)

    QJsonObject obj;
    obj["callerId"] = "Receive";
    obj["fileName"] = fileName;
    obj["transactionType"] = "Receive File Slate";
    obj["processButtonName"] = "Generate Response";
    obj["amount"] = util::nano2one(transInfo.amount);
    obj["transactionId"] = transInfo.transactionId;
    obj["lockHeight"] = transInfo.lock_height > nodeHeight ? util::longLong2Str(transInfo.lock_height) : "-";
    obj["receiverAddress"] = transInfo.fromAddress.isEmpty() ? "-" : transInfo.fromAddress;
    obj["message"] = transInfo.message;
    obj["isFinalize"] = false;

    QVariant retValue;
    QMetaObject::invokeMethod(mainWindow, "updateInitParams", Q_RETURN_ARG(QVariant, retValue), Q_ARG(QVariant, QJsonDocument(obj).toJson(QJsonDocument::Compact)));
}

void MobileWndManager::pageFileTransactionFinalize(QString pageTitle,
                                           const QString & fileName, const util::FileTransactionInfo & transInfo,
                                           int nodeHeight,
                                           QString transactionType, QString processButtonName) {
    Q_UNUSED(pageTitle)

    QJsonObject obj;
    obj["callerId"] = "Finalize";
    obj["fileName"] = fileName;
    obj["transactionType"] = "Finalize Transaction";
    obj["processButtonName"] = "Finalize";
    obj["amount"] = util::nano2one(transInfo.amount);
    obj["transactionId"] = transInfo.transactionId;
    obj["lockHeight"] = transInfo.lock_height > nodeHeight ? util::longLong2Str(transInfo.lock_height) : "-";
    obj["receiverAddress"] = transInfo.fromAddress.isEmpty() ? "-" : transInfo.fromAddress;
    obj["message"] = transInfo.message;
    obj["isFinalize"] = true;

    QVariant retValue;
    QMetaObject::invokeMethod(mainWindow, "updateInitParams", Q_RETURN_ARG(QVariant, retValue), Q_ARG(QVariant, QJsonDocument(obj).toJson(QJsonDocument::Compact)));
}

void MobileWndManager::pageRecieve() {
    mainWindow->setProperty("currentState", state::STATE::RECEIVE_COINS);
    mainWindow->setProperty("initParams", "");
}
void MobileWndManager::pageListening() {
    Q_ASSERT(false); // implement me
}
void MobileWndManager::pageFinalize() {
    mainWindow->setProperty("currentState", state::STATE::FINALIZE);
    mainWindow->setProperty("initParams", "");
}
void MobileWndManager::pageSendStarting() {
    mainWindow->setProperty("currentState", state::STATE::SEND);
    mainWindow->setProperty("initParams", "");
}
void MobileWndManager::pageSendOnline( QString selectedAccount, int64_t amount ) {
    QJsonObject obj;
    obj["isSendOnline"] = true;
    obj["selectedAccount"] = selectedAccount;
    obj["amount"] = QString::number(amount);
    mainWindow->setProperty("initParams", QJsonDocument(obj).toJson(QJsonDocument::Compact));
}
void MobileWndManager::pageSendFile( QString selectedAccount, int64_t amount ) {
    QJsonObject obj;
    obj["isSendOnline"] = false;
    obj["selectedAccount"] = selectedAccount;
    obj["amount"] = QString::number(amount);
    mainWindow->setProperty("initParams", QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void MobileWndManager::pageSendSlatepack( QString selectedAccount, int64_t amount ) {
    Q_ASSERT(false); // implement me
}

void MobileWndManager::pageTransactions() {
    mainWindow->setProperty("currentState", state::STATE::TRANSACTIONS);
}

// slatepack - slatepack string value to show.
// backStateId - state ID of the caller. On 'back' will switch to this state Id
void MobileWndManager::pageShowSlatepack(QString slatepack, int backStateId, QString txExtension, bool enableFinalize) {
    Q_UNUSED(slatepack)
    Q_UNUSED(backStateId)
    Q_UNUSED(txExtension)
    Q_UNUSED(enableFinalize)
    // This page should show the slatepack text so user can copy paste it. Also it will be noce to have a QR code
    // windows. It is usefull for some cases, Jon had some examples. Also other wallets have it.
    Q_ASSERT(false); // implement me
}


void MobileWndManager::pageAccounts() {
    Q_ASSERT(false); // implement me
}
void MobileWndManager::pageAccountTransfer() {
    Q_ASSERT(false); // implement me
}
void MobileWndManager::pageNodeInfo() {
    Q_ASSERT(false); // implement me
}
void MobileWndManager::pageContacts() {
    Q_ASSERT(false); // implement me
}
void MobileWndManager::pageEvents() {
    Q_ASSERT(false); // implement me
}
void MobileWndManager::pageWalletConfig() {
    Q_ASSERT(false); // implement me
}
void MobileWndManager::pageNodeConfig() {
    Q_ASSERT(false); // implement me
}
void MobileWndManager::pageSelectMode() {
    Q_ASSERT(false); // implement me
}

void MobileWndManager::pageWalletHome() {
    mainWindow->setProperty("currentState", state::STATE::WALLET_HOME);
}

void MobileWndManager::pageWalletSettings() {
    mainWindow->setProperty("currentState", state::STATE::WALLET_SETTINGS);
}

void MobileWndManager::pageAccountOptions() {
    mainWindow->setProperty("currentState", state::STATE::ACCOUNT_OPTIONS);
}

void MobileWndManager::pageSwapList() {
    Q_ASSERT(false); // implement me
}

void MobileWndManager::pageSwapNew1() {
    Q_ASSERT(false); // implement me
}

void MobileWndManager::pageSwapNew2() {
    Q_ASSERT(false); // implement me
}

void MobileWndManager::pageSwapNew3() {
    Q_ASSERT(false); // implement me
}

void MobileWndManager::pageSwapEdit(QString swapId, QString stateCmd) {
    Q_UNUSED(swapId)
    Q_UNUSED(stateCmd)
    Q_ASSERT(false); // implement me
}
void MobileWndManager::pageSwapTradeDetails(QString swapId) {
    Q_UNUSED(swapId)
    Q_ASSERT(false); // implement me
}

void MobileWndManager::showBackupDlg(QString swapId, int backupId) {
    Q_UNUSED(swapId)
    Q_UNUSED(backupId)
    Q_ASSERT(false); // implement me
}


}
