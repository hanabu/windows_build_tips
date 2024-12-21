# UTF-8 C sample for Windows

```console
C:\somewhere> cd windows_build_tips\hello_utf8
C:\somewhere\windows_build_tips\hello_utf8> mkdir build
C:\somewhere\windows_build_tips\hello_utf8> cd build
C:\somewhere\windows_build_tips\hello_utf8\build> cmake -G "MinGW Makefiles" ..
...
-- Build files have been written to: C:/somewhere/windows_build_tips/hello_utf8/build
C:\somewhere\windows_build_tips\hello_utf8\build> mingw32-make
...
[100%] Built target hello_utf8
C:\somewhere\windows_build_tips\hello_utf8\build> .\hello_utf8.exe
```