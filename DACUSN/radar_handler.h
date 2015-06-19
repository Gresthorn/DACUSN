/**
 * @file radar_handler.h
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief This header contains structures/enumerate types common for radar units.
 *
 * @section DESCRIPTION
 *
 * This is just small extenstion of stddefs.h and should be avoided to modification if
 * it is possible. It contains only information about enumeration types and structions that
 * have something to do with radar units. This header was created only because of cross linker
 * problems in stddefs.h when trying to include "radarunit.h".
 *
 */

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

