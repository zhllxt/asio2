# 基于c++和asio的网络编程框架asio2教程基础篇：4、使用tcp客户端发送数据时,如何同步获取服务端返回结果

## 问题描述
```cpp
asio2::tcp_client client;

client.bind_recv([&](std::string_view data)
{
	// 第2步，会在这里收到服务端返回的数据
		
	// 发送数据和接收数据完全在两个位置，用户无法在发送数据的代码处直接获取到服务端返回的数据
});

client.start(host, port, "\r\n");

// 第1步，在这里发送数据
client.send("get_user\r\n");
```
我见过的异步网络框架的发送数据和接收数据的处理都和上面的逻辑类似，这种流程有个问题是用户没有办法在发送数据的代码的地方，直接得到远程返回的结果数据。

大家可能都听过rpc这个词，rpc就是“远程过程调用”了，rpc的协议通常都是私有的，规定好的协议。所以，如果用户的协议是自定义的，那rpc通常就也不能用了。

asio2框架从2.7版本后解决了这个问题，我给这个功能取了个名字叫rdc，即remote data call，即远程数据调用，可以解决各种用户自定义协议下的数据同步形式的获取。这个功能对于一些简单的通信场景，比如发一条消息后紧接着收一条消息，还是很有用的。

使用步骤详细描述如下：

