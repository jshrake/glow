# glow
`glow` generates a single C/C++ file for loading OpenGL functions using the official OpenGL specification. The output consists of only the enumerations and functions specified by the  api `(gl | gles1 | gles2)`, spec `(1.0 | ... | 4.5)`, and profile `(core | compatibility)`. 

# output
[glow-headers](https://github.com/jshrake/glow-headers) contains files produced by enumerating all valid combinations of `api`, `spec`, and `profile` with no extensions.

# usage
Requires [go](https://golang.org/).
``` bash
git clone https://github.com/jshrake/glow
cd glow
go get
go build
make download # Downloads (curl) the latest khrplatform.h and gl.xml spec
cat gl.xml | ./glow --api=gl --spec=4.1 --profile=core --debug=true > gl_core_4_1.h
```

See the `gen` target in the [Makefile](Makefile). This target is used to produce the files found in [glow-headers](https://github.com/jshrake/glow-headers).

# examples
Build with `make examples`

## SDL2
```C
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
```

# public domain
`glow` and the resulting output are licensed under the public domain.
[Why public domain](https://github.com/nothings/stb/blob/master/docs/why_public_domain.md)

# inspiration
`glow` is inspired by:
- [glad](https://github.com/Dav1dde/glad)
- [glLoadGen](https://bitbucket.org/alfonse/glloadgen/wiki/Home)
- [gl3w](https://github.com/skaslev/gl3w)
- [stb](https://github.com/nothings/stb)
