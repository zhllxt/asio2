@echo off

set vsdev="C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\IDE\devenv.com"
set curdir=%~dp0

set slnfile="%curdir%\build\asio2.sln"

echo current directory is: %curdir%

del /f /q build.log

:: https://blog.csdn.net/dbyoung/article/details/105305559
:: >               clear and recreate log and logs are not displayed on the terminal
:: >>              append log and logs are not displayed on the terminal
:: -O --output-log append log and logs will displayed on the terminal

powershell -Command "(gc D:\asio2\test\unit\unit_test.hpp).replace('static const int   test_loop_times = 100;', 'static const int   test_loop_times = 1;') | Out-File -Encoding Ascii D:\asio2\test\unit\unit_test.hpp"

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

call clean.bat

cmake -A x64 -T host=x64 . -B build

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:: x64
echo "configure: x64 - Visual Studio 2022 (v143)" >> build.log
:::: Visual Studio 2022 (v143)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v143</PlatformToolset>', '<PlatformToolset>v143</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

echo "configure: x64 - Visual Studio 2019 (v142)" >> build.log
:::: Visual Studio 2019 (v142)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v143</PlatformToolset>', '<PlatformToolset>v142</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

echo "configure: x64 - Visual Studio 2017 (v141)" >> build.log
:::: Visual Studio 2017 (v141)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v142</PlatformToolset>', '<PlatformToolset>v141</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

echo "configure: x64 - Visual Studio 2017 - Windows XP (v141_xp)" >> build.log
:::: Visual Studio 2017 - Windows XP (v141_xp)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v141</PlatformToolset>', '<PlatformToolset>v141_xp</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

echo "configure: x64 - LLVM (clang-cl)" >> build.log
:::: LLVM (clang-cl)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v141_xp</PlatformToolset>', '<PlatformToolset>ClangCL</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

call clean.bat

cmake -A Win32 . -B build

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:: x86
echo "configure: x86 - Visual Studio 2022 (v143)" >> build.log
:::: Visual Studio 2022 (v143)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v143</PlatformToolset>', '<PlatformToolset>v143</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

echo "configure: x86 - Visual Studio 2019 (v142)" >> build.log
:::: Visual Studio 2019 (v142)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v143</PlatformToolset>', '<PlatformToolset>v142</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

echo "configure: x86 - Visual Studio 2017 (v141)" >> build.log
:::: Visual Studio 2017 (v141)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v142</PlatformToolset>', '<PlatformToolset>v141</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

echo "configure: x86 - Visual Studio 2017 - Windows XP (v141_xp)" >> build.log
:::: Visual Studio 2017 - Windows XP (v141_xp)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v141</PlatformToolset>', '<PlatformToolset>v141_xp</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

echo "configure: x86 - LLVM (clang-cl)" >> build.log
:::: LLVM (clang-cl)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v141_xp</PlatformToolset>', '<PlatformToolset>ClangCL</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

call clean.bat

WSL -e cmake -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_CXX_STANDARD=17 -S /mnt/d/asio2/ -B /mnt/d/asio2/build

WSL -e cmake --build /mnt/d/asio2/build -- -k >> build.log 2>&1

WSL -e ctest -C Debug --test-dir /mnt/d/asio2/build >> build.log


call clean.bat

WSL -e cmake -DCMAKE_BUILD_TYPE:STRING="Release" -DCMAKE_CXX_STANDARD=17 -S /mnt/d/asio2/ -B /mnt/d/asio2/build

WSL -e cmake --build /mnt/d/asio2/build -- -k >> build.log 2>&1

WSL -e ctest -C Release --test-dir /mnt/d/asio2/build >> build.log


call clean.bat

WSL -e cmake -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_CXX_STANDARD=20 -S /mnt/d/asio2/ -B /mnt/d/asio2/build

WSL -e cmake --build /mnt/d/asio2/build -- -k >> build.log 2>&1

WSL -e ctest -C Debug --test-dir /mnt/d/asio2/build >> build.log


call clean.bat

