#ifndef _FD_MANAGER_H_
#define _FD_MANAGER_H_

#include <memory>
#include <shared_mutex>
#include <vector>
#include "./utils/singleton.h"

namespace dag{

/**
 * @brief 文件句柄上下文类
 * @details 管理文件句柄类型（是否socket)
 *          是否阻塞，是否关闭，读/写超时时间
 */

class FdCtx : public std::enable_shared_from_this<FdCtx>
{
public:
    using ptr = std::shared_ptr<FdCtx>;
    /**
    * @brief 通过文件句柄构造FdCtx
    */
    FdCtx(int fd);

    /**
    * @brief 析构函数
    */
    ~FdCtx();

    bool init();

    bool isInit() const {return m_isInit;}

    /**
    * @brief 是否socket
    */
    bool isSocket() const {return m_isSocket;}

    /**
    * @brief 是否关闭
    */
    bool isClosed() const {return m_isClosed;}
    
    /**
    * @brief 设置用户主动设置非阻塞
    * @param[in] v 是否阻塞
    */
    void setUserNonblock(bool v) {m_userNonblock = v;}

    /**
    * @brief 获取是否用户主动设置的非阻塞
    */
    bool getUserNonblock() const {return m_userNonblock;}

    /**
    * @brief 设置系统非阻塞
    * @param[in] v 是否阻塞
    */
    void setSysNonblock(bool v) {m_sysNonblock = v;} 

    /**
    * @brief 获取系统非阻塞
    */
    bool getSysNonblock() const {return m_sysNonblock;}

    /**
    * @brief 设置超时时间
    * @param[in] type 类型SO_RCVTIMEO(读超时),SO_SNDTIMEO(写超时)
    * @param[in] v 时间毫秒
    */
    void setTimeout(int type,uint64_t v);

    /**
    * @brief 获取超时时间
    * @param[in] type 类型SO_RCVTIMEO(读超时),SO_SNDTIMEO(写超时)
    */
    uint64_t getTimeout(int type);

private:
    // 是否初始化
    bool m_isInit = false;
    // 是否socket
    bool m_isSocket = false;
    // 是否hook非阻塞
    bool m_sysNonblock = false;
    // 是否用户主动设置阻塞
    bool m_userNonblock = false;
    // 是否关闭
    bool m_isClosed = false;
    // 文件句柄
    int m_fd;
    // 读超时毫秒
    uint64_t m_recvTimeout = (uint64_t)-1;
    // 写超时毫秒
    uint64_t m_sendTimeout = (uint64_t)-1;
};

/**
* @brief 文件句柄管理类
*/
class FdManager {
public:
    /**
    * @brief 无参构造函数
    */
    FdManager();

    /**
    * @brief 获取/创建文件句柄 Fdctx
    * @param[in] fd文件句柄
    * @param[in] auto_create 是否自动创建
    * @return 返回对应文件句柄类std::shared_ptr<FdCtx>
    */
    std::shared_ptr<FdCtx> get(int fd, bool auto_create = false);

    /**
    * @brief 删除文件句柄类
    * @param[in] fd文件句柄
    */
    void del(int fd);

private:
    //读写锁
    std::shared_mutex m_mutex;
    // 文件句柄集合
    std::vector<std::shared_ptr<FdCtx>> m_datas;
};

using FdMgr = dag::Singleton<FdManager> ;

}


#endif
