#include "reciever.h"

reciever::reciever(reciever_method recieveMethod)
    #if defined (__WIN32__)
    : maximum_pipe_size(512)
    #endif
{
    r_method = UNDEFINED;
    last_data_pt = NULL;
    statusMsg = NULL;

    // since for now we are not using serial port, default values are set
    comPort = -1;
    comPortBaudRate = 9600;
    comPortMode = NULL;
    comPortCallibration = false;
    packetReciever = NULL;

    calibrationStatus = calibrate(recieveMethod);

    if(calibrationStatus) set_msg("Calibration successfull.");
    else set_msg("An error occured when trying to set up selected method.");
}

reciever::reciever(reciever_method recieveMethod, int comport_ID, int baud_rate, char *comport_mode)
    #if defined (__WIN32__)
    : maximum_pipe_size(0)
    #endif
{
    r_method = UNDEFINED;
    last_data_pt = NULL;
    statusMsg = NULL;

    comPort = comport_ID;
    comPortBaudRate = baud_rate;
    comPortMode = comport_mode;
    comPortCallibration = false;
    packetReciever = NULL;

    calibrationStatus = calibrate(recieveMethod);

    if(calibrationStatus) set_msg("Calibration successfull.");
    else set_msg("An error occured when trying to set up selected method.");
}

reciever::~reciever()
{
    cancel_previous_method();

    if(comPortMode!=NULL) delete comPortMode;
    if(statusMsg!=NULL) delete statusMsg;
}

void reciever::set_msg(const char *msg)
{
    // free the old message
    if(statusMsg!=NULL) delete statusMsg;
    // create new message
    statusMsg = strdup(msg);
}

rawData * reciever::listen()
{
    if(!calibrationStatus)
    {
        set_msg("Lastly specified data obtaining method is not accessible. Please select new method.");
        return NULL;
    }

    rawData * data = NULL;

    if(r_method==UNDEFINED)
    {
        set_msg("The method specified is undefined. Please select new method.");
        return NULL;
    }
    #if defined (__WIN32__)
    else if(r_method==SYNTHETIC)
    {
        // creating buffer
        char message[maximum_pipe_size];
        bool success = readFromPipe(pipe_connection_handler, message);

        // checking for correctness
        if(!success) {
            set_msg("Could not read from pipe. Pipe was corrupted or server had quit.");
            last_data_pt = NULL;
            return NULL;
        }

        // data conversion
        data = extract_synthetic(message);
        last_data_pt = data; // replacing last known data object pointer
    }
    #endif
    else if(r_method==RS232)
    {

        // recieve new packet for serial link
        bool success;

        // variables for time measurement
        clock_t start = clock();
        clock_t end;
        double elapsed = 0.0;

        while(1) {
            success = packetReciever->recievePacket();

            if(success) break; // if packet is read, break the loop so program can process it

            end = clock();
            elapsed = 1000.0*(double(end-start))/CLOCKS_PER_SEC; // time elapsed in miliseconds

            if(elapsed>500.0) break; // time out

            // we do not want to blow up CPU
            #if defined (__WIN32__)
            Sleep(5);
            #endif
            #if defined(__linux__) || defined(__FreeBSD__)
            usleep(5000);
            #endif
        }

        if(!success) {
            set_msg("Could not read complete packet from serial link. Packet was obviously corrupted or there is a hardware problem.");
            last_data_pt = NULL;
            return NULL;
        }

        // if packet is unreadable, return NULL and save appropriate message
        if(!packetReciever->readPacket())
        {
            // ADD MESSAGES ACCORDING TO CODE HERE!!!
            return NULL;
        }

        // data recieved, now need to convert packet into rawData object
        data = extract_RS232_radar_packet();
        last_data_pt = data;
    }
    else data = last_data_pt = NULL; // when no method was selected

    return data;
}

bool reciever::set_new_method_code(reciever_method recieveMethod, bool kill)
{
    if(!cancel_previous_method())
    {
          // if somehow we could not cancel previous method, need to check if old method was revealed or not
          // if the previous method was UNDEFINED, the cancel_previous_method only returns true so it should
          // never come to this condition
          if(r_method!=UNDEFINED)
          {
              if(kill) r_method = UNDEFINED; // if kill is true, then UNDEFINED is set neverthless and new method will try to start
              else
              {
                  set_msg("The old method could not be cancelled correctly, but was successfuly restarted.");
                  return false;
              }
          }
    }
    else r_method = UNDEFINED;

    // now the new method will try to start. The 'r_method' is here surely UNDEFINED.
    switch(recieveMethod)
    {
        case UNDEFINED:
            r_method = UNDEFINED;
            set_msg("New method is set up as undefined. No data can be get.");

        #if defined (__WIN32__)
        case SYNTHETIC:
            // settings for synthetic data obtaining
            calibrationStatus = calibrate(recieveMethod);

            if(!calibrationStatus) set_msg("The pipe channel cannot be set. Check if server has already started.");
            else r_method = SYNTHETIC;

            return calibrationStatus;

            break;
        #endif

        case RS232:
            calibrationStatus = calibrate(recieveMethod);
            if(!calibrationStatus) set_msg("The COM port could not be opened. Check if COM index is correcly set or COM port is already in use.");
            else r_method = RS232;

            return calibrationStatus;

            break;

        default:
            set_msg("You are trying to set up unavailible method. The old method is allowed.");
            return false;
            break;
    }

    return false;
}

