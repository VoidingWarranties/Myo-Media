#include <iostream>

#include <ctime>

#include <myo/myo.hpp>

class DataCollector : public myo::DeviceListener {
public:
    DataCollector() : current_pose_(), locked_(true), unlocked_at_(0), roll_(0), roll_prev_(0) {}

    void onPose(myo::Myo* myo, uint64_t timestamp, myo::Pose pose)
    {
        if (locked_) {
            if (pose == myo::Pose::thumbToPinky) {
                locked_ = false;
                extendUnlock();
                myo->vibrate(myo::Myo::vibrationShort);
                myo->vibrate(myo::Myo::vibrationShort);
            }
        } else {
            if (pose == myo::Pose::fingersSpread) {
                system("osascript -e 'tell application \"VLC\" to play'");
                extendUnlock();
            } else if (pose == myo::Pose::fist && current_pose_ != myo::Pose::fist) {
                roll_prev_ = roll_;
            }
        }
        current_pose_ = pose;
    }

    void onOrientationData(myo::Myo* myo, uint64_t timestamp, const myo::Quaternion<float>& quat)
    {
        roll_ = atan2(2.0f * (quat.w() * quat.x() + quat.y() * quat.z()),
                      1.0f - 2.0f * (quat.x() * quat.x() + quat.y() * quat.y()));
        roll_ += M_PI;
        roll_ /= 2.0f * M_PI;
    }

    void onPeriodic(myo::Myo* myo)
    {
        if (! locked_) {
            updateLockState(myo);
        }
    }

private:
    myo::Pose current_pose_;
    bool locked_;
    time_t unlocked_at_;
    float roll_;
    float roll_prev_;

    void extendUnlock() { unlocked_at_ = time(0); }
    void updateLockState(myo::Myo* myo)
    {
        if (time(0) - unlocked_at_ > 6) {
            locked_ = true;
            myo->vibrate(myo::Myo::vibrationShort);
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
            hub.run(1000 / 5);
            collector.onPeriodic(myo);
        }

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
