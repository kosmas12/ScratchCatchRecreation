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
static SDL_Rect circlePosition;
static SDL_Color green;
static SDL_Color red;
static SDL_Color blue;
static int steps = 5;
static SDL_Surface *paddle;
SDL_Surface *transparent;

SDL_Surface *LoadBackground(const char *path) {
  SDL_Surface *background;
  SDL_Surface *optimizedBackground;
  background = IMG_Load(path);
  optimizedBackground = SDL_ConvertSurface(background, windowSurface->format, 0);
  SDL_FreeSurface(background);
  return optimizedBackground;
}

void drawPaddle(SDL_Rect position, SDL_Color color) {
  SDL_Rect prevPosition = position;
  prevPosition.x-=steps;
  prevPosition.w+=steps;
  SDL_SetColorKey(windowSurface, SDL_TRUE, SDL_MapRGBA(windowSurface->format, 0, 0, 0, 0));
  SDL_FillRect(windowSurface, &prevPosition, SDL_MapRGBA(paddle->format, 0, 0, 0, 0));
  //SDL_BlitSurface(paddle, NULL, windowSurface, &prevPosition);
  SDL_FillRect(paddle, NULL, SDL_MapRGBA(paddle->format, color.r, color.g, color.b, 255));
  SDL_BlitSurface(paddle, NULL, windowSurface, &position);
  SDL_UpdateWindowSurface(window);
}

void Circle(int center_x, int center_y, int radius, SDL_Color color){
  uint32_t *pixels = (uint32_t *) windowSurface->pixels;
  SDL_PixelFormat *windowFormat = windowSurface->format;
  SDL_LockSurface(windowSurface);

  for(int x=center_x-radius; x<=center_x+radius; x++) {

    for(int y=center_y-radius; y<=center_y+radius; y++) {

      if((std::pow(center_y-y,2)+std::pow(center_x-x,2)) <= std::pow(radius,2)){
        pixels[(y * WIDTH + x)] = SDL_MapRGB(windowFormat, color.r, color.g, color.b);
      }
    }
  }

  SDL_UnlockSurface(windowSurface);
  SDL_UpdateWindowSurface(window);
}

void PutResourcesToScreen() {
  SDL_BlitSurface(backgroundImage, 0, windowSurface, 0);
  drawPaddle(paddlePosition, green);
  Circle(circlePosition.x, circlePosition.y, 20, green);
  SDL_UpdateWindowSurface(window);
}

void Init() {
  SDL_Init(SDL_INIT_EVERYTHING);
  IMG_Init(IMG_INIT_PNG);
  window = SDL_CreateWindow("ScratchCatchRecreation", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
  windowSurface = SDL_GetWindowSurface(window);
  backgroundImage = LoadBackground("background.png");
  paddlePosition = {250, 390, 180, 50};
  circlePosition = {345, 300, 0, 0}; // Width and height don't matter here
  paddle = SDL_CreateRGBSurface(0, 180, 50, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
  green = {0, 92, 3};
  red = {209, 15, 15};
  blue = {0, 50, 130};
  transparent = SDL_CreateRGBSurfaceWithFormat(0, paddlePosition.w, paddlePosition.h, 32, SDL_GetWindowPixelFormat(window));
  SDL_FillRect(transparent, NULL, SDL_MapRGBA(transparent->format, 0, 0, 0, 0));
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
        quitted = true;
        break;
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
          case SDLK_LEFT:
            paddlePosition.x-=steps;
            drawPaddle(paddlePosition, green);
            break;
          case SDLK_RIGHT:
            paddlePosition.x+=steps;
            drawPaddle(paddlePosition, green);
            break;
        }
        break;
    }
  }
}

int main() {
  Init();
  while(!quitted) {
    ProcessInput();
    /*  circlePosition.y++;
    Circle(circlePosition.x, circlePosition.y, 20, red);*/
  }
  Quit(0);
  return 0;
}
