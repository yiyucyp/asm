#pragma once
#include <Windows.h>
#include <dbghelp.h>

class crash_dump
{
public:
    crash_dump(const char* dump_dir, const char* app, MINIDUMP_TYPE dump_type);

private:
    static long WINAPI top_level_exception_filter(PEXCEPTION_POINTERS ExceptionInfo);
    static long WINAPI real_dump(PEXCEPTION_POINTERS ExceptionInfo);
    static const char* sz_dir_;
    static const char* sz_app_;
    static char sz_filename_[256];
    static MINIDUMP_TYPE dump_type_;
};