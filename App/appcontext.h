#ifndef APPCONTEXT_H
#define APPCONTEXT_H

class TaskLoader;
class TaskManager;
class TaskRunnerManager;

class AppContext
{
public:
    AppContext();
    ~AppContext();

    TaskLoader* getTaskLoader();
    TaskManager* getTriggerMonitorWindow();
    TaskRunnerManager* taskRunnerManager;

private:
    TaskLoader* taskLoader;
    TaskManager* triggerMonitorWindow;
};

#endif // APPCONTEXT_H