## 最基础的使用示例
##### 服务端代码
```cpp
// 假定你的协议格式为  id,content
// 协议全部采用字符串形式，比如 1,get_user\r\n
// 其中1表示唯一id    get_user表示内容  id和内容之间用逗号隔开  
// 注意字符串的结尾要加上\r\n表示按\r\n进行拆包

asio2::tcp_server server;

server.bind_recv([&](auto & session_ptr, std::string_view s)
{
	// 查找数据中的逗号
	auto pos = s.find(',');

	// 如果找到逗号了
	if (pos != std::string_view::npos)
	{
		// 取出协议的内容
		// 如果内容是get_user
		// substr的第二个参数8表示取前8个字符，这样做的目的是将结尾的\r\n去掉
		if (s.substr(pos + 1, 8) == "get_user")
		{
			// 重新构造一个结果字符串准备返回给客户端
			std::string str;
			// 首先将接收数据中的id拷贝到结果数据中
			// 必须这样做，才能让客户端在接收到结果数据时，从结果数据中解析出id
			// 然后才能把解析出的id和客户端发送数据时的id对应起来
			str = s.substr(0, s.find(',') + 1);
			// 在结果数据中随便添加一点内容
			str += "my name is hanmeimei";
			str += "\r\n";
			// 直接发送结果数据
			session_ptr->async_send(str);
		}
	}
});

// 发送数据的解析函数，当你发送数据时，框架内部会自动调用这个函数来从发送的数据中解析出
// 唯一id，asio2的tcp在发送数据时，数据最终都会变为std::string_view格式，所以这个
// 解析函数的参数是(std::string_view data)这个参数就表示你真正发送出去的数据
asio2::rdc::option rdc_option
{
	[](std::string_view data)
	{
		// 将协议中前面的字符串形式的id取出来 然后转化为int类型的id
		// 为什么这样转化？请看前面介绍：假定你的协议格式为  id,content
		// 有人可能会问：这里能不能直接返回字符串形式的id呢？可以的，后面深入部分有介绍
		int id = std::stoi(std::string{ data.substr(0, data.find(',')) });

		return id;
	}
};

// 参数"\r\n"表示将数据按照"\r\n"进行拆分(如果对tcp拆包知识不了解可通过网络搜索去了解)
// 参数std::move(rdc_option)表示将上面定义的发送数据的解析函数传到asio2框架内部
// 注意：这里用"\r\n"来作为数据拆分标志，并不意味着，你的协议也必须要用"\r\n"拆分，实际上
// 任何自定义的协议，都是支持的。比如按'\n'单个字符来拆分，那第3个参数就传'\n'就行了，
// 如果是复杂的自定义协议，那第3个参数就传match_role就行了，具体的自定义协议的解析和拆分，
// 请看本博客中关于match condition相关的文章介绍。
// 另外，如果你要是不用“远程数据调用”这个功能的话，下面的第4个参数你是不需要传的，就传前
// 3个参数就行了，也就是说，想用“远程数据调用”这个功能，就传rdc_option，不想用就不传，如果
// 没有传rdc_option这个参数，那么框架在编译期就会把该功能禁用掉，所以用不用“远程数据调用”
// 这个功能，对运行时的性能没有任何损失。
server.start(host, port, "\r\n", std::move(rdc_option));
```
##### 客户端代码
```cpp
asio2::tcp_client client;
	
client.bind_connect([&]()
{
	if (!asio2::get_last_error())
	{
		// 连接成功 演示异步远程调用

		// 为什么字符串是下面的样子，请看上面服务端代码部分对协议的说明
		std::string s = "1,get_user\r\n";

		// async_call 表示异步远程调用
		// 第1个参数是要发送的数据 这个参数和调用async_send函数发送数据时第1个参数的性质完全一样
		// 第2个参数表示回调函数，当服务端返回数据时，或者超时了，这个回调函数就会被调用
		// 第3个参数表示超时时间，这个参数可以为空，为空时会使用默认超时，默认超时可用
		//      client.set_default_timeout(...)来设置
		client.async_call(s, [](std::string_view data)
		{
			// 使用asio2::get_last_error()来判断本次调用是成功还是失败，
			// 比如超时了，那么错误码就是asio::error::timed_out
			if (!asio2::get_last_error())
			{
				// 如果没有错误 可以处理数据了
				// data 就是服务端返回的数据 这里就不演示怎么处理数据了
				// data的内容是服务端返回的"1,my name is hanmeimei\r\n"
				std::ignore = data;
			}
		}, std::chrono::seconds(3));
	}
});

// 发送数据的解析函数 具体含义和上面服务端的发送数据的解析函数相同
asio2::rdc::option rdc_option
{
	[](std::string_view data)
	{
		int id = std::stoi(std::string{ data.substr(0, data.find(',')) });

		return id;
	}
};

// 
client.start(host, port, "\r\n", std::move(rdc_option));

// 演示同步远程调用
std::string s = "2,get_user\r\n";

// 注意call<std::string>这里的这个模板参数，表示返回的数据类型，因为我们的协议用的
// 是一个字符串，所以这里我们就直接用std::string来接收返回的数据了，注意不能用std::string_view
// 因为一旦client.call这个同步远程调用结束返回后，client对象的socket接收缓冲区的
// 内容可能就被销毁无效了。那为什么前面的异步远程调用async_call函数的回调函数参数
// 可以是std::string_view呢？因为asio2框架内部是在接收到数据后直接调用了该回调
// 函数，在回调函数未结束之前，client对象的socket接收缓冲区一直被占用着的。

// 第1个参数表示要发送的数据
// 第2个参数表示超时时间 此参数可以为空 为空时使用默认超时 (具体请看源码中的各个函数重载)
std::string ret = client.call<std::string>(s, std::chrono::seconds(3));
// 调用结束之后使用asio2::get_last_error()来判断调用是成功还是失败
if (!asio2::get_last_error())
{
	// ...
}
// 调用成功后，ret的内容就是服务端返回的"2,my name is hanmeimei\r\n"
```
## 深入功能1：使用自定义类型的返回数据
问：上面的async_call回调函数的参数是(std::string_view data)，其中data表示接收到的数据内容，那么有人可能会问，我能用自定义的数据类型来存储接收到的数据内容吗？
答：可以的。
```cpp
// 首先定义一个用户自定义的类型 然后就是如何把接收到的数据内容转换为自定义类型的问题了
struct userinfo
{
	std::string content;

	// 必须实现一个无参数的构造函数
	userinfo() = default;

	// 如何将接收到的数据转换为userinfo类型呢？
	// 方法1：提供一个(std::string_view s)的构造函数
	userinfo(std::string_view s)
	{
		// s就是接收到到的数据内容，这里就是简单的把s的内容保存到成员变量content中了
		content = s.substr(s.find(',') + 1);
	}

	// 方法2：提供一个operator=(std::string_view s)的等于操作符函数
	void operator=(std::string_view s)
	{
		content = s.substr(s.find(',') + 1);
	}

	// 方法3：提供一个operator<<(std::string_view s)的输入操作符函数
	//void operator<<(std::string_view s)
	//{
	//	content = s.substr(s.find(',') + 1);
	//}
};

// 方法4：提供一个全局的operator<<(...)的输入操作符函数
// 注意方法3和4同时只能有一个 方法1到4只提供一个即可
void operator<<(userinfo& u, std::string_view s)
{
	u.content = s.substr(s.find(',') + 1);
}
```
然后直接按自定义类型进行调用：
```cpp
// 异步远程调用
client.async_call(s, [](userinfo u)
{
	// 注意回调函数的参数是userinfo u
});

// 同步远程调用
userinfo info = client.call<userinfo>(s, std::chrono::seconds(3));
```
## 深入功能2：使用自定义类型的唯一id
问：上面的发送数据的解析函数（即上面代码中声明的变量rdc_option）返回的唯一id的类型是int型的，我能用自定义的数据类型作为唯一id返回吗？
答：可以的。
除了用基本int类型作为唯一id，用字符串std::string等各种c++标准库自带的类型都是可以的。只要这个c++标准库自带的类型能用作multimap的key就行（框架内部是用multimap来存储id的）。
比如，最常见的，你可以用uuid作为唯一id，这时就需要用字符串std::string类型来作为唯一id了。
除此之外，你也可以用你自己的自定义的类型作为id的类型，如下：
```cpp

// 首先定义一个自定义的唯一id类型
struct custom_id
{
	std::string country;
	std::string name;

	// 由于框架内部使用的是multimap来存储id的，所以自定义的类型，必须提供一个比较函数
	// 方法1：提供一个小于的比较操作符成员函数重载
	//bool operator<(const custom_id &p) const
	//{
	//	return (this->country + this->name < p.country + p.name);
	//}
};

// 方法2：提供一个std标准库的std::less的全局函数重载
template <>
struct std::less<custom_id>
{
public:
	bool operator()(const custom_id &p1, const custom_id &p2) const
	{
		return (p1.country + p1.name < p2.country + p2.name);
	}
};
```
然后在发送数据的解析函数中去转换即可，如下：
```cpp
asio2::rdc::option rdc_option
{
	[](std::string_view data)
	{
		// 然后按照你的实际需求将发送数据里面的id相关的信息取出来，转化为
		// 自定义类型变量并返回即可
		// 至于怎么转换，下面只是简单的举例，你完全可以按照你自己的协议去做
		custom_id id;
		id.name = data.substr(0, data.find(','));

		return id;
	}
};
```
## 深入功能3：链式调用
问：上面的异步远程调用函数async_call和同步远程调用函数call里面的参数比较多，容易忘记参数顺序，有没有好的办法处理呢？
答：使用链式调用。
```cpp
// 异步链式调用 timeout async_call response的顺序完全随意
client.timeout(std::chrono::seconds(5)).async_call(s).response([](userinfo info)
{
});
// 同步链式调用 .call函数必须是最后一个调用节点
client.timeout(std::chrono::seconds(3)).call<std::string>(s);
```
## 深入功能4：服务端和客户端互相调用
问：上面演示的都是客户端远程调用服务端的，服务端能不能远程调用客户端呢？
答：可以。使用方法和前面的说明基本完全相同。
```cpp
session_ptr->async_call(str, [](std::string_view data)
{
}, std::chrono::seconds(3));
```
## 深入功能5：发送数据和接收数据使用不同的解析函数
问：上面的演示代码中只使用了“发送数据的解析函数”，如果发送数据所使用的协议，和接收数据所使用的协议不同，无法用同一个解析函数解析出唯一id，该怎么办呢？
答：同时提供发送数据的解析函数，和接收数据的解析函数即可。
实际上框架内部有两个解析函数，一个发送数据时的解析函数，一个接收数据时的解析函数。如果你的rdc_option中只包含了一个解析函数，那么这个解析函数既用来解析发送数据，也用来解析接收数据。
```cpp
asio2::rdc::option rdc_option
{
	[](std::string_view data) // 第1个是send数据的解析函数
	{
		// 然后按照你的实际需求将发送数据里面的id相关的信息取出来，转化为
		// 自定义类型变量并返回即可
		// 至于怎么转换，下面只是简单的举例，你完全可以按照你自己的协议去做
		custom_id id;
		id.name = data.substr(0, data.find(','));

		return id;
	},
	[](std::string_view data) // 第2个是recv数据的解析函数
	{
		// 注意：虽然可以同时提供发送数据的解析函数，和接收数据的解析函数
		// 但是返回的id的类型必须相同，因为框架内部是根据id来查找回调函数的
		// 如果id类型不一样，就没法处理了。
		// 还有，这里只是示意怎么使用，并没有按照协议的不同做不同的解析，这个
		// 用户根据自己的实际需求去做就行了。
		custom_id id;
		id.country = data.substr(0, data.find(','));

		return id;
	}
};

server.start(host, port, "\r\n", std::move(rdc_option));

```
## 深入功能6：两次发送的数据的唯一id相同可以吗？
答：可以的。
```cpp
asio2::rdc::option rdc_option
{
	[](std::string_view data)
	{
		// 实际上，你可以在这里直接返回0 (返回其它的数字或字符串，等等，都可以)
		// 只要每次返回的id都相同，这就表示所有发送出去的的数据，对方在返回结果
		// 数据后，会按照发送时的顺序，依次调用你的回调函数。
		return 0;
	}
};

client.start(host, port, "\r\n", std::move(rdc_option));
```
```cpp
asio2::rdc::option rdc_option
{
	[](std::string_view data)
	{
		// 如果协议中没有包含有效的id,那么就返回0，这样也是可以的
		// 当接收到数据时，会按照发送时的顺序，依次调用id是0的回调函数
		// 当然，发送数据解析出的id如果是0，那么接收数据解析出的id也
		// 一定要是0，只不过可以有多个数据都返回了0这个id
		if (data.find(',') == std::string_view::npos)
			return 0;
		// 如果协议中包含了有效的id 那么就返回该id作为唯一id
		// 返回了id的，当接收到数据时，就会按照接收数据解析出来的id
		// 来调用你对应的回调函数
		return std::stoi(std::string{ data.substr(0, data.find(',')) });
	}
};

client.start(host, port, "\r\n", std::move(rdc_option));
```

