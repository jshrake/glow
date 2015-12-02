all: build

clean:
	go clean

install:
	go install

build:
	go get
	go build

download:
	curl -o template/khrplatform.h https://www.khronos.org/registry/egl/api/KHR/khrplatform.h
	curl -o gl.xml https://cvs.khronos.org/svn/repos/ogl/trunk/doc/registry/public/api/gl.xml

gen: download build
	mkdir -p output
	cat gl.xml | ./glow --api=gl --spec=1.0 > output/gl_core_1_0.h
	cat gl.xml | ./glow --api=gl --spec=1.1 > output/gl_core_1_1.h
	cat gl.xml | ./glow --api=gl --spec=1.2 > output/gl_core_1_2.h
	cat gl.xml | ./glow --api=gl --spec=1.3 > output/gl_core_1_3.h
	cat gl.xml | ./glow --api=gl --spec=1.4 > output/gl_core_1_4.h
	cat gl.xml | ./glow --api=gl --spec=1.5 > output/gl_core_1_5.h
	cat gl.xml | ./glow --api=gl --spec=2.0 > output/gl_core_2_0.h
	cat gl.xml | ./glow --api=gl --spec=2.1 > output/gl_core_2_1.h
	cat gl.xml | ./glow --api=gl --spec=3.0 > output/gl_core_3_0.h
	cat gl.xml | ./glow --api=gl --spec=3.1 > output/gl_core_3_1.h
	cat gl.xml | ./glow --api=gl --spec=3.2 > output/gl_core_3_2.h
	cat gl.xml | ./glow --api=gl --spec=3.3 > output/gl_core_3_3.h
	cat gl.xml | ./glow --api=gl --spec=4.0 > output/gl_core_4_0.h
	cat gl.xml | ./glow --api=gl --spec=4.1 > output/gl_core_4_1.h
	cat gl.xml | ./glow --api=gl --spec=4.2 > output/gl_core_4_2.h
	cat gl.xml | ./glow --api=gl --spec=4.3 > output/gl_core_4_3.h
	cat gl.xml | ./glow --api=gl --spec=4.4 > output/gl_core_4_4.h
	cat gl.xml | ./glow --api=gl --spec=4.5 > output/gl_core_4_5.h
	cat gl.xml | ./glow --api=gl --spec=3.2 --profile=compatibility > output/gl_compat_3_2.h
	cat gl.xml | ./glow --api=gl --spec=3.3 --profile=compatibility > output/gl_compat_3_3.h
	cat gl.xml | ./glow --api=gl --spec=4.0 --profile=compatibility > output/gl_compat_4_0.h
	cat gl.xml | ./glow --api=gl --spec=4.1 --profile=compatibility > output/gl_compat_4_1.h
	cat gl.xml | ./glow --api=gl --spec=4.2 --profile=compatibility > output/gl_compat_4_2.h
	cat gl.xml | ./glow --api=gl --spec=4.3 --profile=compatibility > output/gl_compat_4_3.h
	cat gl.xml | ./glow --api=gl --spec=4.4 --profile=compatibility > output/gl_compat_4_4.h
	cat gl.xml | ./glow --api=gl --spec=4.5 --profile=compatibility > output/gl_compat_4_5.h
	cat gl.xml | ./glow --api=gles1 --spec=1.0 > output/gles_1_0.h
	cat gl.xml | ./glow --api=gles2 --spec=2.0 > output/gles_2_0.h
	cat gl.xml | ./glow --api=gles2 --spec=3.0 > output/gles_3_0.h
	cat gl.xml | ./glow --api=gles2 --spec=3.1 > output/gles_3_1.h
	cat gl.xml | ./glow --api=gles2 --spec=3.2 > output/gles_3_2.h

examples: download build
	cat gl.xml | ./glow --api=gl --spec=4.1 > example/gl_core_4_1.h
	gcc -o example/sdl2_example -Wall -Werror -Weverything -pedantic -ansi -lSDL2 example/sdl2_example.c
