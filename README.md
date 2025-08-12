# .m3u Playlist Generator

Takes a series of directories, generates a playlist from each one. Then generates a total playlist from all specified directories. 

Notes:
- Each filename is URL encoded.
- A specified path is prefixed onto each filename output.
- Added systemd unit placeholders for convenience.

Deps:
- C++ 20.
- libcurl.


**Usage:**  
m3u_generator <directory_1> <directory_2> <prefix_path> <output_directory>

### Parameters

- **directories** — Directories to find media files within.  
- **prefix_path** — The path which will be prefixed onto each file found within the specified directories in the final output playlists.  
- **output_directory** — Directory in which to put the generated m3u playlists. 