#include "../include/async/TaskManager.hpp"
#include "../include/cmd.parse/CommandParser.hpp"

#include <iostream>
#include <stdexcept>

void CmdParserTask::workerFunction()
{
    while(!m_stopped || !m_dataQueue.empty())
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condWaitTask.wait(lock, [this] {return !m_dataQueue.empty() || m_stopped;});
        if (m_dataQueue.empty())
        {
            continue;
        }       

        const auto& dataToParse = m_dataQueue.front();
        auto cmdParser = static_cast<CommandParser*>(m_handle);
        cmdParser->parse(dataToParse);

        m_dataQueue.pop_front();
    }
}

void CmdParserTask::addDataToProcess(const char *data, std::size_t size)
{
    {
       std::lock_guard<std::mutex> lock(m_mutex);
       m_dataQueue.push_back(std::string(data, size));    
       m_condWaitTask.notify_all(); 
    }    
}

handle_t TaskManager::connect(std::size_t bulk) {
    auto cmdParser = new CommandParser(bulk);
    handle_t handle = static_cast<handle_t>(cmdParser);
    m_handleTaskCache.insert(handleTask_t::value_type(handle, nullptr));
    return handle;
}

void TaskManager::receive(handle_t handle, const char *data, std::size_t size) {
    if (size == 0 || size == static_cast<std::size_t>(-1))
    {
        return; //no data to process
    }
    auto foundHandle = this->m_handleTaskCache.find(handle);
    if (foundHandle != m_handleTaskCache.end())
    {
        //here the thread, that works with handle, exists
        if (foundHandle->second.get() == nullptr)
        {
            foundHandle->second.reset(new CmdParserTask(handle));           
        }
        
        foundHandle->second->addDataToProcess(data, size);
    }
    else
    {
        // and a new task, create thread
        throw std::runtime_error("The handle is unknown. Disconnected. Logic error!");
    }
}


void TaskManager::disconnect(handle_t handle) {
    
    auto foundHandle = this->m_handleTaskCache.find(handle);
    if (foundHandle != m_handleTaskCache.end())
    {
        auto cmdParser = static_cast<CommandParser*>(handle);        
        m_handleTaskCache.erase(foundHandle);
        delete cmdParser;
    }
    else
    {
        std::cerr<<"Logic Error. Encountered unregistred handle with adress " << std::hex << handle;
    }
}

TaskManager::~TaskManager()
{
    for (auto ihandleTask = m_handleTaskCache.begin();
        ihandleTask != m_handleTaskCache.end();
        )
    {
        const auto handle = ihandleTask->first;
        auto cmdParser = static_cast<CommandParser*>(handle);        
        m_handleTaskCache.erase(ihandleTask++);
        delete cmdParser;
    }
}