#include <iostream>
#include <fstream>
#include <thread>
#include <cstdint>
#include <vector>

class async
{
public:
	//returns a handle to file that is being read
	static uint32_t read(char const* fname);

	static void wait(uint32_t handle);
private:
	static std::vector<std::thread> read_threads;

	static void read_file(char const* fname);
};

std::vector<std::thread> async::read_threads {};

uint32_t async::read(char const* fname)
{
	//add thread that starts reading the file
	read_threads.emplace_back(read_file, fname);
	return read_threads.size() - 1; //temp, must be fixed as all handles will invalidate when a thread gets removed from the list
}

void async::wait(uint32_t handle)
{
	read_threads[handle].join();
	read_threads.erase(read_threads.begin() + handle - 1);
}

void async::read_file(char const* fname)
{

}

int main()
{
	auto a = async::read("test.txt");
	auto b = async::read("test2.txt");

	async::wait(b);
	async::wait(a);
}