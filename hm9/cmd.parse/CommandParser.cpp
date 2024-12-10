#include "../include/cmd.parse/CommandParser.hpp"


#include <stdexcept>
#include <iostream>
#include <chrono>
#include <sstream>
#include <string>
#include <thread>
#include <fstream>


std::string CommandParser::generateFileNameWithCurrentTime(const std::string& postfix) const
{
    const auto current_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    return "bulk" + std::to_string(current_time) + postfix + ".log";
}

void CommandParser::writeOnFile(const std::string &data)
{
    const auto filename = generateFileNameWithCurrentTime();
    m_fileWriter.write(filename, data);
}

void CommandParser::log_work()
{
    while(!m_stopped || m_bulkFilled)
    {
        {
            std::unique_lock<std::mutex> lk(m_mutex);
            m_condV.wait(lk, [this]() ->bool { return m_bulkFilled || m_stopped; } );
            
            if (!m_bulkFilled)
            {
                continue;
            }
            const auto threadIdStr = getThreadId();
            std::cout << "Thread log work. Thread Id " << threadIdStr 
                << ". CmdParser object address is " << std::hex << this << std::endl;

            bool needDelimiter = false;
            for (auto iCmd : m_currentBulkCmds)
            {
                if (needDelimiter)
                {
                    std::cout << ", ";
                }
                std::cout << iCmd;
                needDelimiter = true;
            }
            
            std::cout << std::endl << std::endl;
            m_bulkFilled = false;
            m_currentBulkCmds.clear();
            lk.unlock();
            m_condV.notify_all();
        }

    }   

}

std::string CommandParser::getThreadId() const
{
    std::ostringstream idStream;
    idStream << "Thread_" << std::this_thread::get_id();

    return idStream.str();
}

void CommandParser::file_work()
{
    while (!m_stopped || !m_bulkCmds.empty())
    {

        cmds_t cmds;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condV.wait(lock, [this]
                         { return m_bulkCmds.empty() || m_stopped; });
            if (m_bulkCmds.empty())
            {
                continue;
            }

            cmds = m_bulkCmds.back();
            m_bulkCmds.pop_back();
            m_condV.notify_all();
        }
        if (!cmds.empty())
        {
            const auto threadIdStr = getThreadId();

            std::cout << "Thread file work. Thread Id " << threadIdStr
                      << ". CmdParser object address is " << std::hex << this << std::endl;

            const auto filename = "Bulk_info_" + generateFileNameWithCurrentTime(threadIdStr);

            std::ofstream file;
            file.open(filename, std::fstream::out);

            for (const auto &data : cmds)
            {
                file << data << std::endl;
            }
            file.close();
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }
}

void CommandParser::completePackage()
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condV.wait(lock, [this ] { return !m_bulkFilled; });        
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_currentBulkCmds.empty())
    {
        m_bulkCmds.push_back(m_currentBulkCmds);
        m_bulkFilled = true;
        m_condV.notify_all();            
    }
      
}

void CommandParser::init()
{
    if (m_packageSize <= 0)
    {
        throw std::runtime_error("Incorrect package size");
    }
    m_currentBulkCmds.reserve(m_packageSize);
    m_fileWriter.init();
}

void CommandParser::initThreads()
{
    m_logThread = std::thread(&CommandParser::log_work, this);
    m_file1Thread = std::thread(&CommandParser::file_work, this);
    m_file2Thread = std::thread(&CommandParser::file_work, this); 
}

bool CommandParser::cmdIsOnlySpace(const std::string &cmd) const 
{
    auto truncSpace = cmd;
    std::erase(truncSpace, ' ');
    return truncSpace.empty();
}

void CommandParser::parse(const std::string &cmd)
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condV.wait(lock, [this ] { return !m_bulkFilled || m_currentBulkCmds.empty(); });        
    }


    if (cmd.empty())
    {
        //std::cerr << "Encountered empty command";
        return;
    }
    if (cmdIsOnlySpace(cmd))
    {
        return;
    }

    if (m_state.modifyState(cmd))
    {
        if (m_state.isDynamicModeStarting()
            || m_state.isDynamicModeCompleted())
        {
            completePackage();
        }
        return;
    }
    writeOnFile(cmd);
    
    m_currentBulkCmds.emplace_back(cmd);

    if (isStaticModeComplete())
    {
        completePackage();
    }
}

void CommandParser::endJob()
{
    if (m_state.isStaticMode() || m_state.isDynamicModeCompleted())
    {
        completePackage();
    }
}


CommandParser::~CommandParser()
{
    endJob();
    m_stopped = true;
    m_condV.notify_all();
    if (m_logThread.joinable())
        m_logThread.join();
    if (m_file1Thread.joinable())
        m_file1Thread.join();
    if (m_file2Thread.joinable()) 
        m_file2Thread.join();
}