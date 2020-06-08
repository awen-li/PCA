#include "analysis/ExternalLib.h"
#include <algorithm>


using namespace llvm;
using namespace std;


vector<string> ExternalLib::m_Normal = 
{
    "log", 
    "log10", 
    "exp", 
    "exp2", 
    "exp10", 
    "strcmp", 
    "strncmp", 
    "strncasecmp", 
    "atoi", 
    "atof", 
    "atol", 
    "atoll", 
    "remove", 
    "unlink", 
    "rename", 
    "memcmp",
    "free", 
    "execl", 
    "execlp", 
    "execle", 
    "execv", 
    "execvp", 
    "chmod", 
    "puts",
    "write", 
    "open", 
    "create", 
    "truncate", 
    "chdir", 
    "mkdir", 
    "rmdir", 
    "read",
    "pipe", 
    "wait", 
    "time", 
    "stat", 
    "fstat", 
    "lstat", 
    "fopen",
    "fopen64",
    "fdopen",
    "open64",
    "fflush", 
    "feof", 
    "fileno", 
    "clearerr",
    "rewind", 
    "ftell", 
    "ferror", 
    "fgetc", 
    "fgetc", 
    "_IO_getc", 
    "fwrite",
    "fread", 
    "fgets", 
    "ungetc", 
    "fputc", 
    "fputs", 
    "putc", 
    "ftell", 
    "rewind",
    "_IO_putc", 
    "fseek", 
    "fgetpos", 
    "fsetpos", 
    "printf", 
    "fprintf", 
    "sprintf",
    "vprintf", 
    "vfprintf", 
    "vsprintf", 
    "scanf", 
    "fscanf", 
    "sscanf", 
    "error",
    "__assert_fail", 
    "modf", 
    "putchar", 
    "isalnum", 
    "isalpha", 
    "isascii", 
    "isatty", 
    "isblank", 
    "iscntrl", 
    "isdigit", 
    "isgraph", 
    "islower", 
    "isprint", 
    "ispunct", 
    "isspace", 
    "isupper", 
    "iswalnum", 
    "iswalpha", 
    "iswctype", 
    "iswdigit", 
    "iswlower", 
    "iswspace", 
    "iswprint", 
    "iswupper", 
    "sin", 
    "cos", 
    "sinf", 
    "cosf", 
    "asin", 
    "acos", 
    "tan", 
    "atan", 
    "fabs", 
    "pow", 
    "floor", 
    "ceil", 
    "sqrt", 
    "sqrtf", 
    "hypot", 
    "random", 
    "tolower", 
    "toupper", 
    "towlower", 
    "towupper", 
    "system", 
    "clock", 
    "exit", 
    "abort", 
    "gettimeofday", 
    "settimeofday", 
    "sleep", 
    "ctime", 
    "strspn", 
    "strcspn", 
    "localtime", 
    "strftime", 
    "qsort", 
    "popen", 
    "pclose", 
    "rand", 
    "rand_r", 
    "srand",
    "seed48", 
    "drand48", 
    "lrand48", 
    "srand48", 
    "__isoc99_sscanf", 
    "__isoc99_fscanf",
    "fclose", 
    "close", 
    "perror", 
    "strerror", 
    "__errno_location", 
    "__ctype_b_loc", 
    "abs", 
    "difftime",  
    "setbuf", 
    "_ZdlPv", 
    "strlen",
    "strcasecmp", 
    "_ZdaPv", 
    "fesetround", 
    "fegetround", 
    "fetestexcept", 
    "feraiseexcept",
    "feclearexcept", 
    "llvm.bswap.i16", 
    "llvm.bswap.i32", 
    "llvm.ctlz.i64", 
    "slurm_get_resume_timeout",
    "llvm.lifetime.start", 
    "llvm.lifetime.end",
    "llvm.lifetime.start.p0i8",
    "llvm.lifetime.end.p0i8",
    "llvm.stackrestore", 
    "llvm.stacksave", 
    "memset",
    "llvm.memset.i32", 
    "llvm.memset.p0i8.i32", 
    "llvm.memset.i64",
    "info",
    "llvm.memset.p0i8.i64", 
    "llvm.va_end",
    "llvm.va_start", 
    "llvm.dbg.declare", 
    "getuid",
    "getopt_long",

    "getpwnam",
    
    "getgrgid",
    "getpid",
    "getpwuid",
    "snprintf",
    "htons",
    "ntohs",
    "tcsetpgrp",
    "tcsetattr",
    "killpg",
    "setpgid",
    "getrlimit",
    "setrlimit",
    "getgroups", 
    "setegid", 
    "seteuid", 
    "setregid", 
    "setreuid", 
    "getgrouplist",
    "setgroups", 
    "initgroups", 
    "getegid", 
    "setgid", 
    "setuid", 
    "sigwait",
    "setsid",

    "fork",
    "pthread_mutex_init",
    "pthread_mutex_lock",
    "pthread_mutex_unlock", 
    "pthread_mutex_destroy",
    "pthread_attr_init",
    "pthread_attr_setscope",
    "pthread_attr_setstacksize",
    "pthread_attr_setdetachstate",
    "pthread_create",
    "pthread_attr_destroy",
    "pthread_cond_init",
    "pthread_cond_wait",
    "pthread_cond_signal",
    "pthread_cond_broadcast",
    "pthread_cond_destroy"
,
    "pthread_setcancelstate",
    "pthread_setcanceltype",
    "pthread_sigmask", 
    "pthread_cond_timedwait", 
    "pthread_join",
    "pthread_kill",
    "pthread_self",
    "pthread_exit",
    "pthread_cancel",
    "pthread_atfork", 
    "waitpid",

    "usleep",
    "setenv",
    "atexit",
    "sigaction",
    "sysconf",
    "_exit",
    "access",
    "ioctl",
    "getlogin", 
     
    "ntohl", 
    "poll", 
    "htonl", 
    "recv", 
    "prctl", 
    "getopt", 
    "chown", 
    "getpriority", 
    "setpriority", 
    "kill",
    "signal",      
     
    "dup2", 
    "fchown", 
    "tcgetattr", 
    "tcgetpgrp", 
    "getpgrp", 
    "gethostname", 
    "getppid", 
    "getpgid", 
    "getgid", 
    "getsid", 
    "div", 
    "accept", 
    "inet_ntop", 
    "sigemptyset", 
    "sigaddset",     
    "socket", 
    "connect",     
    "select", 
    "getchar", 
    "strcasestr", 
    "index", 
    "inet_pton", 
    "creat", 
    "fsync", 
    "link", 
    "vsnprintf", 
    
    "setsockopt", 
    "listen", 
    "bind", 
    "getsockname", 
    "openlog", 
    "syslog", 
    "closelog", 
    "regcomp", 
    "regexec", 
    "dlsym", 
    "dlopen", 
    "dlclose", 
    "dlerror", 
    "pathconf", 
    "opendir", 
    "readdir", 
    "closedir", 
    "inet_addr", 
    "unsetenv", 
    "putenv", 
    "fcntl", 
    "geteuid", 
    "send", 
    "getsockopt", 
    "getpeername", 
    "hstrerror", 
    "getpwuid_r", 
    "getpwnam_r", 
    "bsearch", 
    "getgrnam_r", 
    "getgrgid_r", 
    "setgrent", 
    "getgrent_r", 
    "endgrent", 
    "gethostbyname", 
    "__h_errno_location", 
    "gethostbyaddr", 
    "get_current_dir_name", 
    "asctime", 
    "asctime_r", 
    "ctime_r", 
    "gmtime", 
    "gmtime_r", 
    "localtime_r", 
    "mktime", 
    "strsep", 
    "regfree", 
    "memchr", 
    "sched_getaffinity", 
    "readdir_r", 
    "inet_nsap_addr", 
    "dirname", 
    "glob", 
    "globfree", 
    "dup", 
    "setpwent", 
    "getpwent_r", 
    "endpwent", 
    "setpgrp", 
    "execve", 
    "strsignal", 
    "cfmakeraw", 
    "wait4", 
    "wait3", 
    
    "login_tty", 
    "faccessat", 
    "setresuid", 
    "openpty", 
    "shutdown", 
    "umask", 
    "ptrace", 
    "lseek", 
    "rindex", 
    "mount", 
    "umount", 
    "flock", 
    "mlockall", 
    "uname", 
    "fchmod", 
    "utime", 
    "statvfs", 
    "sysinfo",
    "__xstat64",
    "__lxstat64",
    
};

