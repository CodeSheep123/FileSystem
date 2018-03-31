#include <iostream>
#include <fstream>
#include <thread>
#include <cstdint>
#include <vector>
#include <string>
#include <queue>
#include <sstream>
#include <mutex>
#include <functional>
#include <type_traits>

#pragma warning(disable: 4244)

class AsyncTask
{
public:
	explicit AsyncTask(std::string const& file_name, std::thread& execution_thread, bool auto_start = true);

	void start();
	void pause(uint32_t ms);
	//interrupts the task
	void stop();
	void resume();
	void wait();
	bool is_complete();
	void reset();

	//Will reset the Task
	std::stringstream get_data();
private:
	std::stringstream data;
	std::string file;

	void read(std::string const& file_name);
};

AsyncTask::AsyncTask(std::string const& file_name, std::thread& execution_thread, bool auto_start) : file(file_name)
{
	if (auto_start)
		execution_thread = std::thread(&read, this, file_name);
}

void AsyncTask::read(std::string const& file_name)
{

}

int main()
{
	return 0;
}