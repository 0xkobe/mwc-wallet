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

#ifndef APPCONTEXT_H
#define APPCONTEXT_H

#include <QMap>
#include "../state/state.h"
#include "../state/m_airdrop.h"
#include "../wallet/wallet.h"
#include "../core/HodlStatus.h"
#include "../core/walletnotes.h"
#include <QDebug>

class QAction;

namespace core {

struct SendCoinsParams {
    int inputConfirmationNumber;
    int changeOutputs;

    // Expected to be default values
    SendCoinsParams() :
            inputConfirmationNumber(10), changeOutputs(1) {}

    SendCoinsParams( int _inputConfirmationNumber, int _changeOutputs) :
            inputConfirmationNumber(_inputConfirmationNumber), changeOutputs(_changeOutputs) {}

    void setData(int _inputConfirmationNumber, int _changeOutputs) {
        inputConfirmationNumber = _inputConfirmationNumber;
        changeOutputs = _changeOutputs;
    }

    SendCoinsParams(const SendCoinsParams & o) = default;
    SendCoinsParams & operator = (const SendCoinsParams & o) = default;

    bool operator == (const SendCoinsParams & o ) {return inputConfirmationNumber==o.inputConfirmationNumber && changeOutputs==o.changeOutputs;}

    void saveData(QDataStream & out) const;
    bool loadData(QDataStream & in);
};

struct ContactRecord {
    QString name;
    QString address;

    bool operator ==(const ContactRecord & o) {return name == o.name && address == o.address;}

    void setData(QString name,
                 QString address);

    void saveData( QDataStream & out) const;
    bool loadData( QDataStream & in);
};

// State that applicable to all application.
// Support signal for changes about Locked output
class AppContext : public QObject
{
    Q_OBJECT
public:
    AppContext();
    ~AppContext();

    // add new key/value
    template <class T>
    void pushCookie(QString key, T value);

    // remove and return
    template <class T>
    T pullCookie(QString key);

    // get is exist, don't clean up
    template <class T>
    T getCookie(QString key);


    void initGuiScale(const double _initScaleValue) { initScaleValue = _initScaleValue; }
    double getInitGuiScale() const {return initScaleValue;}

    double getGuiScale() const;
    void setGuiScale(double scale) { guiScale = scale; }


    state::STATE getActiveWndState() const {return activeWndState;}
    void setActiveWndState(state::STATE  state) {activeWndState=state;}

    // Send coins params.
    SendCoinsParams getSendCoinsParams() const {return sendCoinsParams;}
    void setSendCoinsParams(SendCoinsParams params) { sendCoinsParams=params; }

    // Get last path state. Default: Home dir
    QString getPathFor( QString name ) const;
    // update path state
    void updatePathFor( QString name, QString path );

    // IO for Int vectors
    QVector<int> getIntVectorFor( QString name ) const;
    void updateIntVectorFor( QString name, const QVector<int> & data );

    QString getReceiveAccount(const QString & walletDataDir) const { return receiveAccount.value(walletDataDir, "default");}
    void setReceiveAccount(const QString & walletDataDir, const QString & account) {receiveAccount.insert(walletDataDir, account);}

    QString getCurrentAccountName(const QString & walletDataDir) const {return currentAccountName.value(walletDataDir, "default");};
    void setCurrentAccountName(const QString & walletDataDir, const QString & currentAccount) {currentAccountName.insert(walletDataDir, currentAccount);}

    // AirdropRequests will handle differently
    void saveAirdropRequests( const QVector<state::AirdropRequests> & data );
    QVector<state::AirdropRequests> loadAirdropRequests() const;

    // -------------- Contacts
    // Get the contacts
    QVector<ContactRecord> getContacts() const {return contactList;}
    // Add s new contact
    QPair<bool, QString> addContact( const ContactRecord & contact );
    // Remove contact. return false if not found
    QPair<bool, QString> deleteContact( const ContactRecord & contact );
    // Update contact
    QPair<bool, QString> updateContact( const ContactRecord & prevValue, const ContactRecord & newValue );

    // ----- Logs --------
    bool isLogsEnabled() const {return logsEnabled;}
    void setLogsEnabled(bool enabled);

    // ----- Auto Start MQS --------
    bool isAutoStartMQSEnabled() const {return autoStartMQSEnabled;}
    void setAutoStartMQSEnabled(bool enabled);

    // ----- Auto Start Keybase --------
    bool isAutoStartKeybaseEnabled() const {return autoStartKeybaseEnabled;}
    void setAutoStartKeybaseEnabled(bool enabled);

    // ----- Outputs: All/Unspent
    bool isShowOutputAll() const {return showOutputAll;}
    void setShowOutputAll(bool all);

    wallet::MwcNodeConnection getNodeConnection(const QString network) const;
    void updateMwcNodeConnection(const QString network, const wallet::MwcNodeConnection & connection );

    // return false if value not set
    bool getWallet713DataPathWithNetwork( QString & wallet713DataPath, QString & network);
    void setWallet713DataPathWithNetwork( const QString & wallet713DataPath, const QString & network );

