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
:: 
::  .CMakeGenerate.cmd
::     cmake . -B build
::  .CMakeBuild.cmd
::     cmake --build build
::     pause
::  .CMakeTest.cmd
::     ctest --output-on-failure -C Debug --test-dir build -O test.log
::     pause

::powershell -Command "(gc D:\asio2\test\unit\unit_test.hpp).replace('static const int   test_loop_times = 100;', 'static const int   test_loop_times = 1;') | Out-File -Encoding Ascii D:\asio2\test\unit\unit_test.hpp"

echo "configure: default config " >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

call clean.bat

cmake -A x64 -T host=x64 . -B build

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:: x64
:::: Visual Studio 2022 (v143)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v143</PlatformToolset>', '<PlatformToolset>v143</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - Visual Studio 2022 (v143) cpp17 Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - Visual Studio 2022 (v143) cpp17 Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - Visual Studio 2022 (v143) cpp20 Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - Visual Studio 2022 (v143) cpp20 Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - Visual Studio 2022 (v143) cpplatest Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - Visual Studio 2022 (v143) cpplatest Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::: Visual Studio 2019 (v142)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v143</PlatformToolset>', '<PlatformToolset>v142</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - Visual Studio 2019 (v142) cpp17 Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - Visual Studio 2019 (v142) cpp17 Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - Visual Studio 2019 (v142) cpp20 Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - Visual Studio 2019 (v142) cpp20 Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - Visual Studio 2019 (v142) cpplatest Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - Visual Studio 2019 (v142) cpplatest Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::: Visual Studio 2017 (v141)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v142</PlatformToolset>', '<PlatformToolset>v141</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - Visual Studio 2017 (v141) cpp17 Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - Visual Studio 2017 (v141) cpp17 Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - Visual Studio 2017 (v141) cpplatest Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - Visual Studio 2017 (v141) cpplatest Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::: Visual Studio 2017 - Windows XP (v141_xp)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v141</PlatformToolset>', '<PlatformToolset>v141_xp</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - Visual Studio 2017 - Windows XP (v141_xp) cpp17 Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - Visual Studio 2017 - Windows XP (v141_xp) cpp17 Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - Visual Studio 2017 - Windows XP (v141_xp) cpplatest Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - Visual Studio 2017 - Windows XP (v141_xp) cpplatest Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::: LLVM (clang-cl)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v141_xp</PlatformToolset>', '<PlatformToolset>ClangCL</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - LLVM (clang-cl) cpp17 Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - LLVM (clang-cl) cpp17 Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - LLVM (clang-cl) cpp20 Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - LLVM (clang-cl) cpp20 Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - LLVM (clang-cl) cpplatest Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - LLVM (clang-cl) cpplatest Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

call clean.bat

cmake -A Win32 . -B build

xcopy %curdir%\3rd\openssl\prebuilt\windows\x86 %curdir%\bin\x86 /E /I /Y

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:: x86
:::: Visual Studio 2022 (v143)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v143</PlatformToolset>', '<PlatformToolset>v143</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - Visual Studio 2022 (v143) cpp17 Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - Visual Studio 2022 (v143) cpp17 Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - Visual Studio 2022 (v143) cpp20 Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - Visual Studio 2022 (v143) cpp20 Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - Visual Studio 2022 (v143) cpplatest Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - Visual Studio 2022 (v143) cpplatest Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::: Visual Studio 2019 (v142)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v143</PlatformToolset>', '<PlatformToolset>v142</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - Visual Studio 2019 (v142) cpp17 Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - Visual Studio 2019 (v142) cpp17 Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - Visual Studio 2019 (v142) cpp20 Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - Visual Studio 2019 (v142) cpp20 Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - Visual Studio 2019 (v142) cpplatest Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - Visual Studio 2019 (v142) cpplatest Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::: Visual Studio 2017 (v141)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v142</PlatformToolset>', '<PlatformToolset>v141</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - Visual Studio 2017 (v141) cpp17 Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - Visual Studio 2017 (v141) cpp17 Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - Visual Studio 2017 (v141) cpplatest Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - Visual Studio 2017 (v141) cpplatest Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::: Visual Studio 2017 - Windows XP (v141_xp)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v141</PlatformToolset>', '<PlatformToolset>v141_xp</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - Visual Studio 2017 - Windows XP (v141_xp) cpp17 Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - Visual Studio 2017 - Windows XP (v141_xp) cpp17 Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - Visual Studio 2017 - Windows XP (v141_xp) cpplatest Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - Visual Studio 2017 - Windows XP (v141_xp) cpplatest Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::: LLVM (clang-cl)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v141_xp</PlatformToolset>', '<PlatformToolset>ClangCL</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - LLVM (clang-cl) cpp17 Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - LLVM (clang-cl) cpp17 Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - LLVM (clang-cl) cpp20 Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - LLVM (clang-cl) cpp20 Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - LLVM (clang-cl) cpplatest Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - LLVM (clang-cl) cpplatest Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

