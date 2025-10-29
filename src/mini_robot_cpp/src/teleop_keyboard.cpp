
#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <atomic>
#include <chrono>
#include <thread>
#include <mutex>
#include <cmath>
#include <iostream>

using namespace std;
using namespace geometry_msgs::msg;

class TeleopKeyboard : public rclcpp::Node
{
public:
    TeleopKeyboard() : Node("teleop_keyboard"), running_(true), vx_(0.0), wz_(0.0)
    {
        step_forward_ = this->declare_parameter("step_forward", 0.1);
        // make backward step positive and handle sign in logic
        step_backward_ = std::abs(this->declare_parameter("step_backward", -0.1));
        rate_ = this->declare_parameter("rate", 50);

        cmdvel_pub_ = this->create_publisher<Twist>("cmd_vel", 10);

        // Start input thread (will be joined in destructor)
        input_thread = std::thread([this]() { inputloop(); });

        // Timer publishes current vx_/wz_
        timer_ = this->create_wall_timer(std::chrono::milliseconds(1000 / rate_), [this]() {
            Twist msg;
            {
                std::lock_guard<std::mutex> lk(vel_mutex_);
                msg.linear.x = vx_;
                msg.angular.z = wz_;
            }
            cmdvel_pub_->publish(msg);
            RCLCPP_DEBUG(this->get_logger(), "Published cmd_vel: vx=%.2f wz=%.2f", msg.linear.x, msg.angular.z);
        });
    }

    ~TeleopKeyboard()
    {
        running_ = false;
        if (input_thread.joinable()) {
            // Writing a newline to stdin won't always help; just join and rely on getline returning on EOF when process exits
            input_thread.join();
        }
    }

private:
    rclcpp::Publisher<Twist>::SharedPtr cmdvel_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
    std::thread input_thread;
    std::atomic<bool> running_;
    double vx_, wz_;
    std::mutex vel_mutex_;
    double step_forward_;
    double step_backward_;
    int rate_;

    void inputloop()
    {
        std::string line;
        while (running_ && std::getline(std::cin, line))
        {
            if (!running_) break;
            if (line.empty()) continue;
            char c = line[0];
            {
                std::lock_guard<std::mutex> lk(vel_mutex_);
                switch (c) {
                    case 'a':
                        vx_ -= step_backward_;
                        break;
                    case 'l':
                        vx_ += step_forward_;
                        break;
                    case 'k':
                        wz_ += step_forward_;
                        break;
                    case 's':
                        wz_ -= step_backward_;
                        break;
                    case ' ':
                        vx_ = 0.0;
                        wz_ = 0.0;
                        break;
                    case 'q':
                        running_ = false;
                        rclcpp::shutdown();
                        break;
                    default:
                        break;
                }
                RCLCPP_INFO(this->get_logger(), "vx: %.2f, wz: %.2f", vx_, wz_);
            }
        }
    }
};

int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<TeleopKeyboard>();

    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}


