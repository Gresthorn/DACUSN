#ifndef STDDEFS
#define STDDEFS

/**
 * @brief The reciever_method enum covers all possible kinds of recieve methods for UWB sensor network.
 */
enum reciever_method
{
    UNDEFINED = 0, ///< May be used for situations when no method is needed at all (idle method)
    SYNTHETIC = 1 ///< Is used when the data are read by server application from file and sent throught pipe
};

enum visualization_schema
{
    COMMON_FLOW = 0, ///< This schema displays objects as a very simple circles fastly changing their positions.
    COMET_EFFECT = 1 ///< Is displaying targets as moving comet with a little history positions. The history may be changed as the animation duration.
};

/**
 * @brief Sets what behaviour of background rendering can be selected.
 */
enum visualization_tapping_options
{
    RENDER_EVERYTHING = 0, ///< If this option is selected all grids and colors (if allowed) are rendered.
    NO_3 = 1, ///< This option will stop rendering 3rd level grid.
    NO_2 = 2, ///< This option will stop rendering 2nd level grid.
    NO_3_2 = 3, ///< Will disable rendering 2nd and 3rd level grid.
    NO_BACKGROUND = 4, ///< Will stop rendering background during tapping.
    NO_SCENE_UPDATE = 5 ///< This option wil disable periodical scene update function during tapping.
};


#endif // STDDEFS

