#pragma once

#include <iostream>
#include <fstream>
#include <thread>
#include <string>
#include <sstream>
#include <mutex>
#include <functional>
#include <condition_variable>
#include "logpp/log++.h"



class AsyncTask
{
public:


	AsyncTask();

	template<typename Callable, typename ...Args>
	explicit AsyncTask(Callable task, Args&&... args);

	AsyncTask(AsyncTask const& other) = delete;
	AsyncTask(AsyncTask&& other);
	
	AsyncTask& operator=(AsyncTask const& other) = delete;
	AsyncTask& operator=(AsyncTask&& other);

	bool operator==(AsyncTask const& other);
	bool operator!=(AsyncTask const& other);

	~AsyncTask();

	/*Manually starts the task if it is not already running*/
	template<typename Callable, typename ...Args>
	void start(Callable func, Args&&... args);

	/*Interrupts the task until resume() is called*/
	void pause();

	/*Unpauses the task*/
	void resume();

	/*Blocks calling thread until the task is done*/
	void wait();

	bool is_complete();

	/*Stops the task and resets all it's data*/
	virtual void reset();

private:

	std::thread m_exec_thread;

	bool m_running;
	bool m_paused;

	std::condition_variable m_pause_cv;
	std::mutex m_pause_mutex;

protected:
	/*When using a derived class from Task, add this at then end of the eventual loop in the execution function*/
	void handle_pause_update();
	void toggle_running();
};


template<typename Callable, typename... Args>
AsyncTask::AsyncTask(Callable task, Args&&... args) : m_running(false), m_paused(false)
{
	start(task, std::forward<Args>(args)...);
}


template<typename Callable, typename ...Args>
void AsyncTask::start(Callable func, Args&&... args)
{
	/*Only start the task if it is not already running*/
	if (!m_running)
	{
		m_exec_thread = std::thread(func, std::forward<Args>(args)...);
		m_running = true;
	}
}


class AsyncReadTask : public AsyncTask
{
public:
	AsyncReadTask();
	explicit AsyncReadTask(std::string const& filename);

	/*Obtain data that was read from the file. Will reset the task*/
	std::stringstream get_data();

	void start(std::string const& filename);

	virtual void reset() override;

private:
	std::stringstream m_data;
	std::string m_file;

	void read(std::string const& file_name);
};