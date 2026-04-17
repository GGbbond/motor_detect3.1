# 终端 1：配置 CAN 并运行 motor_test
gnome-terminal -- bash -c "echo '123' | sudo -S ifconfig can0 down && sudo ip link set can0 type can bitrate 1000000 && sudo ifconfig can0 up && sudo ifconfig can0 txqueuelen 100000 && cd /home/fzm/motor_inspection3.1/build && ./bin/motor_test"

# 终端 2：运行 Python 客户端
gnome-terminal -- bash -c "python3 /home/fzm/motor_inspection3.1/motor_client.py"
