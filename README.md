# Wallpaper-Engine
A wallpaper engine that allows you to run custom fragment shaders on your wallpaper.

# Video Demo
https://github.com/Spacerulerwill/Wallpaper-Engine/assets/36770307/611977cb-444c-4373-9f9e-0adfbdbfa399

# Writing Wallpapers

Here is an example of a basic wallpaper:

```glsl
#shader metadata
name: "Super Cool Shader"

#section shader
#version 330 core

out vec4 FragColor;

uniform vec2 iResolution; // Screen width and height
uniform vec2 iMouse; // Mouse position on screen
uniform float iTime; // Time in seconds since application started

void main(){
  FragColor = vec4(1.0);
}
```

There are sections to a wallpaper: `metadata` and `shader`. The `metadata` section allows you to write metadata about the wallpaper in YAML format. Currently the only
metadata option is `name` which is a string and is shown on the control section if set. The `metadata` section is optional. The other section, `shader` is where the glsl 
source code goes and is mandatory. You get access to a few default uniforms `iResolution`, `iMouse` and `iTime`. Any other uniforms you add will appear on the control menu
for you to use them.

# Build Instructions

## Windows 
    cd tools
    configure.bat
    cd ../build
    WallpaperEngine.sln
Once you are in the solution you can build the project like normal. Make sure to copy the ```res``` folder to the binary executables location after it has been built.
