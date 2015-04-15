//*****************************************************************************
//*****************************************************************************

#include "xbridgeexchange.h"
#include "util/logger.h"
#include "util/settings.h"
#include "util/util.h"

#include <algorithm>

//*****************************************************************************
//*****************************************************************************
XBridgeExchange::XBridgeExchange()
{
}

//*****************************************************************************
//*****************************************************************************
XBridgeExchange::~XBridgeExchange()
{
}

//*****************************************************************************
//*****************************************************************************
// static
XBridgeExchange & XBridgeExchange::instance()
{
    static XBridgeExchange e;
    return e;
}

//*****************************************************************************
//*****************************************************************************
bool XBridgeExchange::init()
{
    Settings & s = Settings::instance();

    std::vector<std::string> wallets = s.exchangeWallets();
    for (std::vector<std::string>::iterator i = wallets.begin(); i != wallets.end(); ++i)
    {
        std::string label   = s.get<std::string>(*i + ".Title");
        std::string address = s.get<std::string>(*i + ".Address");

        if (address.empty())
        {
            LOG() << "read wallet " << *i << " with empty address>";
            continue;
        }

        std::string decoded = util::base64_decode(address);
        if (address.empty())
        {
            LOG() << "incorrect wallet address for " << *i;
            continue;
        }

        std::copy(decoded.begin(), decoded.end(), std::back_inserter(m_wallets[*i].address));
        if (m_wallets[*i].address.size() != 20)
        {
            LOG() << "incorrect wallet address size for " << *i;
            m_wallets.erase(*i);
            continue;
        }

        m_wallets[*i].title   = label;

        LOG() << "read wallet " << *i << " \"" << label << "\" address <" << address << ">";
    }

    if (isEnabled())
    {
        LOG() << "exchange enabled";
    }

    return true;
}

//*****************************************************************************
//*****************************************************************************
bool XBridgeExchange::isEnabled()
{
    return m_wallets.size() > 0;
}

//*****************************************************************************
//*****************************************************************************
bool XBridgeExchange::haveConnectedWallet(const std::string & walletName)
{
    return m_wallets.count(walletName);
}

//*****************************************************************************
//*****************************************************************************
std::vector<unsigned char> XBridgeExchange::walletAddress(const std::string & walletName)
{
    if (!m_wallets.count(walletName))
    {
        ERR() << "reqyest address for unknown wallet <" << walletName
              << ">" << __FUNCTION__;
        return std::vector<unsigned char>();
    }

    return m_wallets[walletName].address;
}

//*****************************************************************************
//*****************************************************************************
bool XBridgeExchange::createTransaction(const uint256 & id,
                                        const std::vector<unsigned char> & sourceAddr,
                                        const std::string & sourceCurrency,
                                        const boost::uint64_t sourceAmount,
                                        const std::vector<unsigned char> & destAddr,
                                        const std::string & destCurrency,
                                        const boost::uint64_t destAmount,
                                        uint256 & transactionId)
{
    DEBUG_TRACE();

    // transactionId = id;

    XBridgeTransactionPtr tr(new XBridgeTransaction(id,
                                                    sourceAddr, sourceCurrency,
                                                    sourceAmount,
                                                    destAddr, destCurrency,
                                                    destAmount));
    if (!tr->isValid())
    {
        return false;
    }

    uint256 h = tr->hash2();

    XBridgeTransactionPtr tmp;

    {
        boost::mutex::scoped_lock l(m_pendingTransactionsLock);

        if (!m_pendingTransactions.count(h))
        {
            // new transaction or update existing (update timestamp)
            h = tr->hash1();
            m_pendingTransactions[h] = tr;
        }
        else
        {
            // found, check if expired
            if (m_pendingTransactions[h]->isExpired())
            {
                // if expired - delete old transaction
                m_pendingTransactions.erase(h);

                // create new
                h = tr->hash1();
                m_pendingTransactions[h] = tr;
            }
            else
            {
                // try join with existing transaction
                if (!m_pendingTransactions[h]->tryJoin(tr))
                {
                    LOG() << "transaction not joined";
                    // return false;

                    // create new transaction
                    h = tr->hash1();
                    m_pendingTransactions[h] = tr;
                }
                else
                {
                    LOG() << "transactions joined, new id "
                          << util::base64_encode(std::string((char *)(tr->id().begin()), 32));

                    tmp = m_pendingTransactions[h];
                }
            }
        }
    }

    if (tmp)
    {
        // new transaction id
        transactionId = tmp->id();

        // move to transactions
        {
            boost::mutex::scoped_lock l(m_transactionsLock);
            m_transactions[tmp->id()] = tmp;
        }
        {
            boost::mutex::scoped_lock l(m_pendingTransactionsLock);
            m_pendingTransactions.erase(h);
        }

    }

    return true;
}