call clean.bat

echo "configure: x64 - WSL cpp17 Debug|x64" >> build.log

WSL -e cmake -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_CXX_STANDARD=17 -S /mnt/d/asio2/ -B /mnt/d/asio2/build

WSL -e cmake --build /mnt/d/asio2/build -- -k >> build.log 2>&1

WSL -e ctest --output-on-failure -C Debug --test-dir /mnt/d/asio2/build >> build.log


call clean.bat

echo "configure: x64 - WSL cpp17 Release|x64" >> build.log

WSL -e cmake -DCMAKE_BUILD_TYPE:STRING="Release" -DCMAKE_CXX_STANDARD=17 -S /mnt/d/asio2/ -B /mnt/d/asio2/build

WSL -e cmake --build /mnt/d/asio2/build -- -k >> build.log 2>&1

WSL -e ctest --output-on-failure -C Release --test-dir /mnt/d/asio2/build >> build.log


call clean.bat

echo "configure: x64 - WSL cpp20 Debug|x64" >> build.log

WSL -e cmake -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_CXX_STANDARD=20 -S /mnt/d/asio2/ -B /mnt/d/asio2/build

WSL -e cmake --build /mnt/d/asio2/build -- -k >> build.log 2>&1

WSL -e ctest --output-on-failure -C Debug --test-dir /mnt/d/asio2/build >> build.log


call clean.bat

echo "configure: x64 - WSL cpp20 Release|x64" >> build.log

WSL -e cmake -DCMAKE_BUILD_TYPE:STRING="Release" -DCMAKE_CXX_STANDARD=20 -S /mnt/d/asio2/ -B /mnt/d/asio2/build

WSL -e cmake --build /mnt/d/asio2/build -- -k >> build.log 2>&1

WSL -e ctest --output-on-failure -C Release --test-dir /mnt/d/asio2/build >> build.log


:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

call clean.bat

echo "configure: x64 - ndk example cpp17 Release|x64" >> build.log

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_OPTIM := release', 'APP_OPTIM := release') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

call ndk-build -C %curdir%/example/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

echo "configure: x64 - ndk example cpp17 Debug|x64" >> build.log

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_OPTIM := release', 'APP_OPTIM := debug') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

call ndk-build -C %curdir%/example/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

echo "configure: x64 - ndk example cpp20 Debug|x64" >> build.log

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++20') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_OPTIM := debug', 'APP_OPTIM := debug') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

call ndk-build -C %curdir%/example/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

echo "configure: x64 - ndk example cpp20 Release|x64" >> build.log

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++20', 'APP_CPPFLAGS +=-std=c++20') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_OPTIM := debug', 'APP_OPTIM := release') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

call ndk-build -C %curdir%/example/ndk/jni -k >> %curdir%\build.log 2>&1

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++20', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

:: ----------------------------------------------

call clean.bat

echo "configure: x64 - ndk test cpp17 Release|x64" >> build.log

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_OPTIM := release', 'APP_OPTIM := release') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"

call ndk-build -C %curdir%/test/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

echo "configure: x64 - ndk test cpp17 Debug|x64" >> build.log

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_OPTIM := release', 'APP_OPTIM := debug') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"

