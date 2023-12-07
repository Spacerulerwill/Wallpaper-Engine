#ifndef OS_HPP
#define OS_HPP

#include <string>
#include <Windows.h>

// Get the directory of the currently active wallpaper in Windows 10
std::wstring GetWallpaper();
// Set the Windows 10 wallpaper to the path specified
void SetWallpaper(const std::wstring& wallpaperDir);
// Open a Windows 10 file dialog window
bool openFileDialog(std::string* sFilePath);

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

// Get RECT containing width and height of desktop
RECT GetDesktopRect();
// Get a handle to the desktop wallpaper
HWND GetWallpaperHwnd();

#endif // !OS_HPP