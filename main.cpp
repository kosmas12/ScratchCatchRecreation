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
#include <hal/debug.h>
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

#define WIDTH 640
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
static int radius = 20;
static SDL_Color colors[3];
static int currentColorNum = 0;
static SDL_Color currentColor;
static int score = 0;
static TTF_Font *font;
static SDL_GameController *controller;
static SDL_AudioDeviceID deviceId;
static SDL_AudioSpec wavSpec;
static Uint32 wavLength;
static Uint8 *wavBuffer;

void printLn(const char *string) {
#if defined(NXDK)
  debugPrint("%s\n", string);
#else
  std::cout << string << std::endl;
#endif
}

SDL_Surface *LoadBackground(const char *path) {
  SDL_Surface *background;
  SDL_Surface *optimizedBackground;
  background = IMG_Load(path);
  // Optimization here is basically making it use the window's pixel format so SDL doesn't have to convert it on every blit.
  optimizedBackground = SDL_ConvertSurface(background, windowSurface->format, 0);
  // No need for the original anymore
  SDL_FreeSurface(background);
  return optimizedBackground;
}

void GetBGToFG(SDL_Rect pos) {

  uint32_t *windowPixels = (uint32_t *) windowSurface->pixels;
  SDL_LockSurface(windowSurface); // Lock surface for direct pixel access capability
  uint32_t *bgPixels = (uint32_t *) backgroundImage->pixels;
  SDL_LockSurface(backgroundImage); // Lock surface for direct pixel access capability

  int leftEdgeY = pos.y;
  int rightEdgeY = pos.y+pos.h;
  int leftEdgeX = pos.x;
  int rightEdgeX = pos.x+pos.w;

  for (int curY = leftEdgeY; curY < rightEdgeY; ++curY) {
    for (int curX = leftEdgeX; curX < rightEdgeX; ++curX) {
      windowPixels[curY * WIDTH + curX] = bgPixels[curY * WIDTH + curX];
    }
  }

  //SDL_BlitSurface(backgroundImage, &pos, windowSurface, &pos);
  SDL_UnlockSurface(backgroundImage);
  SDL_UnlockSurface(windowSurface);
}

int square(int n)
{
  // handle negative input
  if (n < 0) {
    n = -n;
  }
  // Initialize result
  int res = n;

  // Add n to res n-1 times
  for (int i = 1; i < n; i++) {
    res += n;
  }
  return res;
}

SDL_Rect eraseOldCircle() {
  //Calculate position of ball
  SDL_Rect pos = {circlePosition.x-radius, circlePosition.y-(radius+paddleSteps), square(radius), radius+radius+paddleSteps};
  GetBGToFG(pos);
  return pos;
}

void drawBox(int startX, int startY, int endX, int endY, SDL_Color color) {
  uint32_t *pixels = (uint32_t *) windowSurface->pixels;
  SDL_PixelFormat *windowFormat = windowSurface->format;
  SDL_LockSurface(windowSurface); // Lock surface for direct pixel access capability

  for (int x = startX; x <= endX; ++x) {
    for (int y = startY; y <= endY; ++y) {
      pixels[y * WIDTH + x] = SDL_MapRGB(windowFormat, color.r, color.g, color.b);
    }
  }
  SDL_UnlockSurface(windowSurface);
}

void drawPaddle(SDL_Rect position, SDL_Color color, int direction) { //direction 0 = left, direction 1 = right
  SDL_Rect prevPosition;
  // Calculate previous position, paddle has only moved the number of steps
  if (direction == 0) {
    prevPosition.x = position.x + paddleSteps;
  }
  else {
    prevPosition.x = position.x - paddleSteps;
  }
  prevPosition.y = position.y;
  prevPosition.h = position.h+1;
  prevPosition.w = position.w;
  // Copy the part of the background image that the paddle overwrote in the previous position back to the window
  GetBGToFG(prevPosition);
  // Draw paddle
  drawBox(paddlePosition.x, paddlePosition.y, paddlePosition.x+paddlePosition.w, paddlePosition.y+paddlePosition.h, color);
  // Apply changes to window
  const SDL_Rect rects[] = {prevPosition, position};
  SDL_UpdateWindowSurfaceRects(window, rects, 2);
}