call ndk-build -C %curdir%/test/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

echo "configure: x64 - ndk test cpp20 Debug|x64" >> build.log

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++20') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_OPTIM := debug', 'APP_OPTIM := debug') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"

call ndk-build -C %curdir%/test/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

echo "configure: x64 - ndk test cpp20 Release|x64" >> build.log

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++20', 'APP_CPPFLAGS +=-std=c++20') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_OPTIM := debug', 'APP_OPTIM := release') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"

call ndk-build -C %curdir%/test/ndk/jni -k >> %curdir%\build.log 2>&1

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++20', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"



echo "configure: ASIO2_ENABLE_LOG = TRUE" >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

powershell -Command "(gc D:\asio2\include\asio2\config.hpp).replace('//#define ASIO2_USE_WEBSOCKET_RPC', '#define ASIO2_ENABLE_LOG') | Out-File -Encoding Ascii D:\asio2\include\asio2\config.hpp"

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

call clean.bat

cmake -A x64 -T host=x64 . -B build

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:: x64
:::: Visual Studio 2022 (v143)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v143</PlatformToolset>', '<PlatformToolset>v143</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - Visual Studio 2022 (v143) cpp17 Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - Visual Studio 2022 (v143) cpp17 Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - Visual Studio 2022 (v143) cpp20 Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - Visual Studio 2022 (v143) cpp20 Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - Visual Studio 2022 (v143) cpplatest Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - Visual Studio 2022 (v143) cpplatest Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::: Visual Studio 2019 (v142)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v143</PlatformToolset>', '<PlatformToolset>v142</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - Visual Studio 2019 (v142) cpp17 Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - Visual Studio 2019 (v142) cpp17 Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - Visual Studio 2019 (v142) cpp20 Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - Visual Studio 2019 (v142) cpp20 Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - Visual Studio 2019 (v142) cpplatest Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - Visual Studio 2019 (v142) cpplatest Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::: Visual Studio 2017 (v141)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v142</PlatformToolset>', '<PlatformToolset>v141</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - Visual Studio 2017 (v141) cpp17 Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - Visual Studio 2017 (v141) cpp17 Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - Visual Studio 2017 (v141) cpplatest Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - Visual Studio 2017 (v141) cpplatest Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::: Visual Studio 2017 - Windows XP (v141_xp)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v141</PlatformToolset>', '<PlatformToolset>v141_xp</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - Visual Studio 2017 - Windows XP (v141_xp) cpp17 Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - Visual Studio 2017 - Windows XP (v141_xp) cpp17 Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - Visual Studio 2017 - Windows XP (v141_xp) cpplatest Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - Visual Studio 2017 - Windows XP (v141_xp) cpplatest Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::: LLVM (clang-cl)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v141_xp</PlatformToolset>', '<PlatformToolset>ClangCL</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - LLVM (clang-cl) cpp17 Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - LLVM (clang-cl) cpp17 Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - LLVM (clang-cl) cpp20 Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - LLVM (clang-cl) cpp20 Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
echo "configure: x64 - LLVM (clang-cl) cpplatest Debug|x64" >> build.log
%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x64 - LLVM (clang-cl) cpplatest Release|x64" >> build.log
%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

call clean.bat

cmake -A Win32 . -B build

