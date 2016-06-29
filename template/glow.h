/*
 * glow-{{.Version}}: public-domain OpenGL function loader
 *
 * This file was generated with glow https://github.com/jshrake/glow
 * 
 * Version: {{.Version}}
 * Date: {{.Date}}
 * Language: C
 * Specification: {{.Specification}}
 * API: {{.Api}}
 * Profile: {{.Profile}}
 * Command-line: {{.CommandLine}}
 * Extensions:
{{range .Extensions}}
 * {{.}}{{end}}
 * 
 * -----
 * USAGE
 * -----
 * Do this:
 *    #define GLOW_IMPLEMENTATION
 * before you include this file in *one* C or C++ file to create the implementation.
 *
 * -------
 * EXAMPLE
 * -------
 * #include ...
 * #include ...
 * #define GLOW_IMPLEMENTATION
 * #include "glow.h"
 * 
 {{if .Debug}}
 * - #define GLOW_DEBUG before the #include to enable pre and post function callbacks{{end}}
 * - #define GLOW_ASSERT(x) before the #include to avoid including assert.h
 * - #define GLOW_NO_STDIO before the #include to avoid including stdio.h for print debugging
 *
 * Users are encouraged to call glow_init or glow_init_with after OpenGL context creation
 * to eagerly load the OpenGL function pointers. If no initialization function
 * is called, the function pointers are lazily loaded during the first call.
 *
 * Users can specify their own function to load the OpenGL procedures via glow_init_with.
 * Calling glow_init is equivalent to calling glow_init_with(&glow_get_proc);
 *
 */
#ifndef GLOW_INCLUDE_GLOW_H
#define GLOW_INCLUDE_GLOW_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef GLOW_STATIC
#define GLOWDEF static
#else
#define GLOWDEF extern
#endif
/*
 * ---------------------------
 * Glow API
 * ---------------------------
 */
typedef void* (*glow_load_proc_t)(char const *name);
/*
 * Platform specific function for returning OpenGL function pointers by name
 * Returns NULL on failure to to open the OpenGL library image and on failure
 * to load the procedure given by name.
 */
GLOWDEF void *glow_get_proc(char const *name);
/*
 * Eagerly load OpenGL function pointers using glow_get_proc. Returns the
 * number of functions that failed to load. A return value of 0 indicates
 * success.
 */
GLOWDEF int glow_init(void);
/*
 * Eagerly load OpenGL function pointers with the specified loading function.
 * Returns the number of functions that failed to load. A return value of 0
 * indicates success.
 */
GLOWDEF int glow_init_with(glow_load_proc_t get_proc);

{{if .Debug}}
typedef void (*glow_debug_proc_t)(char const *name, void *funcptr, ...);
/*
 * Specify a callback to call before OpenGL function calls
 */
GLOWDEF void glow_set_pre_callback(glow_debug_proc_t callback);
/*
 * Specify a callback to call after OpenGL function calls
 */
GLOWDEF void glow_set_post_callback(glow_debug_proc_t callback);
{{end}}

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif

/*
 * ---------------------------
 * Required includes
 * ---------------------------
 */
{{/*khrplatform.h is required for the gles api*/}}
{{if ne .Api "gl"}}{{template "khrplatform.h"}}
{{end}}{{range .Includes}}
{{.}}{{end}}

/*
 * ---------------------------
 * Versions
 * ---------------------------
 */{{range .VersionDefs}}
#define {{.}}{{end}}

/*
 * ---------------------------
 * Types
 * ---------------------------
 */{{range .Typedefs}}
{{.}}{{end}}

/*
 * ---------------------------
 * Enums
 * ---------------------------
 */{{range .Enums}}
#define {{.Name}} {{.Value}}{{end}}

/*
 * ---------------------------
 * Commands
 * ---------------------------
 */
{{range .Commands}}
typedef {{.ReturnType}} (APIENTRYP {{.PfnName}})({{.Params}});
GLOWDEF {{.PfnName}} glow_{{.Name}};{{end}}

{{if .Debug}}
#ifndef GLOW_DEBUG{{end}}
{{range .Commands}}
#define {{.Name}} glow_{{.Name}}{{end}}
{{if .Debug}}
#endif /* GLOW_DEBUG */
{{end}}
{{if .Debug}}
#ifdef GLOW_DEBUG
{{range .Commands}}
GLOWDEF {{.PfnName}} glow_debug_{{.Name}};
#define {{.Name}} glow_debug_{{.Name}}{{end}}
#endif /* GLOW_DEBUG */
{{end}}

