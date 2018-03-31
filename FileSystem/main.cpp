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

//#TODO: Remove unnecessary #include's

class AsyncException
{
public:
	explicit AsyncException(std::string msg) : m_msg(msg)
	{

	}

	std::string what()
	{
		return m_msg;
	}
private:
	std::string m_msg;
};

class AsyncTask
{
public:
	explicit AsyncTask(std::string const& file_name);
	AsyncTask(std::string const& file_name, std::thread& execution_thread);

	/*Manually starts the task if it is not already running*/
	void start(std::thread& execution_thread);

	/*Pauses the task for the specified amount of milliseconds*/
	void sleep(uint32_t ms);

	/*Interrupts the task until resume() is called*/
	void pause();

	/*Unpauses the task*/
	void resume();
	
	/*Blocks calling thread until the task is done*/
	void wait();

	bool is_complete();

	/*Stops the task and resets all it's data*/
	void reset();

	/*Obtain data that was read from the file. Will reset the task*/
	std::stringstream get_data();

private:
	std::stringstream m_data;
	std::string m_file;

	bool m_running;

	void read(std::string const& file_name);
};

AsyncTask::AsyncTask(std::string const& file_name, std::thread& execution_thread) : m_file(file_name), m_running(false)
{		
	start(execution_thread);
}

void AsyncTask::start(std::thread& execution_thread)
{
	/*Only start the task if it is not already running*/
	if (!m_running)
	{
		execution_thread = std::thread(&AsyncTask::read, this, m_file);
		m_running = true;
	}
}

void AsyncTask::read(std::string const& file_name)
{
	std::ifstream in(file_name);

	if (!in.good() || !in.is_open()) //#CHECK: Is this checking the same twice?
	{
		//make sure to lock, as we are trying to access stdout from a thread
		std::lock_guard<std::mutex> lck { std::mutex() };

		//#TODO: Use my fancy log++ library
		std::cerr << "Failed to open file " << file_name << "\n";
	}

	in.seekg(0, std::ios::end);
	auto length = in.tellg();
}

int main()
{
	return 0;
}