# RoboticStudio2-JAMC
| Jono | Ayberk | Mattia | Connor |

1. Clone Project
```bash
git clone https://github.com/FrozenBreadstick/RoboticStudio2-JAMC
```
2. Navigate to your ROS2 Workspace SOURCE directory
```bash
~/ros2_ws/src
```
3. Link the project directory to your ROS2 Workspace
```bash
ln -s ~/git/RoboticStudio2-JAMC
```
4. Compile with colcon in the ros2_ws folder
```bash
~/ros2_ws
colcon build --symlink-install --packages-select jamc
```

### Tip: 
- Add an alias for colcon build to your .bashrc file to make it easier!
```bash
nano ~/.bashrc
alias cb='colcon build --symlink-install --packages-select`
```
So you can just type:
```bash
cb PACKAGE_NAME
```
To compile a package.

