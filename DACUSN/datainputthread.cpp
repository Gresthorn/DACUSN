#include "datainputthread.h"

dataInputThread::dataInputThread(QObject * parent, QList<rawData *> *raw_data_stack, QMutex *raw_data_stack_mutex, uwbSettings *setts, QMutex *settings_mutex)
    : QThread(parent)
{
    errorCounterGlobal = errorCounter = 0;

    rawDataStack = raw_data_stack;
    rawDataStackMutex = raw_data_stack_mutex;

    settings = setts;
    settingsMutex = settings_mutex;

    settingsMutex->lock();
    recieverHandler = new reciever(settings->getRecieverMethod());
    settingsMutex->unlock();

    // checking if the selected method was successfully established
    if(recieverHandler->calibration_status()) qDebug() << "The reciever was configured";
    else qDebug() << recieverHandler->check_status_message();
}

dataInputThread::~dataInputThread()
{

}

void dataInputThread::run()
{
    // starting infinite loop for data recieving
    rawData * dataTempPointer;
    while(1)
    {
        dataTempPointer = recieverHandler->listen();
        if(dataTempPointer == NULL)
        {
            // something is wrong
            if(recieverHandler->curr_method_code()==UNDEFINED)
            {
                // no method is specified, making an idle state for 2 seconds
                qDebug() << "Reciever responds from idle state";
                Sleep(2000);
            }
            else
            {
                // if no UNDEFINED, the current method is probably corrupted
                qDebug() << recieverHandler->check_status_message();

                errorCounter++;
                errorCounterGlobal++;

                settingsMutex->lock();
                if(errorCounter>=settings->getMaximumRecieverErrorCount()) Sleep(2000);
                settingsMutex->unlock();
            }
        }
        else
        {
            qDebug() << dataTempPointer->getSyntheticTargetsCount();
            errorCounter = 0;
        }
    }
}

