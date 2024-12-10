# TinyX11VNC

## Introduce
This project is written at 09/12/2024. It refrenced more libraries, including libvncserver, libxcb, etc.
Next, I will continue to enhance some features of this project.
I also trying to support macOS and some Linux distributions.

## How to use it?
1. Clone this repository on your Linux machine.
2. Run `make` command.
3. You can found a executable file called `tiny-x11vnc`.
4. Run `tiny-x11vnc`, and using any vnc client to connection your Linux machine.
5. The basic mouse events, keyboard events and screen info have been implemented.

## Dependency Tools
Please ensure some libraries exists on your machine.
1. libvncserver
2. libxcb, libxcb-xtest, libxcb-xfixes, libxcb-keysyms, libxcb-damage

## Todo List
- [x] Complete a basic VNC server and enabled keyboard, mouse, and screen capture.
- [ ] Feature: Enable auto-scaling for X11 VNC server.
- [ ] Feature: Support XDamage to reduce CPU usage and frequency of framebuffer updates.
- [ ] Feature: Enable all keys on keyboard for X11 VNC server.
- [ ] Feature: Enhance mouse pointer to fit some shapes, for instance: arrow, watch, hand, resize, etc.


## Demo

## Unit Test

## Component Test
