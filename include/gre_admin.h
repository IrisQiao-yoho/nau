#ifndef _GRE_ADMIN_H__
#define	_GRE_ADMIN_H__

// 引入标准库：输入输出、文件操作、字符串处理等基础功能
#include <iostream>         // 标准输入输出（打印日志、错误信息）
#include <fstream>          // 文件流（读取配置文件）
#include <string>           // 字符串处理
#include <vector>           // 动态数组（存储对端设备列表）
#include <chrono>           // 时间相关（定时检查）
#include <thread>           // 线程休眠（周期等待）
#include <cstdio>           // C风格文件操作（执行命令并获取输出）
#include <cstdlib>          // 系统命令执行（system/popen等）
#include <stdexcept>        // 异常处理（抛出错误信息）
#include <sstream>          // 字符串流（解析命令输出）
#include <algorithm>        // 算法库（min/max函数，用于排序设备编号）
#include <map>              // 映射容器（存储设备时间戳）
#include <nlohmann/json.hpp>// 引入JSON解析库：解析配置文件
// // 引入MongoDB C++驱动：用于连接数据库并获取对端设备信息
// #include <mongocxx/client.hpp>        // MongoDB客户端
// #include <mongocxx/instance.hpp>      // MongoDB全局实例（初始化驱动）
// #include <mongocxx/uri.hpp>           // MongoDB连接地址解析
// #include <bsoncxx/json.hpp>           // BSON与JSON格式转换
// #include <bsoncxx/builder/stream/document.hpp>  // 构建BSON查询语句
#endif