//*****************************************************************************
//*****************************************************************************

#ifndef XBRIDGEAPP_H
#define XBRIDGEAPP_H

#include "xbridge.h"
#include "xbridgesession.h"
#include "util/uint256.h"

#include <QApplication>

#include <thread>
#include <atomic>
#include <vector>
#include <map>

#include <Ws2tcpip.h>

//*****************************************************************************
//*****************************************************************************
class XBridgeApp : public QApplication
{
    friend void callback(void * closure, int event,
                         const unsigned char * info_hash,
                         const void * data, size_t data_len);

    Q_OBJECT;

public:
    XBridgeApp(int argc, char *argv[]);
    virtual ~XBridgeApp();

signals:
    void showLogMessage(const QString & msg);

public:
    const unsigned char * myid() const { return m_myid; }
    const std::string path() const     { return m_path; }

    bool initDht();
    bool stopDht();

    void logMessage(const QString & msg);

    // store session addresses in local table
    void storageStore(XBridgeSessionPtr session, const unsigned char * data);
    // clear local table
    void storageClean(XBridgeSessionPtr session);

public slots:
    // generate new id
    void onGenerate();
    // dump local table
    void onDump();
    // search id
    void onSearch(const std::string & id);
    // send messave via xbridge
    void onSend(const std::vector<unsigned char> & message);
    void onSend(const XBridgePacketPtr packet);
    void onSend(const std::vector<unsigned char> & id, const std::vector<unsigned char> & message);
    void onSend(const std::vector<unsigned char> & id, const XBridgePacketPtr packet);
    // call when message from xbridge network received
    void onMessageReceived(const std::vector<unsigned char> & id, const std::vector<unsigned char> & message);
    // broadcast message
    void onBroadcastReceived(const std::vector<unsigned char> & message);
    // broadcast send list of wallets
    void onSendListOfWallets();

public:
    static void sleep(const unsigned int umilliseconds);

private:
    void dhtThreadProc();
    void bridgeThreadProc();

private:
    unsigned char     m_myid[20];

    std::string       m_path;

    std::thread       m_dhtThread;
    std::atomic<bool> m_dhtStarted;
    std::atomic<bool> m_dhtStop;

    std::atomic<bool> m_signalGenerate;
    std::atomic<bool> m_signalDump;
    std::atomic<bool> m_signalSearch;
    std::atomic<bool> m_signalSend;

    typedef std::vector<unsigned char> UcharVector;
    typedef std::pair<UcharVector, UcharVector> MessagePair;

    std::list<std::string> m_searchStrings;
    std::list<MessagePair> m_messages;

    const bool        m_ipv4;
    const bool        m_ipv6;

    sockaddr_in       m_sin;
    sockaddr_in6      m_sin6;
    unsigned short    m_dhtPort;

    std::vector<sockaddr_storage> m_nodes;

    std::thread       m_bridgeThread;
    XBridge m_bridge;

    boost::mutex m_sessionsLock;
    typedef std::map<std::vector<unsigned char>, XBridgeSessionPtr> SessionMap;
    SessionMap m_sessions;

    boost::mutex m_messagesLock;
    typedef std::set<uint256> ProcessedMessages;
    ProcessedMessages m_processedMessages;
};

#endif // XBRIDGEAPP_H