WSL -e cmake -DCMAKE_BUILD_TYPE:STRING="Release" -DCMAKE_CXX_STANDARD=20 -S /mnt/d/asio2/ -B /mnt/d/asio2/build

WSL -e cmake --build /mnt/d/asio2/build -- -k >> build.log 2>&1

WSL -e ctest -C Release --test-dir /mnt/d/asio2/build >> build.log


:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

call clean.bat

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_OPTIM := release', 'APP_OPTIM := release') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

ndk-build -C %curdir%/example/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_OPTIM := release', 'APP_OPTIM := debug') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

ndk-build -C %curdir%/example/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++20') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_OPTIM := debug', 'APP_OPTIM := debug') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

ndk-build -C %curdir%/example/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++20', 'APP_CPPFLAGS +=-std=c++20') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_OPTIM := debug', 'APP_OPTIM := release') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

ndk-build -C %curdir%/example/ndk/jni -k >> %curdir%\build.log 2>&1

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++20', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

:: ----------------------------------------------

call clean.bat

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_OPTIM := release', 'APP_OPTIM := release') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"

ndk-build -C %curdir%/test/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_OPTIM := release', 'APP_OPTIM := debug') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"

ndk-build -C %curdir%/test/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++20') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_OPTIM := debug', 'APP_OPTIM := debug') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"

ndk-build -C %curdir%/test/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++20', 'APP_CPPFLAGS +=-std=c++20') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_OPTIM := debug', 'APP_OPTIM := release') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"

ndk-build -C %curdir%/test/ndk/jni -k >> %curdir%\build.log 2>&1

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++20', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"


:: ASIO2_ENABLE_LOG -----------------------------------------------------------

powershell -Command "(gc D:\asio2\include\asio2\config.hpp).replace('//#define ASIO2_USE_WEBSOCKET_RPC', '#define ASIO2_ENABLE_LOG') | Out-File -Encoding Ascii D:\asio2\include\asio2\config.hpp"
powershell -Command "(gc D:\asio2\include\asio2\config.hpp).replace('//#define ASIO_NO_EXCEPTIONS', '#define ASIO_NO_EXCEPTIONS') | Out-File -Encoding Ascii D:\asio2\include\asio2\config.hpp"

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

call clean.bat

cmake -A x64 -T host=x64 . -B build

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:: x64
echo "configure: x64 - Visual Studio 2022 (v143)" >> build.log
:::: Visual Studio 2022 (v143)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v143</PlatformToolset>', '<PlatformToolset>v143</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

echo "configure: x64 - Visual Studio 2019 (v142)" >> build.log
:::: Visual Studio 2019 (v142)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v143</PlatformToolset>', '<PlatformToolset>v142</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

echo "configure: x64 - Visual Studio 2017 (v141)" >> build.log
:::: Visual Studio 2017 (v141)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v142</PlatformToolset>', '<PlatformToolset>v141</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

echo "configure: x64 - Visual Studio 2017 - Windows XP (v141_xp)" >> build.log
:::: Visual Studio 2017 - Windows XP (v141_xp)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v141</PlatformToolset>', '<PlatformToolset>v141_xp</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

echo "configure: x64 - LLVM (clang-cl)" >> build.log
:::: LLVM (clang-cl)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v141_xp</PlatformToolset>', '<PlatformToolset>ClangCL</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

call clean.bat

cmake -A Win32 . -B build

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:: x86
echo "configure: x86 - Visual Studio 2022 (v143)" >> build.log
:::: Visual Studio 2022 (v143)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v143</PlatformToolset>', '<PlatformToolset>v143</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

echo "configure: x86 - Visual Studio 2019 (v142)" >> build.log
:::: Visual Studio 2019 (v142)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v143</PlatformToolset>', '<PlatformToolset>v142</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

echo "configure: x86 - Visual Studio 2017 (v141)" >> build.log
:::: Visual Studio 2017 (v141)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v142</PlatformToolset>', '<PlatformToolset>v141</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

