
/**********************************************************************

CurrentMonitor.h
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#ifndef CURRENTMONITOR_H
#define CURRENTMONITOR_H

#include "Arduino.h"
#include "Config.h"

#define CURRENT_SAMPLE_SMOOTHING 0.01
#define CURRENT_SAMPLE_MAX 3200 // 2,7 V
#define CURRENT_SAMPLE_TIME 1

struct CurrentMonitor
{
  int m_pin;
  float m_current;
  char *m_msg;
  CurrentMonitor(int, char *);
  void check();
  void over(INTERFACE*);
  void setup(INTERFACE*);
  float current();
};

#endif
