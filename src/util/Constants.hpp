#pragma once

float vertices[] = {
		 1.0f, 1.0f, 1.0f, 1.0f,   // top right
		 1.0f, -1.0f, 1.0f, 0.0f,  // bottom right
		 -1.0f, -1.0f, 0.0f, 0.0f,   // bottom left
		 -1.0f, 1.0f, 0.0f, 1.0f  // top left 
};
unsigned int indices[] = {
	0, 1, 3,  // first Triangle
	1, 2, 3   // second Triangle
};