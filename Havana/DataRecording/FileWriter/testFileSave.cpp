#include "SignatecDAQ.h"

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename T>
class Queue
{
public:
    T pop()
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        while (queue_.empty())
        {
            cond_.wait(mlock);
        }
        auto item = queue_.front();
        queue_.pop();
        return item;
    }

    void pop(T& item)
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        while (queue_.empty())
        {
            cond_.wait(mlock);
        }
        item = queue_.front();
        queue_.pop();
    }

    void push(const T& item)
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        queue_.push(item);
        mlock.unlock();
        cond_.notify_one();
    }

    void push(T&& item)
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        queue_.push(std::move(item));
        mlock.unlock();
        cond_.notify_one();
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
};



#include <Windows.h>

const int nScans = 1300;
const int nAlines = 1024;

const char *FilePath = "F:\\ssd_test.dat";

class FileWriter
{
public:
    FileWriter();
    ~FileWriter();

    void write(const uint16_t *frame);

private:
    // file
    HANDLE hFile;

	// thread
	std::thread _thread;
	bool _running;
	void run();

    // queue
    std::queue<uint16_t *> _bufferPool;
    std::mutex _bufferPoolLock;
    Queue<uint16_t *> _writeQueue;
};

using namespace std;








void test_fwrite()
{
    FileWriter writer;

    SignatecDAQ daq;

    daq.nScans = nScans;
    daq.nAlines = nAlines;
    daq.UseInternalTrigger = true;

    uint16_t *buffer = new uint16_t[2 * nScans * nAlines];
    daq.DidAcquireData += [&daq, &writer, buffer](int frame_count, const np::Array<uint16_t, 2> &frame)
    {
        memcpy(buffer, frame.raw_ptr(), 2 * nScans * nAlines * sizeof(uint16_t));
        writer.write(buffer);
    };

    // Run acquisition until press enter key
    daq.Start();
    puts("Press enter to stop...");
    getchar();

    // Stop acquisition
    daq.Stop();

    puts("Press enter to exit...");
    getchar();
}