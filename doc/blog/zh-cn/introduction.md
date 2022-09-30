# 基于c++和asio的网络编程框架asio2教程基础篇：1、基本概念和使用说明
由于asio2没有写技术文档，因此打算写几篇文章介绍一下如何使用它，主要是针对新手。
## 1、asio2如何使用？
 - asio2这个框架的使用很简单，以VS2017举例：**在VS2017的项目上点右键 - 属性 - C/C++ - 常规 - 附加包含目录，将asio2-master/3rd和asio2-master/include这两个目录添加进去（假定你在github下载的是master分支的压缩包），然后在你的源代码中#include <asio2/asio2.hpp>即可开始使用了**
 - 在/asio2/example目录下有大量的示例代码，支持cmake，可使用cmake生成visual studio的解决方案后，直接用visual studio打开去看示例即可。
 - 关于asio库的使用方法，网上能搜到大量的文章和代码，这里不介绍了。我主要是通过看boost\libs\asio\example下的官方示例来学习asio的。　　
## 2、asio2是什么？
 - asio2是在C++17基础上，基于asio库进行二次封装，编写的一个网络编程的框架，目的是想降低对asio本身使用的难度。
## 3、为什么不直接使用asio?为什么要做asio2?
 - 因为asio“是一个库，而不是一个框架”，直接使用asio库的话，就要使用它的很多的API，要处理很多细节，所以这里才进行二次包装，又做了一个“框架”，目的就是把他的各种API再包装一遍，使用起来更简单，代码量大大减少，否则直接使用asio要考虑的细节就会多很多。
## 4、直接使用asio和使用asio2有什么区别？
 - 我觉得区别主要还是在于使用asio的话需要自己去做这些工作：“在程序退出时，连接如何正常关闭，如何保证未发送完的数据一定是在发送之后程序才会退出”，也就是说程序如何优雅的退出这个问题。
 - 看起来这个功能似乎不难做，但还是花了我大量时间去思考和试验。使用asio2的话就不需要再考虑这些问题了，比如使用tcp，那么构造一个tcp_server对象，然后直接调用tcp_server.start()启动服务，退出时直接调用tcp_server.stop()关闭服务即可，所有的连接正常关闭，数据发送，资源释放等这些问题，都不需要你再去考虑了。在tcp_server.stop()函数结束之前，会确保上面的这些问题完美处理之后，stop()函数才会结束的（stop()函数是阻塞的，假如有1千个连接，会一直阻塞到这个1千个连接都正常关闭了才会结束阻塞）。
## 5、asio2有什么优点？
 - 除了上面第2条所说的优雅的退出之外，其它的优点大概有以下这些：
 - 接口比较简单，主要有start-启动服务,stop-停止服务,bind_recv-绑定接收数据的回调函数,bind_connect-绑定连接成功的回调函数，......等等。
 - 支持tcp,udp,http,websocket,rpc,ssl,串口等，而且形成了统一的接口，也就是这些组件的接口函数基本都一样。
 - header-only，不需要单独去编译，只要包含一下头文件，就可以用了。（我个人对非header only而是需要编译的库深恶痛绝，跟非header-only的库需要配置各种编译选项等一系列问题相比，我觉得header-only的编译慢点完全不是问题）
## 6、asio2有什么缺点？
 - 只支持C++，且只支持C++17以上(包含C++17)。
 - 纯异步的，对同步支持非常不好。（asio2这个框架只针对大部分的通用情形去考虑的）
## 7、asio2的实现思路
 - 大致思路就是使用C++的CRTP模板技术，和多重派生，使用多个小模块拼接组合成一个完整组件的方式（看代码中tcp,udp,http,websocket,rpc,ssl这些组件都是通过继承多个类来完成的），减少代码量。
 - 相比virtual，使用CRTP的好处是效率更高一些，缺点是源码看起来更晦涩难懂了。
 - 关于CRTP编译期多态和virtual运行期多态的效率可以看看这个评测：[https://eli.thegreenplace.net/2013/12/05/the-cost-of-dynamic-virtual-calls-vs-static-crtp-dispatch-in-c/
](https://eli.thegreenplace.net/2013/12/05/the-cost-of-dynamic-virtual-calls-vs-static-crtp-dispatch-in-c/)
## 8、关于asio2的头文件路径包含的详细说明
解压github下载的asio2压缩包之后，里面有3rd,include,test,example等文件夹，其中3rd文件夹包含了asio,cereal,fmt等一些开源库，asio2的代码需要用到这些开源库；include文件夹里面又包含了一个asio2文件夹，这个asio2文件夹是真正的asio2的相关代码；test文件夹是性能测试和单元测试相关的代码；example文件夹是各种使用示例代码；

最新版本的asio2代码可以使用cmake直接生成vs的解决方案等，打开vs解决方案即可直接编译，不需要任何其它额外设置（至于cmake如何使用，不会的话自己网络搜索）。

如果你对vs项目的头文件路径包含不熟悉，你可以直接参考cmake生成的vs解决方案里的头文件路径包含是如何设置的。

通常，如果你建立了自己的项目的vs解决方案（而不是使用的asio2里的cmake生成的解决方案），那么你需要在你的vs头文件路径包含中添加以下两项（假定你在github下载的是master分支的压缩包）：asio2-master/3rd和asio2-master/include

如果你对项目头文件路径包含很熟悉，比如你有你自己的第三方库目录（比如你习惯把你自己用到的第三方库都放在这个目录下），那么你只需要把asio2-master/3rd里面的全部文件（是asio2-master/3rd里面的文件而不是asio2-master/3rd这个目录本身），和asio2-master/include里面的全部文件，放到你自己的第三方库目录即可。
## 9、在DLL中使用asio2的注意事项
#### 1、不能在Windows DLL中直接声明一个asio2的server或client对象，会导致死锁。

比如像下面这样在dllmain.cpp中直接声明了一个asio2全局对象：

```cpp
asio2::tcp_client client;
```

这就会导致在DLL的入口函数DllMain中出现死锁，死锁的原因是std::thread引起的，参考asio2/util/thread_pool.hpp开头的说明

需要用下面的方式，即声明一个指针，然后自己在DLL中做一个导出函数如Init()，在EXE中手动调用你的DLL中的导出函数Init();
然后在Init()中创建指针。如：


```cpp
// 在dllmain.cpp先声明一个全局的指针对象
std::shared_ptr<asio2::tcp_client> client;
```


```cpp
// 这是你dll中的导出函数Init
void Init()
{
    client = std::make_shared<asio2::tcp_client>();
}
```

// 然后在你的EXE中手动调用这个Init导出函数即可。

#### 2、不能在Windows Dll的DLL_PROCESS_DETACH块中调用asio2的server或client对象的stop函数，会导致stop函数永远阻塞无法返回。
原因是由于在DLL_PROCESS_DETACH时，通过PostQueuedCompletionStatus投递的IOCP事件永远得不到执行。

解决办法依然是自己在DLL中做一个导出函数如Uninit()，在EXE中手动调用你的DLL中的导出函数Uninit();在Uninit函数中调用对象的stop函数；如：
```cpp
// 这是你dll中的导出函数Uninit
void Uninit()
{
    client->stop();
}
```
## 项目地址：
github : [https://github.com/zhllxt/asio2](https://github.com/zhllxt/asio2)
码云 : [https://gitee.com/zhllxt/asio2](https://gitee.com/zhllxt/asio2)

### 最后编辑于2022-06-23
