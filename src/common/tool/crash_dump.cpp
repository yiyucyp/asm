#include "crash_dump.h"
#include <experimental/filesystem>
#include <fstream>
#include <future>

namespace fs = std::experimental::filesystem;

const char* crash_dump::sz_dir_= nullptr;
const char* crash_dump::sz_app_ = nullptr;
MINIDUMP_TYPE crash_dump::dump_type_ = MiniDumpWithFullMemory;
char crash_dump::sz_filename_[];

crash_dump::crash_dump(const char* dump_dir, const char* app, MINIDUMP_TYPE type)
{
    LPTOP_LEVEL_EXCEPTION_FILTER pLast = ::SetUnhandledExceptionFilter(crash_dump::top_level_exception_filter);

#ifdef _WIN64

    unsigned char code[] = { 0x33, 0xc0, 0xc3};

    // xor eax, eax
    // ret
#else
    unsigned char code[] = {0x33, 0xc0, 0xc2, 0x04, 0x00};
    // xor eax, eax
    // ret 4
#endif // _WIN64



    int size = sizeof(code);
    SIZE_T dw;
    DWORD dwOldFlag, dwTempFlag;
    VirtualProtect(SetUnhandledExceptionFilter, size, PAGE_READWRITE, &dwOldFlag);
    WriteProcessMemory(INVALID_HANDLE_VALUE, SetUnhandledExceptionFilter, code, size, &dw);
    VirtualProtect(SetUnhandledExceptionFilter, size, dwOldFlag, &dwTempFlag);

    pLast = ::SetUnhandledExceptionFilter(crash_dump::top_level_exception_filter);
    //create dir
    if (sz_dir_)
    {
        std::error_code err;
        fs::create_directory(sz_dir_, err);
        //CreateDirectory(sz_dir_, NULL);
    }
    sz_dir_ = dump_dir;
    sz_app_ = app;
    dump_type_ = type;
    memset(sz_filename_, 0, sizeof(sz_filename_));

}


long WINAPI crash_dump::top_level_exception_filter(PEXCEPTION_POINTERS ExceptionInfo)
{
    std::async(real_dump, ExceptionInfo);
    return real_dump(ExceptionInfo);
}

long WINAPI crash_dump::real_dump(PEXCEPTION_POINTERS ExceptionInfo)
{
    MINIDUMP_EXCEPTION_INFORMATION eInfo;
    eInfo.ThreadId = GetCurrentThreadId();
    eInfo.ExceptionPointers = ExceptionInfo;
    eInfo.ClientPointers = FALSE;

    sprintf(sz_filename_, "%s%s-%p", sz_dir_, sz_app_, ExceptionInfo->ExceptionRecord->ExceptionAddress);
    HANDLE hFile = CreateFileA(
        sz_filename_,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL /*| FILE_FLAG_WRITE_THROUGH*/,
        NULL);

    std::ofstream out(sz_filename_);


    if (hFile == INVALID_HANDLE_VALUE)
    {
        MessageBoxA(NULL, "DUMP DIR", "", 0);
        return EXCEPTION_CONTINUE_SEARCH;
    }

    BOOL bResult = ::MiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        hFile,
        MiniDumpWithFullMemory,
        &eInfo,
        NULL,
        NULL);

   return EXCEPTION_EXECUTE_HANDLER;
}