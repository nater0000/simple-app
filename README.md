# simple-app

This is currently based on a MSVC sample app.
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