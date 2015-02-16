#ifndef STACKMANAGER_H
#define STACKMANAGER_H

#include <QObject>
#include <QMutex>
#include <QVector>
#include <QThread>

#include <QDebug>

#include "rawdata.h"
#include "stddefs.h"
#include "uwbsettings.h"

class stackManager : public QObject
{
    Q_OBJECT

public:
    stackManager(QVector<rawData * > * raw_data_stack, QMutex * raw_data_stack_mutex, uwbSettings * setts, QMutex * settings_mutex);
    ~stackManager();

    Q_INVOKABLE void runWorker(void);

private:
    QVector<rawData * > * rawDataStack;
    QMutex * rawDataStackMutex;
    uwbSettings * settings;
    QMutex * settingsMutex;

    unsigned int idleTime;
    unsigned int stackControlPeriodicity;
    unsigned int lastStackCount;
    unsigned int stackWarningCount;
    unsigned int maxStackWarningCount;

    void stackControl(void);
    void dataProcessing(rawData * data);
};

#endif // STACKMANAGER_H