#ifdef __cplusplus
}
#endif
#endif  /* GLOW_INCLUDE_GLOW_H */
/*
 * END HEADER, BEGIN IMPLEMENTATION
 */
#ifdef GLOW_IMPLEMENTATION

#include <stddef.h> /* for size_t */

#ifndef GLOW_NO_STDIO
#include <stdio.h>
#endif /* GLOW_NO_STDIO */

#ifndef GLOW_ASSERT
#include <assert.h>
#define GLOW_ASSERT(x) assert(x)
#endif /* GLOW_ASSERT */

#if defined(__APPLE__)
#include <dlfcn.h>
void *glow_get_proc(char const *name) {
  static void *lib = NULL;
  if (lib == NULL) {
    lib = dlopen("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", RTLD_LAZY);
  }
  GLOW_ASSERT(lib && "Can't open OpenGL shared library for function loading!");
  return lib ? dlsym(lib, name) : NULL;
}
#else
void *glow_get_proc(char const *name) {
  GLOW_ASSERT(0 && "Platform not supported. You must call glow_init_with.");
  return NULL;
}
#endif

{{range .Commands}}
static {{.ReturnType}} APIENTRY glow_lazy_{{.Name}}({{.Params}}) {
  glow_{{.Name}} = ({{.PfnName}})((glow_get_proc("{{.Name}}")));
  if (glow_{{.Name}} == NULL) {
    GLOW_ASSERT(0 && "glow error: failed to load {{.Name}}\n");
#ifndef GLOW_NO_STDIO
    printf("glow error: failed to load {{.Name}}\n");
#endif
  }{{if eq .ReturnType "void"}}
  glow_{{.Name}}({{.Args}});{{else}}
  return glow_{{.Name}}({{.Args}});{{end}}
}
{{.PfnName}} glow_{{.Name}} = glow_lazy_{{.Name}};{{end}}

{{if .Debug}}
static void glow_pre_callback_default_(char const *name, void *funcptr, ...) {
  (void)name;
  (void)funcptr;
}
static void glow_post_callback_default_(char const *name, void *funcptr, ...) {
  (void)name;
  (void)funcptr;
}
static glow_debug_proc_t glow_pre_callback_ = glow_pre_callback_default_;
static glow_debug_proc_t glow_post_callback_ = glow_post_callback_default_;
#ifdef GLOW_DEBUG
{{range .Commands}}
static {{.ReturnType}} glow_debug_impl_{{.Name}}({{.Params}}) {
{{if eq .ReturnType "void"}}
  glow_pre_callback_("{{.Name}}", (void*)glow_{{.Name}}{{.ArgsWithLeadingComma}});
  glow_{{.Name}}({{.Args}});
  glow_post_callback_("{{.Name}}", (void*)glow_{{.Name}}{{.ArgsWithLeadingComma}});
{{else}}
  {{.ReturnType}} ret;
  glow_pre_callback_("{{.Name}}", (void*)glow_{{.Name}}{{.ArgsWithLeadingComma}});
  ret = glow_{{.Name}}({{.Args}});
  glow_post_callback_("{{.Name}}", (void*)glow_{{.Name}}{{.ArgsWithLeadingComma}});
  return ret;
{{end}}}
{{.PfnName}} glow_debug_{{.Name}} = glow_debug_impl_{{.Name}};
{{end}}
#endif /* GLOW_DEBUG */
{{end}}


int glow_init(void) {
  return glow_init_with(&glow_get_proc);
}

int glow_init_with(glow_load_proc_t get_proc) {
  int unresolved_count = 0;
{{range .Commands}}
  glow_{{.Name}} = ({{.PfnName}})get_proc("{{.Name}}");
  if (glow_{{.Name}} == NULL) {
    GLOW_ASSERT(0 && "glow error: failed to load {{.Name}}\n");
#ifndef GLOW_NO_STDIO
    printf("glow error: failed to load {{.Name}}\n");
#endif
    unresolved_count += 1;
  } {{end}}
  return unresolved_count;
}

{{if .Debug}}
void glow_set_pre_callback(glow_debug_proc_t callback) {
  glow_pre_callback_ = callback;
}

void glow_set_post_callback(glow_debug_proc_t callback) {
  glow_post_callback_ = callback;
}
{{end}}
#endif  /* GLOW_IMPLEMENTATION */