xcopy %curdir%\3rd\openssl\prebuilt\windows\x86 %curdir%\bin\x86 /E /I /Y

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:: x86
:::: Visual Studio 2022 (v143)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v143</PlatformToolset>', '<PlatformToolset>v143</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - Visual Studio 2022 (v143) cpp17 Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - Visual Studio 2022 (v143) cpp17 Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - Visual Studio 2022 (v143) cpp20 Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - Visual Studio 2022 (v143) cpp20 Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - Visual Studio 2022 (v143) cpplatest Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - Visual Studio 2022 (v143) cpplatest Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::: Visual Studio 2019 (v142)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v143</PlatformToolset>', '<PlatformToolset>v142</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - Visual Studio 2019 (v142) cpp17 Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - Visual Studio 2019 (v142) cpp17 Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - Visual Studio 2019 (v142) cpp20 Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - Visual Studio 2019 (v142) cpp20 Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - Visual Studio 2019 (v142) cpplatest Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - Visual Studio 2019 (v142) cpplatest Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::: Visual Studio 2017 (v141)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v142</PlatformToolset>', '<PlatformToolset>v141</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - Visual Studio 2017 (v141) cpp17 Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - Visual Studio 2017 (v141) cpp17 Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - Visual Studio 2017 (v141) cpplatest Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - Visual Studio 2017 (v141) cpplatest Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::: Visual Studio 2017 - Windows XP (v141_xp)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v141</PlatformToolset>', '<PlatformToolset>v141_xp</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - Visual Studio 2017 - Windows XP (v141_xp) cpp17 Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - Visual Studio 2017 - Windows XP (v141_xp) cpp17 Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - Visual Studio 2017 - Windows XP (v141_xp) cpplatest Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - Visual Studio 2017 - Windows XP (v141_xp) cpplatest Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::: LLVM (clang-cl)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<PlatformToolset>v141_xp</PlatformToolset>', '<PlatformToolset>ClangCL</PlatformToolset>' | Out-File %%i"
)
:::::: ISO C++17 Standard (/std:c++17)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - LLVM (clang-cl) cpp17 Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - LLVM (clang-cl) cpp17 Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: ISO C++20 Standard (/std:c++20)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - LLVM (clang-cl) cpp20 Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - LLVM (clang-cl) cpp20 Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log
:::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
)
echo "configure: x86 - LLVM (clang-cl) cpplatest Debug|x86" >> build.log
%vsdev% %slnfile% /rebuild "Debug" /out build.log
ctest --output-on-failure -C Debug --test-dir build >> build.log
echo "configure: x86 - LLVM (clang-cl) cpplatest Release|x86" >> build.log
%vsdev% %slnfile% /rebuild "Release" /out build.log
ctest --output-on-failure -C Release --test-dir build >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

call clean.bat

echo "configure: x64 - WSL cpp17 Debug|x64" >> build.log

WSL -e cmake -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_CXX_STANDARD=17 -S /mnt/d/asio2/ -B /mnt/d/asio2/build

WSL -e cmake --build /mnt/d/asio2/build -- -k >> build.log 2>&1

WSL -e ctest --output-on-failure -C Debug --test-dir /mnt/d/asio2/build >> build.log


call clean.bat

echo "configure: x64 - WSL cpp17 Release|x64" >> build.log

WSL -e cmake -DCMAKE_BUILD_TYPE:STRING="Release" -DCMAKE_CXX_STANDARD=17 -S /mnt/d/asio2/ -B /mnt/d/asio2/build

WSL -e cmake --build /mnt/d/asio2/build -- -k >> build.log 2>&1

WSL -e ctest --output-on-failure -C Release --test-dir /mnt/d/asio2/build >> build.log


call clean.bat

echo "configure: x64 - WSL cpp20 Debug|x64" >> build.log

WSL -e cmake -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_CXX_STANDARD=20 -S /mnt/d/asio2/ -B /mnt/d/asio2/build

WSL -e cmake --build /mnt/d/asio2/build -- -k >> build.log 2>&1

WSL -e ctest --output-on-failure -C Debug --test-dir /mnt/d/asio2/build >> build.log


call clean.bat

echo "configure: x64 - WSL cpp20 Release|x64" >> build.log

WSL -e cmake -DCMAKE_BUILD_TYPE:STRING="Release" -DCMAKE_CXX_STANDARD=20 -S /mnt/d/asio2/ -B /mnt/d/asio2/build

WSL -e cmake --build /mnt/d/asio2/build -- -k >> build.log 2>&1

WSL -e ctest --output-on-failure -C Release --test-dir /mnt/d/asio2/build >> build.log


:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

call clean.bat

echo "configure: x64 - ndk example cpp17 Release|x64" >> build.log

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_OPTIM := release', 'APP_OPTIM := release') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

