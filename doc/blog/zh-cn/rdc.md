# 基于c++和asio的网络编程框架asio2教程基础篇：4、使用tcp客户端发送数据时,如何同步获取服务端返回结果

## 问题描述
我们先看使用异步框架遇见的一个问题：
```cpp
// 我们调用send函数向服务端发送数据
client.send("abc");
```
假定服务端收到abc之后，回复123，问题是：我们在哪里接收这个123呢？如下：
```cpp
client.bind_recv([&](std::string_view data)
{
	// 会在这个回调函数里收到 123
});
```
这有个问题就是我们没有办法在发送数据的地方，直接得到远程回复的结果。
```cpp
// 比如我想这样：发送完abc之后，等待服务端回复，回复的结果直接存储到我的变量里。
// (注意这里只是举例，实际使用方法和这里不同)
std::string res = client.send("abc");
```
好的，框架已经支持这种功能，我给这个功能取了个名字叫rdc，即remote data call，即远程数据调用。

最简单的例子：  
```cpp
asio2::tcp_client client;

// 数据解析函数
asio2::rdc::option rdc_option
{
	[](std::string_view data) { return 0; }
};

// 启动，把rdc_option传进去
client.start("127.0.0.1", 8080, rdc_option);

// 同步远程调用
std::string res = client.call<std::string>("abc"); // res == "123"
```
上面这个例子只是作为最简单的演示代码，没有处理tcp拆包，实际使用过程中会有问题。这个问题先放一边，看完后面复杂的协议处理部分自然就明白了。

我们先思考一下，我们如何实现这个功能？

简单的思路就是，发送数据时，将发送的数据取一个id，将id保存到一个map中，当收到回复时，从回复数据中也取一个id，然后到map中查找有没有这个id，如果找到了，就把回复的数据给那个id即可。(id怎么取？把回复的数据给那个id：怎么给？这个问题放一边，只要知道，我们实现这个功能基本上就是这个思路，就行了)

我们再看上面的例子：
```cpp
std::string res = client.call<std::string>("abc"); // res == "123"
```
发abc得123，这只是最简单的情况，实际情况下服务端可能会发送很多数据，如果服务端发了123，又发了567，上面的res这个变量内容到底是123还是567呢？

这就需要我们思考一个问题：怎么把发送的数据和接收的数据对应起来？我发送abc之后，res变量的内容一定要是123，而不能是567。

怎么办呢？对，就是取id？

上面的数据abc，有id吗？能取id吗？不能。所以上面的数据解析函数里是 return 0; 因为没法取id

```cpp
// 数据解析函数
asio2::rdc::option rdc_option
{
	[](std::string_view data) { return 0; /* 所以这里用了return 0;*/ }
};
```
为什么用 return 0; 其实return 0;的流程是这样的：

使用call函数发送数据时，调用数据解析函数(即上面的rdc_option)，把数据传过去(即传到std::string_view data这里了)，返回一个id(即return 0)，把这个id 0存到map中，当收到回复123时，也调用数据解析函数，也把数据传过去(这次传的数据时接收的数据)，也返回一个id(仍然是return 0)，到map中查找这个id 0，找到了，于是把123这个数据赋给了res那个变量。

所以，如果先收到了567，也会把567赋给res变量。这是因为这里没有取id，id永远是0，id是0的意思就是：不管收到什么数据，都会认为是这次call函数的回复。

所以，如果想用这个功能，那你的协议必须要有id对应关系，否则，虽然能用，但是数据可能是错的，对不上号。

那么下面假定你的协议是个字符串，格式是这样的：id,内容
也就是说开头是id，id是个字符串，后面是内容，内容也是字符串，id和内容之间用一个逗号分隔。

