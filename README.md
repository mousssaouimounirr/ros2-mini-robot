# 🤖 ROS2 Mini Robot (C++) (working on it)

Workspace ROS 2 Humble en C++ para un robot diferencial (simulación).  
Incluye un nodo de prueba y CI con GitHub Actions.

## Build & Run
```bash
cd ~/ros2_mini_robot_ws
colcon build --symlink-install
source install/setup.bash
ros2 run mini_robot_cpp hello_node