call ndk-build -C %curdir%/example/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

echo "configure: x64 - ndk example cpp17 Debug|x64" >> build.log

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_OPTIM := release', 'APP_OPTIM := debug') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

call ndk-build -C %curdir%/example/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

echo "configure: x64 - ndk example cpp20 Debug|x64" >> build.log

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++20') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_OPTIM := debug', 'APP_OPTIM := debug') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

call ndk-build -C %curdir%/example/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

echo "configure: x64 - ndk example cpp20 Release|x64" >> build.log

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++20', 'APP_CPPFLAGS +=-std=c++20') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_OPTIM := debug', 'APP_OPTIM := release') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

call ndk-build -C %curdir%/example/ndk/jni -k >> %curdir%\build.log 2>&1

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++20', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

:: ----------------------------------------------

call clean.bat

echo "configure: x64 - ndk test cpp17 Release|x64" >> build.log

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_OPTIM := release', 'APP_OPTIM := release') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"

call ndk-build -C %curdir%/test/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

echo "configure: x64 - ndk test cpp17 Debug|x64" >> build.log

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_OPTIM := release', 'APP_OPTIM := debug') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"

call ndk-build -C %curdir%/test/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

echo "configure: x64 - ndk test cpp20 Debug|x64" >> build.log

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++20') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_OPTIM := debug', 'APP_OPTIM := debug') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"

call ndk-build -C %curdir%/test/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

echo "configure: x64 - ndk test cpp20 Release|x64" >> build.log

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++20', 'APP_CPPFLAGS +=-std=c++20') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_OPTIM := debug', 'APP_OPTIM := release') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"

call ndk-build -C %curdir%/test/ndk/jni -k >> %curdir%\build.log 2>&1

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++20', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"


powershell -Command "(gc D:\asio2\include\asio2\config.hpp).replace('#define ASIO2_ENABLE_LOG', '//#define ASIO2_USE_WEBSOCKET_RPC') | Out-File -Encoding Ascii D:\asio2\include\asio2\config.hpp"

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::



echo "configure: ASIO_NO_EXCEPTIONS = TRUE" >> build.log

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

