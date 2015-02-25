#ifndef RADAR_HANDLER
#define RADAR_HANDLER

#include "radarunit.h"
/**
 * @brief The radar_handler structure simplifies access to 'radarUnit' objects and holds a pointer to it.
 */

struct radar_handler
{
    class radarUnit * radar; ///< Pointer to the 'radarUnit' object
    bool updated; ///< If the data where updated since the last filtration/rendering
    unsigned int id; ///< ID of the 'radarUnit'
};

#endif // RADAR_HANDLER