echo "configure: x86 - Visual Studio 2017 - Windows XP (v141_xp)" >> build.log
:::: Visual Studio 2017 - Windows XP (v141_xp)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v141</PlatformToolset>', '<PlatformToolset>v141_xp</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

echo "configure: x86 - LLVM (clang-cl)" >> build.log
:::: LLVM (clang-cl)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v141_xp</PlatformToolset>', '<PlatformToolset>ClangCL</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
%vsdev% %slnfile% /rebuild "Debug|x86" /out build.log
ctest -C Debug --test-dir build >> build.log
%vsdev% %slnfile% /rebuild "Release|x86" /out build.log
ctest -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

call clean.bat

WSL -e cmake -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_CXX_STANDARD=17 -S /mnt/d/asio2/ -B /mnt/d/asio2/build

WSL -e cmake --build /mnt/d/asio2/build -- -k >> build.log 2>&1

WSL -e ctest -C Debug --test-dir /mnt/d/asio2/build >> build.log


call clean.bat

WSL -e cmake -DCMAKE_BUILD_TYPE:STRING="Release" -DCMAKE_CXX_STANDARD=17 -S /mnt/d/asio2/ -B /mnt/d/asio2/build

WSL -e cmake --build /mnt/d/asio2/build -- -k >> build.log 2>&1

WSL -e ctest -C Release --test-dir /mnt/d/asio2/build >> build.log


call clean.bat

WSL -e cmake -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_CXX_STANDARD=20 -S /mnt/d/asio2/ -B /mnt/d/asio2/build

WSL -e cmake --build /mnt/d/asio2/build -- -k >> build.log 2>&1

WSL -e ctest -C Debug --test-dir /mnt/d/asio2/build >> build.log


call clean.bat

WSL -e cmake -DCMAKE_BUILD_TYPE:STRING="Release" -DCMAKE_CXX_STANDARD=20 -S /mnt/d/asio2/ -B /mnt/d/asio2/build

WSL -e cmake --build /mnt/d/asio2/build -- -k >> build.log 2>&1

WSL -e ctest -C Release --test-dir /mnt/d/asio2/build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

call clean.bat

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_OPTIM := release', 'APP_OPTIM := release') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

ndk-build -C %curdir%/example/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_OPTIM := release', 'APP_OPTIM := debug') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

ndk-build -C %curdir%/example/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++20') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_OPTIM := debug', 'APP_OPTIM := debug') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

ndk-build -C %curdir%/example/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++20', 'APP_CPPFLAGS +=-std=c++20') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_OPTIM := debug', 'APP_OPTIM := release') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

ndk-build -C %curdir%/example/ndk/jni -k >> %curdir%\build.log 2>&1

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++20', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

:: ----------------------------------------------

call clean.bat

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_OPTIM := release', 'APP_OPTIM := release') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"

ndk-build -C %curdir%/test/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_OPTIM := release', 'APP_OPTIM := debug') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"

ndk-build -C %curdir%/test/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++20') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_OPTIM := debug', 'APP_OPTIM := debug') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"

ndk-build -C %curdir%/test/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++20', 'APP_CPPFLAGS +=-std=c++20') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_OPTIM := debug', 'APP_OPTIM := release') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"

ndk-build -C %curdir%/test/ndk/jni -k >> %curdir%\build.log 2>&1

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++20', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"


powershell -Command "(gc D:\asio2\include\asio2\config.hpp).replace('#define ASIO2_ENABLE_LOG', '//#define ASIO2_USE_WEBSOCKET_RPC') | Out-File -Encoding Ascii D:\asio2\include\asio2\config.hpp"
powershell -Command "(gc D:\asio2\include\asio2\config.hpp).replace('#define ASIO_NO_EXCEPTIONS', '//#define ASIO_NO_EXCEPTIONS') | Out-File -Encoding Ascii D:\asio2\include\asio2\config.hpp"

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

powershell -Command "(gc D:\asio2\test\unit\unit_test.hpp).replace('static const int   test_loop_times = 1;', 'static const int   test_loop_times = 100;') | Out-File -Encoding Ascii D:\asio2\test\unit\unit_test.hpp"

