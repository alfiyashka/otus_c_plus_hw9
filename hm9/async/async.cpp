#include "../include/async/async.h"
#include "../include/async/TaskManager.hpp"

#include <iostream>


namespace async {

TaskManager& taskManager = TaskManager::getInstance();

handle_t connect(std::size_t bulk)
{
    return taskManager.connect(bulk);
}

void receive(handle_t handle, const char *data, std::size_t size)
{
    taskManager.receive(handle, data, size);
}

void disconnect(handle_t handle)
{
    taskManager.disconnect(handle);
}

}