void Circle(int center_x, int center_y, int radius, SDL_Color color) {
  SDL_Rect pos = {center_x+(radius+paddleSteps), center_y+(radius+paddleSteps), square(radius), square(radius)};
  SDL_Rect bottom = eraseOldCircle();

  const SDL_Rect rects[] = {pos, bottom};

  uint32_t *pixels = (uint32_t *) windowSurface->pixels;
  SDL_PixelFormat *windowFormat = windowSurface->format;
  SDL_LockSurface(windowSurface); // Lock surface for direct pixel access capability

  int radiussqrd = square(radius);

  for(int x=center_x-radius; x<=center_x+radius; x++) {
    int dxsqrd = square(center_x - x);
    for(int y=center_y-radius; y<=center_y+radius; y++) {
      int dysqrd = square(center_y - y);
      if(dysqrd + dxsqrd <= radiussqrd) {
        pixels[(y * WIDTH + x)] = SDL_MapRGB(windowFormat, color.r, color.g, color.b);
      }
    }
  }

  SDL_UnlockSurface(windowSurface);
  SDL_UpdateWindowSurfaceRects(window, rects, 2);
}

void PutResourcesToScreen() {
  SDL_BlitSurface(backgroundImage, nullptr, windowSurface, nullptr); // Copy background to window
  drawPaddle(paddlePosition, green, 0);
  SDL_UpdateWindowSurface(window);
}

void OpenFirstController() {
  //Open controller

  for (int i = 0; i < SDL_NumJoysticks(); i++) { // For the time that i is smaller than the number of connected Joysticks
    if(SDL_IsGameController(i)) { // If i (which we use to iterate through the connected controllers) as a port number is a Game Controller
      controller = SDL_GameControllerOpen(i); // Open the controller
      if(controller) { // If we find that we opened a controller
        printLn("Controller 1 opened");
        break; // Exit the loop
      }
    }
  }
}

void CloseFirstController() {
  if (controller != NULL) {
    SDL_GameControllerClose(controller);
    printLn("Controller 1 closed.");
  }
}

void Quit(int status) {
  SDL_FreeSurface(backgroundImage);
  SDL_DestroyWindow(window);
  //SDL_FreeSurface(paddle);
  CloseFirstController();
  TTF_CloseFont(font);
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
  exit(status);
}

int Init() {

#if defined(NXDK)
  XVideoSetMode(WIDTH, HEIGHT, 32, REFRESH_DEFAULT);
#endif
  if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS|SDL_INIT_TIMER|SDL_INIT_AUDIO|SDL_INIT_GAMECONTROLLER) != 0) {
    printLn("Couldn't initialize SDL! Reason: ");
    printLn(SDL_GetError());
    exit(1);
  }
  SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
  if (IMG_Init(IMG_INIT_PNG) < 0) {
    printLn("Couldn't initialize SDL_image! Reason: ");
    printLn(SDL_GetError());
    SDL_Quit();
    exit(2);
  }
  if (TTF_Init() != 0) {
    printLn("Couldn't initialize SDL_ttf! Reason: ");
    printLn(SDL_GetError());
    IMG_Quit();
    SDL_Quit();
    exit(3);
  }

  font = TTF_OpenFont(HOME"Roboto-Regular.ttf", 20);

  if (!font) {
    printLn("Couldn't initialize font! Reason: ");
    printLn(SDL_GetError());
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    exit(4);
  }

  window = SDL_CreateWindow("ScratchCatchRecreation", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
  if (!window) {
    printLn( "Couldn't create Window! Reason: ");
    printLn(SDL_GetError());
    Quit(5);
  }
  windowSurface = SDL_GetWindowSurface(window);
  backgroundImage = LoadBackground(HOME"background.png");
  paddlePosition = {250, 390, 180, 50};
  circlePosition = {345, 60, 0, 0}; // Width and height don't matter here
/*#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  Uint32 Rmask = 0x000000FF;
  Uint32 Gmask = 0x0000FF00;
  Uint32 Bmask = 0x00FF0000;
  Uint32 Amask = 0xFF000000;
#else
  Uint32 Amask = 0x000000FF;
  Uint32 Rmask = 0x0000FF00;
  Uint32 Gmask = 0x00FF0000;
  Uint32 Bmask = 0xFF000000;
#endif*/
  green = {0, 92, 3, 255};
  red = {209, 15, 15, 255};
  blue = {0, 50, 130, 255};
  colors[0] = green;
  colors[1] = red;
  colors[2] = blue;
  currentColor = colors[currentColorNum];
  OpenFirstController();
  PutResourcesToScreen();
   if(SDL_LoadWAV(HOME"win.wav", &wavSpec, &wavBuffer, &wavLength) == NULL) {
     printLn("Couldn't load wav file. Reason: ");
     printLn(SDL_GetError());
   }
  deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);
  if (!deviceId) {
    printLn( "The audio device couldn't be opened, thus audio playback is impossible. Reason: ");
    printLn(SDL_GetError());
  }
  return 0;
}

