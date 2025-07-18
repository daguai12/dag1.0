#ifndef _DAG_NONCOPYABLE_H_
#define _DAG_NONCOPYABLE_H_

namespace dag {
    class NonCopyable {
    public:
        NonCopyable(const NonCopyable& ) = delete; 

        NonCopyable &operator=(const NonCopyable& ) = delete;
    
        NonCopyable(NonCopyable&& ) = delete;

        NonCopyable &operator=(NonCopyable&& ) = delete;

    protected:
        NonCopyable() = default;
        
        ~NonCopyable() = default;
    };

};

#endif
