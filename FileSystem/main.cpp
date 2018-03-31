#include <iostream>
#include <fstream>
#include <thread>
#include <string>
#include <sstream>
#include <mutex>
#include <chrono>

#pragma warning(disable: 4244)


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
//		std::lock_guard<std::mutex> lck { std::mutex() };

		//#TODO: Use my fancy log++ library
		std::cerr << "Failed to open file " << file_name << "\n";

		//lock_guard goes out of scope, destructor called and stdout is unlocked
	}


	/*---Read data---*/

	std::string line;
	while (std::getline(in, line))
	{
		line += "\n"; //getline removes the newline character, but we want to give the stringstream the exact same data as the file

		m_data << line;

		while (m_paused) //check if we have to pause
		{
			std::unique_lock<std::mutex> lk { m_pause_mutex };
			m_pause_cv.wait(lk);
			lk.unlock();
		}
	}
	

	/*---Cleanup---*/

	in.close();
	m_running = false;
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

std::stringstream AsyncTask::get_data()
{
	wait();

	auto data = std::move(m_data);
	reset();
	return std::move(data);
}

int main()
{
	AsyncTask task(R"(C:\Users\michi_000\Desktop\C++\Custom Libraries\FileSystem\Debug\async_file.txt)", false);

	auto start = std::chrono::system_clock::now();

	task.start();

	std::cout << "Task has started, waiting for it to finish...\n";

//	task.pause();

//	std::this_thread::sleep_for(std::chrono::seconds(5));

//	task.resume();

	task.wait();

	auto end = std::chrono::system_clock::now();

	std::cout << "Task has finished in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms, obtaining its data...\n";

	auto stream = task.get_data();

	std::string first_line;
	std::getline(stream, first_line);

	std::cout << "First line of the data obtained by the task is: " << first_line;

	std::cin.get();

	return 0;
}