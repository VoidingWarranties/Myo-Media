#include <iostream>

#include "EventListener.h"
#include "ApplicationControlManager.h"

#include <myo/myo.hpp>

int main() {
  try {
    myo::Hub hub("com.voidingwarranties.myo-media");
    myo::Myo* myo = hub.waitForMyo(10000);
    if (!myo) {
      throw std::runtime_error("Unable to find a Myo!");
    }

    ApplicationControlManager app_control;
    EventListener listener([&app_control]() { app_control.togglePlay(); },
                           [&app_control]() { app_control.nextTrack(); },
                           [&app_control]() { app_control.previousTrack(); },
                           [&app_control](int vol_inc) {
                             app_control.incrementVolumeBy(vol_inc);
                           });
    hub.addListener(&listener);

    // Event loop.
    while (true) {
      hub.run(1000 / 20);
      listener.onPeriodic(myo);
    }
  } catch (const std::exception& ex) {
    std::cerr << "Error: " << ex.what() << std::endl;
    return 1;
  }

  return 0;
}
