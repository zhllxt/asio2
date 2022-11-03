# asio做tcp的自动拆包时，asio的match condition如何使用的详细说明

首先说点基础概念：因为tcp是流式的，所以如果你自己定制有协议（一般如数据头，命令，数据长度，数据内容，校验），
在tcp下发送你自己的协议的数据包时，就必须要考虑tcp的封包和拆包的问题了。

目前asio使用的人也比较多了吧，我觉得asio对tcp的封包和拆包的支持和设计做的很不错，如果你用asio去做一次对
tcp的拆包，会发现真的是相当方便的。

我在asio的基础上包装了一个网络框架asio2（见[https://github.com/zhllxt/asio2](https://github.com/zhllxt/asio2)或[https://gitee.com/zhllxt/asio2](https://gitee.com/zhllxt/asio2)），里面包含了对TCP数据拆包功能的完整支持(当然内部还是用的asio本身来实现的)，按指定的单个字符，按字符串，按用户自定义协议 对数据自动进行拆包,最近发现有些人问起asio的这个自动拆包match condition如何使用的问题，而网上也搜不到match condition的详细使用说明或资料，于是这里写一下，给大家提供点思路。详细代码和说明如下：

```cpp

// 假定协议格式为：数据头、数据长度、数据内容
// 类型如下：
// int head;   // 数据头      假定数据头的值固定为0x01020304
// int length; // 数据长度
// ...         // 数据内容    数据内容的长度是可变的

// 接下来示意用asio的match condition如何来进行解析
using iterator = asio::buffers_iterator<asio::streambuf::const_buffers_type>;
std::pair<iterator, bool> match_role(iterator begin, iterator end)
{
	// 当asio内部接收到数据后，将数据存储在它的缓冲区中，然后就会调用你的match_role函数，
	// 参数begin表示数据的起始位置，也就是数据的开头
	// 参数end表示数据的结尾向后偏移一个字节，类似字符串char*的结尾会向后偏移一个字节，那个\0
	// 注意end虽然是数据的尾部，但它只是所接收到的数据的尾部，并不一定是你的整包数据的尾部
	// 这里我来举个例子仔细说明：
	// 1、假定你构造好了一包完整的数据，如下：
	//    0x01 02 03 04   00 00 00 01      41
	//    <----头----->   <--长度--->   <-数据->  41是字符A
	// 2、发送该包数据
	// 3、对端开始接收数据
	//    我们拿极端情况举例：假定对端第1次只收到1个字节0x01，asio内部会把这1个字节放到它的缓冲区中，
	//    缓冲区内此时有1个字节：0x01 此时begin就是0x01这个数据的指针地址，end是0x01这个位置向后偏移
	//    一个字节；对端第2次又只收到1个字节0x02，asio内部会把字节0x02接着放到缓冲区中，缓冲区中此
	//    时有2个字节：0x01 02 此时begin还是0x01这个数据的指针地址，而end是0x02这个位置向后偏移一个
	//    字节。依次类推...，记住begin永远是数据的开始位置
	// 那么这个iterator是个什么东西呢？简单理解，你可以把它当成一个char*来用就行了，有++自增，有*取值

	// 接下来开始实例怎么解析：
	iterator i = begin; // 首先把begin保存到一个临时变量i中，你可以简单理解为char * i = begin;

	// 用end减去begin就是当前缓冲区中的数据长度，我们先看这个长度够不够最小的长度，最小的长度就是head
	// 类型的长度4个字节 加上 length类型的长度4个字节，共8个字节，如果当前缓冲区中的数据不够8个字节，
	// 就还得接收下一次的数据
	if (end - begin < 4 + 4)
	{
		// 返回值的含义：首先必须返回一个pair类型
		// pair的第一个参数要填一个iterator，可以理解为填一个char*即可，表示满足你要求的那一包数据的结尾位置
		// pair的第二个参数表示当前缓冲区的数据是否满足你的要求，如果满足就是true，不满足就是false
		// 如果pair的第二个参数你填了false，就意味着告诉asio当前缓冲区中的数据还不够，你再给我接着接收，
		// 当下一次又接收到数据时，asio又会“再次调用”这里的match_role函数，而这个“再次调用”的第一个参
		// 数begin就是此时你在pair的第一个参数中填写的这个地址值。
		// 如果pair的第二个参数你填了true，意味着当前缓冲区的数据已经满足要求了，会发生下面的事情：
		// 当这个pair返回到asio的内部之后，asio内部会做两个操作：1、把从“begin到pair的第一个参数”这一段
		// 数据通过回调函数发给你，2、你在你的回调函数里处理这些数据过后，asio就会把这段数据清理掉了，然后下
		// 一次再进到这里的match_role函数时，参数begin就是刚才你通过pair第一个参数所给的那个数据的位置了，
		// 这里由于长度不足，所以我们把pair的第一个参数填为begin，表示下一次解析时还从begin这个位置开始解析，
		// 第二个参数填为false，表示当前缓冲区中的数据不满足要求，需要接着接收

		return std::pair(begin, false);
	}

	// 最小长度够了，接着判断数据头head是不是0x01020304
	// i.operator->() 表示取出i里面数据的指针，也就是将i变为了const char *类型
	// 取出char*指针再强转为int*指针，再取*号，就转换成一个整数了，注意asio2已经处理过大小端的问题了，你不用考虑
	// 强制转换时的大小端问题
	int head = *(reinterpret_cast<const int*>(i.operator->()));
	if (head != 0x01020304)
	{
		// 如果head不等于0x01020304则说明数据非法，说明客户端可能根本不是你自己的客户端，如果是你自己的客户
		// 端，而且按照协议发的数据，那么是不可能出现这种情况的，所以此时最好直接断开这个连接，怎么断开呢？
		// 往后面看有关于这个问题的说明。
		return std::pair(begin, true);
	}

	// 最小长度和数据头都对了，开始判断数据内容的长度
	i += 4; // i向后偏移4个字节，到length这个位置处
	int length = *(reinterpret_cast<const int*>(i.operator->()));// 求出这包数据的内容的长度
	// end - begin等于总长度，再减去8表示真正的数据的内容的长度
	// 如果内容长度小于求出的长度，则还需要接着接收，所以这里还是返回pair(begin, false)
	if (end - begin - 8 < length)
		return std::pair(begin, false);

	// 现在最小长度和数据头和内容长度都对了，
	i += 4; // i再向后偏移4个字节，到真正的数据内容这个位置处
	// 返回值中的i + length也就是这一包数据的结尾处了，true表示数据满足要求了，这样asio就会把从begin到i + length
	// 这个指针位置处的数据全部返回给你，当下一次asio再次调用这里的match_role时，传入的begin就是i + length向后偏移
	// 一个字节的位置了，相当于把下一包数据的开始位置传给你了
	return std::pair(i + length, true);
}

// 下面是我asio2原demo中的示例，这里用了while循环，其实用不用while循环没有任何差别，或者说用while循环完全没有任何
// 意义，因为看代码就知道这个while循环只可能执行一次，那我为什么还写了个while呢，这好像是参考哪里的示例不记得了，
// 没怎么管就直接搬上来了，这段示例代码我选择也放上来而没有删除，主要给大家多个参考吧。
// 还有要注意这下面的解析所用的协议和我上面举例用的协议是不同的，上面协议中的包头和长度都是int类型的，而下面不是的。
// 当然大家可能会有这种经验：如果一次性接收了多包完整的数据，一般解析数据时是要用while循环解析的，否则会出问题，
// 但asio如果一次性接收到多包完整的数据的话，这里是不需要用while循环来解析的，因为每返回一次pair(i + length, true)
// 之后，asio内部后面的代码中会接着再调用这里的match_role函数，于是又再次开始解析，相当于asio帮你做了那个while循环
//while (i != end)
//{
//	if (*i != '#')
//		return std::pair(begin, true); // head character is not #, return and kill the client

//	i++;
//	if (i == end) break;

//	int length = std::uint8_t(*i); // get content length

//	i++;
//	if (i == end) break;

//	if (end - i >= length)
//		return std::pair(i + length, true);

//	break;
//}
//return std::pair(begin, false);

```

**还有一个比较常见的问题是：使用了match condition后，如果发现接收的数据非法，怎么关闭连接呢？**

```cpp
class match_role
{
public:
	explicit match_role(char c) : c_(c) {}

	template <typename Iterator>
	std::pair<Iterator, bool> operator()(Iterator begin, Iterator end) const
	{
		Iterator i = begin;
		while (i != end)
		{
			// 发现数据不正确后，如何关闭非法连接？

			// 方法1：直接调用连接的关闭函数即可，但是这有个前提，这个match condition必须是一个类，
			// 而不能是一个全局函数(asio的match condition可以是一个类，也可以是一个函数)，而且要
			// 在这个类中添加一个init函数来保存连接指针，具体请看下面的init函数说明。
			if (*i != c_)
			{
				session_ptr_->stop();
				break;
			}

			// 方法2：直接返回std::pair(begin, true);然后在bind_recv的回调函数里判断，
			// 如果接收到的数据长度是0就关闭连接。
			// 返回值pair的第二个参数true表示数据解析成功，而pair的第一个参数给的是begin表示数据的长度是0。
			// 于是bind_recv的回调函数就会被触发，在bind_recv回调函数中判断一下数据长度，如果长度是0就表
			// 示数据非法，直接关闭连接即可。（具体代码请参考示例程序tcp_server_custom.cpp）：
			if (*i != c_)
				return std::pair(begin, true);

			i++;
			if (i == end) break;

			int length = std::uint8_t(*i); // get content length

			i++;
			if (i == end) break;

			if (end - i >= length)
				return std::pair(i + length, true);

			break;
		}
		return std::pair(begin, false);
	}

	// 当连接刚刚创建完成之后，框架会自动调用这个init函数，你可以在这个函数里把这个连接
	// 对应的session_ptr保存起来，留着后面使用
	void init(std::shared_ptr<asio2::tcp_session>& session_ptr)
	{
		session_ptr_ = session_ptr;
	}

private:
	char c_;

	// 这里直接用智能指针不会导致循环引用
	std::shared_ptr<asio2::tcp_session> session_ptr_;
};

server.bind_recv([&](auto & session_ptr, std::string_view s)
{
	// 在这里判断数据长度是不是0，如果是0就表示数据非法，直接关闭连接
	if (s.size() == 0)
	{
		printf("close illegal client : %s %u\n",
			session_ptr->remote_address().c_str(), session_ptr->remote_port());
		// 关闭连接
		session_ptr->stop();
		return;
	}

	printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());

});
```

关键词：tcp 自动 封包 拆包 asio match condition role asio2


### 最后编辑于：2022-11-04
