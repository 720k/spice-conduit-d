#include "ConsoleAppExitStrategy.h"
#include "ConsoleApplication.h"
#include <QCoreApplication>

int main(int argCount, char *argValues[]) {
    ConsoleApplication app(argCount, argValues);
    ConsoleAppExitStrategy exitStrategy; // ENTER to QUIT!
    return app.run();
}