vector<string> ExternalLib::m_Malloc = 
{
    "malloc", 
    "valloc", 
    "calloc", 
    "strdup", 
    "strndup",
    "getenv",
    "memalign", 
    "posix_memalign",
};

vector<string> ExternalLib::m_Realloc = 
{
    "realloc", 
    "strtok", 
    "strtok_r",
};


vector<string> ExternalLib::m_Memcpy = 
{
    "llvm.memcpy.i32", 
    "llvm.memcpy.p0i8.p0i8.i32", 
    "llvm.memcpy.i64",
    "llvm.memcpy.p0i8.p0i8.i64", 
    "llvm.memmove.i32", 
    "llvm.memmove.p0i8.p0i8.i32",
    "llvm.memmove.i64", 
    "llvm.memmove.p0i8.p0i8.i64", 
    "memccpy", 
    "memmove", 
    "bcopy",
    "llvm.va_copy",
};

vector<string> ExternalLib::m_RetArg0 = 
{
    "fgets",    
    "gets",       
    "stpcpy",  
    "strcat",  
    "strchr",
    "strcpy",   
    "strerror_r", 
    "strncat", 
    "strncpy", 
    "strpbrk",
    "strptime", 
    "strrchr",    
    "strstr",  
    "getcwd", 
};

vector<string> ExternalLib::m_Cast = 
{
    "strtod",  
    "strtof",  
    "strtol", 
    "strtold",
    "strtoll", 
    "strtoul",
    "strtoull",
};



