@echo off

del /f /s /q *.vcxproj
del /f /s /q *.vcxproj.filters
del /f /s /q *.vcxproj.user
del /f /s /q *.sln
del /f /s /q cmake_install.cmake
del /f /s /q CMakeCache.txt
del /f /s /q *.vsp
del /f /s /q *.psess
del /f /s /q *.ninja
del /f /s /q *.ninja_log
del /f /s /q *.ninja_deps
del /f /s /q *.log.tmp*
del /f /s /q Makefile
del /f /s /q CTestTestfile.cmake

rmdir /s /q Testing
rmdir /s /q CMakeFiles
rmdir /s /q bin
rmdir /s /q x64
rmdir /s /q x86
rmdir /s /q Win32
rmdir /s /q ARM64
rmdir /s /q ARM
rmdir /s /q Debug
rmdir /s /q Release
rmdir /s /q out
rmdir /s /q obj
rmdir /s /q build
rmdir /s /q cmake-build-debug
rmdir /s /q cmake-build-release

rmdir /s /q example\CMakeFiles
rmdir /s /q example\bin
rmdir /s /q example\x64
rmdir /s /q example\x86
rmdir /s /q example\Win32
rmdir /s /q example\ARM64
rmdir /s /q example\ARM
rmdir /s /q example\Debug
rmdir /s /q example\Release
rmdir /s /q example\out
rmdir /s /q example\obj
rmdir /s /q example\build
rmdir /s /q example\cmake-build-debug
rmdir /s /q example\cmake-build-release

rmdir /s /q test\CMakeFiles
rmdir /s /q test\bin
rmdir /s /q test\x64
rmdir /s /q test\x86
rmdir /s /q test\Win32
rmdir /s /q test\ARM64
rmdir /s /q test\ARM
rmdir /s /q test\Debug
rmdir /s /q test\Release
rmdir /s /q test\out
rmdir /s /q test\obj
rmdir /s /q test\build
rmdir /s /q test\cmake-build-debug
rmdir /s /q test\cmake-build-release

rmdir /s /q example\ndk\libs
rmdir /s /q example\ndk\obj

rmdir /s /q test\ndk\libs
rmdir /s /q test\ndk\obj


call :DeleteFolders1 "%cd%" "CMakeFiles"

call :DeleteFolders1 "%cd%\example" "Debug"
call :DeleteFolders1 "%cd%\example" "Relase"
call :DeleteFolders1 "%cd%\example" "cmake-build-debug"
call :DeleteFolders1 "%cd%\example" "cmake-build-release"

call :DeleteFolders1 "%cd%\test" "Debug"
call :DeleteFolders1 "%cd%\test" "Relase"
call :DeleteFolders1 "%cd%\test" "cmake-build-debug"
call :DeleteFolders1 "%cd%\test" "cmake-build-release"

call :DeleteFolders2 "%cd%\example" ".dir"
call :DeleteFolders2 "%cd%\example" ".tlog"

call :DeleteFolders2 "%cd%\test" ".dir"
call :DeleteFolders2 "%cd%\test" ".tlog"

exit /b

:DeleteFolders1
for /d %%i in ("%~1\*") do (
    if "%%~nxi"=="%~2" (
        echo Deleting "%%i"
        rmdir /S /Q "%%i"
    ) else (
        call :DeleteFolders1 "%%i" "%~2"
    )
)
exit /b

:DeleteFolders2
for /d %%i in ("%~1\*") do (
    echo "%%~nxi" | find /i "%~2" > nul
    if not errorlevel 1 (
        echo Deleting "%%i"
        rmdir /S /Q "%%i"
    ) else (
        call :DeleteFolders2 "%%i" "%~2"
    )
)
exit /b
