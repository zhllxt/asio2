openssl的编译方法可以直接看下载的openssl包中的INSTALL文件,不管是想编译成静态库动态库等等都可以通过该文件的说明自己去编译适合自己所用的.

## Windows下编译openssl步骤

安装ActivePerl http://www.activestate.com/activeperl/downloads/
如果安装目录是C:\Perl64\ 将perl的bin路径放到电脑的环境变量PATH
安装nasm https://www.nasm.us/
如果安装目录是C:\Program Files\NASM 将C:\Program Files\NASM路径放到电脑的环境变量PATH

1.到https://github.com/openssl/openssl下载openssl源码,比如下载的源码包为openssl-OpenSSL_1_1_1h.zip,解压缩
2.打开 适用于 VS 2017 的 x64 本机工具命令提示(如果要编译32位的则打开VS 2017的开发人员命令提示符即可),进入到解压缩的openssl文件夹中
3.perl Configure VC-WIN64A no-shared
4.nmake
5.nmake test
6.nmake install
7.以上编译步骤实际参考自下载的源码包中的openssl/INSTALL文件

附:32位编译方法和上面步骤相同,只不过某些编译参数不同而已,如perl Configure VC-WIN32 no-shared

## Linux下编译openssl步骤

看INSTALL文件即可,很简单,只需要注意编译静态库时加上no-shared选项,如./config no-shared
./Configure no-shared
make
make test
make install
以上编译步骤实际参考自下载的源码包中的openssl/INSTALL文件中的Building OpenSSL段落和Installing OpenSSL段落

## Arm下的openssl交叉编译步骤

1.下载arm gcc编译器
  打开https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-a/downloads
  页面上显示有GNU-A和GNU-RM
  下载GNU-A这个页面里面的arm gcc编译器:gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf.tar.xz
  如果下载的其它的编译器包,在编译时可能会提示无法识别的指令"-pthread",这里你到这个包里面搜索一下,看有没有pthread的静态库,如果没有,那这个包可能就不能用
  另:名字中带有none-linux的包似乎是只能编译操作系统内核不能编译application 所以优先找那些不带none-linux的下载
2.解压gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf.tar.xz包到某个文件夹,如/usr/local/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf
3.vim /etc/profile (将把交叉编译工具链的路径添加到系统环境变量PATH中去)
4.在profile中最后一行添加 export PATH=$PATH:/usr/local/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin
5.重启系统
6.arm-linux-gnueabihf-gcc -v 查看版本,如果显示正常说明arm gcc编译器安装成功
7.下载并解压openssl-OpenSSL_1_1_1h.zip包
8.进入到解压后的目录中,执行命令
  ./config no-asm no-shared --api=0.9.8 --prefix=/opt/openssl --openssldir=/usr/local/ssl CROSS_COMPILE=arm-linux-gnueabihf- CC=gcc
  命令中的--api=0.9.8表示编译后老版本中已废弃的API仍然可以使用,如果不加这个选项,可能在使用这些库时会提示报错
  如果提示无法识别的指令"-m64",那就编辑目录下的Makefile文件,找到所有-m64选项然后直接删掉即可
  源码头文件添加(在包含openssl头文件之前)#define OPENSSL_API_COMPAT 0x00908000L
9.make
10.make install
11.编译其它代码的指令和标准linux下的gcc编译指令一样,如
// 编译
arm-linux-gnueabihf-g++ -c -x c++ main.cpp -I /usr/local/include  -I /opt/openssl/include -g2 -gdwarf-2 -o main.o -Wall -Wswitch -W"no-deprecated-declarations" -W"empty-body" -Wconversion -W"return-type" -Wparentheses -W"no-format" -Wuninitialized -W"unreachable-code" -W"unused-function" -W"unused-value" -W"unused-variable" -O3 -fno-strict-aliasing -fno-omit-frame-pointer -fthreadsafe-statics -fexceptions -frtti -std=c++17
// 链接
arm-linux-gnueabihf-g++ -o main.out -Wl,--no-undefined -Wl,-L/opt/openssl/lib -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -pthread -lrt -ldl -Wl,-rpath=. main.o -lstdc++fs -l:libssl.a -l:libcrypto.a
附:经过测试,用此编译器编译的可执行文件在树莓派4下可以正常运行,可用关键字"树莓派交叉编译"来查询相关结果
