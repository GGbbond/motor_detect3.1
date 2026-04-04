#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// --- 平台检测与头文件处理 ---
#ifdef _WIN32
    // Windows 环境
    #include <winsock2.h>
    #include <ws2_32.h>
    #pragma comment(lib, "ws2_32.lib")
    #define sleep_ms(ms) Sleep(ms)
    typedef int socklen_t;
#else
    // Linux 环境
    #include <unistd.h>
    #include <pthread.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #define sleep_ms(ms) usleep((ms) * 1000)
    
    // --- 关键修复：为 Linux 定义 Windows 兼容的类型 ---
    typedef int SOCKET;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

// --- 物理引擎常量 (对应 C++ 类中的参数) ---
#define TORQUE_RATE 0.1f
#define POS_RATE 0.05f
#define INERTIA 0.5f
#define FRICTION 0.02f

// --- MotorSimulator 结构体 (模拟 C++ 类) ---
typedef struct {
    double target_torque;
    double current_torque;
    double target_pos;
    double current_pos;
    double velocity;
} MotorSimulator;

// --- MotorSimulator 方法 (模拟 C++ 成员函数) ---
void MotorSimulator_Init(MotorSimulator* motor) {
    motor->target_torque = 0.0;
    motor->current_torque = 0.0;
    motor->target_pos = 0.0;
    motor->current_pos = 0.0;
    motor->velocity = 0.0;
}

void MotorSimulator_SetTargetTorque(MotorSimulator* motor, double val) {
    motor->target_torque = val;
}

void MotorSimulator_SetTargetPos(MotorSimulator* motor, double val) {
    motor->target_pos = val;
}

double MotorSimulator_GetTorque(MotorSimulator* motor) {
    return motor->current_torque;
}

double MotorSimulator_GetPos(MotorSimulator* motor) {
    return motor->current_pos;
}

// 核心物理更新逻辑 (对应 C++ 的 motor.update)
void MotorSimulator_Update(MotorSimulator* motor, double dt) {
    // 简单的物理模型：加速度 = (目标扭矩 - 摩擦) / 惯量
    double accel = (motor->target_torque - motor->velocity * FRICTION) / INERTIA;
    
    // 更新速度
    motor->velocity += accel * TORQUE_RATE;
    
    // 更新位置
    motor->current_pos += motor->velocity * POS_RATE;
    
    // 更新当前扭矩读数
    motor->current_torque = motor->target_torque;
}

// --- 全局变量与线程控制 ---
MotorSimulator g_motor;
#ifdef _WIN32
    HANDLE h_update_thread;
    CRITICAL_SECTION g_mutex;
#else
    pthread_t update_thread;
    pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
int g_running = 1;
SOCKET g_client_sock = INVALID_SOCKET;

// --- 发送数据线程函数 ---
#ifdef _WIN32
DWORD WINAPI update_thread_func(LPVOID arg) {
#else
void* update_thread_func(void* arg) {
#endif
    char buffer[128];
    
    while (g_running) {
        // 加锁
        #ifdef _WIN32
            EnterCriticalSection(&g_mutex);
        #else
            pthread_mutex_lock(&g_mutex);
        #endif

        // 1. 更新物理状态 (20ms)
        MotorSimulator_Update(&g_motor, 0.02);

        // 2. 发送数据
        int len = snprintf(buffer, sizeof(buffer), 
                          "TORQUE %.2f\nPOS %.2f\n", 
                          MotorSimulator_GetTorque(&g_motor), 
                          MotorSimulator_GetPos(&g_motor));
        
        // 解锁
        #ifdef _WIN32
            LeaveCriticalSection(&g_mutex);
        #else
            pthread_mutex_unlock(&g_mutex);
        #endif

        // 发送 (不加锁，避免阻塞物理计算太久)
        if (g_client_sock != INVALID_SOCKET) {
            send(g_client_sock, buffer, len, 0);
        }

        sleep_ms(20);
    }
    return 0;
}

// --- 主函数 ---
int main() {
    // 初始化物理引擎
    MotorSimulator_Init(&g_motor);

    // 初始化网络 (Windows 需要 WSAStartup)
    #ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            printf("WSAStartup failed.\n");
            return 1;
        }
        InitializeCriticalSection(&g_mutex);
    #endif

    SOCKET server_sock, client_sock;
    struct sockaddr_in server, client;
    
    // 创建 Socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == INVALID_SOCKET) {
        printf("Could not create socket.\n");
        return 1;
    }

    // 绑定
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(9999);
    if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("Bind failed.\n");
        return 1;
    }

    // 监听
    listen(server_sock, 3);
    printf("Motor Server (C) listening on port 9999...\n");

    // 接受连接
    int c = sizeof(struct sockaddr_in);
    client_sock = accept(server_sock, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock == INVALID_SOCKET) {
        printf("Accept failed.\n");
        return 1;
    }
    g_client_sock = client_sock;
    printf("Connection accepted!\n");

    // 启动更新线程
    #ifdef _WIN32
        h_update_thread = CreateThread(NULL, 0, update_thread_func, NULL, 0, NULL);
    #else
        pthread_create(&update_thread, NULL, update_thread_func, NULL);
    #endif

    // 主循环：接收指令
    char recv_buf[1024];
    int recv_size;
    while (1) {
        recv_size = recv(client_sock, recv_buf, sizeof(recv_buf) - 1, 0);
        if (recv_size <= 0) {
            printf("Client disconnected.\n");
            break;
        }
        recv_buf[recv_size] = '\0';

        // 解析指令并更新目标值 (加锁)
        #ifdef _WIN32
            EnterCriticalSection(&g_mutex);
        #else
            pthread_mutex_lock(&g_mutex);
        #endif

        if (strncmp(recv_buf, "SET_TORQUE", 10) == 0) {
            double val = atof(recv_buf + 11);
            MotorSimulator_SetTargetTorque(&g_motor, val);
            printf("Set target torque to: %.2f\n", val);
        } else if (strncmp(recv_buf, "SET_POS", 7) == 0) {
            double val = atof(recv_buf + 8);
            MotorSimulator_SetTargetPos(&g_motor, val);
            printf("Set target position to: %.2f\n", val);
        }

        #ifdef _WIN32
            LeaveCriticalSection(&g_mutex);
        #else
            pthread_mutex_unlock(&g_mutex);
        #endif
    }

    // 清理
    g_running = 0;
    #ifdef _WIN32
        WaitForSingleObject(h_update_thread, INFINITE);
        CloseHandle(h_update_thread);
        DeleteCriticalSection(&g_mutex);
        closesocket(client_sock);
        closesocket(server_sock);
        WSACleanup();
    #else
        pthread_join(update_thread, NULL);
        close(client_sock);
        close(server_sock);
    #endif

    return 0;
}
