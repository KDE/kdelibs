
int _k__get_dummy_used()
{
    return 0;
}

#ifdef Q_OS_WIN
__declspec(dllexport) int __dummy_function() { return 5; }
#endif