但是：如果两次发送的数据的id相同的话，一旦网络不稳定，是会出现问题的。我用下面的流程举例：
1、client调用call或async_call，发送数据，假如id是0，超时是3秒，框架会把0存到multimap中； 
2、server在3秒内一直不回复； 
3、client的超时到了，你的回调函数被调用，同时框架内部会把multimap中id是0的元素删掉（实际只删第1个id是0的元素，假如有多个id是0的话）
4、client再次调用call或async_call，发送数据，id还是0，框架会把0再存到multimap中； 
5、此时server回复了，但回复的信息是针对第1步中那个id为0的消息进行回复的；
6、client中，你的回调函数被调用，但在这个回调函数中接收的数据实际上是第1次call(或async_call)的结果，而不是第2次call的结果，因为两次id都是0，框架是无法识别哪个回复对应哪个请求的，只能按照顺序取出multimap中保存的回调函数进行调用。

但是如果每个数据的id都是不同的，那就不会存在上面的问题。

所以如果你的协议中，有id相同的情况的话，而且对数据的正确性要求非常严格，那就不能这样用了，网络的延迟和波动等等问题很难说，长时间运行的话，是一定会出问题的。当然如果对数据的正确性要求不那么严格，那id相同也没什么问题。

## 深入功能7：服务端是别人写的不能改，只有客户端是我写的能用这个功能吗？
答：可以的。
实际上这个功能只和你的协议以及响应模式有关（就是说你发送一条，对方就给你回一条，而不是给你回多条，也就是只要是一发一收，一对一的关系就能用）
## 深入功能8：call，async_call 和 send，async_send可以混用吗？
答：可以的。
如果你想在发送数据的代码的地方直接取得对方返回的结果数据，那就用call或async_call。
如果只是想发送数据，并不想立即取得结果，那就用send或async_send即可。
不管是send或async_send还是call或async_call，都会发送数据，区别在于，send或async_send发送数据时，框架内部没有从你发送的数据中解析出id，当然也就没有存储该id了。send或async_send函数把数据发出去之后就不管了。但call或async_call把数据发出去之后，如果收到对方回复的数据了，框架内部还会解析接收到数据，然后调用你提供的回调函数等。

