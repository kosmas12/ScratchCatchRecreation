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
#if defined(NXDK)
#include <hal/video.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#define HOME "D:\\"
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#define HOME "./"
#endif
#include <chrono>
#include <random>
#include <sstream>

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
static int ballSteps = 25;
static int paddleSteps = 15;
static SDL_Surface *paddle;
static int radius = 20;
static SDL_Color colors[3];
static int currentColorNum = 0;
static SDL_Color currentColor;
static int score = 0;
static TTF_Font *font;


SDL_Surface *LoadBackground(const char *path) {
  SDL_Surface *background;
  SDL_Surface *optimizedBackground;
  background = IMG_Load(path);
  optimizedBackground = SDL_ConvertSurface(background, windowSurface->format, 0);
  SDL_FreeSurface(background);
  return optimizedBackground;
}

void drawPaddle(SDL_Rect position, SDL_Color color, int direction) {
  SDL_Rect prevPosition;
  //direction 0 = left, direction 1 = right
  if (direction == 0) {
    prevPosition.x = position.x + paddleSteps;
    prevPosition.w = position.w + paddleSteps;
  }
  else {
    prevPosition.x = position.x - paddleSteps;
    prevPosition.w = position.w - paddleSteps;
  }
  prevPosition.y = position.y;
  prevPosition.h = position.h;
  SDL_BlitSurface(backgroundImage, &prevPosition, paddle, NULL);
  SDL_BlitSurface(paddle, NULL, windowSurface, &prevPosition);
  SDL_FillRect(paddle, NULL, SDL_MapRGB(paddle->format, color.r, color.g, color.b));
  SDL_BlitSurface(paddle, NULL, windowSurface, &position);
  SDL_UpdateWindowSurface(window);
}

void eraseOldCircle() {
  SDL_Rect oldPos = {circlePosition.x-(radius+paddleSteps), circlePosition.y-(radius+paddleSteps), radius*radius, radius*2+paddleSteps};
  SDL_BlitSurface(backgroundImage, &oldPos, windowSurface, &oldPos);
}

void Circle(int center_x, int center_y, int radius, SDL_Color color) {
  if (center_y == paddlePosition.y) {
    center_y = 30;
  }
  eraseOldCircle();

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
  drawPaddle(paddlePosition, green, 0);
  SDL_UpdateWindowSurface(window);
}

int Init() {

#if defined(NXDK)
  XVideoSetMode(WIDTH, HEIGHT, 32, REFRESH_DEFAULT);
#endif
  if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS|SDL_INIT_TIMER) != 0) {
    std::cout << "Couldn't initialize SDL! Reason: " << SDL_GetError() << std::endl;
    exit(1);
  }
  if (IMG_Init(IMG_INIT_PNG) < 0) {
    std::cout << "Couldn't initialize SDL_image! Reason: " << SDL_GetError() << std::endl;
    SDL_Quit();
    exit(2);
  }
  if (TTF_Init() != 0) {
    std::cout << "Couldn't initialize SDL_ttf! Reason: " << SDL_GetError() << std::endl;
    IMG_Quit();
    SDL_Quit();
    exit(3);
  }

  font = TTF_OpenFont(HOME"Roboto-Regular.ttf", 20);

  if (!font) {
    std::cout << "Couldn't initialize font! Reason: " << SDL_GetError() << std::endl;
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    exit(4);
  }

  window = SDL_CreateWindow("ScratchCatchRecreation", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
  windowSurface = SDL_GetWindowSurface(window);
  backgroundImage = LoadBackground(HOME"background.png");
  paddlePosition = {250, 390, 180, 50};
  circlePosition = {345, 70, 0, 0}; // Width and height don't matter here
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  Uint32 Rmask = 0x000000FF;
  Uint32 Gmask = 0x0000FF00;
  Uint32 Bmask = 0x00FF0000;
  Uint32 Amask = 0xFF000000;
#else
  Uint32 Amask = 0x000000FF;
  Uint32 Rmask = 0x0000FF00;
  Uint32 Gmask = 0x00FF0000;
  Uint32 Bmask = 0xFF000000;
#endif
  paddle = SDL_CreateRGBSurface(0, 180, 50, 32, Rmask, Gmask, Bmask, Amask);
  green = {0, 92, 3};
  red = {209, 15, 15};
  blue = {0, 50, 130};
  colors[0] = green;
  colors[1] = red;
  colors[2] = blue;
  currentColor = colors[currentColorNum];
  PutResourcesToScreen();
  return 0;
}

