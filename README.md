# simple-app
Simple App is reclaiming complex web interfaces for low level programmers.
Build a sophisticated UI using web technologies including html, css, javascript, then launch it inside a simple C++ application with no internet connection or external dependencies required in a re-distributable 1 megabyte exe (not including the size of the UI files).
This is accomplished by filling the Window with a Microsoft WebView2 win32 component, and navigating the view to a local 'index.html' which is retrieved through an embedded civetweb C web and websocket server.
Optionally, by injecting uiweb's javascript websocket server into 'index.html' a new layer of application interactions become available that could not exist on a website.

### Notes
This app is based on the Microsoft Visual Studio C++ template.
Updated to a flashy demo GUI by @JadeZaher (https://github.com/JadeZaher/NathanCDudeHelp)
Updated with webui C++ wrapper and a busy loop invoking a javascript function to interact with the site.
Using civetweb (via webui), we can run an entire website from a local directory (or more elaborate backend features with effort).
Updated with a Webview (https://github.com/webview/webview) front end component. This may get removed or replaced.
Updated with a Microsoft Webview2 (https://developer.microsoft.com/en-us/microsoft-edge/webview2) which is wrapped by Webview on Windows, but can also be used directly.
Updated with more web gui visuals
Note that Webview2 is not required for redistributing on most Win10+ systems
Note that one expectation for this project is to make this work cross-platform via gcc+gtk


### TODO
- Integrate webui build into project
- Fix current build issues due to external dependencies
- [Feature] Add 'Press any key': C++ input handler
- [Feature] Add 'Press any key': next.html
- Refactor initialization into an interface
- Create a new repo for MSI packager
- Impl write index (+subdir) to appdata [bin2cpp]
- Compile index (+subdir) into binary [bin2cpp]
x Make a prettier gui
x Push to git
x Separate webserver API from window API
x Integrate refactored webui [depends on webui fork+modify]
y Integrate webview (chromium) [using MS Webview2 instead]
n Modify webui build scripts
x Refactor .sln,.vcproj into ./scripts/
x Refactor ./src/website/* into ./website/*
- Refactor Windows Resource into platform-specific feature
- Add cmake for basic project compilation
- Add gcc implementation
- Add Windows Resource feature usage into gcc implementation
? Add gradle


### Possible improvements
- In post-build steps, xcopy "$(SolutionDir)..\website\" "$(TargetDir)website\" /y


### Forked webui
- Rename 'window'
- Refactor civetweb into ./vendor
- Refactor .html and ./src/.html into ./website/
- Refactor .sln,.vcproj into ./scripts/
- 