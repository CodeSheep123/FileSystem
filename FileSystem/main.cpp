#include "AsyncTask.h"

#include <chrono>

void print_numbers(int amount)
{
	std::ofstream out("out.txt");
	for (int i = 0; i < amount; ++i)
	{
		out << i << "\n";
	}

	out.close();
}


int main()
{
	AsyncReadTask task(R"(C:\Users\michi_000\Desktop\C++\Custom Libraries\FileSystem\Debug\async_file.txt)");
	AsyncReadTask second_task(R"(C:\Users\michi_000\Desktop\C++\Custom Libraries\FileSystem\Debug\async_file_2.txt)");

	AsyncTask printing(print_numbers, 100000);

	printing.cancel();

	auto start = std::chrono::system_clock::now();

	std::cout << "Task has started, waiting for it to finish...\n";

//	task.pause();

//	std::this_thread::sleep_for(std::chrono::seconds(5));

//	task.resume();

	task.wait();
	second_task.wait();
	printing.wait();


	auto end = std::chrono::system_clock::now();

	std::cout << "Tasks finished in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms, obtaining all data...\n";
	
	auto stream = task.get_data();
	auto stream2 = second_task.get_data();

	std::string first_line;
	std::getline(stream, first_line);

	std::string second_line;
	std::getline(stream2, second_line);

	std::cout << "First line of the data obtained by task 1 is: " << first_line << "\n";
	std::cout << "First line of the data obtained by task 2 is: " << second_line << "\n";

	std::cin.get();

	return 0;
}