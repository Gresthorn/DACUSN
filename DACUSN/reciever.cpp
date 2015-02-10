#include "reciever.h"

reciever::reciever(reciever_method recieveMethod) : maximum_pipe_size(512)
{
    r_method = UNDEFINED;
    last_data_pt = NULL;
    statusMsg = NULL;

    calibrationStatus = calibrate(recieveMethod);

    if(calibrationStatus) set_msg("Calibration successfull.");
    else set_msg("An error occured when trying to set up selected method.");
}

reciever::~reciever()
{
    if(statusMsg != NULL) delete statusMsg;
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
              if(kill) r_method = UNDEFINED; // if kill is true, then UNDEFINED is set neverthless an new method will try to start
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
        case SYNTHETIC:
            // settings for synthetic data obtaining
            calibrationStatus = calibrate(recieveMethod);

            if(!calibrationStatus) set_msg("The pipe channel cannot be set. Check if server has already started.");
            else r_method = SYNTHETIC;

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
    else return false;

    return false;
}

bool reciever::cancel_previous_method()
{
    if(r_method==UNDEFINED) return true; // nothing speciall is required
    else if(r_method==SYNTHETIC)
    {
        // closing pipe channel
        closeConnection(pipe_connection_handler);
        return true;
    }

    return true;
}

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

    double * coordinates = new double[data->getSyntheticTargetsCount()*2];
    double * toas = new double[data->getSyntheticTargetsCount()*2];

    // conversion of coordinates
    int a, b, c;
    b = c = 0;
    while(i<used)
    {
        for(a=0; a<4; a++)
        {
            if(a<2) coordinates[b++] = atof(tokens[i++]);
            else toas[c++] = atof(tokens[i++]);
        }
    }

    data->setSyntheticCoordinates(coordinates);
    data->setSyntheticToas(toas);

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

