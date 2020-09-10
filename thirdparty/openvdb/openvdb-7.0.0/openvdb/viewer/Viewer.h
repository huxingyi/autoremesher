// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#ifndef OPENVDB_VIEWER_VIEWER_HAS_BEEN_INCLUDED
#define OPENVDB_VIEWER_VIEWER_HAS_BEEN_INCLUDED

#include <openvdb/openvdb.h>
#include <string>


namespace openvdb_viewer {

class Viewer;

enum { DEFAULT_WIDTH = 900, DEFAULT_HEIGHT = 800 };


/// @brief Initialize and return a viewer.
/// @param progName      the name of the calling program (for use in info displays)
/// @param background    if true, run the viewer in a separate thread
/// @note Currently, the viewer window is a singleton (but that might change
/// in the future), so although this function returns a new Viewer instance
/// on each call, all instances are associated with the same window.
Viewer init(const std::string& progName, bool background);

/// @brief Destroy all viewer windows and release resources.
/// @details This should be called from the main thread before your program exits.
void exit();


/// Manager for a window that displays OpenVDB grids
class Viewer
{
public:
    /// Set the size of and open the window associated with this viewer.
    void open(int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT);

    /// Display the given grids.
    void view(const openvdb::GridCPtrVec&);

    /// @brief Process any pending user input (keyboard, mouse, etc.)
    /// in the window associated with this viewer.
    void handleEvents();

    /// @brief Close the window associated with this viewer.
    /// @warning The window associated with this viewer might be shared with other viewers.
    void close();

    /// Resize the window associated with this viewer.
    void resize(int width, int height);

    /// Return a string with version number information.
    std::string getVersionString() const;

private:
    friend Viewer init(const std::string&, bool);
    Viewer();
};

} // namespace openvdb_viewer

#endif // OPENVDB_VIEWER_VIEWER_HAS_BEEN_INCLUDED
