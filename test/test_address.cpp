#include "address.h"
#include "logger.h"
#include <cstdint>
#include <map>

using namespace dag ;
    
dag::Logger::ptr g_logger = DAG_LOG_ROOT();

void test() {
    std::vector<dag::Address::ptr> addrs;

    DAG_LOG_INFO(g_logger) << "begin";
    bool v = dag::Address::Lookup(addrs, "localhost:3080");
    DAG_LOG_INFO(g_logger) << "endl";

    if(!v) {
        DAG_LOG_ERROR(g_logger) << "lookup fail";
        return;
    }

    for(size_t i = 0; i < addrs.size(); ++i) {
        DAG_LOG_ERROR(g_logger) << "lookup fail";
        return;
    }

    auto addr = dag::Address::LookupAny("localhost::4080");
    if(addr) {
        DAG_LOG_INFO(g_logger) << *addr;
    } else {
        DAG_LOG_ERROR(g_logger) << "error";
    }
}

void test_iface() {
    std::multimap<std::string, std::pair<dag::Address::ptr, uint32_t>> results;

    bool v = dag::Address::GetInterfaceAddresses(results);
    if(!v) {
        DAG_LOG_ERROR(g_logger) << "GetInterfaceAddresses fail";
        return;
    }

    for(auto& i : results) {
        DAG_LOG_INFO(g_logger) << i.first << " - " << i.second.first->toString() << " - "
            << i.second.second;
    }

}

int main()
{
    test();
    test_iface();
    return 0;
}
