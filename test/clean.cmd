del /f /s /q *.vcxproj
del /f /s /q *.vcxproj.filters
del /f /s /q *.vcxproj.user
del /f /s /q *.sln
del /f /s /q cmake_install.cmake
del /f /s /q CMakeCache.txt
del /f /s /q *.vsp
del /f /s /q *.psess

rmdir /s /q .vs
rmdir /s /q asio2_test_project
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
rmdir /s /q .idea

rmdir /s /q ndk\libs
rmdir /s /q ndk\obj
