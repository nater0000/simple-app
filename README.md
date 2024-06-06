# simple-app
Simple App is reclaiming complex web interfaces for low level programmers.
Build a sophisticated UI using web technologies including html, css, javascript, then launch it inside a simple C++ application with no internet connection or external dependencies required in a re-distributable 1 megabyte exe (not including the size of the UI files).
This is accomplished by filling the Window with a Microsoft WebView2 win32 component, and navigating the view to a local 'index.html' which is retrieved through an embedded civetweb C web and websocket server.
Optionally, by injecting uiweb's javascript websocket server into 'index.html' a new layer of application interactions become available that could not exist on a website.

## Dev environment setup
* Install Microsoft Visual Studio

### Optional environment setup for embedding assets
* Install msys2 (https://www.msys2.org/)
```https://github.com/msys2/msys2-installer/releases/download/2024-05-07/msys2-x86_64-20240507.exe```
* Launch msys2 urct64, for example:
```C:\msys64\ucrt64.exe```
* In the msys2 terminal, run the following command to install binutils
```pacman -S binutils```

## Building simple-app *without* embedded assets
* Clone the repository
* Open the Visual Studio solution (scripts/simple-app.sln)
* Open the Project Properties for simple-app
* Navigate to C/C++ -> Preprocessor
* Within 'Preprocessor Definitions' remove USE_CEMBED
* Build the simple-app project
* Copy the simple-app.exe output file into your Interface directory
* Run simple-app.exe (index.html must exist in the same directory)

## Building simple-app with embedded assets
* Clone the repository
* Open the Visual Studio solution (scripts/simple-app.sln)
* Build the c-embed project
* Run c-embed.exe with your Interface directory as a parameter
```c-embed.exe "C:\website"```
* Open the Project Properties for simple-app
* Navigate to C/C++ -> Preprocessor
* Within 'Preprocessor Definitions' update EMBEDDED_ROOT_DIR with your Interface directory
```EMBEDDED_ROOT_DIR="C:\website"```
* Build the simple-app project
* Run simple-app.exe (no need for external assets!)

### Notes
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


### TODO
- [ ] Fork c-embed
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
