#include <iostream>
#include "../fiber_lib/logger.h"
#include "../fiber_lib/utils/util.h"

using namespace dag;

void test1() {
    Logger::ptr logger1(new Logger("test_logger1",LogLevel::DEBUG));
    LogAppender::ptr append(new StdoutLogAppender());
    logger1->addAppender(append);
    DAG_LOG_INFO(logger1) << "I'm test1";

    DAG_LOG_FMT_ERROR(logger1, "%s DAG_LOG_ERROR", __FUNCTION__);
    std::cout << "\n\n";
}


void test2() {
    LogFormatter::prt formatter2(new LogFormatter("formatter1... [%p] [%c] %N"));
    LogAppender::ptr logappend2(new StdoutLogAppender());
    logappend2->setFormatter(formatter2);
    Logger::ptr logger2(new Logger("test_logger_2"));
    logger2->addAppender(logappend2);
    DAG_LOG_DEBUG(logger2) << "I'm test2";
    DAG_LOG_INFO(logger2) << "I'm test2";
    DAG_LOG_ERROR(logger2) << "I'm test2";
    DAG_LOG_FATAL(logger2) << "I'm test2";
}

void test3(){
    static Logger::ptr logger3 = DAG_LOG_NAME("root");
    DAG_LOG_DEBUG(logger3) << "I`m test3";
}

void test4(){
    Logger::ptr logger4(new Logger("test_logger_4",LogLevel::DEBUG));
    LogAppender::ptr append(new FileLogAppender("./test_output"));
    logger4->addAppender(append);
    DAG_LOG_DEBUG(logger4) << "I'm test4";
}

int main() {
    test1();
    test2();
    test3();
    test4();
    return 0;
}