下面开始说这种情况怎么使用该功能：
##### 客户端代码
```cpp
asio2::tcp_client client;

// 数据解析函数，当你发送数据时，框架内部会自动调用这个函数来从发送的数据中解析出
// 唯一id，asio2的tcp在发送数据时，数据最终都会变为std::string_view格式，所以这个
// 解析函数的参数是(std::string_view data)这个参数就表示你真正发送出去的数据
asio2::rdc::option rdc_option
{
	[](std::string_view data)
	{
		// 把逗号之前的id取出来，转换为整数(其实转不转为整数都行，直接返回字符串形式的id也支持)
		std::string strid = data.substr(0, data.find(','));
		
		int id = std::stoi(strid);

		return id;
	}
};

// 启动，第3个参数表示用\n做数据边界
// 另外，如果你要是不用“远程数据调用”这个功能的话，下面的第4个参数你是不需要传的，就传前
// 3个参数就行了，也就是说，想用“远程数据调用”这个功能，就传rdc_option，不想用就不传，如果
// 没有传rdc_option这个参数，那么框架在编译期就会把该功能禁用掉，所以用不用“远程数据调用”
// 这个功能，对运行时的性能基本没有任何损失。
client.start("127.0.0.1", 8080, '\n', rdc_option);

// 要发送的数据：id是1，接着是逗号，接着是内容abc，接着是\n，为什么要用\n 
// \n是做tcp拆包用的，注意看上面的client.start的第3个参数\n，就表示按照\n
// 进行拆包(关于tcp数据拆包的知识，如果不熟悉的先网络搜索了解一下)
std::string msg = "1,abc\n";

// 调用call函数发送数据，并等待服务端回复结果
std::string res = client.call<std::string>(msg); // res == "1,123\n"

// 这里res的值一定是1,123\n 而不会是其它数据

// 注意call<std::string>这里的这个模板参数，表示返回的数据类型，因为我们的协议用的
// 是字符串，所以这里我们就用std::string来接收返回的数据，注意不能用std::string_view
// 因为一旦client.call这个同步远程调用结束返回后，client对象的socket接收缓冲区的
// 内容可能就被销毁无效了，也就意味着std::string_view的内容也会无效。
```
##### 服务端代码
```cpp
asio2::tcp_server server;

server.bind_recv([&](auto & session_ptr, std::string_view data)
{
	// 这里就简单的判断一下，如果收到1,abc\n就回复1,123\n
	// 实际情况肯定要复杂很多，按照实际要求去处理即可。
	if (data == "1,abc\n")
		session_ptr->async_send("1,123\n");
});

server.start("0.0.0.0", 8080, '\n'); // 服务端同样也按\n拆包
```

##### 异步远程调用
```cpp
// 第1个参数是要发送的数据 这个参数和调用async_send函数发送数据时第1个参数的性质完全一样
// 第2个参数表示回调函数，当服务端返回数据时，或者超时了，这个回调函数就会被调用
// 第3个参数表示超时时间，这个参数可以为空，为空时会使用默认超时，默认超时可用
//    client.set_default_timeout(...)来设置
client.async_call(msg, [](std::string_view data)
{
	// 使用asio2::get_last_error()来判断本次调用是成功还是失败，
	// 比如超时了，那么错误码就是asio::error::timed_out
	if (!asio2::get_last_error())
	{
		// 如果没有错误 可以处理数据了
		// data 就是服务端返回的数据 这里就不演示怎么处理数据了
		// data的内容是服务端返回的"1,123\n"
		std::cout << data;
	}
}, std::chrono::seconds(3));
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
// 服务端启动时必须也要传rdc_option
server.start("0.0.0.0", 8080, "\r\n", std::move(rdc_option));

// 然后有了session_ptr后，就可以用它来调用了
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
// 因为http client在发送数据时，框架底层的数据格式是web_request类型
// 而http client在接收数据时，框架底层的数据格式是web_response类型
// 发送和接收数据的类型不一样，所以必须提供两个解析函数

// 为什么都是return 0;呢？这是由http 1.1协议的特性决定的，http client先发送请求，
// 然后http server再回复请求，而且http协议通常是一对一的，即一次请求对应
// 一次回复，所以再用id就没有多大意义了，但又必须得返回个id，所以就return 0了
asio2::rdc::option rdc_option
{
	[](http::web_request &) { return 0; },
	[](http::web_response&) { return 0; }
};

if (client.start(host, port, rdc_option))
{
	http::web_request req(http::verb::get, "/", 11);

	// 同步调用
	http::web_response rep = client.call<http::web_response>(req);

	// 异步调用
	// 注意有个细节：上面的同步调用时返回的是http::web_response对象，而这里的异步
	// 调用的回调函数的参数http::web_response&是个引用类型，因为同步调用函数如果
	// 返回引用会不安全，这个和上面介绍的tcp下同步调用不能返回std::string_view
	// 是相同的原因。
	client.async_call(std::move(req), [](http::web_response& rep)
	{
		std::ignore = rep;
	});
}
```
上面是http client使用远程数据调用的方法，那http server能远程调用http client吗？

不能的，这还是由http协议的特性决定的，只能由http client主动向http server发送请求，然后http server回复，而不能由http server发送请求，http client来回复。(目前asio2框架里只支持到http 1.1，http 2.0协议上的不同暂不考虑)

但是，http session如果升级到websocket之后，是支持向websocket client主动发出远程数据调用的。

###### 现在http client已经内置了rdc option，如果你没传rdc option就用内置的rdc option，所以现在http client可以这样使用：
```cpp
asio2::http_client client;

client.start("127.0.0.1", "8080");

http::web_request req1(http::verb::get, "/index.html", 11);
http::web_response res1 = client.call<http::web_response>(req1);

http::web_request req2(http::verb::get, "/login.html", 11);
http::web_response res2 = client.call<http::web_response>(req2);
```
上面这个用法相比 http_client::execute 的好处是：http_client::execute每次都创建一个新连接，用完之后连接就关掉了，而上面这个用法，不会关闭连接，而是一直使用同一个连接，这样就不存在每次建立连接时的tcp三次握手这个问题，效率要好一些，而且http client有自动重连功能，即使由于超时或其它原因，断开之后也会再次连接上服务端。

### 最后编辑于2022-12-18
