# Introduction
Simple App is reclaiming complex web-style interfaces for native application development!

Leverage a sophisticated UI using web technologies including html, css, javascript, that runs inside a native C++ application with no internet connection or external dependencies required.

This is accomplished by filling the Window with a Microsoft WebView2 component, and navigating the View to a local html file which is retrieved through an embedded civetweb C web and websocket server.

Additionally, by injecting webui's javascript websocket server into the DOM, a new layer of native application interactions becomes available that could not exist on a website.


# Setup

## Dev environment setup
* Install Microsoft Visual Studio (https://visualstudio.microsoft.com/downloads/)

## Optional environment setup for embedding assets
* Install msys2 (https://www.msys2.org/)
* Current installer: https://github.com/msys2/msys2-installer/releases/download/2024-05-07/msys2-x86_64-20240507.exe
* Launch ``msys2`` terminal
 >ie using the default path: ``C:\msys64\ucrt64.exe``
* In the msys2 terminal, run the following command to install binutils: 
 >pacman -S binutils
 * Exit the terminal
 
# Build

## Building with embedded assets
* Clone this repository
 >git clone https://github.com/nater0000/simple-app.git
* Open the Visual Studio solution (scripts/simple-app.sln)
* Build the ``c-embed`` project
* Run ``c-embed.exe`` with your Interface directory as a parameter
 >c-embed.exe "C:\website"
* Open the ``Project Properties`` for ``simple-app``
* Navigate to ``C/C++``->``Preprocessor``
* Within ``Preprocessor Definitions`` update ``EMBEDDED_ROOT_DIR`` with your Interface directory
 >ie ``EMBEDDED_ROOT_DIR="C:\website"``

> [!CAUTION]
> The ``c-embed.exe`` parameter and ``EMBEDDED_ROOT_DIR`` are *case-sensitive* and *must* match
* Build the ``simple-app`` project
* Run ``simple-app.exe``
> [!NOTE]
> No need for external assets!!

## Optionally, building *without* embedded assets
* Clone this repository
 >git clone https://github.com/nater0000/simple-app.git
* Open the Visual Studio solution (scripts/simple-app.sln)
* Open the ``Project Properties`` for ``simple-app``
* Navigate to ``C/C++``->``Preprocessor``
* Within ``Preprocessor Definitions`` remove ``USE_CEMBED``
* Build the ``simple-app`` project
* Copy or move the simple-app.exe output file into your Interface directory (ie ``C:\website\``)
* Run ``simple-app.exe``
> [!WARNING]
> ``index.html`` must exist in the same directory as ``simple-app.exe``

# Run

## Running simple-app
* A file named ``index.html`` must be in the Interface directory
>[!WARNING]
>Make sure all ``.html`` files include ``webui.js`` for proper functionality
> ``<script src="webui.js"></script>``
* If the assets have been embedded, ``simple-app.exe`` is portable and stand-alone
* If the assets are not embedded, a file named ``index.html`` must be in the same directory as ``simple-app.exe``

# About

## Notes
This app is based on the Microsoft Visual Studio C++ template.

Updated to a flashy demo GUI by @JadeZaher (https://github.com/JadeZaher/NathanCDudeHelp)

Updated with webui C++ wrapper and a busy loop invoking a javascript function to interact with the site.

Updated with c-embed to package interface assets directly into the application

Using civetweb (via webui), we can run an entire website from a local directory (or more elaborate backend features with effort).

Updated with a Webview (https://github.com/webview/webview) front end component. This may get removed or replaced.

Updated with a Microsoft Webview2 (https://developer.microsoft.com/en-us/microsoft-edge/webview2) which is wrapped by Webview on Windows, but can also be used directly.

Updated with more web gui visuals

Note that Webview2 is not required for redistributing on most Win10+ systems

Note that one expectation for this project is to make this work cross-platform via gcc+gtk


## TODO
- [ ] Fork c-embed
- [ ] Refactor msys2-ucrt invocations to use a Preprocessor Definition
- [ ] Update README with instructions for non-default msys2 installation location
- [x] Integrate webui build into project
- [x] Fix current build issues due to external dependencies
- [ ] [Feature] Add 'Press any key': C++ input handler
- [ ] [Feature] Add 'Press any key': next.html
- [ ] Refactor initialization into an interface
- [ ] Create a new repo for MSI packager
- [x] Impl write index (+subdir) to appdata [bin2cpp] [using c-embed instead]
- [x] Compile index (+subdir) into binary [bin2cpp] [using c-embed instead]
- [x] Make a prettier gui
- [x] Push to git
- [x] Separate webserver API from window API
- [x] Integrate refactored webui [depends on webui fork+modify]
- [x] Integrate webview (chromium) [using MS Webview2 instead]
- [x] Modify webui build scripts
- [x] Refactor .sln,.vcproj into ./scripts/
- [x] Refactor ./src/website/* into ./website/*
- [ ] Refactor Windows Resource into platform-specific feature
- [ ] Add cmake for basic project compilation
- [ ] Add gcc implementation
- [ ] Add Windows Resource feature usage into gcc implementation


### Possible improvements
- [ ] In post-build steps, xcopy "$(SolutionDir)..\website\" "$(TargetDir)website\" /y
- [ ] Add gradle or similar


### Forked webui
- [ ] Rename 'window'
- [x] Refactor civetweb into ./vendor
- [x] Refactor .html and ./src/.html into ./website/
- [x] Refactor .sln,.vcproj into ./scripts/


### Forked c-embed
- [x] Support Microsoft Visual Studio (MSVC)
