#ifndef __AVLOG_H__
#define __AVLOG_H__

#include <string>
#include <functional>
#include <stdio.h>

#define CSVERSION	"0.2"

void attach(void *ctx_arg);
class AVLog {
private:
    static pthread_mutex_t gMutexLog;
    AVLog(const std::string filename);
    ~AVLog();
    static AVLog *avlog_;
    static FILE* gFilePtr;
    
    void init_log(std::string fname);
    static void sighandler(int sig);
public: 
    AVLog(const AVLog &) = delete;
    void operator = (const AVLog &) = delete;
    
    static AVLog * GetInstance(const std::string filename);

    static void ffmpeglog(void* p1, int p2, const char* pFormat, va_list args);
    static void log_to_file(const char *pFormat,...);
    static void DestoryInstance();

};



#endif // __AVLOG_H__