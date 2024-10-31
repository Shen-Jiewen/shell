#ifndef PAL_H
#define PAL_H

// 平台抽象层接口，用于不同平台的硬件抽象
typedef struct {
    void (*init)();                  // 平台初始化函数
    int (*get_char)();               // 从输入中读取一个字符
    void (*uart_send)(const char *str); // 发送字符串到串口
    void (*delay)(int ms);           // 延时函数，单位为毫秒
} PalInterface;

// 获取平台接口的单例指针
PalInterface* get_pal_interface();

#endif // PAL_H
