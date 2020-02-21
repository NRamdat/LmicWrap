#ifndef LMICWRAP_H
#define LMICWRAP_H

#include <lmic.h>
#include <hal/hal.h>

void onEvent (ev_t ev);

class LmicWrap
{
  public:
    LmicWrap();
    ~LmicWrap();
    void init();
    //void send_data(unsigned char mydata[], unsigned char size);
    static void prepData(osjob_t*, unsigned char mydata[]);
    void startJoining();
    void evJoined();
    void sendData();
};

#endif
