#ifndef COMMON_H
#define COMMON_H

#include <QObject>

namespace Common
{

    enum TaskRunnerEventType
    {
        ProcessStateChanged
    };

    // who started the task ?
    enum TaskRunnerReason
    {
        Triggered,
        Manual
    };

    // fascilitates wiring of signals between (otherwise) unrelated components
    class GlobalSignals : public QObject
    {
        Q_OBJECT
        public:
        signals: void taskModified(const QString taskName); // a task has been saved to disk

    };


}



#endif // COMMON_H
