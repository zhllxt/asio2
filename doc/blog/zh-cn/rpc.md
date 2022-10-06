# 基于c++和asio的网络编程框架asio2教程使用篇：使用rpc模块编写rpc server和rpc client

rpc的基础概念这里就不再介绍了，不熟悉的可以网络搜索，先了解一下。asio2框架实现了轻量级的rpc功能，使用起来非常简单。

## 最简单的例子
##### 服务端代码
```cpp
int add(int a, int b)
{
	return a + b;
}

asio2::rpc_server server;

server.bind("add", add); // 绑定rpc函数，第1个参数是字符串，表示rpc函数的名字是什么，第2个参数是真正的rpc函数

server.start("0.0.0.0", 8010); // 启动服务端
```
##### 客户端代码
```cpp
asio2::rpc_client client;
	
client.start("127.0.0.1", 8010); // 连接服务端

int sum = client.call<int>("add", 1, 2); // 调用rpc函数，得到结果：sum == 3
```
## 同步调用
```cpp
// 最简单的同步调用如下：
// 第1个参数是个字符串，表示rpc函数的名字，后面的参数表示rpc函数的参数（这里的1,2即是add函数的参数）
int sum = client.call<int>("add", 1, 2);

// 如果函数调用失败，怎么办？在哪里获取通知？
int sum = client.call<int>("add", 1, 2);
// 使用asio2::get_last_error()来判断是否发生错误
if (asio2::get_last_error()) // 有错误
	printf("add failed : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());

// 怎么设置同步调用的超时时间？
int sum = client.call<int>(std::chrono::seconds(3), "add", 1, 2);
```
## 异步调用
```cpp
// 最简单的异步调用如下：
// 第1个参数是回调函数，后面的参数是rpc函数名称和rpc函数参数
// 回调函数的参数即是rpc函数的返回值
client.async_call([](int sum)
{
	if (!asio2::get_last_error()) // 没有错误
	{
		ASIO2_ASSERT(sum == 1 + 2);
	}
	else // 有错误
	{
		printf("error : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}
}, "add", 1, 2);

// 如何指定异步调用的超时时间？
client.async_call([](int sum)
{
}, std::chrono::seconds(3), "add", 1, 2);

// 如果rpc函数返回值是void怎么办？
// 比如有个心跳函数:
void heartbeat(){}
// 那么可以像下面这样调用即可，也就是说回调函数参数为空即可
client.async_call([]()
{
}, std::chrono::seconds(3), "heartbeat");

// 如果你不关心调用结果，也就是说不关心rpc函数的返回值，你可以直接调用，可以不关心调用成功还是失败，
// 这种情况下当服务器收到rpc请求后，是不会给客户端回复的。
client.async_call("heartbeat");
```
## 链式调用
不管是同步调用，还是异步调用，都有“超时设置，rpc函数名称，rpc函数参数”等参数，由于参数很多，而且参数的位置不能出错，所以在实际使用时容易忘记各个参数的前后位置，增加了心智负担，所以框架也提供了链式调用功能，如下：
```cpp
// 同步调用的链式调用，如下：
int sum = client.call<int>("add", 1, 2); // ok
int sum = client.timeout(std::chrono::seconds(3)).call<int>("add", 1, 2); // ok
// 同步调用时.call函数必须在链的最后一个
int sum = client.call<int>("add", 1, 2).timeout(std::chrono::seconds(3)); // 错误 

// 异步调用的链式调用，如下：
client.timeout(std::chrono::seconds(3)).response([](int sum){}).async_call("add", 1, 2); // ok
client.response([](int sum){}).timeout(std::chrono::seconds(3)).async_call("add", 1, 2); // ok
// 异步调用时async_call可以在链的任意位置 所以下面都是正确的
client.async_call("add", 1, 2).timeout(std::chrono::seconds(3)).response([](int sum){}); // ok 
client.timeout(std::chrono::seconds(3)).async_call("add", 1, 2).response([](int sum){}); // ok 
client.response([](int sum){}).async_call("add", 1, 2); // ok
client.async_call("add", 1, 2).response([](int sum){}); // ok
```
## 双向调用
上面所举的例子中，都是在客户端调用服务端的rpc函数。
框架既支持客户端调用服务端的rpc函数，同样也支持服务端调用客户端的rpc函数，如下：
```cpp
// bind_connect是server提前绑定的回调函数，当有一个客户端连接上来之时，此回调函数会被调用，关于bind_connect知识请参考其它文章。
// 当然，并不是只能在bind_connect这里，也可以在其它地方调用客户端的rpc函数。
server.bind_connect([&](auto & session_ptr)
{
	// 这里session_ptr表示客户端的连接对象
	// 这个客户端连接上来之后，server通过session_ptr向该客户端发起一个rpc函数调用
	session_ptr->async_call([](int v)
	{
		if (!asio2::get_last_error())
		{
			ASIO2_ASSERT(v == 15 - 6);
		}
	}, std::chrono::seconds(5), "sub", 15, 6);
});
```
## 嵌套调用
当业务流程复杂时，会出现嵌套调用rpc函数的需求，框架同样支持，如下：
```cpp
// server端提前绑定一个rpc函数"cat"（这里这个rpc函数是个lambda函数）
server.bind("cat", [&](std::shared_ptr<asio2::rpc_session>& session_ptr, std::string a, std::string b)
{
	// 当客户端调用rpc函数"cat"时，会执行到这里来.....

	// server端收到客户端的调用请求时，在这里用session_ptr嵌套的给该client发送一个rpc调用请求
	session_ptr->async_call([session_ptr](int v) mutable
	{
		// 当server端发送的调用请求，收到了回复时，再次嵌套的给该client发送一个rpc调用请求，如此等等。
		session_ptr->async_call([](int v)
		{
			if (!asio2::get_last_error())
			{
				ASIO2_ASSERT(v == 15 + 18);
			}
			printf("async_add : %d err : %d %s\n", v, asio2::last_error_val(), asio2::last_error_msg().c_str());
		}, "add", 15, 18);

		if (!asio2::get_last_error())
		{
			ASIO2_ASSERT(v == 15 - 8);
		}
		printf("sub : %d err : %d %s\n", v, asio2::last_error_val(), asio2::last_error_msg().c_str());
	}, "sub", 15, 8);

	return a + b;
});
```
## 服务端如何知道是哪个客户端调用的rpc函数？
```cpp
int add(int a, int b)
{
	return a + b;
}
```
比如上面的add函数，当有1个server和1000个client，且1000个client都会调用这个add函数时，怎么知道是哪个client调用的呢？

