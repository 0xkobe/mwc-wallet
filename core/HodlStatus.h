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

#ifndef MWC_QT_WALLET_STATUS_H
#define MWC_QT_WALLET_STATUS_H

#include <QObject>
#include <QMap>
#include "../wallet/wallet.h"
#include <QSet>

namespace state {
struct StateContext;
}

namespace core {

// currently submitted HODL outputs as a server see them
struct HodlOutputInfo {
    QString    outputCommitment;
    int64_t    valueNano = 0L;
    int        height = 0;

    void setData( const QString & outputCommitment, int64_t valueNano, int height );

    static HodlOutputInfo create(const QString & outputCommitment, int64_t valueNano, int height) {
        HodlOutputInfo item;
        item.setData(outputCommitment, valueNano, height);
        return item;
    }
};

struct HodlClaimStatus {
    int64_t HodlAmount;
    int64_t claimedMwc;
    QString status;
    QString date;

    void setData( int64_t HodlAmount, int64_t claimedMwc, const QString & status, const QString & date );
};

// the status of all hodl process.
// The status is updated from several sources. That is why because of
// race conditions ti can be changed in any order and combination
class HodlStatus : public QObject {
    Q_OBJECT
public:
    HodlStatus( state::StateContext * context );

    void setHodlStatus( const QString & hodlStatus, const QString & errKey );
    void setHodlOutputs( const QVector<HodlOutputInfo> & hodlOutputs, const QString & errKey ); //
    void setWalletOutputs( const QString & account, const QVector<wallet::WalletOutput> & outputs, const QString & errKey ); //
    void setClaimAmount( int64_t amount2claim, const QString & errKey );

    void setError( const QString & errKey, const QString & error );

    QString getRootPubKey() const {return rootPubKey;}
    void setRootPubKey( const QString & pubKey );

    // Hex representation of HSA256 hash from rootpublickey binary representation
    QString getRootPubKeyHash() const {return rootPubKeyHash;}

    bool hasHodlStatus() const;
    bool hasHodlOutputs() const;
    bool hasAmountToClaim() const;


    bool hasErrors() const;
    QString getErrorsAsString() const; //

    int64_t getAmountToClaim() const { return amount2claim; }

    QString getHodlStatus() const {return hodlStatus;}
    QString getAccountStatus() const {return accountStatus;}

    QVector<HodlClaimStatus> getClaimsRequestStatus() const;


    bool hasAnyOutputsInHODL() const { return !hodlOutputs.isEmpty();}
    QVector<wallet::WalletOutput> getWalltOutputsForAccount(QString accountName) const {return walletOutputs.value(accountName);}
    bool isOutputInHODL(const QString & output) const {return hodlOutputCommitment.contains(output);}

private slots:
    void onLoginResult(bool ok);
    void onLogout();

private:
signals:
    void onHodlStatusWasChanged();

private:
    // Logout repond
    void resetData();
private:
    state::StateContext * context;

    QString rootPubKey;
    QString rootPubKeyHash; // HSA256 hash

    QString hodlStatus; // Replay from /v1/getNextStartDate
    QString accountStatus; // Replay from checkAddresses

    uint availableData = 0;

    QMap<QString, QVector<wallet::WalletOutput> > walletOutputs; // Available outputs from the wallet. Key: account name, value outputs for this account
    QVector<HodlOutputInfo>    hodlOutputs;
    QSet<QString>              hodlOutputCommitment;

    int64_t amount2claim = 0;

    //
    QMap<QString, QString> requestErrors;

};

}

#endif //MWC_QT_WALLET_STATUS_H