但是如果你在启动某组件时没有传入rdc数据解析器（比如这样client.start(host, port, "\r\n");没有第4个参数，也就是说没有传rdc_option），那么call和async_call是用不了。

还有一点是用来发送数据的async_send函数有个重载版本，也可以传入一个回调函数，如
client.async_send("abc",[](std::size_t bytes_sent) 
{
});
这个回调函数只是用来通知你，这次发送的数据，是发送成功了还是发送失败了。并不是收到了远端给你的回复数据的通知。所以这点不要搞混淆了，只有call和async_call的回调函数才是“收到了远端的回复数据的通知”。

## 实现流程简要说明
流程是这样的：

1、客户端调用call或async_call发送数据
2、asio2框架内部调用rdc_option中的解析函数解析发送的数据，得到id 存起来(存到一个multimap中了)
3、服务端返回数据，框架内部调用rdc_option中的解析函数解析接收的数据，得到id ，到multimap中找前面存起来的那个id，找到了，就调用该id对应的回调函数

服务端远程调用客户端流程是一样的。

**上面是用tcp通信来举例的，实际上asio2框架里的所有组件tcp udp http websocket ssl 串口，全部支持远程数据调用功能。**

其中udp websocket组件中的使用方式和tcp中的使用方式完全一样，因为这几个协议的形为一致，而且内部都是用std::string_view来传递数据的。这里就不再为udp websocket列举代码来介绍怎么用了。

