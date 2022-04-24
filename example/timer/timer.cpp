#include <asio2/asio2.hpp>

int main()
{
	asio2::timer timer;

	// 1    - timer id
	// 1000 - timer inteval, 1000 milliseconds
	timer.start_timer(1, 1000, []()
	{
		printf("timer 1, loop infinite\n");
	});

	// "id2"- timer id, this timer id is a string
	// 2000 - timer inteval, 2000 milliseconds
	// 1    - timer repeat times
	timer.start_timer("id2", 2000, 1, []()
	{
		printf("timer id2, loop once\n");
	});

	// 3    - timer id
	// 3000 - timer inteval, 3000 milliseconds
	timer.start_timer(3, std::chrono::milliseconds(3000), []()
	{
		printf("timer 3, loop infinite\n");
	});

	// 4    - timer id
	// 4000 - timer inteval, 4000 milliseconds
	// 4    - timer repeat times
	timer.start_timer(4, std::chrono::milliseconds(4000), 4, []()
	{
		printf("timer 4, loop 4 times\n");
	});

	// 5    - timer id
	// std::chrono::milliseconds(1000) - timer inteval, 1000 milliseconds
	// std::chrono::milliseconds(5000) - timer delay for first execute, 5000 milliseconds
	timer.start_timer(5, std::chrono::milliseconds(1000), std::chrono::milliseconds(5000), []()
	{
		printf("timer 5, loop infinite, delay 5 seconds\n");
	});

	// 6    - timer id
	// std::chrono::milliseconds(1000) - timer inteval, 1000 milliseconds
	// 6    - timer repeat times
	// std::chrono::milliseconds(6000) - timer delay for first execute, 6000 milliseconds
	timer.start_timer(6, std::chrono::milliseconds(1000), 6, std::chrono::milliseconds(6000), []()
	{
		printf("timer 6, loop 6 times, delay 6 seconds\n");
	});

	while (std::getchar() != '\n');

	return 0;
}
