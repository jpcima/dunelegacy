#include "DataTypes.h"
#include "FileClasses/FileManager.h"
#include "FileClasses/TextManager.h"

// SDL stuff
int                  currentZoomlevel;           ///< 0 = the smallest zoom level, 1 = medium zoom level, 2 = max

// abstraction layers
std::unique_ptr<FileManager>         pFileManager;               ///< manager for loading files from PAKs
std::unique_ptr<TextManager>         pTextManager;               ///< manager for loading and managing texts and providing localization

// misc
SettingsClass    settings;                       ///< the settings read from the settings file