//*****************************************************************************
//*****************************************************************************
bool XBridgeExchange::updateTransactionWhenHoldApplyReceived(const uint256 & id)
{
    boost::mutex::scoped_lock l(m_transactionsLock);
    if (!m_transactions.count(id))
    {
        // unknown transaction
        LOG() << "unknown transaction, id <"
              << util::base64_encode(std::string((char *)(id.begin()), 32))
              << ">";
        return false;
    }

    if (m_transactions[id]->increaseStateCounter(XBridgeTransaction::trJoined) == XBridgeTransaction::trHold)
    {
        return true;
    }

    return false;
}

//*****************************************************************************
// TODO store payments id
//*****************************************************************************
bool XBridgeExchange::updateTransactionWhenPayApplyReceived(const uint256 & id,
                                                            const uint256 & /*paymentId*/)
{
    boost::mutex::scoped_lock l(m_transactionsLock);
    if (!m_transactions.count(id))
    {
        // unknown transaction
        LOG() << "unknown transaction, id <"
              << util::base64_encode(std::string((char *)(id.begin()), 32))
              << ">";
        return false;
    }

    // TODO process paymentId

    // update transaction state
    if (m_transactions[id]->increaseStateCounter(XBridgeTransaction::trHold) == XBridgeTransaction::trPaid)
    {
        return true;
    }

    return false;
}

//*****************************************************************************
//*****************************************************************************
bool XBridgeExchange::updateTransactionWhenCommitApplyReceived(const uint256 & id)
{
    boost::mutex::scoped_lock l(m_transactionsLock);
    if (!m_transactions.count(id))
    {
        // unknown transaction
        LOG() << "unknown transaction, id <"
              << util::base64_encode(std::string((char *)(id.begin()), 32))
              << ">";
        return false;
    }

    // update transaction state
    if (m_transactions[id]->increaseStateCounter(XBridgeTransaction::trPaid) == XBridgeTransaction::trFinished)
    {
        return true;
    }

    return false;
}

//*****************************************************************************
//*****************************************************************************
bool XBridgeExchange::updateTransaction(const uint256 & hash)
{
    // DEBUG_TRACE();
    m_walletTransactions.insert(hash);
    return true;
}

//*****************************************************************************
//*****************************************************************************
bool XBridgeExchange::cancelTransaction(const uint256 & /*hash*/)
{
    DEBUG_TRACE();
    return true;
}

//*****************************************************************************
//*****************************************************************************
const XBridgeTransactionPtr XBridgeExchange::transaction(const uint256 & hash)
{
    {
        boost::mutex::scoped_lock l(m_transactionsLock);

        if (m_transactions.count(hash))
        {
            return m_transactions[hash];
        }
    }

    // TODO not search in pending transactions
//    {
//        boost::mutex::scoped_lock l(m_pendingTransactionsLock);

//        if (m_pendingTransactions.count(hash))
//        {
//            return m_pendingTransactions[hash];
//        }
//    }

    // return XBridgeTransaction::trInvalid;
    return XBridgeTransactionPtr(new XBridgeTransaction);
}

//*****************************************************************************
//*****************************************************************************
std::vector<StringPair> XBridgeExchange::listOfWallets() const
{
    std::vector<StringPair> result;
    for (WalletList::const_iterator i = m_wallets.begin(); i != m_wallets.end(); ++i)
    {
        result.push_back(std::make_pair(i->first, i->second.title));
    }
    return result;
}
