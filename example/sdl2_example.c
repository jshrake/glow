/*
 * Minimal ANSI C example using glow with SDL2
 * gcc -o sdl2_example  -Wall -Werror -Weverything -pedantic -ansi -lSDL2 sdl2_example.c
 */

#include <SDL2/SDL.h>
#include <stdio.h>
#define GLOW_IMPLEMENTATION
#define GLOW_DEBUG
#include "gl_core_4_1.h"

static void pre_cb(char const *name, void *funcptr, ...) {
  (void)funcptr;
  printf("Pre callback: %s\n", name);
}

static void post_cb(char const *name, void *funcptr, ...) {
  (void)funcptr;
  printf("Post callback: %s\n", name);
}

int main(int argc, char *argv[]) {
  SDL_Window *window;
  bool quit;
  (void)argc;
  (void)argv;
  SDL_Init(SDL_INIT_VIDEO);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  window =
      SDL_CreateWindow("${APPLICATION_NAME}", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, 600, 800, SDL_WINDOW_OPENGL);
  if (window == NULL) {
    printf("Could not create window: %s\n", SDL_GetError());
    return 1;
  }
  SDL_GL_CreateContext(window);
  /*
   * Uncomment the below code to eagerly load the OpenGL functions with
   * the SDL function loader. Alternatively, just call glow_init()
   */
  /*
  {
    int failures = glow_init_with(&SDL_GL_GetProcAddress);
    if (failures) {
      printf("glow failed to load %d functions\n", failures);
      return failures;
    }
  }
  */
  /*Callbacks only used when using #define GLOW_DEBUG*/
  glow_set_pre_callback(&pre_cb);
  glow_set_post_callback(&post_cb);

  quit = false;
  while (!quit) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        quit = true;
      }
    }
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(window);
  }
  return 0;
}
