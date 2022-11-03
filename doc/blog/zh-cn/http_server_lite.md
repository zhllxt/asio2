
# 使用asio2开发一个简易的http server
大约200行代码开发一个简易的静态的http server，同时支持http和https，源代码地址：[https://github.com/zhllxt/http_server_lite](https://github.com/zhllxt/http_server_lite)

下载该项目后，打开bin目录里的http_server_lite.exe，然后在你的浏览器输入[http://localhost](http://localhost)可以测试一下效果，实际上和这个网址里的效果是一样的：[https://technext.github.io/soffer/](https://technext.github.io/soffer/)

这个项目是用cmake构建的，跨平台，支持windows,macos,linux,arm,android等，所以只要了解基本的cmake用法，就可以编译你想要的平台下的http server了。github的仓库里只包含了windows下的可执行程序。

下面说一下项目的大致逻辑：

## 首先是配置文件
配置文件用的是json格式，可以配置多个站点，每个站点使用不同的端口，如下：

```cpp
[
  // 第1个站点配置
  {
    "protocol": "http", // 协议类型， http 还是 https
    "host": "0.0.0.0",  // http server 监听的ip地址
    "port": 80,         // http server 监听的端口号
    "path": "./wwwroot",   // 站点所在的文件夹，可以是相对路径，也可以是绝对路径
    "index": "index.html"  // 默认的页面文件
  },
  // 第2个站点配置
  {
    "protocol": "https",
    "host": "0.0.0.0",
    "port": 443,
    "path": "D:/website/main",
    "index": "index.html",
    "cert_file": "D:/website/main/cert/www.xxx.com.pem", // ssl证书文件
    "key_file": "D:/website/main/cert/www.xxx.com.key"   // ssl证书文件
  },
  // 第3个站点配置
  {
    "protocol": "http",
    "host": "0.0.0.0",
    "port": 8080,
    "path": "D:/website/work",
    "index": "index.html"
  }
  // 第n......个站点配置
]
```

## 具体代码
```cpp
#define ASIO2_ENABLE_SSL // 添加这个宏定义来让asio2支持ssl

#include <asio2/http/http_server.hpp>
#include <asio2/http/https_server.hpp>
#include <asio2/external/fmt.hpp>
#include <asio2/external/json.hpp>
#include <asio2/util/string.hpp>
#include <fstream>

int main()
{
	// 使用nlohmann::json 这个库用来处理json非常方便
	using json = nlohmann::json;

	// 打开配置文件
	std::ifstream file("config.json", std::ios::in | std::ios::binary);
	if (!file)
	{
		fmt::print("open the config file 'config.json' failed.\n");
		return 0;
	}
	
	// 构造一个json对象，用来解析配置文件
	json cfg;
	try
	{
		cfg = json::parse(file);
	}
	catch (json::exception const& e)
	{
		fmt::print("load the config file 'config.json' failed: {}\n", e.what());
		return 0;
	}

	// 判断一下，配置文件的内容必须是一个数组类型的
	if (!cfg.is_array())
	{
		fmt::print("the content of the config file 'config.json' is incorrect.\n");
		return 0;
	}

	// 创建一个io线程池，为什么要使用io线程池？
	// 因为asio2的每一个http server对象默认会启动cpu*2个数量的线程，假如cpu是4核，那就是
	// 启动了8个线程，如果配置文件中配置了10个站点，那就会启动10个http server，那总共就开
	// 启了80个线程，这显然不够合理。所以这里使用一个线程池，iopool线程池默认也是启动cpu*2
	// 个数量的线程，后面会把这个iopool线程池传入到每个http server中，这样不管有多少个
	// server，都共用这一个线程池即可。
	asio2::iopool iopool;

	// 必须先调用iopool的启动函数，否则后面在调用server.start时会阻塞住无法结束。
	iopool.start();

	// 用来记录总共启动了多少个http server
	int server_count = 0;

	// 构造一个lambda函数，不管是http server还是https server，后面都调用这个lambda函数来
	// 做相关的server设置和server启动操作
	auto start_server = [&server_count](auto server,
		std::string host, std::uint16_t port, std::string path, std::string index) mutable
	{
		// 设置http server的站点的文件夹
		// 这里变量server是一个shared_ptr类型的智能指针
		server->set_root_directory(path);

		// 设置回调函数，这里设置的是server的启动回调函数，可以在这里判断启动成功还是失败。
		// 注意这里把server对象捕获到lambda函数里用的是原始指针，不能把智能指针本身捕获
		// 到lambda函数里，会造成循环引用。因为框架已经保证了，当进到回调函数里时，server
		// 对象一定是存在的，所以这里用原始指针没有什么问题。当然，仅从代码的表面的合理
		// 性来看，用weak_ptr更符合代码规范(新手如果不太理解这段话，先放一边不用纠结)。
		server->bind_start([host, port, &server_count, pserver = server.get()]() mutable
		{
			// 启动失败，打印失败信息
			if (asio2::get_last_error())
				fmt::print("start http server failure : {} {} {}\n",
					host, port, asio2::last_error_msg());
			else
			{
				// 启动成功
				server_count++;
				fmt::print("start http server success : {} {}\n",
					pserver->listen_address(), pserver->listen_port());
			}
		}).bind_stop([host, port]()
		{
			fmt::print("stop http server success : {} {} {}\n",
				host, port, asio2::last_error_msg());
		});

		// 绑定http处理程序，这里绑定是的"/"，意思就是如果用户在浏览器中打开的是
		// http://www.yousite.com这样的网址，那么我们的http server收到的url请求的
		// 目标字符串实际上就是"/"，此时我们就给他返回变量index对应的文件即可，
		// 注意index是个字符串变量，是从config.json配置文件中读取出来的。
		// 另外，这里的bind函数是一个模板函数，默认会对GET和POST的请求都作出响应。
		server->bind("/", [index](http::web_request& req, http::web_response& rep)
		{
			asio2::ignore_unused(req, rep);

			// fill_file表示用一个文件来响应http请求
			// 后面框架就会自动使用这个rep来回复对方。
			rep.fill_file(index);
		});

		// 绑定星号通配符，这个意思是除了上面绑定的"/"请求之外，其它所有的请求都
		// 由这个回调函数来处理，这个处理比较简单，我们把请求utl的目标取出来(即
		// req.target())，并把这个目标当成一个文件名(因为这里我们只是一个静态的
		// http server，所以就把http请求全部当成是请求文件就行了)，然后调用
		// response的fill_file函数，并把文件名传进去即可。
		server->bind("*", [](http::web_request& req, http::web_response& rep)
		{
			rep.fill_file(req.target());
		});

		// 启动server
		return server->start(host, port);
	};

	// 遍历json数组
	for (auto& site : cfg)
	{
		try
		{
			// 解析json对象
			std::string protocol = site["protocol"].get<std::string>();
			std::string     host = site["host"].get<std::string>();
			std::uint16_t   port = site["port"].get<std::uint16_t>();
			std::string     path = site["path"].get<std::string>();
			std::string    index = site["index"].get<std::string>();

			if (protocol.empty())
			{
				fmt::print("Must specify protocol.\n");
				continue;
			}

			if (host.empty())
			{
				host = "0.0.0.0";
				fmt::print("The host is empty, will use {} to instead.\n", host);
			}

			if (path.empty())
			{
				fmt::print("Must specify path.\n");
				continue;
			}

			if (index.empty())
			{
				index = "index.html";
				fmt::print("The index is empty, will use {} to instead.\n", index);
			}

			if /**/ (asio2::iequals(protocol, "http"))
			{
				// 创建http server，在构造函数中传入了iopool线程池
				std::shared_ptr<asio2::http_server> server = std::make_shared<asio2::http_server>(iopool);
				// 设置并启动http server
				start_server(server, host, port, path, index);
				// 注意：这里创建了http server对象的智能指针之后，并没有保存它，当代码
				// 执行到这里，似乎server 的生命周期就结束了，这会不会有问题呢？不会。
				// 当给server传入了iopool参数，并调用了server.start之后，实际上
				// server对象的智能指针已经被iopool所捕获了，所以后面程序结束前一定要
				// 调用iopool.stop，否则这些server对象就不会被正确的释放。
			}
			else if (asio2::iequals(protocol, "https"))
			{
				std::string cert_file = site["cert_file"].get<std::string>();
				std::string key_file = site["key_file"].get<std::string>();

				if (cert_file.empty())
				{
					fmt::print("Must specify cert_file.\n");
					continue;
				}
				if (key_file.empty())
				{
					fmt::print("Must specify key_file.\n");
					continue;
				}

				// 创建https server，在构造函数中传入了ssl版本和iopool线程池
				std::shared_ptr<asio2::https_server> server =
					std::make_shared<asio2::https_server>(asio::ssl::context::sslv23, iopool);
				try
				{
					// 绑定ssl证书
					// 阿里云可以申请免费的ssl证书，有兴趣可以白嫖一下，试试把自己的站点加上
					// ssl支持，阿里云的下载证书那里点下载“其它”即可，下载的压缩包里有个pem
					// 结尾的文件和key结尾的文件，在config.json文件里配置一下就行了。
					server->use_certificate_chain_file(cert_file);
					server->use_private_key_file(key_file, asio::ssl::context::pem);
					server->use_tmp_dh_file(cert_file);
				}
				catch (asio::system_error const& e)
				{
					fmt::print("start http server failure : {} {}, load ssl certificate file failed : {}\n",
						host, port, e.what());
					continue;
				}
				// 设置并启动http server
				start_server(server, host, port, path, index);
			}
			else
			{
				fmt::print("start http server failure : {} {}, invalid protocol: {}\n", host, port, protocol);
			}
		}
		catch (json::exception const& e)
		{
			fmt::print("read the config file 'config.json' failed: {}\n", e.what());
		}
	}

	if (server_count == 0)
	{
		// 如果没有配置站点或server全部都启动失败了，这里打印信息后程序就结束了
		fmt::print("No available http server was found in the config file.\n");
	}
	else
	{
		// 否则就阻塞住主进程，直到收到SIGINT或SIGTERM信号才会退出程序。
		// 通常按ctrl + c会触发SIGINT信号，在linux系统下可以用命令
		// kill -s SIGINT PID向此进程发送SIGINT信号来优雅的关闭此进程，
		// PID是此进程的ID号，可以用ps -aux | grep "http"这种命令来
		// 查询一下此进程的PID。
		iopool.wait_signal(SIGINT, SIGTERM);
	}

	// 程序退出之前一定要调用iopool.stop
	iopool.stop();

	fmt::print("progress exited.\n");

	return 0;
}

```

### 最后编辑于：2022-11-04
