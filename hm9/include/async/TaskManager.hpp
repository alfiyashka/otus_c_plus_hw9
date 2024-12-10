#ifndef _TASK_MANAGER_HPP
#define _TASK_MANAGER_HPP

#include <map>
#include <atomic>
#include <memory>
#include <thread>
#include <deque>
#include <condition_variable>

using handle_t = void *;

class CmdParserTask 
{

private:
	std::thread m_thread;
	std::atomic<bool> m_stopped;

    using dataQueue_t = std::deque<std::string>;

    // queue of data to process 
    dataQueue_t m_dataQueue;

    // command parser
    handle_t m_handle;

    std::mutex m_mutex;
    std::condition_variable m_condWaitTask;
    
    /**
     * thread work function
     */
    void workerFunction();
    
public:

    /**
     * adds data to process into queue
     * @param data data start pointer
     * @param size data size
     */
    void addDataToProcess(const char *data, std::size_t size);

    /** constructor */
    CmdParserTask(handle_t handle): m_handle(handle)
    {
        m_thread = std::thread(&CmdParserTask::workerFunction, this);
    }

    /**
     * destructor
     */
    ~CmdParserTask()
    {
        m_stopped = true;
        m_condWaitTask.notify_one();
        if (m_thread.joinable())
            m_thread.join();
    }
};

class TaskManager
{
    using handleTask_t = std::map<handle_t, std::shared_ptr<CmdParserTask>>;

    handleTask_t m_handleTaskCache;

    TaskManager() = default;
    ~TaskManager();

    TaskManager(const TaskManager& t) = delete;
    TaskManager& operator=(const TaskManager& t) = delete;
    
public:

    handle_t connect(std::size_t bulk);
    void receive(handle_t handle, const char *data, std::size_t size);
    void disconnect(handle_t handle);

    static TaskManager& getInstance()
    {
        static TaskManager instance;
        volatile int dummy{};
        return instance;
    }
};


#endif
