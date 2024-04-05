#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <memory>
#include <iostream>

// カスタムアロケータの定義
template<typename T>
class MyAllocator {
public:
    using value_type = T;

    MyAllocator() = default;
    template<typename U> constexpr MyAllocator(const MyAllocator<U>&) noexcept {}

    T* allocate(std::size_t n) {
        auto p = std::malloc(n * sizeof(T));
        if (!p) throw std::bad_alloc();
        return static_cast<T*>(p);
    }

    void deallocate(T* p, std::size_t) noexcept {
        std::free(p);
    }
};
template <typename T, typename U>
bool operator==(const MyAllocator<T>&, const MyAllocator<U>&) { return true; }
template <typename T, typename U>
bool operator!=(const MyAllocator<T>&, const MyAllocator<U>&) { return false; }

class CustomAllocatorNode : public rclcpp::Node {
public:
    CustomAllocatorNode()
    : Node("custom_allocator_node") {
        publisher_ = this->create_publisher<sensor_msgs::msg::PointCloud2_<MyAllocator<void>>>("point_cloud", 10);
        timer_ = this->create_wall_timer(
            std::chrono::seconds(1),
            std::bind(&CustomAllocatorNode::timer_callback, this));
    }

private:
    void timer_callback() {
        MyAllocator<void> my_allocator;
        auto output_msg = std::allocate_shared<sensor_msgs::msg::PointCloud2_<MyAllocator<void>>>(my_allocator);

        //publisher_->publish(*output_msg);
        RCLCPP_INFO(this->get_logger(), "Published a PointCloud2 message.");
    }

    rclcpp::Publisher<sensor_msgs::msg::PointCloud2_<MyAllocator<void>>>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<CustomAllocatorNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}

