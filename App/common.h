#ifndef COMMON_H
#define COMMON_H


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


}



#endif // COMMON_H
