#ifndef _PXFINDER_H_
#define _PXFINDER_H_

// #define MOVE_OFFSET 0.2 // how many pixels
extern float MOVE_OFFSET;

extern void pxfinder_init();
extern void pxfinder_draw(GsOT ot, int f); // f=1 execute FntFlush

#endif /* _PXFINDER_H_ */