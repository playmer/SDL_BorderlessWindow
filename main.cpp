/*This source code copyrighted by Lazy Foo' Productions (2004-2020)
and may not be redistributed without written permission.*/

//Using SDL, standard IO, and strings
#include <SDL3/SDL.h>
#include <SDL3/SDL_syswm.h>
#include <stdio.h>
#include <string>

#include <Windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//Loads individual image
SDL_Surface* loadSurface( std::string path );

//The window we'll be rendering to
SDL_Window* gWindow = NULL;
  
//The surface contained by the window
SDL_Surface* gScreenSurface = NULL;

//Current displayed image
SDL_Surface* gStretchedSurface = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Current displayed texture
SDL_Texture* gTexture = NULL;

//Loads individual image as texture
SDL_Texture* loadTexture( std::string path );

SDL_HitTestResult HitTestCallback(SDL_Window* win, const SDL_Point* area, void* data) {
  //printf("Hittest\n");

  int winWidth, winHeight;
  SDL_GetWindowSize(win, &winWidth, &winHeight);
        
  const int RESIZE_AREA = 4;
  const int RESIZE_AREAC = RESIZE_AREA*2;

  // Resize top
  if (area->x < RESIZE_AREAC && area->y < RESIZE_AREAC) return SDL_HITTEST_RESIZE_TOPLEFT;
  if (area->x > winWidth-RESIZE_AREAC && area->y < RESIZE_AREAC) return SDL_HITTEST_RESIZE_TOPRIGHT;
  if (area->x < RESIZE_AREA) return SDL_HITTEST_RESIZE_LEFT;
  if (area->y < RESIZE_AREA) return SDL_HITTEST_RESIZE_TOP;

  // Title bar
  if (area->y < 22 && area->x < winWidth-128) return SDL_HITTEST_DRAGGABLE;

  if (area->x < RESIZE_AREAC && area->y > winHeight-RESIZE_AREAC) return SDL_HITTEST_RESIZE_BOTTOMLEFT;
  if (area->x > winWidth-RESIZE_AREAC && area->y > winHeight-RESIZE_AREAC) return SDL_HITTEST_RESIZE_BOTTOMRIGHT;
  if (area->x > winWidth-RESIZE_AREA) return SDL_HITTEST_RESIZE_RIGHT;
  if (area->y > winHeight-RESIZE_AREA) return SDL_HITTEST_RESIZE_BOTTOM;
  
  //printf("SDL_HITTEST_NORMAL\n");
  return SDL_HITTEST_NORMAL;
}


// we cannot just use WS_POPUP style
// WS_THICKFRAME: without this the window cannot be resized and so aero snap, de-maximizing and minimizing won't work
// WS_SYSMENU: enables the context menu with the move, close, maximize, minize... commands (shift + right-click on the task bar item)
// WS_CAPTION: enables aero minimize animation/transition
// WS_MAXIMIZEBOX, WS_MINIMIZEBOX: enable minimize/maximize
enum class Style : DWORD {
    windowed         = WS_OVERLAPPEDWINDOW | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
    aero_borderless  = WS_POPUP            | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
    basic_borderless = WS_POPUP            | WS_THICKFRAME              | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX
};

SDL_HitTestResult HitTestCallback(SDL_Window* win, const SDL_Point* area, void* data);

SDL_Window* CreateSdlWindow()
{
  //Create window
  SDL_SetHintWithPriority("SDL_BORDERLESS_RESIZABLE_STYLE", "1", SDL_HINT_OVERRIDE);
  SDL_SetHintWithPriority("SDL_BORDERLESS_WINDOWED_STYLE", "1", SDL_HINT_OVERRIDE);
  auto window = SDL_CreateWindow( "SDL Tutorial", 800, 600, SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE);
  if (window == nullptr)
    return nullptr;

  SDL_SetWindowHitTest(window, &HitTestCallback, nullptr);
  SDL_SysWMinfo wmInfo;
  SDL_GetWindowWMInfo(window, &wmInfo, SDL_SYSWM_CURRENT_VERSION);
  HWND handle = wmInfo.info.win.window;
  
  //::SetWindowLongPtrW(handle, GWL_STYLE, static_cast<LONG>(Style::aero_borderless));

  
  //SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
  //SDL_AddEventWatch(&incSDLEventWatcher, NULL);

  // when switching between borderless and windowed, restore appropriate shadow state
  static const MARGINS shadow_state[2]{ { 0,0,0,0 },{ 1,1,1,1 } };
  ::DwmExtendFrameIntoClientArea(handle, &shadow_state[1]);

  // redraw frame
  //::SetWindowPos(handle, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
  //::ShowWindow(handle, SW_SHOW);

  return window;
}

