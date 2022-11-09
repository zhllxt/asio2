# 关于asio2项目example目录中的几个tcp示例的说明
在 [/asio2/example/tcp/](https://github.com/zhllxt/asio2/tree/master/example/tcp) 目录里有以下这几个关于tcp的示例代码：

tcp_client_character，tcp_client_custom，tcp_client_datagram，tcp_client_general

这几个示例是演示怎么做tcp拆包的，下面详细说明一下（服务端相对应的示例是一样的意思，不再描述）：

至于为什么tcp数据要做拆包，不了解的可以网络详细搜索一下，不管你是自己手撸网络代码还是使用第三方开源库，都得处理tcp的拆包问题，asio2只是帮你处理了一些步骤，简化了使用过程。

## tcp_client_character
```cpp
// 第3个参数传\n 就是按字符\n进行拆包
//client.start(host, port, '\n');

// 第3个参数传\r\n 就是按字符串\r\n进行拆包
client.start(host, port, "\r\n");
```
这个示例演示的是用**单个字符或字符串**来做tcp拆包的，此时数据边界就是单个字符或字符串；

举例：比如按\n来拆包，
client发送 123\n45678\nABCDEFG\n
假定server一次性接收完上面的所有数据，也就是server一次性收到了 123\n45678\nABCDEFG\n ，那么接收数据的回调函数是怎么触发的呢？是触发1次还是触发3次？答案：3次

```cpp
bind_recv([&](std::string_view data)
{
	// 第1次触发时 data=123\n
	// 第2次触发时 data=45678\n
	// 第3次触发时 data=ABCDEFG\n
	// 也就是说不管收到的是什么数据，框架一定会检测\n这个字符，只有检测到\n这个字符了，
	// 框架才会把缓冲区中，从缓冲区开头到\n这个位置的，这一段数据在这里通知你。
	printf("recv : %zu %.*s\n", data.size(), (int)data.size(), data.data());

	client.async_send(data);
});
```
如果数据安全性要求不高，就用\n或\r\n来做为数据边界就可以了，不管是组包还是拆包，都是很方便的。

当然，示例这里用的\n或\r\n并不是说只能用\n或\r\n，你用任何其它的字符或字符串来做数据边界都是可以的，只是你在发送数据组包的时候要注意，你的数据内容本身里面不能包含了用于做数据边界的字符，否则对方来拆包处理的时候就会有问题。这里用\n或\r\n来做示例是因为\n或\r\n比较通用，我用过一些厂家的产品就是用\r\n来做数据边界的，主要是处理起来很方便。

下面说另一种情况：如果client只发送了 123\n45678\nABCD 也就是后面有一部分数据没有发，那接收数据的回调函数是怎么触发的？答案：触发2次
```cpp
bind_recv([&](std::string_view data)
{
	// 第1次触发时 data=123\n
	// 第2次触发时 data=45678\n
	// 第2次触发完之后，接收缓冲区里现在的数据是ABCD 框架查找的时候找不到\n，所以是
	// 不会把ABCD拿来通知你的，如果之后client又发送了EFG\n也就是发送了后面那部分的数
	// 据，那么现在接收缓冲区里的数据就是ABCDEFG\n了，此时框架检测到了\n，于是这个
	// bind_recv的回调函数就会第3次被触发，此时data=ABCDEFG\n
	// 如果是按字符串\r\n拆包，则一定要收到\r\n才会触发回调，收到\r不行，收到\n也不行。
	printf("recv : %zu %.*s\n", data.size(), (int)data.size(), data.data());

	client.async_send(data);
});
```
以上描述了一些基本的情况，更多复杂的情况没有描述，总之道理都一样。

后面的“custom，datagram”和这个道理都是类似的，只是数据格式不一样而已。
## tcp_client_custom
这个示例演示的是，自定义协议的拆包。

上面用\n或\r\n来做数据边界是比较简单的方法，通常用户会有一套复杂的自定义协议，比如“数据头+数据类型+数据长度+数据内容+校验”这种，此时对tcp数据做拆包就是个很麻烦的过程。

在asio2框架里是怎么使用的，如下：

#### 首先定义一个类，每当框架接收到数据时，就会调用这个类进行拆包
```cpp
// 这里用一个较简单的自定义协议来举例，没有用上面所说的“数据头+数据类型+数据长度+数据内容+校验”这种
// 因为它太麻烦了，但是原理是一样的，不管多复杂的数据格式，都可以像这样来处理。
// 第1个字节      数据头   : #       这里数据头是#号
// 第2个字节      数据长度 : 就是接下来的内容的长度
// 第3个字节往后  数据内容 : 内容的长度是可变的
class match_role
{
public:
	explicit match_role(char c) : c_(c) {}

	template <typename Iterator>
	std::pair<Iterator, bool> operator()(Iterator begin, Iterator end) const
	{
		Iterator p = begin;
		while (p != end)
		{
			// how to convert the Iterator to char* 
			[[maybe_unused]] const char * buf = &(*p);

			// eg : How to close illegal clients
			if (*p != c_)
				return std::pair(begin, true); // head character is not #, return and kill the client

			p++;
			if (p == end) break;

			int length = std::uint8_t(*p); // get content length

			p++;
			if (p == end) break;

			if (end - p >= length)
				return std::pair(p + length, true);

			break;
		}
		return std::pair(begin, false);
	}

private:
	char c_;
};
```

```cpp
// 然后在启动时将刚才定义的类传到start函数里。
server.start(host, port, match_role('#'));
```

关于自定义协议的拆包，就不再过多描述了，这里有一篇更详细的讲解原理的文章：[asio做tcp的自动拆包时，asio的match condition如何使用的详细说明](https://blog.csdn.net/zhllxt/article/details/127670983)

## tcp_client_datagram
大家在使用websocket时，有没有注意到，每次触发你的数据回调时，都是一份完整的数据，你收到的数据不会是半包或一包半这样的情况，使用websocket后，你不需要再做数据拆包的工作了，websocket框架已经帮你做过了，那websocket协议是怎么实现的呢？

实际上websocket是在数据开始位置添加了一个数据头，这个数据头表示接下来的数据内容的长度（具体协议比这要复杂），于是收到数据时就可以根据数据头里的长度来判断出是否接收完整了。

asio2的tcp_datagram也用了相同的原理：每当你发送数据时，asio2这个框架就会自动在你的数据开始的位置插入一个数据头，这个数据头是一个整数，用来表示你的真正的数据内容的长度，同样，在接收时，也会先解析这个数据头，解析出数据内容的长度，解析成功后会在bind_recv的回调函数里通知你，通知里面的数据不包含数据头，只有你的真正的数据内容。

```cpp
void on_recv(asio2::tcp_client& client, std::string_view data)
{
	// data这个变量是你的数据内容，不包含asio2框架自动添加的数据头
	printf("recv : %.*s\n", (int)data.size(), data.data());

	client.async_send(data);
}
```

```cpp
// 像这样在启动时将start函数的第3个参数设置为asio2::use_dgram即表示使用此功能
client.start(host, port, asio2::use_dgram);
```

要注意：使用了asio2框架时，如果server端使用了use_dgram参数，那么client也要使用use_dgram参数，保证server和client要匹配（就是说要保证server和client的数据协议要一致），否则 asio2框架在解析数据时会出问题。

如果你的应用场景简单，服务端客户端程序都是你自己一个人写的，就可以使用这个功能，这样你可以不用关心复杂的协议解析处理了。

这个模式会保证收到的数据一定和对端发过来的数据完全一样。下面举例描述一下：

比如
client.async_send("123");
client.async_send("abc");
那么
server端的recv回调函数一定会触发2次，第1次的数据是123，第2次的数据是abc。
也就是说只要你调用了async_send或send，那么对端就一定会收到你调用async_send或send时的，那个完整的不多不少的数据，你调用了几次send，对方就会收到几次数据。

关于asio2的dgram模式下数据头的规则如下(和websocket的部分规则类似，稍微做了改变)：
部分代码在这里：[/asio2/include/asio2/base/detail/condition_wrap.hpp - dgram_match_role](https://github.com/zhllxt/asio2/blob/master/include/asio2/base/detail/condition_wrap.hpp)
websocket协议在这里：[RFC 6455: The WebSocket Protocol](https://www.rfc-editor.org/rfc/rfc6455#section-5.2)
如果数据内容长度小于等于253个字节，则数据头是1个字节，数据头的值等于数据内容的长度，之后是数据内容。
如果数据内容长度大于等于254小于等于65535字节，则数据头是3个字节，第1个字节的值等于254，后2个字节表示内容的长度。
如果数据内容大于65535字节，则数据头是9个字节，第1个字节的值等于255，后8个字节表示内容的长度。

## tcp_client_general
```cpp
// 这个例子中start函数没有第3个参数
client.start(host, port);
```

这个示例的意思是，没有做任何数据拆包的工作，无论收到什么数据，都会触发recv回调函数。比如你一次性发送了数据 123abc\n 并且期望对方也一次性完整接收到123abc\n，但实际情况可能不是这样，有可能bind_recv回调函数会触发多次，比如第1次收到123，第2次收到abc\n 总之只要收到操作系统socket的数据了，就立即通知回调函数，不管数据是否完整，因此在这种情况下收到什么数据的情况都有可能。

所以此时你必须自己做数据拆包的工作。

如果你就想这样用，并且就想自己做拆包的话，那接下来说一下可以怎么做：

##### 首先是一个简单的示例
```cpp
// 首先定义一个buffer用来保存接收的数据
std::string buffer;

// 下面是recv回调函数触发时，对收到的数据进行拆包的过程
bind_recv([&](std::string_view data)
{
	// 把收到的数据添加到你的buffer的尾部
	// 就像上面的描述一样，这里可能会多次触发，比如
	// 第1次触发时 data=123
	// 第2次触发时 data=abc\n
	buffer += data;
	
	// 第1次触发后，buffer=123          此时buffer.find('\n')肯定找不到
	// 第2次触发后，buffer=123abc\n     此时buffer.find('\n')就可以找到了

	// 在一个循环中查找\n
	// 这里只用\n来做个示意，其它的复杂协议的处理原理是一样的
	// 必须循环查找，因为可能一次性收到多包完整的数据
	while (true)
	{
		// 在buffer中查找\n
		auto pos = buffer.find('\n');

		// 如果找到了，说明缓冲区中已经有完整的数据了
		if (pos != std::string::npos)
		{
			// 取出这包完整的数据
			std::string msg = buffer.substr(0, pos + 1);

			// 调用你的业务函数，把数据传过去
			//do_work(msg);

			// 处理完这包数据了，把你的buffer中的这段数据删掉
			buffer.erase(0, pos + 1);

			// 接下来会继续执行while循环，继续查找\n，如果找到了就会继续处理
		}
		else
		{
			// 如果找不到就跳出循环，等待下一次接收数据后再放到buffer中再找\n
			break;
		}
	}
});
```

##### 深入一点的处理
上面的处理存在一个问题就是：再定义一个buffer，还需要把数据再往自己的这个buffer里面拷贝，这无形中增加了一些冗余的内存拷贝操作。

框架内部肯定已经有接收缓冲区了，收到的数据肯定也已经在缓冲区里了，那能不能不再自己去定义一个buffer了呢，直接去框架的缓冲区里面去操作数据呢？答案是可以的。

像这样做即可：

```cpp
// 第3个参数传入asio2::hook_buffer，这样就可以直接操作asio2框架内部的buffer了
client.start(host, port, asio2::hook_buffer);
```

```cpp
// 下面是recv回调函数触发时，对收到的数据进行拆包的过程
bind_recv([&](std::string_view data)
{
	// 这次和前面不一样了
	// 第1次触发时 data=123
	// 第2次触发时 data=123abc\n

	// 也就是说前面的没有使用hook_buffer的情况下，每次recv回调触发结束之后，asio2框架
	// 就会清空他内部的缓冲区，而使用了hook_buffer以后，每次recv回调触发之后，asio2
	// 框架不会清空他内部的缓冲区，而是把新收到的数据接着添加到缓冲区的尾部，此时处理
	// 完数据之后，你就需要自己手动清空缓冲区，如果你不手动清空缓冲区的话，那么缓冲区
	// 就会越来越大，表现出来的就是data这个string_view的长度会越来越长。

	// data 这个变量实际上就是整个缓冲区的全部的数据视图。
	
	// 仍然是在一个循环中查找\n
	while (true)
	{
		// 在data中查找\n
		auto pos = data.find('\n');

		// 如果找到了，说明缓冲区中已经有完整的数据了
		if (pos != std::string::npos)
		{
			// 取出这包完整的数据
			std::string_view msg = data.substr(0, pos + 1);

			// 调用你的业务函数，把数据传过去
			//do_work(msg);

			// 处理完这包数据了，我们要开始清理缓冲区了，否则缓冲区会越来越大
			// 这次清理缓冲区的方式和前面的不一样了，因为这次要清理的是asio2
			// 框架内部的缓冲区
			// consume(pos + 1)的意思是把缓冲区前面的pos + 1个字节的内容清理
			// 掉，清理掉之后，缓冲区后半部分的有效数据会移动到缓冲区的开头，
			// 这个形为和vector类似，调用consume之后缓冲区中的数据长度就变小了
			// 如果是server就是session_ptr->buffer().consume(pos + 1);
			client.buffer().consume(pos + 1);

			// 由于清理缓冲区之后，缓冲区的内容发生了变化，而data变量还是针对
			// 以前的缓冲区的内容，所以要重新给data变量赋值，否则继续下一次
			// while循环时在data中查找\n会出问题
			// 如果是server就是session_ptr->buffer().data_view();
			// 给string_view赋值是没有内存拷贝操作的，不用担心效率问题。
			data = client.buffer().data_view();

			// 接下来会继续执行while循环，继续查找\n，如果找到了就会继续处理
		}
		else
		{
			// 如果找不到就跳出循环，等待下一次接收数据后，asio2框架会把数据再
			// 次添加到其内部缓冲区的尾部，然后这里的bind_recv被触发后，再次
			// 查找 \n 即可
			break;
		}
	}
});
```

各个功能的具体的代码可以参考项目中的example示例。

### 最后编辑于：2022-11-04