方法1：
```cpp
// 将rpc函数的第1个参数改为连接对象的指针，如下，此时通过session_ptr就能知道是哪个client调用的了
// 当然，如果你不关心是哪个client调用的，那么std::shared_ptr<asio2::rpc_session>& session_ptr这个参数可以不要，
// 也就是说下面这种带session_ptr的方式和上面那种不带session_ptr的方式，都支持，而且都只需server.bind("add", add)
// 即可，不同的版本不需要其它不同的bind操作。
int add(std::shared_ptr<asio2::rpc_session>& session_ptr, int a, int b)
{
	return a + b;
}
```
方法2：
```cpp
int add(int a, int b)
{
	// 调用get_current_caller函数直接获取即可(注意模板参数必须完全匹配)
	std::shared_ptr<asio2::rpc_session> session_ptr = 
		asio2::get_current_caller<std::shared_ptr<asio2::rpc_session>>();
	return a + b;
}
```
## 怎么让rpc函数支持自定义的结构体？
比如我有个结构体，如下：
```cpp
struct userinfo
{
	std::string name;
	int age;
	std::map<int, std::string> purview;
};
```
然后有个rpc函数，如下：
```cpp
userinfo get_user(std::string name)
{
	// 根据参数name找到对应的userinfo并返回（这里不再写“根据name找到userinfo”的代码了）
	userinfo u;
	u.name = name;
	u.age = 100;
	u.purview = { {1,"read"},{2,"write"} };
	return u;
}
```
那么只需要像下面这样修改一下结构体userinfo即可。
也就是说手工给结构体userinfo添加一个序列化的成员函数即可。
给userinfo添加了序列化的成员函数之后，userinfo这个结构体就可以像普通的int, std::string这些基础类一样使用了，不需要再做任何其它的操作，这个userinfo就可以用作rpc函数的参数，或rpc函数的返回值了。
```cpp
struct userinfo
{
	std::string name;
	int age;
	std::map<int, std::string> purview;

	// 添加一个模板形式的序列化函数，函数名称和函数参数必须保持和下面示例的一样才行。
	template <class Archive>
	void serialize(Archive & ar)
	{
		ar(name);
		ar(age);
		ar(purview);
	}
};
```
## 怎么让rpc函数支持第三方开源库里面的类型？
比如程序中经常有json操作，我们一般都会找一个开源json库来使用。
比如使用了nlohmann::json这个库，此时，我想把nlohmann::json这个类作为rpc函数的参数或返回值，如下：
```cpp
nlohmann::json test_json(nlohmann::json j)
{
	std::string s = j.dump();

	return nlohmann::json::parse(s);
}
```
这时该怎么呢？因为此时是没法像自定义结构体userinfo那样，去手工给json类添加一个序列化的成员函数的。
那么像下面这样添加两个全局的序列化和反序列化函数即可，如下：
```cpp
void operator<<(asio2::rpc::oarchive& sr, const nlohmann::json& j)
{
	sr << j.dump(); // j.dump()是把json对象转换为std::string
}

void operator>>(asio2::rpc::iarchive& dr, nlohmann::json& j)
{
	std::string v;
	dr >> v;
	j = nlohmann::json::parse(v); // json::parse(v) 是把std::string转换为json对象
}
```
添加了两个全局的序列化和反序列化函数之后，这个json类型就可以像普通的int, std::string这些基础类一样使用了，不需要再做任何其它的操作。