    // HODL registration time.
    int64_t getHodlRegistrationTime(const QString & hash) const;
    void    setHodlRegistrationTime(const QString & hash, int64_t time);

    // HODL outputs data
    void saveHodlOutputs( const QString & rootPubKeyHash, const QMap<QString, core::HodlOutputInfo> & hodlOutputs );
    QMap<QString, core::HodlOutputInfo> loadHodlOutputs(const QString & rootPubKeyHash );

    // setWalletNotes called in main.cpp after StateContext is created
    void setWalletNotes(core::WalletNotes* _walletNotes);
    // these methods are called by WalletNotes whenever the notes have been updated so the maps will always contain the latest notes
    void setOutputNotesMap(QMap<QString, QMap<QString, QMap<QString, QString>>> _outputNotesMap) { outputNotesMap = _outputNotesMap; }
    void setTxnNotesMap(QMap<QString, QMap<QString, QMap<QString, QString>>> _txnNotesMap) { txnNotesMap = _txnNotesMap; }

    // output note
    void initOutputNotes(const QString & account, const QVector<wallet::WalletOutput> & outputs);
    QString getNote(const QString& account, const QString& outputCommitment);
    void updateNote(const QString& account, const QString& outputCommitment, const QString& newNote);
    void deleteNote(const QString& account, const QString& outputCommitment);

    // transaction note uses transaction txIdx as note id
    void initTransactionNotes(const QString & account, const QVector<wallet::WalletTransaction> & transactions);
    QString getNote(const QString& account, int64_t txIdx);
    void updateNote(const QString& account, int64_t txIdx, const QString& newNote) const;
    void deleteNote(const QString& account, int64_t txIdx) const;

    // Outputs can be locked from spending.
    bool isLockOutputEnabled() const {return lockOutputEnabled;}
    bool isLockedOutputs(const QString & output) const;
    void setLockOutputEnabled(bool enabled);
    void setLockedOutput(const QString & output, bool lock);

    // Fluff transactions
    bool isFluffSet() const { return fluffTransactions; }
    void setFluff(bool fluffSetting);

private:
signals:
    void onOutputLockChanged(QString commit);

private:
    bool loadData();
    void saveData() const;

private:
    // 16 bit hash from the password. Can be used for the password verification
    // Don't use many bits because we don't want it be much usable for attacks.
//    int passHash = -1;

    QMap<QString,QString> receiveAccount; // Key: mwc713 data dir; Value: Selected account
    QMap<QString,QString> currentAccountName; // Key: mwc713 data dir; Value: Current account

    // Active window that is visible
    state::STATE activeWndState = state::STATE::LISTENING;

    // Send coins params.
    SendCoinsParams sendCoinsParams;

    // Current Path dirs
    QMap<QString,QString> pathStates;
    QMap<QString,QVector<int> > intVectorStates;

    double guiScale = -1.0;
    double initScaleValue = 1.0; // default scale value

    bool logsEnabled = true;

    bool autoStartMQSEnabled = true;
    bool autoStartKeybaseEnabled = true;

    // Because of Cursom node logic, we have to track config changes
    wallet::MwcNodeConnection  nodeConnectionMainNet;
    wallet::MwcNodeConnection  nodeConnectionFlooNet;

    // Contact list
    QVector<ContactRecord> contactList;

    // wallet 713 path need to be duplicated because of the running mode.
    QString wallet713DataPath;
    QString network;

    bool showOutputAll = false; // Show all or Unspent outputs

    QMap<QString, qulonglong> hodlRegistrations;

    WalletNotes* walletNotes;
    // The app context is created early on in main.cpp and tries to load the app data
    // but the wallet notes object cannot be created until the StateContext has
    // been created later in main.cpp. So we store the output and transaction notes
    // until the wallet notes object gets created at which time they are loaded into
    // the wallet notes object. Then each time the wallet notes are updated, they
    // are loaded back into these app context note maps so we don't have to
    // worry about the wallet notes object getting destroyed before the app context.
    QMap<QString, QMap<QString, QMap<QString, QString>>> outputNotesMap;
    QMap<QString, QMap<QString, QMap<QString, QString>>> txnNotesMap;

    // Outputs can be locked from spending.
    bool lockOutputEnabled = false; // By default it is false
    QSet<QString> lockedOutputs; // Outputs that was locked (it is manual operation)

    // Allow users to by-pass the stem-phase of the dandelion protocol
    // and directly fluff their transactions.
    bool fluffTransactions = false;     // default to normal dandelion protocol
};

template <class T>
QMap<QString, T> & getCookieMap() {
    static QMap<QString, T> cookieMap;
    return cookieMap;
}

// add new key/value
template <class T>
void AppContext::pushCookie(QString key, T value) {
    getCookieMap<T>()[key] = value;
}

// remove and return
template <class T>
T AppContext::pullCookie(QString key) {
    return getCookieMap<T>().take(key);
}

// get is exist, don't clean up
template <class T>
T AppContext::getCookie(QString key) {
    return getCookieMap<T>().value(key);
}



}

#endif // APPCONTEXT_H