powershell -Command "(gc D:\asio2\include\asio2\config.hpp).replace('//#define ASIO_NO_EXCEPTIONS', '#define ASIO_NO_EXCEPTIONS') | Out-File -Encoding Ascii D:\asio2\include\asio2\config.hpp"

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::
::call clean.bat
::
::cmake -A x64 -T host=x64 . -B build
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::
:::: x64
:::::: Visual Studio 2022 (v143)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<PlatformToolset>v143</PlatformToolset>', '<PlatformToolset>v143</PlatformToolset>' | Out-File %%i"
::)
:::::::: ISO C++17 Standard (/std:c++17)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x64 - Visual Studio 2022 (v143) cpp17 Debug|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x64 - Visual Studio 2022 (v143) cpp17 Release|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
:::::::: ISO C++20 Standard (/std:c++20)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x64 - Visual Studio 2022 (v143) cpp20 Debug|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x64 - Visual Studio 2022 (v143) cpp20 Release|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
:::::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x64 - Visual Studio 2022 (v143) cpplatest Debug|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x64 - Visual Studio 2022 (v143) cpplatest Release|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::
:::::: Visual Studio 2019 (v142)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<PlatformToolset>v143</PlatformToolset>', '<PlatformToolset>v142</PlatformToolset>' | Out-File %%i"
::)
:::::::: ISO C++17 Standard (/std:c++17)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x64 - Visual Studio 2019 (v142) cpp17 Debug|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x64 - Visual Studio 2019 (v142) cpp17 Release|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
:::::::: ISO C++20 Standard (/std:c++20)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x64 - Visual Studio 2019 (v142) cpp20 Debug|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x64 - Visual Studio 2019 (v142) cpp20 Release|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
:::::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x64 - Visual Studio 2019 (v142) cpplatest Debug|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x64 - Visual Studio 2019 (v142) cpplatest Release|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::
:::::: Visual Studio 2017 (v141)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<PlatformToolset>v142</PlatformToolset>', '<PlatformToolset>v141</PlatformToolset>' | Out-File %%i"
::)
:::::::: ISO C++17 Standard (/std:c++17)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x64 - Visual Studio 2017 (v141) cpp17 Debug|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x64 - Visual Studio 2017 (v141) cpp17 Release|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
:::::::: ISO C++20 Standard (/std:c++20)
:::::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x64 - Visual Studio 2017 (v141) cpplatest Debug|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x64 - Visual Studio 2017 (v141) cpplatest Release|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::
:::::: Visual Studio 2017 - Windows XP (v141_xp)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<PlatformToolset>v141</PlatformToolset>', '<PlatformToolset>v141_xp</PlatformToolset>' | Out-File %%i"
::)
:::::::: ISO C++17 Standard (/std:c++17)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x64 - Visual Studio 2017 - Windows XP (v141_xp) cpp17 Debug|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x64 - Visual Studio 2017 - Windows XP (v141_xp) cpp17 Release|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
:::::::: ISO C++20 Standard (/std:c++20)
:::::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x64 - Visual Studio 2017 - Windows XP (v141_xp) cpplatest Debug|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x64 - Visual Studio 2017 - Windows XP (v141_xp) cpplatest Release|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::
:::::: LLVM (clang-cl)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<PlatformToolset>v141_xp</PlatformToolset>', '<PlatformToolset>ClangCL</PlatformToolset>' | Out-File %%i"
::)
:::::::: ISO C++17 Standard (/std:c++17)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x64 - LLVM (clang-cl) cpp17 Debug|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x64 - LLVM (clang-cl) cpp17 Release|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
:::::::: ISO C++20 Standard (/std:c++20)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x64 - LLVM (clang-cl) cpp20 Debug|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x64 - LLVM (clang-cl) cpp20 Release|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
:::::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x64 - LLVM (clang-cl) cpplatest Debug|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Debug|x64" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x64 - LLVM (clang-cl) cpplatest Release|x64" >> build.log
::%vsdev% %slnfile% /rebuild "Release|x64" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::
::call clean.bat
::
::cmake -A Win32 . -B build
::
::xcopy %curdir%\3rd\openssl\prebuilt\windows\x86 %curdir%\bin\x86 /E /I /Y
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::
:::: x86
:::::: Visual Studio 2022 (v143)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<PlatformToolset>v143</PlatformToolset>', '<PlatformToolset>v143</PlatformToolset>' | Out-File %%i"
::)
:::::::: ISO C++17 Standard (/std:c++17)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x86 - Visual Studio 2022 (v143) cpp17 Debug|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Debug" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x86 - Visual Studio 2022 (v143) cpp17 Release|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Release" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
:::::::: ISO C++20 Standard (/std:c++20)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x86 - Visual Studio 2022 (v143) cpp20 Debug|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Debug" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x86 - Visual Studio 2022 (v143) cpp20 Release|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Release" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
:::::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x86 - Visual Studio 2022 (v143) cpplatest Debug|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Debug" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x86 - Visual Studio 2022 (v143) cpplatest Release|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Release" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::
:::::: Visual Studio 2019 (v142)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<PlatformToolset>v143</PlatformToolset>', '<PlatformToolset>v142</PlatformToolset>' | Out-File %%i"
::)
:::::::: ISO C++17 Standard (/std:c++17)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x86 - Visual Studio 2019 (v142) cpp17 Debug|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Debug" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x86 - Visual Studio 2019 (v142) cpp17 Release|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Release" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
:::::::: ISO C++20 Standard (/std:c++20)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x86 - Visual Studio 2019 (v142) cpp20 Debug|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Debug" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x86 - Visual Studio 2019 (v142) cpp20 Release|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Release" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
:::::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x86 - Visual Studio 2019 (v142) cpplatest Debug|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Debug" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x86 - Visual Studio 2019 (v142) cpplatest Release|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Release" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::
:::::: Visual Studio 2017 (v141)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<PlatformToolset>v142</PlatformToolset>', '<PlatformToolset>v141</PlatformToolset>' | Out-File %%i"
::)
:::::::: ISO C++17 Standard (/std:c++17)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x86 - Visual Studio 2017 (v141) cpp17 Debug|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Debug" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x86 - Visual Studio 2017 (v141) cpp17 Release|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Release" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
:::::::: ISO C++20 Standard (/std:c++20)
:::::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x86 - Visual Studio 2017 (v141) cpplatest Debug|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Debug" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x86 - Visual Studio 2017 (v141) cpplatest Release|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Release" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::
:::::: Visual Studio 2017 - Windows XP (v141_xp)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<PlatformToolset>v141</PlatformToolset>', '<PlatformToolset>v141_xp</PlatformToolset>' | Out-File %%i"
::)
:::::::: ISO C++17 Standard (/std:c++17)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x86 - Visual Studio 2017 - Windows XP (v141_xp) cpp17 Debug|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Debug" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x86 - Visual Studio 2017 - Windows XP (v141_xp) cpp17 Release|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Release" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
:::::::: ISO C++20 Standard (/std:c++20)
:::::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x86 - Visual Studio 2017 - Windows XP (v141_xp) cpplatest Debug|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Debug" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x86 - Visual Studio 2017 - Windows XP (v141_xp) cpplatest Release|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Release" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::
:::::: LLVM (clang-cl)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<PlatformToolset>v141_xp</PlatformToolset>', '<PlatformToolset>ClangCL</PlatformToolset>' | Out-File %%i"
::)
:::::::: ISO C++17 Standard (/std:c++17)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpplatest</LanguageStandard>', '<LanguageStandard>stdcpp17</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x86 - LLVM (clang-cl) cpp17 Debug|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Debug" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x86 - LLVM (clang-cl) cpp17 Release|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Release" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
:::::::: ISO C++20 Standard (/std:c++20)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp17</LanguageStandard>', '<LanguageStandard>stdcpp20</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x86 - LLVM (clang-cl) cpp20 Debug|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Debug" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x86 - LLVM (clang-cl) cpp20 Release|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Release" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
:::::::: Preview - Features from the Latest C++ Working Draft (/std:c++latest)
::for /f %%i in ('dir /b /s /a:-d *.vcxproj') do (
::  powershell -Command "(gc %%i) -replace '<LanguageStandard>stdcpp20</LanguageStandard>', '<LanguageStandard>stdcpplatest</LanguageStandard>' | Out-File %%i"
::)
::echo "configure: x86 - LLVM (clang-cl) cpplatest Debug|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Debug" /out build.log
::ctest --output-on-failure -C Debug --test-dir build >> build.log
::echo "configure: x86 - LLVM (clang-cl) cpplatest Release|x86" >> build.log
::%vsdev% %slnfile% /rebuild "Release" /out build.log
::ctest --output-on-failure -C Release --test-dir build >> build.log
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::
::call clean.bat
::
::echo "configure: x64 - WSL cpp17 Debug|x64" >> build.log
::
::WSL -e cmake -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_CXX_STANDARD=17 -S /mnt/d/asio2/ -B /mnt/d/asio2/build
::
::WSL -e cmake --build /mnt/d/asio2/build -- -k >> build.log 2>&1
::
::WSL -e ctest --output-on-failure -C Debug --test-dir /mnt/d/asio2/build >> build.log
::
::
::call clean.bat
::
::echo "configure: x64 - WSL cpp17 Release|x64" >> build.log
::
::WSL -e cmake -DCMAKE_BUILD_TYPE:STRING="Release" -DCMAKE_CXX_STANDARD=17 -S /mnt/d/asio2/ -B /mnt/d/asio2/build
::
::WSL -e cmake --build /mnt/d/asio2/build -- -k >> build.log 2>&1
::
::WSL -e ctest --output-on-failure -C Release --test-dir /mnt/d/asio2/build >> build.log
::
::
::call clean.bat
::
::echo "configure: x64 - WSL cpp20 Debug|x64" >> build.log
::
::WSL -e cmake -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_CXX_STANDARD=20 -S /mnt/d/asio2/ -B /mnt/d/asio2/build
::
::WSL -e cmake --build /mnt/d/asio2/build -- -k >> build.log 2>&1
::
::WSL -e ctest --output-on-failure -C Debug --test-dir /mnt/d/asio2/build >> build.log
::
::
::call clean.bat
::
::echo "configure: x64 - WSL cpp20 Release|x64" >> build.log
::
::WSL -e cmake -DCMAKE_BUILD_TYPE:STRING="Release" -DCMAKE_CXX_STANDARD=20 -S /mnt/d/asio2/ -B /mnt/d/asio2/build
::
::WSL -e cmake --build /mnt/d/asio2/build -- -k >> build.log 2>&1
::
::WSL -e ctest --output-on-failure -C Release --test-dir /mnt/d/asio2/build >> build.log


:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

call clean.bat

echo "configure: x64 - ndk example cpp17 Release|x64" >> build.log

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_OPTIM := release', 'APP_OPTIM := release') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

call ndk-build -C %curdir%/example/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

echo "configure: x64 - ndk example cpp17 Debug|x64" >> build.log

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_OPTIM := release', 'APP_OPTIM := debug') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

call ndk-build -C %curdir%/example/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

echo "configure: x64 - ndk example cpp20 Debug|x64" >> build.log

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++20') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_OPTIM := debug', 'APP_OPTIM := debug') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

call ndk-build -C %curdir%/example/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

echo "configure: x64 - ndk example cpp20 Release|x64" >> build.log

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++20', 'APP_CPPFLAGS +=-std=c++20') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_OPTIM := debug', 'APP_OPTIM := release') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

call ndk-build -C %curdir%/example/ndk/jni -k >> %curdir%\build.log 2>&1

powershell -Command "(gc %curdir%\example\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++20', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\example\ndk\jni\Application.mk"

:: ----------------------------------------------

call clean.bat

echo "configure: x64 - ndk test cpp17 Release|x64" >> build.log

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_OPTIM := release', 'APP_OPTIM := release') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"

call ndk-build -C %curdir%/test/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

echo "configure: x64 - ndk test cpp17 Debug|x64" >> build.log

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_OPTIM := release', 'APP_OPTIM := debug') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"

