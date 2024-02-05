#include "appcontext.h"

#include "task.h"
#include "utils.h"

AppContext::AppContext()
{
    taskLoader = new TaskLoader(Lb::dataDirectory());
}

AppContext::~AppContext()
{
    delete taskLoader;
}

TaskLoader* AppContext::getTaskLoader()
{
    return taskLoader;
}
