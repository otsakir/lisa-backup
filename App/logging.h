#ifndef LOGGING_H
#define LOGGING_H

#include <QString>

namespace Logging {

    void setUiConsole(void* newUiConsole);
    void logToUiConsole(QString message);

}

#endif // LOGGING_H