call ndk-build -C %curdir%/test/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

echo "configure: x64 - ndk test cpp20 Debug|x64" >> build.log

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++17', 'APP_CPPFLAGS +=-std=c++20') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_OPTIM := debug', 'APP_OPTIM := debug') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"

call ndk-build -C %curdir%/test/ndk/jni -k >> %curdir%\build.log 2>&1

call clean.bat

echo "configure: x64 - ndk test cpp20 Release|x64" >> build.log

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++20', 'APP_CPPFLAGS +=-std=c++20') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"
powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_OPTIM := debug', 'APP_OPTIM := release') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"

call ndk-build -C %curdir%/test/ndk/jni -k >> %curdir%\build.log 2>&1

powershell -Command "(gc %curdir%\test\ndk\jni\Application.mk).replace('APP_CPPFLAGS +=-std=c++20', 'APP_CPPFLAGS +=-std=c++17') | Out-File -Encoding Ascii %curdir%\test\ndk\jni\Application.mk"


powershell -Command "(gc D:\asio2\include\asio2\config.hpp).replace('#define ASIO_NO_EXCEPTIONS', '//#define ASIO_NO_EXCEPTIONS') | Out-File -Encoding Ascii D:\asio2\include\asio2\config.hpp"

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

::powershell -Command "(gc D:\asio2\test\unit\unit_test.hpp).replace('static const int   test_loop_times = 1;', 'static const int   test_loop_times = 100;') | Out-File -Encoding Ascii D:\asio2\test\unit\unit_test.hpp"

