//*****************************************************************************
//*****************************************************************************

#ifndef XBRIDGESESSION_H
#define XBRIDGESESSION_H

#include "xbridge.h"
#include "xbridgepacket.h"
#include "FastDelegate.h"

#include <memory>
#include <boost/noncopyable.hpp>

//*****************************************************************************
//*****************************************************************************
class XBridgeSession
        : public std::enable_shared_from_this<XBridgeSession>
        , private boost::noncopyable
{
public:
    XBridgeSession();

    void start(XBridge::SocketPtr socket);

    bool sendXBridgeMessage(const std::vector<unsigned char> & message);

    bool processPacket(XBridgePacketPtr packet);

private:
    void disconnect();

    void doReadHeader(XBridgePacketPtr packet,
                      const std::size_t offset = 0);
    void onReadHeader(XBridgePacketPtr packet,
                      const std::size_t offset,
                      const boost::system::error_code & error,
                      std::size_t transferred);

    void doReadBody(XBridgePacketPtr packet,
                    const std::size_t offset = 0);
    void onReadBody(XBridgePacketPtr packet,
                    const std::size_t offset,
                    const boost::system::error_code & error,
                    std::size_t transferred);

    bool encryptPacket(XBridgePacketPtr packet);
    bool decryptPacket(XBridgePacketPtr packet);

    bool processInvalid(XBridgePacketPtr packet);
    bool processAnnounceAddresses(XBridgePacketPtr packet);
    bool processXBridgeMessage(XBridgePacketPtr packet);
    bool processXBridgeBroadcastMessage(XBridgePacketPtr packet);

    bool processTransaction(XBridgePacketPtr packet);
    bool processTransactionHoldApply(XBridgePacketPtr packet);
    bool processTransactionPayApply(XBridgePacketPtr packet);
    bool processTransactionCommitApply(XBridgePacketPtr packet);
    bool processTransactionCancel(XBridgePacketPtr packet);

    bool processBitcoinTransactionHash(XBridgePacketPtr packet);

private:
    XBridge::SocketPtr m_socket;

    typedef std::map<const int, fastdelegate::FastDelegate1<XBridgePacketPtr, bool> > PacketProcessorsMap;
    PacketProcessorsMap m_processors;
};

typedef std::shared_ptr<XBridgeSession> XBridgeSessionPtr;

#endif // XBRIDGESESSION_H
