# Wallpaper-Engine  [![Lines Of Code](https://tokei.rs/b1/github/Spacerulerwill/Wallpaper-Engine?category=code)](https://github.com/XAMPPRocky/tokei)
A wallpaper engine that allows you to run custom fragment shaders on your wallpaper.

# Video Demo
https://github.com/Spacerulerwill/Wallpaper-Engine/assets/36770307/611977cb-444c-4373-9f9e-0adfbdbfa399

# Writing Shaders

When writing shaders you get access to a couple default uniforms:

```glsl
#version 330 core

out vec4 FragColor;

uniform ivec2 iResolution; // Screen width and height
uniform ivec2 iMouse; // Mouse position on screen
uniform float iTime; // Time in seconds since application started

void main(){
  FragColor = vec4(1.0);
}
```

Any other uniforms will be automatically added to the ImGUI menu so you can change them at any time
