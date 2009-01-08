/* Game of Life in X 
 * This code is based on some Xlib tutorial (I don't remember which one)
 */

#include <X11/Xlib.h> // Every Xlib program must include this
#include <assert.h>   // I include this to test return values the lazy way
#include <unistd.h>   // So we got the profile for 10 seconds
#include <stdlib.h>

#include <time.h>
#include <signal.h>
#include <getopt.h>
#include <stdio.h>

Display* display;
Window win;
GC gc;

#define XRES 512
#define YRES 512
#define PROB 8

int state[2][XRES][YRES];
int gen = 0;

int draw(int plane){
  char buf[64];
  int len;
  int x, y;

  XClearWindow(display, win);
    
  len = sprintf(buf, "Generation: %d", gen);
  XDrawString(display, win,gc, 0, 10, buf, len);

  for (x=0; x<XRES; x++)
    for (y=0; y<YRES; y++)
      if (state[plane][x][y])
	XDrawPoint(display, win, gc, x, y+10);

  XFlush(display);

}

int init(){
  int plane, x, y;
  for (plane=0; plane<2; plane++)
    for (x=0; x<XRES; x++)
      for (y=0; y<YRES; y++)
	state[plane][x][y] = (rand() % PROB == 0);
}

int nx(int x){
  switch (x){
  case -1:
    return XRES - 1;
  case XRES:
    return 0;
  default:
    return x;
  }
}
int ny(int y){
  switch (y){
  case -1:
    return YRES - 1;
  case YRES:
    return 0;
  default:
    return y;
  }
}

int step(int from, int to){

  int x, y;
  
  for (x=0; x<XRES; x++)
    for (y=0; y<YRES; y++){
      int c=0;

      if (state[from][nx(x-1)][ny(y-1)]) c++;
      if (state[from][nx(x-1)][ny(y  )]) c++;
      if (state[from][nx(x  )][ny(y-1)]) c++;
      if (state[from][nx(x+1)][ny(y+1)]) c++;
      if (state[from][nx(x+1)][ny(y  )]) c++;
      if (state[from][nx(x  )][ny(y+1)]) c++;
      if (state[from][nx(x-1)][ny(y+1)]) c++;
      if (state[from][nx(x+1)][ny(y-1)]) c++;
      
      if (c==3){
	state[to][x][y]=1;
      }else if (c==2){
	state[to][x][y]=state[from][x][y];
      }else{
	state[to][x][y]=0;
      }
    }
      
}

int main(int argc, char**argv){

  unsigned long whiteColor;
  unsigned long blackColor;  


  display = XOpenDisplay(NULL);
  assert(display);
  
  whiteColor = WhitePixel(display, DefaultScreen(display));
  blackColor=BlackPixel(display, DefaultScreen(display));
  
  win = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, 
			    512, 522, 0, whiteColor, blackColor);
  
  
  XSelectInput(display, win, StructureNotifyMask);
  XMapWindow(display, win);
  
  gc = XCreateGC(display, win, 0, NULL);
  XSetForeground(display, gc, whiteColor);

  //  XSelectInput(display, win, ExposureMask);

  for(;;) {
    XEvent e;
    XNextEvent(display, &e);
    if (e.type == MapNotify)
      break;
  }


  double p = 0;

  init();

  while(1){
    draw(gen%2);
    step(gen%2, (gen+1)%2);
    gen++;
    //  sleep(1);
  }

 
  for(;;) {
    XEvent e;
    XNextEvent(display, &e);
    //if (e.type == MapNotify)
    //  break;
  }
 
}
