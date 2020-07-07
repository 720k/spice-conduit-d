#ifndef CONSOLEAPPEXITSTRATEGY_H
#define CONSOLEAPPEXITSTRATEGY_H

#include <QObject>
#include <future>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(catConsoleAppExitStrategy)

class ConsoleAppExitStrategy : public QObject
{
    Q_OBJECT
public:
    explicit                    ConsoleAppExitStrategy(QObject *parent = nullptr);
                                ~ConsoleAppExitStrategy();

signals:
private:
    void                        timerEvent(QTimerEvent *event);

    bool                        exitFlag_=false;
    std::future<void>           exitFn_;
    int                         timerId_;
};

#endif // CONSOLEAPPEXITSTRATEGY_H
