# 1. Install Android NDK (my installed version is android-ndk-r22b)
# 2. Open cmd.exe, enter this directory(\asio2\example\ndk\jni\), run "ndk-build"

# all  arm64-v8a   armeabi-v7a  x86  x86_64
# APP_ABI :=  arm64-v8a   

APP_PLATFORM := android-16

APP_STL += c++_static

APP_CPPFLAGS += -fexceptions -frtti

APP_CPPFLAGS +=-std=c++17

APP_OPTIM := release