void ProcessInput() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_CONTROLLERBUTTONDOWN:
        switch (event.cbutton.button) {
          case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            if (paddlePosition.x-10 > 0) {
              paddlePosition.x-=10;
            }
            else {
              paddlePosition.x = 0;
            }
            drawPaddle(paddlePosition, green, 0);
            break;
          case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            paddlePosition.x+=10;
            drawPaddle(paddlePosition, green, 1);
            break;
          case SDL_CONTROLLER_BUTTON_A:
            if (currentColorNum < 2) {
              currentColorNum++;
            }
            else {
              currentColorNum = 0;
            }
            currentColor = colors[currentColorNum];
        }
        break;
      case SDL_CONTROLLERDEVICEADDED:
        OpenFirstController();
        break;
      case SDL_CONTROLLERDEVICEREMOVED:
        CloseFirstController();
        break;
      case SDL_QUIT:
        quitted = true;
        Quit(0);
        break;
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
          case SDLK_LEFT:
            if (paddlePosition.x-10 > 0) {
              paddlePosition.x-=10;
            }
            else {
              paddlePosition.x = 0;
            }
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

int compare() {
  SDL_Color circleRGB;
  SDL_PixelFormat *windowFormat = windowSurface->format;
  // Get pixel data from the circle, whole circle is same color so all pixels are same
  Uint32 circlePixel = get_pixel32(windowSurface, circlePosition.x, circlePosition.y);
  // Unlock surface since get_pixel doesn't unlock automatically
  SDL_UnlockSurface(windowSurface);
  SDL_GetRGB(circlePixel, windowFormat, &circleRGB.r, &circleRGB.g, &circleRGB.b); // Get color from pixel data
  SDL_Color paddleRGB;
  // Same as circle, but for paddle. Using a different x y value within the paddle's range should have no different result
  Uint32 paddlePixel = get_pixel32(windowSurface, paddlePosition.x, paddlePosition.y);
  SDL_UnlockSurface(windowSurface);
  SDL_GetRGB(paddlePixel, windowFormat, &paddleRGB.r, &paddleRGB.g, &paddleRGB.b);
  // Check if colors are same and if the circle is within the paddle's range
  if (paddleRGB.r == circleRGB.r && paddleRGB.g == circleRGB.g && paddleRGB.b == circleRGB.b
  && circlePosition.x > paddlePosition.x && circlePosition.x < (paddlePosition.x + paddlePosition.w)) {
    return 1;
  }
  return 0;
}

void startNewCircle() {
  eraseOldCircle();
  circlePosition.y = 60;
  srand(time(nullptr));
  circlePosition.x = (rand() + radius*2) % WIDTH-20;
}

void PrintScore() {
  SDL_Rect scorePos = {0, 0, 140, 20};
  GetBGToFG(scorePos);
  std::stringstream scoreStream;
  scoreStream << "Score: " << score;
  std::string scoreString = scoreStream.str();
  SDL_Surface *scoreText = TTF_RenderText_Blended(font, scoreString.c_str(), red);
  scoreStream.str("");
  SDL_BlitSurface(scoreText, nullptr, windowSurface, &scorePos);
  SDL_FreeSurface(scoreText);
  SDL_UpdateWindowSurfaceRects(window, &scorePos, 1);
}

void ChangeScore() {
  if (compare()) {
    SDL_QueueAudio(deviceId, wavBuffer, wavLength);
    SDL_PauseAudioDevice(deviceId, 0);
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
      SDL_Delay(50/ballSteps); // The more steps, the smaller the wait is, thus movement is faster
      Circle(circlePosition.x, circlePosition.y, radius, circleColor);
      ProcessInput();
      circlePosition.y++;
    }
    ChangeScore();
  }
  Quit(0);
  return 0;
}
