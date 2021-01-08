/*
ScratchCatchRecreation Copyright (C) 2021  Kosmas Raptis
Email: keeperkosmas6@gmail.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.*/

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define WIDTH 720
#define HEIGHT 480

static SDL_Window *window;
static SDL_Surface *windowSurface;
static SDL_Surface *backgroundImage;
static bool quitted;
static SDL_Rect paddlePosition;
static SDL_Color green;
static SDL_Color red;
static SDL_Color blue;

SDL_Surface *LoadBackground(const char *path) {
  SDL_Surface *background;
  SDL_Surface *optimizedBackground;
  background = IMG_Load(path);
  optimizedBackground = SDL_ConvertSurface(background, windowSurface->format, 0);
  SDL_FreeSurface(background);
  return optimizedBackground;
}

void drawPaddle(SDL_Rect position, SDL_Color color) {
  SDL_FillRect(windowSurface, &position, SDL_MapRGB(windowSurface->format, color.r, color.g, color.b));
}

void PutResourcesToScreen() {
  SDL_BlitSurface(backgroundImage, 0, windowSurface, 0);
  drawPaddle(paddlePosition, green);
  SDL_UpdateWindowSurface(window);
}

void Init() {
  SDL_Init(SDL_INIT_EVERYTHING);
  IMG_Init(IMG_INIT_PNG);
  window = SDL_CreateWindow("ScratchCatchRecreation", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
  windowSurface = SDL_GetWindowSurface(window);
  backgroundImage = LoadBackground("background.png");
  paddlePosition = {250, 390, 180, 50};
  green = {0, 92, 3};
  red = {209, 15, 15};
  blue = {0, 50, 130};
  PutResourcesToScreen();
}

void Quit(int status) {
  SDL_FreeSurface(backgroundImage);
  SDL_DestroyWindow(window);
  IMG_Quit();
  SDL_Quit();
  exit(status);
}

void ProcessInput() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        quitted = 1;
        break;
    }
  }
}

int main() {
  Init();
  while(!quitted) {
    ProcessInput();
  }
  Quit(0);
  return 0;
}