bool init()
{
  //Initialization flag
  bool success = true;

  //Initialize SDL
  if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
  {
    printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
    success = false;
  }
  else
  {
    //Set texture filtering to linear
    if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
    {
      printf( "Warning: Linear texture filtering not enabled!" );
    }

    gWindow = CreateSdlWindow();

    if( gWindow == NULL )
    {
      printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
      success = false;
    }
    else
    {
      //Create renderer for window
      gRenderer = SDL_CreateRenderer( gWindow, NULL, SDL_RENDERER_ACCELERATED );
      if( gRenderer == NULL )
      {
        printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
        success = false;
      }
      else
      {
        //Initialize renderer color
        SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
      }
    }
  }

  return success;
}

bool loadMedia()
{
  //Loading success flag
  bool success = true;

  //Load PNG texture
  gTexture = loadTexture( "../../../image.bmp" );
  if( gTexture == NULL )
  {
    printf( "Failed to load texture image!\n" );
    success = false;
  }

  return success;
}

void close()
{
  //Free loaded image
  SDL_DestroyTexture( gTexture );
  gTexture = NULL;

  //Destroy window  
  SDL_DestroyRenderer( gRenderer );
  SDL_DestroyWindow( gWindow );
  gWindow = NULL;
  gRenderer = NULL;

  //Quit SDL subsystems
  SDL_Quit();
}

SDL_Texture* loadTexture( std::string path )
{
  //The final texture
  SDL_Texture* newTexture = NULL;

  //Load image at specified path
  SDL_Surface* loadedSurface = SDL_LoadBMP( path.c_str() );
  if( loadedSurface == NULL )
  {
    printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), SDL_GetError() );
  }
  else
  {
    //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
    if( newTexture == NULL )
    {
      printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
    }

    //Get rid of old loaded surface
    SDL_DestroySurface( loadedSurface );
  }

  return newTexture;
}


int main( int argc, char* args[] )
{
  //Start up SDL and create window
  if( !init() )
  {
    printf( "Failed to initialize!\n" );
  }
  else
  {
    //Load media
    if( !loadMedia() )
    {
      printf( "Failed to load media!\n" );
    }
    else
    {  
      //Main loop flag
      bool quit = false;

      //Event handler
      SDL_Event e;

      //While application is running
      while( !quit )
      {
        //Handle events on queue
        while( SDL_PollEvent( &e ) != 0 )
        {
          //User requests quit
          if( e.type == SDL_EVENT_QUIT )
          {
            quit = true;
          }
          else if (e.type == SDL_EVENT_KEY_DOWN)
          {
            switch (e.key.keysym.sym)
            {
            case SDLK_ESCAPE:
              quit = true;
              break;
            case SDLK_f:
              SDL_MaximizeWindow(gWindow);
              break;
            }
          }
        }
        
        int w, h;
        SDL_GetCurrentRenderOutputSize(gRenderer, &w, &h);
        //SDL_GL_GetDrawableSize(gWindow, &w, &h);
        
        const SDL_Rect rect{0,0, w, h};
        SDL_SetRenderViewport(gRenderer, &rect);

        //Clear screen
        SDL_RenderClear( gRenderer );

        //Render texture to screen
        SDL_RenderTexture( gRenderer, gTexture, NULL, NULL );

        //Update screen
        SDL_RenderPresent( gRenderer );
      }
    }
  }

  //Free resources and close SDL
  close();

  return 0;
}