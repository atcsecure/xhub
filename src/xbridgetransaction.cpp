//*****************************************************************************
//*****************************************************************************

#include "xbridgetransaction.h"
#include "util/logger.h"
#include "util/util.h"


//*****************************************************************************
//*****************************************************************************
XBridgeTransaction::XBridgeTransaction()
    : m_state(trInvalid)
    , m_stateCounter(0)
{

}

//*****************************************************************************
//*****************************************************************************
XBridgeTransaction::XBridgeTransaction(const uint256 & id,
                                       const std::vector<unsigned char> & sourceAddr,
                                       const std::string & sourceCurrency,
                                       const boost::uint64_t sourceAmount,
                                       const std::vector<unsigned char> & destAddr,
                                       const std::string & destCurrency,
                                       const boost::uint64_t destAmount)
    : m_id(id)
    , m_state(trNew)
    , m_stateCounter(0)
    , m_sourceCurrency(sourceCurrency)
    , m_destCurrency(destCurrency)
    , m_sourceAmount(sourceAmount)
    , m_destAmount(destAmount)
    , m_first(id)
{
    m_first.setSource(sourceAddr);
    m_first.setDest(destAddr);
}

//*****************************************************************************
//*****************************************************************************
XBridgeTransaction::~XBridgeTransaction()
{
}

//*****************************************************************************
//*****************************************************************************
uint256 XBridgeTransaction::id() const
{
    return m_id;
}

//*****************************************************************************
// state of transaction
//*****************************************************************************
XBridgeTransaction::State XBridgeTransaction::state() const
{
    return m_state;
}

//*****************************************************************************
//*****************************************************************************
XBridgeTransaction::State XBridgeTransaction::increaseStateCounter(XBridgeTransaction::State state)
{
    if (state == trJoined && m_state == state)
    {
        if (++m_stateCounter >= 2)
        {
            m_state = trHold;
            m_stateCounter = 0;
        }
        return m_state;
    }
    else if (state == trHold && m_state == state)
    {
        if (++m_stateCounter >= 2)
        {
            m_state = trPaid;
            m_stateCounter = 0;
        }
        return m_state;
    }
    else if (state == trPaid && m_state == state)
    {
        if (++m_stateCounter >= 2)
        {
            m_state = trFinished;
            m_stateCounter = 0;
        }
        return m_state;
    }


    return trInvalid;
}

//*****************************************************************************
//*****************************************************************************
bool XBridgeTransaction::isValid() const
{
    // TODO implementation
    return m_state != trInvalid;
}

//*****************************************************************************
//*****************************************************************************
bool XBridgeTransaction::isExpired() const
{
    // TODO implementation
    return false;
}

//*****************************************************************************
//*****************************************************************************
void XBridgeTransaction::drop()
{
    LOG() << "drop transaction <"
          << util::base64_encode(std::string((char *)(m_id.begin()), 32))
          << ">";
    m_state = trDropped;
}

//*****************************************************************************
//*****************************************************************************
uint256 XBridgeTransaction::hash1() const
{
    return util::hash(m_sourceCurrency.begin(), m_sourceCurrency.end(),
                      BEGIN(m_sourceAmount), END(m_sourceAmount),
                      m_destCurrency.begin(), m_destCurrency.end(),
                      BEGIN(m_destAmount), END(m_destAmount));
}

//*****************************************************************************
//*****************************************************************************
uint256 XBridgeTransaction::hash2() const
{
    return util::hash(m_destCurrency.begin(), m_destCurrency.end(),
                      BEGIN(m_destAmount), END(m_destAmount),
                      m_sourceCurrency.begin(), m_sourceCurrency.end(),
                      BEGIN(m_sourceAmount), END(m_sourceAmount));
}

//*****************************************************************************
//*****************************************************************************
uint256 XBridgeTransaction::firstId() const
{
    return m_first.id();
}

//*****************************************************************************
//*****************************************************************************
std::vector<unsigned char> XBridgeTransaction::firstAddress() const
{
    return m_first.source();
}

//*****************************************************************************
//*****************************************************************************
std::vector<unsigned char> XBridgeTransaction::firstDestination() const
{
    return m_first.dest();
}

//*****************************************************************************
//*****************************************************************************
std::string XBridgeTransaction::firstCurrency() const
{
    return m_sourceCurrency;
}

//*****************************************************************************
//*****************************************************************************
boost::uint64_t XBridgeTransaction::firstAmount() const
{
    return m_sourceAmount;
}

//*****************************************************************************
//*****************************************************************************
uint256 XBridgeTransaction::secondId() const
{
    return m_second.id();
}

//*****************************************************************************
//*****************************************************************************
std::vector<unsigned char> XBridgeTransaction::secondAddress() const
{
    return m_second.source();
}

//*****************************************************************************
//*****************************************************************************
std::vector<unsigned char> XBridgeTransaction::secondDestination() const
{
    return m_second.dest();
}

//*****************************************************************************
//*****************************************************************************
std::string XBridgeTransaction::secondCurrency() const
{
    return m_destCurrency;
}

//*****************************************************************************
//*****************************************************************************
boost::uint64_t XBridgeTransaction::secondAmount() const
{
    return m_destAmount;
}

//*****************************************************************************
//*****************************************************************************
bool XBridgeTransaction::tryJoin(const XBridgeTransactionPtr other)
{
    DEBUG_TRACE();

    if (m_state != trNew || other->state() != trNew)
    {
        // can be joined only new transactions
        return false;
    }

    if (m_sourceCurrency != other->m_destCurrency ||
        m_destCurrency != other->m_sourceCurrency)
    {
        // not same currencies
        ERR() << "not same currencies. transaction not joined" << __FUNCTION__;
        // assert(false || "not same currencies. transaction not joined");
        return false;
    }

    if (m_sourceAmount != other->m_destAmount ||
        m_destAmount != other->m_sourceAmount)
    {
        // not same currencies
        ERR() << "not same amount. transaction not joined" << __FUNCTION__;
        // assert(false || "not same amount. transaction not joined");
        return false;
    }

    // join second member
    m_second = other->m_first;
//    m_second.setSource(other->m_first.source());
//    m_second.setDest(other->m_first.dest());

    // generate new id
    m_id = util::hash(BEGIN(m_id), END(m_id),
                      BEGIN(other->m_id), END(other->m_id));

    m_state = trJoined;

    return true;
}
