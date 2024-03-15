# simple-app

This is currently based on a MSVC sample app.
Updated with webui C++ wrapper and a busy loop invoking a javascript function to interact with the site.
Since this is effectively launching a separate (browser) app, we cannot interact with the UI directly,
but we can run an entire website from a local directory (or more elaborate backend features with effort).

### TODO
- Push to git
- Separate webserver API from window API
- Impl write index (+subdir) to appdata
- Compile index (+subdir) into binary
- Integrate refactored webui [depends on webui fork+modify]
- Integrate webview (chromium)
- Modify webui build scripts
- Refactor .sln,.vcproj into ./scripts/
- Refactor ./src/website/* into ./website/*
- Refactor Windows Resource into platform-specific feature
- Add cmake for basic project compilation
- Add gcc implementation
- Add Windows Resource feature usage into gcc implementation
? Add gradle


### Possible improvements
- In post-build steps, xcopy "$(SolutionDir)..\website\" "$(TargetDir)website\" /y

- 
### Forked webui
- Rename 'window'
- Refactor civetweb into ./vendor
- Refactor .html and ./src/.html into ./website/
- Refactor .sln,.vcproj into ./scripts/
- 