但http的使用方式是不一样的，下面用代码说明：
## http下远程数据调用步骤
```cpp
asio2::http_client client;

// http client 的远程数据调用必须同时提供发送数据的解析函数和接收数据的解析函数
// 因为http client在发送数据时，框架底层的数据格式是request类型
// 而http client在接收数据时，框架底层的数据格式是response类型
// 发送和接收数据的类型不一样，所以必须提供两个解析函数

// 为什么都是return 0;呢？这是由http 1.1协议的特性决定的，http client先发送请求，
// 然后http server再回复请求，而且http协议通常是一对一的，即一次请求对应
// 一次回复，所以再用id就没有多大意义了，但又必须得返回个id，所以就return 0了
asio2::rdc::option rdc_option
{
	[](http::request &) { return 0; },
	[](http::response&) { return 0; }
};

if (client.start(host, port, rdc_option))
{
	http::request req(http::verb::get, "/", 11);

	// 同步调用
	http::response rep = client.call<http::response>(req);

	// 异步调用
	// 注意有个细节：上面的同步调用时返回的是http::response对象，而这里的异步
	// 调用的回调函数的参数http::response&是个引用类型，因为同步调用函数如果
	// 返回引用会不安全，这个和上面介绍的tcp下同步调用不能返回std::string_view
	// 是相同的原因。
	client.async_call(std::move(req), [](http::response& rep)
	{
		std::ignore = rep;
	});
}
```
上面是http client使用远程数据调用的方法，那http server能远程调用http client吗？

不能的，这还是由http协议的特性决定的，只能由http client主动向http server发送请求，然后http server回复，而不能由http server发送请求，http client来回复。(目前asio2框架里只支持到http 1.1，http 2.0协议上的不同暂不考虑)

其实把http组件也做上远程数据调用这个功能，我感觉意义并不大，但还是做上吧，反正即使不用这个功能的话，对性能没有任何损失（都是模板实现的，如果不用这个功能的话，编译期已经去掉该功能了）。

但是，http session如果升级到websocket之后，是支持向websocket client主动发出远程数据调用的。

### 最后编辑于2022-06-23
