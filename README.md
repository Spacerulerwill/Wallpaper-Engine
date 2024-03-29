# Wallpaper-Engine
A wallpaper engine that allows you to run custom fragment shaders on your wallpaper.

# Video Demo
https://github.com/Spacerulerwill/Wallpaper-Engine/assets/36770307/611977cb-444c-4373-9f9e-0adfbdbfa399

# Writing Wallpapers

Here is an example of a basic wallpaper:

```glsl
#section metadata
name: "Super Cool Shader" # the title of the shader
uniforms
  float: 
    testFloat:            # name of the uniform in GLSL
      name: Test Float    # the name that will be displayed next to the slider
      min: 0.0            # the max slider value
      max: 1000.0         # the max slider value
  int: {}                 # we have no int uniforms
  bool: {}                # we have no boolean uniforms

#section shader
#version 330 core

out vec4 FragColor;       // Output color for pixel

uniform vec2 iResolution; // Screen width and height
uniform vec2 iMouse;      // Mouse position on screen
uniform float iTime;      // Time in seconds since application started

uniform float testFloat; 

void main(){
  FragColor = vec4(1.0);
}
```

There are sections to a wallpaper: `metadata` and `shader`. The `metadata` section allows you to write metadata about the wallpaper in YAML format. The other section, `shader` is where the glsl 
source code goes and is mandatory. You get access to a few default uniforms `iResolution`, `iMouse` and `iTime`. Any other uniforms you add will appear on the control menu for you to use them.

# Build Instructions

## Windows 
    git clone https://github.com/Spacerulerwill/Wallpaper-Engine --recursive
    cd WallpaperEngine/tools
    configure
    cd ../build
    WallpaperEngine.sln
Once you are in the solution you can build and run the project like normal.
