#ifndef APPCONTEXT_H
#define APPCONTEXT_H

class TaskLoader;
class TaskManager;
class TaskRunnerManager;
namespace Common {
    class GlobalSignals;
}

class AppContext
{
public:
    AppContext();
    ~AppContext();

    TaskLoader* getTaskLoader();
    TaskManager* getTriggerMonitorWindow();
    TaskRunnerManager* taskRunnerManager;
    Common::GlobalSignals* globalSignals;

private:
    TaskLoader* taskLoader;
    TaskManager* triggerMonitorWindow;
};

#endif // APPCONTEXT_H
