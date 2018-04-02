#include "AsyncTask.h"


#pragma region AsyncTaskImpl

AsyncTask::AsyncTask()
{

}

AsyncTask::AsyncTask(AsyncTask&& other) : m_exec_thread(std::move(other.m_exec_thread)),
	m_running(other.m_running),
	m_paused(other.m_paused)
{
	other.m_running = false;
}

AsyncTask& AsyncTask::operator=(AsyncTask&& other)
{
	if (*this != other)
	{
		m_exec_thread = std::move(other.m_exec_thread);
		m_running = other.m_running;
		m_paused = other.m_paused;
	}

	other.m_running = false;

	return *this;
}

bool AsyncTask::operator==(AsyncTask const& other)
{
	return m_exec_thread.get_id() == other.m_exec_thread.get_id();
}

bool AsyncTask::operator!=(AsyncTask const& other)
{
	return !(*this == other);
}

void AsyncTask::handle_pause_update()
{
	while (m_paused) //check if we have to pause
	{
		std::unique_lock<std::mutex> lk { m_pause_mutex };
		m_pause_cv.wait(lk);
		lk.unlock();
	}
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

}

void AsyncTask::toggle_running()
{
	m_running = !m_running;
}

AsyncTask::~AsyncTask()
{
	m_exec_thread.join();
}

#pragma endregion Implementation for AsyncTask class

#pragma region AsyncReadTaskImpl


std::stringstream AsyncReadTask::get_data()
{
	wait();

	auto data = std::move(m_data);
	reset();
	return std::move(data);
}


void AsyncReadTask::read(std::string const& file_name)
{
	/*---Open File---*/

	std::ifstream in(file_name);


	logpp::Console::log_assert(in.good() && in.is_open(), "Could not open file: " + file_name);


	/*---Read data---*/

	std::string line;
	while (std::getline(in, line))
	{
		line += "\n"; //getline removes the newline character, but we want to give the stringstream the exact same data as the file

		m_data << line;

		handle_pause_update();
	}


	/*---Cleanup---*/

	in.close();
	toggle_running();
}

AsyncReadTask::AsyncReadTask()
{

}

AsyncReadTask::AsyncReadTask(std::string const& filename) : AsyncTask(&AsyncReadTask::read, this, filename), m_file(filename)
{
	
}

void AsyncReadTask::start(std::string const& file_name)
{
	AsyncTask::start(&AsyncReadTask::read, this, file_name);
}

void AsyncReadTask::reset() 
{
	AsyncTask::reset();

	m_data.clear();
	m_file = "";
}

#pragma endregion Implementation for AsyncReadTask class