VOID ExternalLib::InitExtLib ()
{
    for (auto it  = m_Normal.begin(), ie =  m_Normal.end(); it !=ie; it++)
    {
        m_ExtFuncMap[*it] = EXT_Normal;
    }

    for (auto it  = m_Malloc.begin(), ie =  m_Malloc.end(); it !=ie; it++)
    {
        m_ExtFuncMap[*it] = EXT_Malloc;
    }

    for (auto it  = m_Realloc.begin(), ie =  m_Realloc.end(); it !=ie; it++)
    {
        m_ExtFuncMap[*it] = EXT_ReMalloc;
    }

    for (auto it  = m_Memcpy.begin(), ie =  m_Memcpy.end(); it !=ie; it++)
    {
        m_ExtFuncMap[*it] = EXT_Memcpy;
    }

    for (auto it  = m_RetArg0.begin(), ie =  m_RetArg0.end(); it !=ie; it++)
    {
        m_ExtFuncMap[*it] = EXT_RetArg0;
    }

    for (auto it  = m_Cast.begin(), ie =  m_Cast.end(); it !=ie; it++)
    {
        m_ExtFuncMap[*it] = EXT_Cast;
    }

    vector<string> Empty;
    
    m_Normal.clear(); 
    m_Normal.swap(Empty);
    
    m_Malloc.clear();
    m_Malloc.swap(Empty);
    
    m_Realloc.clear();
    m_Realloc.swap(Empty);
    
    m_Memcpy.clear();
    m_Memcpy.swap(Empty);
    
    m_RetArg0.clear();
    m_RetArg0.swap(Empty);

    m_Cast.clear();
    m_Cast.swap(Empty);

    return;
}

EXT_TYPE ExternalLib::Search (std::string Str)
{
    auto It = m_ExtFuncMap.find(Str);
    if (It != m_ExtFuncMap.end())
    {
        return It->second;
    }

    return EXT_Null;
}

VOID ExternalLib::CacheExtType(const llvm::Function *Func)
{
    m_CacheType = Search (Func->getName());
    return;
}

bool ExternalLib::IsDealWithPts ()
{
    return (m_CacheType != EXT_Normal);
}


bool ExternalLib::IsRealloc ()
{
    return (m_CacheType == EXT_ReMalloc);
}


bool ExternalLib::IsMalloc ()
{
    return (m_CacheType == EXT_Malloc);
}

bool ExternalLib::IsRetArg0 ()
{
    return (m_CacheType == EXT_RetArg0);
}


bool ExternalLib::IsMemcpy ()
{
    return (m_CacheType == EXT_Memcpy);
}

bool ExternalLib::IsCast ()
{
    return (m_CacheType == EXT_Cast);
}


vector<string> DebugLib::m_Debug = 
{
    "error",
    "info",
    "verbose",
    "debug",
    "debug2",
    "debug3",
    "debug4",
    "debug5",
    "schedlog",
    "log_msg",
    "log_oom"    
};

bool DebugLib::IsDebugFunction (string FuncName)
{
    auto it = m_DebugFuncSet.find (FuncName);
    if (it == m_DebugFuncSet.end())
    {
        return false;
    }

    return true;
}


VOID DebugLib::InitDebugLib ()
{
    for (auto it  = m_Debug.begin(), ie =  m_Debug.end(); it !=ie; it++)
    {
        m_DebugFuncSet.insert(*it);
    }
}


