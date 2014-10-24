#include <iostream> 
#include <ctime>
#include <algorithm>
#include <cstdio>
#include <cstdlib>

#include <myo/myo.hpp>

class DataCollector : public myo::DeviceListener {
public:
    DataCollector() : current_pose_(), locked_(true), unlocked_at_(0), base_volume_(0), roll_(0), roll_prev_(0) {}

    void onPose(myo::Myo* myo, uint64_t timestamp, myo::Pose pose)
    {
        if (locked_) {
            if (pose == myo::Pose::thumbToPinky) {
                std::cout << "Unlocked" << std::endl;
                locked_ = false;
                extendUnlock();
                myo->vibrate(myo::Myo::vibrationShort);
                myo->vibrate(myo::Myo::vibrationShort);
            }
        } else {
            if (pose == myo::Pose::fingersSpread) {
                std::cout << "Toggling pause / play" << std::endl;
                if (0 != std::system("osascript -e 'tell application \"VLC\" to play'")) {
                    throw std::runtime_error("Unable to pause / play VLC!");
                }
                extendUnlock();
            } else if (pose == myo::Pose::fist && current_pose_ != myo::Pose::fist) {
                std::cout << "Adjusting volume" << std::endl;
                roll_prev_ = roll_;
                char volume_cstr[4];
                FILE* volume_p = popen("osascript -e 'get output volume of (get volume settings)'", "r");
                if (volume_p == NULL) {
                    throw std::runtime_error("Unable to adjust volume!");
                }
                std::fgets(volume_cstr, 4, volume_p);
                base_volume_ = std::atoi(volume_cstr);
                pclose(volume_p);
            }
        }
        current_pose_ = pose;
    }

    void onOrientationData(myo::Myo* myo, uint64_t timestamp, const myo::Quaternion<float>& quat)
    {
        // -pi < roll_ <= +pi
        roll_ = atan2(2.0f * (quat.w() * quat.x() + quat.y() * quat.z()),
                      1.0f - 2.0f * (quat.x() * quat.x() + quat.y() * quat.y()));
    }

    void onPeriodic(myo::Myo* myo)
    {
        if (! locked_) {
            if (current_pose_ == myo::Pose::fist) {
                extendUnlock();
                updateVolume();
            }
            updateLockState(myo);
        }
    }

private:
    myo::Pose current_pose_;
    bool locked_;
    time_t unlocked_at_;
    int base_volume_;
    float roll_, roll_prev_;

    void extendUnlock() { unlocked_at_ = time(0); }
    void updateLockState(myo::Myo* myo)
    {
        if (time(0) - unlocked_at_ > 3) {
            std::cout << "Locked" << std::endl;
            locked_ = true;
            myo->vibrate(myo::Myo::vibrationShort);
        }
    }
    void updateVolume()
    {
        float roll_diff = roll_ - roll_prev_;
        // Ensure that roll_diff is continuous from -pi/2 to +pi/2.
        if (roll_diff > M_PI) {
            roll_diff -= (2 * M_PI);
        }
        if (roll_diff < -M_PI) {
            roll_diff += (2 * M_PI);
        }
        roll_diff *= 45 / M_PI; // Change this to your preference.
        int new_volume = base_volume_ + roll_diff;
        if (new_volume < 0) {
            new_volume = 0;
        } else if (new_volume > 100) {
            new_volume = 100;
        }
        std::string volume_cmd = "osascript -e 'set volume output volume "
                                 + std::to_string(new_volume) + "'";
        if (0 != std::system(volume_cmd.c_str())) {
            throw std::runtime_error("Unable to adjust volume!");
        }
    }
};

int main()
{
    try {
        myo::Hub hub("come.voidingwarranties.myo-media");

        myo::Myo* myo = hub.waitForMyo(10000);

        if (! myo) {
            throw std::runtime_error("Unable to find a Myo!");
        }

        DataCollector collector;
        hub.addListener(&collector);

        while (true) {
            hub.run(1000 / 20);
            collector.onPeriodic(myo);
        }

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