void Quit(int status) {
  SDL_FreeSurface(backgroundImage);
  SDL_DestroyWindow(window);
  SDL_FreeSurface(paddle);
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
        Quit(0);
        break;
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
          case SDLK_LEFT:
            paddlePosition.x-=10;
            drawPaddle(paddlePosition, green, 0);
            break;
          case SDLK_RIGHT:
            paddlePosition.x+=10;
            drawPaddle(paddlePosition, green, 1);
            break;
          case SDLK_SPACE:
            if (currentColorNum < 2) {
              currentColorNum++;
            }
            else {
              currentColorNum = 0;
            }
            currentColor = colors[currentColorNum];
            break;
        }
        break;
    }
  }
}

Uint32 get_pixel32(SDL_Surface *surface, int x, int y ) {
  SDL_LockSurface(surface);
  //Convert the pixels to 32 bit
  Uint32 *pixels = (Uint32 *)surface->pixels;

  //Get the requested pixel
  return pixels[ ( y * surface->w ) + x ];
}

int compare(SDL_Surface *paddle, SDL_Surface *circle) {
  SDL_Color circleRGB;
  Uint32 circlePixel = get_pixel32(circle, circlePosition.x, circlePosition.y);
  SDL_UnlockSurface(circle);
  SDL_GetRGB(circlePixel, windowSurface->format, &circleRGB.r, &circleRGB.g, &circleRGB.b);
  SDL_Color paddleRGB;
  Uint32 paddlePixel = get_pixel32(paddle, 10, 10);
  SDL_UnlockSurface(paddle);
  SDL_GetRGB(paddlePixel, paddle->format, &paddleRGB.r, &paddleRGB.g, &paddleRGB.b);
  if (paddleRGB.r == circleRGB.r && paddleRGB.g == circleRGB.g && paddleRGB.b == circleRGB.b) {
    return 1;
  }
  return 0;
}

void startNewCircle() {
  eraseOldCircle();
  circlePosition.y = 60;
  srand(time(NULL));
  circlePosition.x = (rand() + radius*2) % 620;
}

void PrintScore() {
  SDL_Rect scorePos = {0, 0, 140, 20};
  SDL_BlitSurface(backgroundImage, &scorePos, windowSurface, &scorePos);
  std::stringstream scoreStream;
  scoreStream << "Score: " << score;
  std::string scoreString = scoreStream.str();
  SDL_Surface *score = TTF_RenderText_Blended(font, scoreString.c_str(), red);
  scoreStream.str("");
  SDL_BlitSurface(score, NULL, windowSurface, &scorePos);
}

void ChangeScore() {
  if (compare(paddle, windowSurface)) {
    score++;
  }
  else {
    if (score > 0) {
      score--;
    }
  }
}

int main() {
  Init();
  while(!quitted) {
    PrintScore();
    startNewCircle();
    SDL_Color circleColor = colors[rand() % 3];
    while (circlePosition.y + radius < paddlePosition.y) {
      drawPaddle(paddlePosition, currentColor, 0);
      SDL_Delay(50/ballSteps);
      Circle(circlePosition.x, circlePosition.y, radius, circleColor);
      ProcessInput();
      circlePosition.y++;
    }
    ChangeScore();
  }
  Quit(0);
  return 0;
}
