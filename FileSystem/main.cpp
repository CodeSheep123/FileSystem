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
	explicit AsyncTask(std::string const& file_name, bool auto_start = true);
	~AsyncTask();

	/*Manually starts the task if it is not already running*/
	void start();

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

	std::thread m_exec_thread;

	bool m_running;
	bool m_paused;

	std::condition_variable m_pause_cv;
	std::mutex m_pause_mutex;

	void read(std::string const& file_name);
};

AsyncTask::AsyncTask(std::string const& file_name, bool auto_start) : m_file(file_name), 
	m_running(false), m_paused(false)
{	
	if (auto_start)
		start();
}

AsyncTask::~AsyncTask()
{
	m_exec_thread.join();
}

void AsyncTask::start()
{
	/*Only start the task if it is not already running*/
	if (!m_running)
	{
		m_exec_thread = std::thread(&AsyncTask::read, this, m_file);
		m_running = true;
	}
}

void AsyncTask::read(std::string const& file_name)
{
	/*---Open File---*/

	std::ifstream in(file_name);

	if (!in.good() || !in.is_open()) //#CHECK: Is this checking the same twice?
	{
		//make sure to lock, as we are trying to access stdout from a thread
		std::lock_guard<std::mutex> lck { std::mutex() };

		//#TODO: Use my fancy log++ library
		std::cerr << "Failed to open file " << file_name << "\n";

		//lock_guard goes out of scope, destructor called and stdout is unlocked
	}

	while (m_paused) //not sure if this is actually the right place to put it, or if it is actually useful here, so #TODO
	{
		std::unique_lock<std::mutex> lk { m_pause_mutex };
		m_pause_cv.wait(lk);
		lk.unlock();
	}

	/*---Read data---*/

	in.seekg(0, std::ios::end);
	auto length = in.tellg();

	//read all data
	auto* buffer = new char[length];
	in.get(buffer, length); //#TODO: Exception handling here, like what if not the entire file could be read?

	//write buffer data to the data stringstream
	m_data << buffer;
	
	/*---Cleanup---*/

	delete[] buffer; //no memory leaks
	in.close();
	m_running = false;
}

//#TODO: Implement
void AsyncTask::sleep(uint32_t ms)
{
	
}

void AsyncTask::pause()
{
	if (m_paused) //don't double pause
		return;
	std::lock_guard<std::mutex> lk { m_pause_mutex };
	m_paused = true;
}

void AsyncTask::resume()
{
	std::lock_guard<std::mutex> lk { m_pause_mutex };
	m_paused = false;
	m_pause_cv.notify_one();
}

void AsyncTask::wait()
{
	while (m_running)
	{
		//wait
	}
}

bool AsyncTask::is_complete()
{
	return !m_running;
}

void AsyncTask::reset()
{
	wait();
	m_running = false;
	m_data.clear();
	m_file = "";
}

int main()
{
	AsyncTask task("async_file.txt", false);

	task.start();

	return 0;
}