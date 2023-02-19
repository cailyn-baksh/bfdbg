# bfdbg

A Brainfuck debugger & editor. This curses program allows you to run and write
brainfuck code while watching what it is doing to the memory tape in real time.

## Features

This is more of a todo list for devs than a feature list for users

 - [x] Modular subwindow system (panes)
 - [x] Fixed framerate rendering with event processing in downtime
 - [x] Output view pane
 - [x] Memory view pane
   - [ ] Address column
   - [ ] Multi-byte cells
 - [ ] Program view pane
 - [ ] Brainfuck interpreter in separate thread
 - [ ] Pane scrolling

## Building

This program requires the following tools and libraries
 - GNU Make
 - A C compiler which
   - Supports C23 with GNU extensions
   - Includes the Concurrency Support Library (threads and atomics)
 - `unistd.h` must be available in the build environment (getopt)
 - ncurses and its headers must be installed and in the system path

The makefile is written for use with clang (v15.0.7), however this can be
overridden by setting the `CC` variable on the command line. Additionally, the
linker (default `lld`) can be changed by setting the `LD` variable.

To compile this program yourself, simply run `make`. 

