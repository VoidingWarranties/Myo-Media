#include <ctime>

#include <myo/myo.hpp>

class EventListener : public myo::DeviceListener {
 public:
  EventListener(const std::function<void()>& togglePlay,
                const std::function<void()>& nextTrack,
                const std::function<void()>& prevTrack,
                const std::function<void(int)>& incVolBy)
      : togglePlay_(togglePlay),
        nextTrack_(nextTrack),
        previousTrack_(prevTrack),
        incrementVolumeBy_(incVolBy),
        arm_(myo::armUnknown),
        x_direction_(myo::xDirectionUnknown),
        current_pose_(myo::Pose::unknown),
        roll_(0),
        roll_prev_(0),
        volume_prev_(0),
        locked_(true),
        unlocked_at_time_(0) {}

  // Overridden virtual functions of myo::DeviceListener
  void onArmRecognized(myo::Myo* myo, uint64_t timestamp, myo::Arm arm,
                       myo::XDirection x_direction) {
    arm_ = arm;
    x_direction_ = x_direction;
  }
  void onPose(myo::Myo* myo, uint64_t timestamp, myo::Pose pose) {
    if (locked_) {
      if (pose == myo::Pose::thumbToPinky) {
        std::cout << "Unlocked" << std::endl;
        locked_ = false;
        extendUnlock();
        myo->vibrate(myo::Myo::vibrationShort);
        myo->vibrate(myo::Myo::vibrationShort);
      }
    } else {
      // Map waveIn and waveOut to always mean wave left and right respectively.
      if (arm_ == myo::armLeft) {
        if (pose == myo::Pose::waveIn)
          pose = myo::Pose::waveOut;
        else if (pose == myo::Pose::waveOut)
          pose = myo::Pose::waveIn;
      }
      switch (pose.type()) {
        case myo::Pose::fingersSpread:
          togglePlay_();
          extendUnlock();
          break;
        case myo::Pose::waveIn:
          previousTrack_();
          extendUnlock();
          break;
        case myo::Pose::waveOut:
          nextTrack_();
          extendUnlock();
          break;
        case myo::Pose::fist:
          roll_prev_ = roll_;
          volume_prev_ = 0;
          extendUnlock();
          break;
      }
    }
    current_pose_ = pose;
  }
  void onOrientationData(myo::Myo* myo, uint64_t timestamp,
                         const myo::Quaternion<float>& quat) {
    // -pi < roll_ <= +pi
    roll_ = atan2(2.0f * (quat.w() * quat.x() + quat.y() * quat.z()),
                  1.0f - 2.0f * (quat.x() * quat.x() + quat.y() * quat.y()));
  }

  // Helper function intended to be called every time hub.run is called.
  void onPeriodic(myo::Myo* myo) {
    if (!locked_) {
      if (current_pose_ == myo::Pose::fist) {
        extendUnlock();
        updateVolume();
      }
      updateLockState(myo);
    }
  }

 private:
  std::function<void()> togglePlay_, nextTrack_, previousTrack_;
  std::function<void(int)> incrementVolumeBy_;

  myo::Arm arm_;
  myo::XDirection x_direction_;
  myo::Pose current_pose_;
  float roll_, roll_prev_, volume_prev_;
  bool locked_;
  time_t unlocked_at_time_;

  EventListener() {}
  void extendUnlock() { unlocked_at_time_ = time(0); }
  void updateLockState(myo::Myo* myo) {
    if (time(0) - unlocked_at_time_ > 3) {
      std::cout << "Locked" << std::endl;
      locked_ = true;
      myo->vibrate(myo::Myo::vibrationShort);
    }
  }
  // Helper function for adjusting the volume. This function calls the user
  // provided incrementVolumeBy function.
  void updateVolume() {
    float roll_diff = roll_ - roll_prev_;
    // Swap the rotation direction depending on the orientation of the Myo.
    if (x_direction_ != myo::xDirectionTowardWrist) {
      roll_diff *= -1;
    }
    // Ensure that roll_diff is continuous from -pi/2 to +pi/2.
    // Is there a better way to do this which allows infinite rotation instead
    // of limiting it to -pi/2 to +pi/2?
    if (roll_diff > M_PI) {
      roll_diff -= (2 * M_PI);
    }
    if (roll_diff < -M_PI) {
      roll_diff += (2 * M_PI);
    }
    int volume_diff = roll_diff * 45 / M_PI;  // Change this to your preference.
    int volume_increment = volume_diff - volume_prev_;
    incrementVolumeBy_(volume_increment);
    volume_prev_ = volume_diff;
  }
};
