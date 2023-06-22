#include "QtWidgets/qplaintextedit.h"


namespace Logging {

    QPlainTextEdit* uiConsole = 0;

    void setUiConsole(void* newUiConsole)
    {
        // TODO - assert uiConsole is 0
        uiConsole = (QPlainTextEdit*) newUiConsole;
    }

    void logToUiConsole(QString message)
    {
        // TODO - assert uiConsole is not 0
        uiConsole->appendHtml(message);
    }


}
