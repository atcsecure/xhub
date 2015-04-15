//*****************************************************************************
//*****************************************************************************

#ifndef XBRIDGEEXCHANGE_H
#define XBRIDGEEXCHANGE_H

#include "util/uint256.h"
#include "xbridgetransaction.h"

#include <string>
#include <set>
#include <map>

#include <boost/cstdint.hpp>
#include <boost/thread/mutex.hpp>

//*****************************************************************************
//*****************************************************************************
typedef std::pair<std::string, std::string> StringPair;

//*****************************************************************************
//*****************************************************************************
struct WalletParam
{
    std::string                title;
    std::vector<unsigned char> address;
};

//*****************************************************************************
//*****************************************************************************
class XBridgeExchange
{
public:
    static XBridgeExchange & instance();

protected:
    XBridgeExchange();
    ~XBridgeExchange();

public:
    bool init();

    bool isEnabled();
    bool haveConnectedWallet(const std::string & walletName);

    std::vector<unsigned char> walletAddress(const std::string & walletName);

    bool createTransaction(const uint256 & id,
                           const std::vector<unsigned char> & sourceAddr,
                           const std::string & sourceCurrency,
                           const boost::uint64_t sourceAmount,
                           const std::vector<unsigned char> & destAddr,
                           const std::string & destCurrency,
                           const boost::uint64_t destAmount,
                           uint256 & transactionId);

    bool updateTransactionWhenHoldApplyReceived(const uint256 & id);
    bool updateTransactionWhenPayApplyReceived(const uint256 & id, const uint256 & paymentId);
    bool updateTransactionWhenCommitApplyReceived(const uint256 & id);

    bool updateTransaction(const uint256 & hash);
    bool cancelTransaction(const uint256 & hash);

    const XBridgeTransactionPtr transaction(const uint256 & hash);

    std::vector<StringPair> listOfWallets() const;

private:
    // connected wallets
    typedef std::map<std::string, WalletParam> WalletList;
    WalletList                               m_wallets;

    boost::mutex                             m_pendingTransactionsLock;
    std::map<uint256, XBridgeTransactionPtr> m_pendingTransactions;

    boost::mutex                             m_transactionsLock;
    std::map<uint256, XBridgeTransactionPtr> m_transactions;

    std::set<uint256>                        m_walletTransactions;
};

#endif // XBRIDGEEXCHANGE_H
