#include <iostream>

class ApplicationControlManager {
 public:
  enum class supported_apps_t {
    NOTHING,  // Don't perform any app specific media operations.
    VLC,
    ITUNES
  };

  ApplicationControlManager(
      supported_apps_t initial_app = supported_apps_t::NOTHING)
      : selected_app_(initial_app) {}

  void SwitchControlTo(supported_apps_t new_app) { selected_app_ = new_app; }

  void togglePlay() const {
    switch (selected_app_) {
      case supported_apps_t::NOTHING:
        break;
      case supported_apps_t::VLC:
        std::cout << "Toggling VLC pause / play" << std::endl;
        if (0 != std::system("osascript -e 'tell application \"VLC\" to play'"))
          throw std::runtime_error("Unable to pause / play VLC!");
        break;
      case supported_apps_t::ITUNES:
        break;
    }
  }

  void nextTrack() const {
    switch (selected_app_) {
      case supported_apps_t::NOTHING:
        break;
      case supported_apps_t::VLC:
        std::cout << "Advancing VLC to the next track" << std::endl;
        if (0 != std::system("osascript -e 'tell application \"VLC\" to next'"))
          throw std::runtime_error("Unable to advance VLC to the next track!");
        break;
      case supported_apps_t::ITUNES:
        break;
    }
  }

  void previousTrack() const {
    switch (selected_app_) {
      case supported_apps_t::NOTHING:
        break;
      case supported_apps_t::VLC:
        std::cout << "Advancing VLC to the previous track" << std::endl;
        if (0 !=
            std::system("osascript -e 'tell application \"VLC\" to previous'"))
          throw std::runtime_error(
              "Unable to advance VLC to the previous track!");
        break;
      case supported_apps_t::ITUNES:
        break;
    }
  }

  void incrementVolumeBy(int adjust_amount) {
    // A switch statement could also be used here to control the
    // applications volume instead of the system volume.
    std::string cmd =
        "osascript -e 'set volume output volume ((output volume of (get volume "
        "settings)) + " +
        std::to_string(adjust_amount) + ")'";
    // An error is printed here instead of throwing an exception because it is
    // not uncommon for the volume increment to fail. For example it will fail
    // when an external soundcard is used which does not support volume control.
    if (0 != std::system(cmd.c_str()))
      std::cerr << "Unable to adjust system volume!" << std::endl;
  }

  void stepForward() {
    switch (selected_app_) {
      case supported_apps_t::NOTHING:
        break;
      case supported_apps_t::VLC:
        if (0 !=
            std::system("osascript -e 'tell application \"VLC\" to step forward'"))
          throw std::runtime_error(
              "Unable to step VLC forward!");
        break;
      case supported_apps_t::ITUNES:
        break;
    }
  }

  void stepBackward() {
    switch (selected_app_) {
      case supported_apps_t::NOTHING:
        break;
      case supported_apps_t::VLC:
        if (0 !=
            std::system("osascript -e 'tell application \"VLC\" to step backward'"))
          throw std::runtime_error(
              "Unable to step VLC backward!");
        break;
      case supported_apps_t::ITUNES:
        break;
    }
  }

 private:
  supported_apps_t selected_app_;
};
