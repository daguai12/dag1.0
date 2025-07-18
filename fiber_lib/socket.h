#ifndef _DAG_SOCKET_H_
#define _DAG_SOCKET_H_

#include <bits/types/struct_iovec.h>
#include <cstdint>
#include <memory>
#include <sys/socket.h>
#include <sys/types.h>
#include "address.h"
#include "./utils/noncopyable.h"

namespace dag {

/**
* @brief 套接字分装
*/
class Socket : public std::enable_shared_from_this<Socket>, NonCopyable {
public:
    using ptr = std::shared_ptr<Socket>;
    using weak_ptr = std::weak_ptr<Socket>;

    enum Type {
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM };

    enum Family {
        IPv4 = AF_INET,
        IPv6 = AF_INET6,
        UNIX = AF_UNIX,
    };

    static Socket::ptr CreateTCP(dag::Address::ptr address);
    static Socket::ptr CreateUDP(dag::Address::ptr address);

    static Socket::ptr CreateTCPSocket();
    static Socket::ptr CreateUDPSocket();

    Socket(int family, int type, int protocol = 0);

    virtual ~Socket();

    int64_t getSendTimeout();
    void SetSendTimeout(int64_t v);


    int64_t getRecvTimeout();
    void SetRecvTimeout(int64_t v);

    bool getOption(int level, int option, void* result, socklen_t* len);

    template<class T>
    bool getOption(int level, int option, T& result) {
        size_t length = sizeof(T);
        return getOption(level, option, &result, &length);
    }
    
    bool setOption(int level, int option, const void* result, size_t len);
    template<class T>
    bool setOption(int level, int option,const T& value){
        return setOption(level, option, &value, sizeof(T));
    }

    Socket::ptr accept();

    bool bind(const Address::ptr addr);
    bool connect(const Address::ptr addr, uint64_t timeout_ms = -1);
    bool listen(int backlog = SOMAXCONN);
    bool close();

    int send(const void* buffer,size_t length, int flags = 0);
    int send(const iovec* buffers, size_t length, int flags = 0);
    int sendTo(const void* buffer, size_t length, const Address::ptr to, int flags = 0);
    int sendTo(const iovec* buffer, size_t length, const Address::ptr to, int flags = 0);

    int recv(void* buffer, size_t length, int flags = 0);
    int recv(iovec* buffer, size_t length, int flags = 0);
    int recvFrom(void* buffer, size_t length, Address::ptr from, int flags = 0);
    int recvFrom(iovec* buffer, size_t length, Address::ptr from, int flags = 0);

    Address::ptr getRemoteAddress();
    Address::ptr getLocalAddress();

    int getFamily() const { return m_family;}
    int getType() const {return m_type;}
    int getProtocol() const {return m_protocol;}

    bool isConnected() const {return m_isConnected;};
    bool isValid() const;
    int getError();

    std::ostream& dump(std::ostream& os) const;
    int getSocket() const {return m_sock;};

    bool cancelRead();
    bool cancelWrite();
    bool cancelAccept();
    bool cancelAll();
private:
    void initSock();
    void newSock();
    bool init(int sock);
protected:
    int m_sock;
    int m_family;
    int m_type;
    int m_protocol;
    int m_isConnected;

    Address::ptr m_localAddress;
    Address::ptr m_remoteAddress;

};

}

#endif
