#include <iostream>
#include <fstream>
#include <thread>
#include <cstdint>
#include <vector>
#include <string>
#include <queue>
#include <sstream>

struct AsyncRequest
{
	std::string file;

	//will contain all data when reading is complete
	std::stringstream data;
};

class Async
{
public:
	void wait_for_request(AsyncRequest r);
	AsyncRequest add_request(std::string path);

private:
	std::thread m_read_thread;
	std::queue<AsyncRequest> requests;
};

int main()
{

}