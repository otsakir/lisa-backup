#ifndef TERMINAL_H
#define TERMINAL_H



namespace Terminal {

int runCommandInTerminal(QString commandLine);

int runShellCommand(QString commandString, QString* pout = nullptr);


} // Terminal namespace

#endif // TERMINAL_H