## 异步rpc函数
上面举例中的rpc函数都是非常简单的函数，实际项目中的函数一般都比较复杂，那些复杂的rpc函数，它们的返回值，不一定能立即计算得到，而是需要交给其它的工作线程去处理，处理之后才能得到结果，至于那个异步的处理过程到底需要多久却是不确定的。
下面还是用add函数来举例：
```cpp
int add(int a, int b)
{
	int result;

	// 当收到rpc调用时，需要交给其它的工作线程中去异步处理
	std::thread([&result, a, b]() mutable
	{
		result = a + b; // 此时处理完毕，才有了结果
	}).detach();

	// return result; // 这里如果直接返回result是不对的，因为上面的异步调用无法确定在什么时候才会完成
}
```
那这该怎么呢？框架也已经支持了，像下面这样修改即可如下：
```cpp
// 第1，函数的返回值要用rpc::future包裹起来
rpc::future<int> add(int a, int b)
{
	// 第2，定义两个辅助变量，promise和future ，如下：
	rpc::promise<int> promise;
	rpc::future<int> f = promise.get_future();

	// 第3，把promise传到那个异步的工作线程中
	std::thread([a, b, promise = std::move(promise)]() mutable
	{
		// ...... 比如这里经过了很多工作处理......
		// 第4，处理完毕后，给promise设置值，这个值就是这个add函数的返回值了
		promise.set_value(a + b);

		// 代码执行到这里之后，promise变量会析构，当promise析构时，asio2框架就会自动给客户端回复了，
		// 回复时的结果就是上面promise.set_value函数中设置的那个值。
	}).detach();

	// 第5，在这里返回上面那个定义的变量future即可。注意比较和同步rpc函数的区别，同步rpc函数直接返回了a+b;
	return f;
}
```
## 其它
调用rpc函数的默认超时是5秒，可以通过下面这个函数进行设置：
注意这个函数设置的是全局的超时设置，如果你在调用rpc函数时，传递的参数中又包含了超时设置，那么那一次的rpc调用就会使用那个单独设置的超时，如果在调用rpc函数时，传递的参数中没有超时参数，那就使用默认的超时。
```cpp
client.set_default_timeout(std::chrono::seconds(3));
```

更多功能或用法请参考工程示例。

### QQ群：833425075

## 项目地址：
github : [https://github.com/zhllxt/asio2](https://github.com/zhllxt/asio2)
码云 : [https://gitee.com/zhllxt/asio2](https://gitee.com/zhllxt/asio2)

### 最后编辑于：2022-06-23