bool reciever::calibrate(reciever_method recieveMethod)
{

    if(recieveMethod==UNDEFINED)
    {
        // probably never can UNDEFINED get here, but - just in case
        r_method = recieveMethod;
        return true;
    }
    #if defined (__WIN32__)
    else if(recieveMethod==SYNTHETIC)
    {
        // doing calibration for pipe communication
        pipe_connection_handler = connectToPipe();
        if(pipe_connection_handler==NULL || pipe_connection_handler==INVALID_HANDLE_VALUE) return false; // cannot connect to pipe
        else {
            r_method = recieveMethod;
            return true; // everything is ok and done
        }
    }
    #endif
    else if(recieveMethod==RS232)
    {
        // do callibration for RS232 communication
        if(RS232_OpenComport(comPort, comPortBaudRate, comPortMode)>0)
        {
            // function returns number grater than zero if error occured
            // reasons of error may differ: invalid comport number, invalid baudrate or mode, comport in use, etc.
            comPortCallibration = false;
            return false;
        }
        else
        {
            // comport opened successfuly and is ready for recieving data
            comPortCallibration = true;
            packetReciever = new uwbPacketRx(comPort);
            r_method = recieveMethod;
            return true;
        }

    }
    else return false;

    return false;
}

bool reciever::cancel_previous_method()
{
    if(r_method==UNDEFINED) return true; // nothing special is required
    #if defined (__WIN32__)
    else if(r_method==SYNTHETIC)
    {
        // closing pipe channel
        closeConnection(pipe_connection_handler);
        return true;
    }
    #endif
    else if(r_method==RS232)
    {
        if(comPortCallibration)
        {
            // correclty close comport
            RS232_CloseComport(comPort);
            if(packetReciever!=NULL) delete packetReciever;
            packetReciever = NULL;

            return true;
        }
        else
        {
            if(packetReciever!=NULL) delete packetReciever;
            packetReciever = NULL;
            return true;
        }
    }

    return true;
}

#if defined (__WIN32__)
HANDLE reciever::connectToPipe(void)
{
    // trying to connect to the pipe
    HANDLE connection = CreateFileA("\\\\.\\pipe\\uwb_pipe",
                                    GENERIC_READ,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);

    if(connection!=INVALID_HANDLE_VALUE) return connection;
    else return NULL; // pipe does not exist or error occured
}

bool reciever::readFromPipe(HANDLE connection, char * buffer)
{
    DWORD bytesRecieved = 0;
    bool pipe_control;

    // reading from pipe
    pipe_control = ReadFile(
                connection, buffer,
                (maximum_pipe_size-1)*sizeof(char),
                &bytesRecieved, NULL);

    if(!pipe_control) closeConnection(connection); // server disconnected, or another error occured

    return pipe_control;
}

void reciever::closeConnection(HANDLE connection)
{
    CloseHandle(connection);
}


rawData * reciever::extract_synthetic(char * msg)
{
    char * msg_copy = strdup(msg);

    size_t allocation = 1;
    size_t used = 0;

    char delimiter = '#'; // delimiter separating different values in the text document

    char ** tokens = (char **)calloc(allocation, sizeof(char *));
    char * token, * rest = msg_copy;

    // go through the string and find the first occurance of delimiter by strsep function
    while((token = strsep(&rest, &delimiter)) != NULL)
    {
        if(allocation == used)
        {
            // if we filled all allocated memory, allocate new but allocation block is increased 2 times
            allocation *= 2;
            tokens = (char **)realloc(tokens, allocation*sizeof(char *));
        }

        tokens[used++] = strdup(token); // duplicating splitted string
    }

    // if no string (no) value was found free the memory and return NULL
    if(used == 0)
    {
        free(tokens);
        return NULL;
    } else {
        // reallocating and free the unused space of array
        tokens = (char **)realloc(tokens, used*sizeof(char *));
    }

    free(msg_copy);

    rawData * data = new rawData;

    size_t i = 0;
    // classification of obtained values and their conversion to numbers
    data->setSyntheticRadarId((short)(atoi(tokens[i++])));
    data->setSyntheticTime(atof(tokens[i++]));
    data->setSyntheticTargetsCount((short)(atoi(tokens[i++])));

    #if defined MTT_ARRAY_FIT && MTT_ARRAY_FIT==1
        float * coordinates = new float[MAX_N*2];
        float * toas = new float[MAX_N*2];
    #else
        float * coordinates = new float[data->getSyntheticTargetsCount()*2];
        float * toas = new float[data->getSyntheticTargetsCount()*2];
    #endif

    // conversion of coordinates
    int a, b, c;
    b = c = 0;
    while(i<used)
    {
        for(a=0; a<4; a++)
        {
            if(a<2) coordinates[b++] = (float)(atof(tokens[i++]));
            else toas[c++] = (float)(atof(tokens[i++]));
        }
    }

    data->setSyntheticCoordinates(coordinates);
    data->setSyntheticToas(toas);
    data->setRecieverMethod(SYNTHETIC);

    return data;
}
#endif

rawData * reciever::extract_RS232_radar_packet()
{
    rawData * data = new rawData;

    data->setUwbPacketRadarId(packetReciever->getRadarId());
    data->setUwbPacketRadarTime(packetReciever->getRadarTime());
    data->setUwbPacketPacketNumber(packetReciever->getPacketCount());
    data->setUwbPacketTargetsCount(packetReciever->getDataCount()/2);
    data->setUwbPacketCoordinates(packetReciever->getData());
    data->setRecieverMethod(RS232);
    return data;
}

char * reciever::strsep( char** stringp, const char* delim )
{

  // if strsep function is not present in string.h, use the following algorithm
  char* result;

  if ((stringp == NULL) || (*stringp == NULL)) return NULL;

  result = *stringp;

  while (**stringp && **stringp!=*delim) ++*stringp;

  if (**stringp) *(*stringp)++ = '\0';
  else             *stringp    = NULL;

  return result;
}

