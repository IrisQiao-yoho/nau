#include <gre_admin.h>
#include <nau_main.h>

using namespace std;

int check_interval = 10;
// 配置参数结构体：集中管理所有固定配置和动态读取的参数
/*
struct Config {
    const string config_path = "/home/NAU_config.json";  // 固定配置文件路径
    // 本地
    string sat_proto;                                    // 协议版本
    string local_id;                                     // 本地设备ID
    string down_if;                                      // 下挂网口（流量入口）
    string up_if;                                        // 上行网口（流量出口）
    string up_ip;                                        // 本端上行网口IP（本端用于建立隧道的IP）
    string down_ip;                                      // 本端下挂网口IP（本端用于策略路由）
    string mng_ip;                                       // 网联网关IP

    int check_interval = 10;                              // 配置检查间隔（秒）
};*/

/*
// 重载 << 运算符
ostream& operator<<(ostream& os, const Config& cfg) {
    os << "{" << endl
       << "  协议版本: " << cfg.sat_proto << "," << endl
       << "  本地ID: " << cfg.local_id << "," << endl
       << "  下挂网口: " << cfg.down_if << " (IP: " << cfg.down_ip << ")," << endl
       << "  上行网口: " << cfg.up_if << " (IP: " << cfg.up_ip << ")," << endl
       << "  网联网关IP: " << cfg.mng_ip << "," << endl
       << "  检查间隔: " << cfg.check_interval << "秒" << endl
       << "}";
    return os;
}*/


// 重载 << 运算符
ostream& operator<<(ostream& os, const Unit& unit) {
    os << "{" << endl
       << "  协议版本: " << unit.sat_protocol << "," << endl
       << "  对端ID: " << unit.NAU_ID << "," << endl
       << "  对端用户IP: " << unit.UIDIP << " (MAC: " << unit.UIDMAC << ")," << endl
       << "  上行网口IPv4: " << unit.IFUP4 << "," << endl
       << "  上行网口IPv6: " << unit.IFUP4 << "," << endl
       << "  创建时间: " << unit.CTIME << endl
       << "  更新时间: " << unit.UTIME << endl
       << "}";
    return os;
}

/**
 * 执行系统命令并返回执行结果
 * cmd 要执行的命令字符串
 * output 可选参数，用于存储命令输出
 * return 命令是否执行成功（true=成功）
 */
bool run_command(const string& cmd, string* output = nullptr) {
    cout << "[CMD] " << cmd << endl;  // 打印执行的命令（日志）
    // 执行命令并获取输出管道（"r"表示读取命令输出）
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {  // 管道创建失败（命令执行失败）
        cerr << "命令执行失败: " << cmd << endl;
        return false;
    }
    char buffer[1024] = {0};  // 读取命令输出的缓冲区
    string result;            // 存储命令输出结果
    // 循环读取命令输出，直到结束
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;  // 累加输出内容
    }
    int ret = pclose(pipe);  // 关闭管道并获取命令返回值（0表示成功）
    if (output) {            // 若需要保存输出，写入output
        *output = result;
    }
    return ret == 0;  // 返回命令是否成功执行
}

/**
 * 读取配置文件，提取sat_proto（协议版本）和本地NAU_ID
 * config_path 配置文件路径
 * sat_proto 输出参数，存储协议版本（4=IPv4，6=IPv6）
 * local_id 输出参数，存储本地设备ID
 */
/*void read_config(Config& config) {
    // 打开配置文件
    ifstream file(config.config_path);
    if (!file.is_open()) {  // 打开失败则抛出异常
        throw runtime_error("无法打开配置文件: " + config.config_path);
    }

    nlohmann::json j;  // 创建JSON对象
    file >> j;         // 从文件中读取JSON数据

    // 读取sat_proto
    config.sat_proto = j.value("sat_protocol", "");
    // 读取本地NAU_ID
    config.local_id = j.value("NAU_ID", "");
    // 读取网联网关地址
    config.mng_ip = j.value("MNG_ip", "");
    // 读取上下行网口名及地址
    const auto& down = j["if_down"][0];
    const auto& up = j["if_up"][0];
    config.down_ip = down.value("address","");
    config.down_if = down.value("iface","");
    config.up_if = up.value("iface","");
    config.up_ip = up.value("address","");
}*/

/**
 * 清理所有本程序创建的GRE隧道（仅删除greXY格式的隧道）
 */
void clean_all_gre_tunnels() {
    string output;  // 存储命令输出
    // 执行命令：获取所有带@NONE后缀的gre隧道（如gre12@NONE）
    if (!run_command("ip link show | grep 'gre[0-50][0-50]'", &output)) {
        cerr << "获取GRE隧道列表失败" << endl;
        return;
    }

    // 将输出转换为字符串流，便于逐行处理
    istringstream iss(output);
    string line;
    // 逐行解析命令输出
    while (getline(iss, line)) {
        // 查找第一个冒号（格式如"14: gre12@NONE: <POINTOPOINT..."）
        size_t colon_pos = line.find(':');
        if (colon_pos == string::npos) {  // 未找到冒号，跳过
            continue;
        }

        // 查找冒号后的第一个空格（分隔隧道名称）
        size_t space_pos = line.find(' ', colon_pos + 2);
        if (space_pos == string::npos) {  // 未找到空格，跳过
            continue;
        }

        // 提取带@NONE的完整名称（如"gre12@NONE"）
        string full_iface = line.substr(colon_pos + 1, space_pos - colon_pos - 1);

        // 从完整名称中提取核心隧道名（去掉@NONE后缀）
        size_t at_pos = full_iface.find('@');  // 查找@的位置
        if (at_pos == string::npos) {     // 没有@后缀，跳过（异常情况）
            cout << "5" << endl;
            continue;
        }
        string core_iface = full_iface.substr(0, at_pos);  // 截取@之前的部分（如"gre12"）
        cout << "删除隧道名: " << core_iface << endl;
        // 删除
        run_command("ip link del " + core_iface);
    }
}

/**
 * 清理某一个本程序创建的GRE隧道（仅删除greXY格式的隧道）
 */
void clean_gre_tunnels(const Unit& remote_unit,const Unit& local_unit) {
    // 生成隧道名称：greXY（X、Y为编号，小号在前）
    int remote_id = stoi(remote_unit.NAU_ID);
    int local_id = stoi(local_unit.NAU_ID);
    int x = min(local_id, remote_id);  // 较小的编号
    int y = max(local_id, remote_id);  // 较大的编号
    string tunnel_name = "gre" + to_string(x) + to_string(y);
    // 删除
    run_command("ip link del " + tunnel_name);
}

/**
 * 创建单个GRE隧道（按规则命名和分配IP）
 * unit 对端设备信息
 * config 本端设备信息
 */
void create_gre_tunnel(const Unit& remote_unit, const Unit& local_unit) {
    // 1. 生成隧道名称：greXY（X、Y为编号，小号在前）
    int remote_id = stoi(remote_unit.NAU_ID);
    int local_id = stoi(local_unit.NAU_ID);
    int x = min(local_id, remote_id);  // 较小的编号
    int y = max(local_id, remote_id);  // 较大的编号
    string tunnel_name = "gre" + to_string(x) + to_string(y);
 
    // 执行命令检查隧道是否存在，若存在则删除
    string check_cmd = "ip link show " + tunnel_name + " >/dev/null 2>&1";
    int ret = system(check_cmd.c_str());
    if (ret == 0) {  // 隧道存在（命令返回0表示成功找到）
        cout << "隧道 " << tunnel_name << " 已存在，先删除旧隧道..." << endl;
        run_command("ip link del " + tunnel_name);  // 删除旧隧道
    }

    // 2. 生成隧道IP地址（根据协议版本）
    string tunnel_ip4;
    string tunnel_ip6;
    // IPv4地址格式：X.Y.0.Z/16（Z为本地编号）
    tunnel_ip4 = to_string(x) + "." + to_string(y) + ".0." + local_unit.NAU_ID + "/16";
    // IPv6地址格式：fd00:XY::Z/64（XY为编号组合，Z为本地编号）
    tunnel_ip6 = to_string(x) + ":" + to_string(y) + "::" + local_unit.NAU_ID + "/64";

    // 3. 构建隧道创建命令（区分IPv4和IPv6）
    string cmd;
    string local_ip;
    if (stoi(local_unit.sat_protocol) == 4)
    {
        local_ip = (local_unit.IFUP4.find('/') != string::npos) ? local_unit.IFUP4.substr(0, local_unit.IFUP4.find('/')) : local_unit.IFUP4;
    }
    else
    {
        local_ip = (local_unit.IFUP6.find('/') != string::npos) ? local_unit.IFUP6.substr(0, local_unit.IFUP6.find('/')) : local_unit.IFUP6;
    }
    if (stoi(local_unit.sat_protocol) == 4) {
        // IPv4 GRE隧道创建命令
        string remote_ip = (remote_unit.IFUP4.find('/') != string::npos) ? remote_unit.IFUP4.substr(0, remote_unit.IFUP4.find('/')) :remote_unit.IFUP4;
        cmd = "ip tunnel add " + tunnel_name + 
              " mode gre local " + local_ip +  // 本地端点IP
              " remote " + remote_ip +          // 对端端点IP
              " ttl 255";                         // TTL设为255（避免中途被丢弃）
    } else {
        // IPv6 GRE隧道创建命令（使用ip6gre模式）
        string remote_ip = (remote_unit.IFUP6.find('/') != string::npos) ? remote_unit.IFUP6.substr(0, remote_unit.IFUP6.find('/')) : remote_unit.IFUP6;
        cmd = "ip -6 tunnel add " + tunnel_name + 
              " mode ip6gre local " + local_ip + 
              " remote " + remote_ip + 
              " ttl 255";
    }

    // 执行隧道创建命令
    if (!run_command(cmd)) {
        cerr << "创建隧道 " << tunnel_name << " 失败" << endl;
        return;
    }

    // 启动隧道（新创建的隧道默认是关闭的）
    run_command("ip link set " + tunnel_name + " up");
    // 为隧道配置IP地址
    run_command("ip addr add " + tunnel_ip4 + " dev " + tunnel_name);
    run_command("ip addr add " + tunnel_ip6 + " dev " + tunnel_name);
}

/**
 * 从 rt_tables 文件中获取所有有效表编号，并返回最大编号
 * return 最大表编号（默认返回 99，如果无有效表则返回 0）
 */
uint32_t get_max_table_number() {
    ifstream file("/etc/iproute2/rt_tables");
    if (!file.is_open()) {
        throw runtime_error("无法打开文件: /etc/iproute2/rt_tables（可能需要root权限）");
    }

    uint32_t max_num = 0;
    string line;

    while (getline(file, line)) {
        // 忽略空行和注释行
        if (line.empty()) continue;
        size_t first_non_space = line.find_first_not_of(" \t");
        if (first_non_space != string::npos && line[first_non_space] == '#') {
            continue;
        }

        // 提取表编号
        size_t last_non_space = line.find_last_not_of(" \t");
        if (last_non_space == string::npos) continue;
        string trimmed_line = line.substr(0, last_non_space + 1);
        size_t split_pos = trimmed_line.find_first_of(" \t");
        if (split_pos == string::npos) continue;

        // 转换为数字并更新最大值（系统预留表 253-255 忽略）
        try {
            uint32_t num = stoul(trimmed_line.substr(0, split_pos));
            if (num < 253) {  // 有效表编号范围 1-252
                max_num = max(max_num, num);
            }
        } catch (...) {
            continue;  // 非数字编号，忽略
        }
    }

    // 默认从 100 开始（如果没有有效表）
    return (max_num == 0) ? 99 : max_num;  // +1 后为 100
}

/**
 * 创建立策略路由表
 * 若已存在则不变
 * 若不存在则创立
 */
void create_gre_table(const Unit& remote_unit, const Unit& local_unit) {
    // 1. 生成路由表名称（如 GRE23_RT，取local_id和NAU_ID的大小排序）
    int remote_id = stoi(remote_unit.NAU_ID);
    int local_id = stoi(local_unit.NAU_ID);
    int x = min(local_id, remote_id);
    int y = max(local_id, remote_id);
    string table_name = "GRE" + to_string(x) + to_string(y) + "_RT";

    // 2. 检查表是否已存在
    ifstream in_file("/etc/iproute2/rt_tables");
    if (!in_file.is_open()) {
        cerr << "错误：无法打开文件 /etc/iproute2/rt_tables（需root权限）" << endl;
        return;
    }

    bool exists = false;
    string line;
    while (getline(in_file, line)) {
        // 跳过空行和注释行
        if (line.empty()) continue;
        size_t first_non_space = line.find_first_not_of(" \t");
        if (first_non_space != string::npos && line[first_non_space] == '#') {
            continue;
        }

        // 提取表名并比较
        size_t last_non_space = line.find_last_not_of(" \t");
        if (last_non_space == string::npos) continue;
        string trimmed_line = line.substr(0, last_non_space + 1);
        size_t split_pos = trimmed_line.find_first_of(" \t");
        if (split_pos == string::npos) continue;

        string name = trimmed_line.substr(split_pos);
        size_t name_start = name.find_first_not_of(" \t");
        if (name_start == string::npos) continue;
        name = name.substr(name_start);

        if (name == table_name) {
            exists = true;
            break;
        }
    }
    in_file.close();  // 关闭输入流，准备后续写入

    // 3. 若已存在，直接返回
    if (exists) {
        cout << "路由表 " << table_name << " 已存在，无需创建" << endl;
        return;
    }

    // 4. 若不存在，计算新表编号（最大编号 +1）
    uint32_t new_table_num;
    try {
        new_table_num = get_max_table_number() + 1;
        if (new_table_num >= 253) {  // 超过最大有效编号（1-252）
            throw runtime_error("路由表编号已达上限（252）");
        }
    } catch (const exception& e) {
        cerr << "错误：无法计算新表编号 - " << e.what() << endl;
        return;
    }

    // 5. 写入新表项到 rt_tables（追加到文件末尾）
    ofstream out_file("/etc/iproute2/rt_tables", ios::app);  // 追加模式
    if (!out_file.is_open()) {
        cerr << "错误：无法写入文件 /etc/iproute2/rt_tables（需root权限）" << endl;
        return;
    }

    // 写入格式：[编号] [表名]（加注释说明）
    out_file << new_table_num << "\t" << table_name << endl;  // 制表符分隔

    if (out_file.good()) {
        cout << "成功创建路由表：" << new_table_num << " " << table_name << endl;
    } else {
        cerr << "错误：写入路由表失败" << endl;
    }
    out_file.close();
}

/**
 * 配置策略路由表规则
 * 默认路由走特定gre
 * 来自{下挂口}的{v4用户目的地址}进入对应{源接入单元ID，目的接入单元ID}的gre隧道
 * 来自{下挂口}的{v6用户目的地址}进入对应{源接入单元ID，目的接入单元ID}的gre隧道
 * 下挂路由
 */
void config_gre_table(const Unit& remote_unit, const Unit& local_unit) {
    // 隧道名称+路由表名称
    int remote_id = stoi(remote_unit.NAU_ID);
    int local_id = stoi(local_unit.NAU_ID);
    int x = min(local_id, remote_id);
    int y = max(local_id, remote_id);
    string tunnel_name = "gre" + to_string(x) + to_string(y);
    string table_name = "GRE" + to_string(x) + to_string(y) + "_RT";

    // 本地下挂的ipv4、ipv6地址
    string local_down_ip4;
    string local_down_ip6;
    if (local_unit.UIDIP.find(':') != string::npos){
        size_t lpos1 = local_unit.UIDIP.find(':');
        size_t lpos2 = local_unit.UIDIP.find('/');
        int ipv6_mask = stoi(local_unit.UIDIP.substr(lpos2 + 1));
        local_down_ip4 = local_unit.UIDIP.substr(lpos1+6, lpos2 - (lpos1 + 5)) + to_string(32 - (128 - ipv6_mask));
        local_down_ip6 = local_unit.UIDIP;
    }else{
        local_down_ip4 = local_unit.UIDIP;
        size_t lpos = local_unit.UIDIP.find('/');
        int ipv4_mask = stoi(local_unit.UIDIP.substr(lpos+1));
        local_down_ip6 = "2001:db9::" + local_unit.UIDIP.substr(0, lpos+1) + to_string(128 - (32 - ipv4_mask));
    }
    cout << "local_down_ip4:" << local_down_ip4 << endl;
    cout << "local_down_ip6:" << local_down_ip6 << endl;
    // 对端下挂的ipv4、ipv6地址
    string remote_down_ip4;
    string remote_down_ip6;

    auto clean = [](std::string s) {
    if (s.size() >= 3 &&
        (unsigned char)s[0] == 0xEF &&
        (unsigned char)s[1] == 0xBB &&
        (unsigned char)s[2] == 0xBF) {
        return s.substr(3);
    }
    return s;
};

    std::string uid = clean(remote_unit.UIDIP);
    if (remote_unit.UIDIP.find(':') != string::npos){
        size_t rpos1 = uid.find(':');
        size_t rpos2 = uid.find('/');
        int ipv6_mask = stoi(uid.substr(rpos2 + 1));
        remote_down_ip4 = uid.substr(rpos1+6, rpos2 - (rpos1 + 5)) + to_string(32 - (128 - ipv6_mask));
        remote_down_ip6 = uid;
    }else{
        remote_down_ip4 = uid;
        size_t rpos = uid.find('/');
        int ipv4_mask = stoi(uid.substr(rpos + 1));
        remote_down_ip6 = "2001:db9::" + uid.substr(0, rpos+1) + to_string(128 - (32 - ipv4_mask));
    }
    cout << "remote_down_ip4:" << remote_down_ip4 << endl;
    cout << "remote_down_ip6:" << remote_down_ip6 << endl;
    // 删除旧规则，避免重复
    run_command("ip rule del lookup " + table_name + " 2>/dev/null || true");
    run_command("ip -6 rule del lookup " + table_name + " 2>/dev/null || true");

    // 创建新规则
    run_command("ip rule add from " + local_down_ip4 + " to " + remote_down_ip4 + " table " +table_name);
    run_command("ip -6 rule add from " + local_down_ip6 + " to " + remote_down_ip6 + " table " +table_name);

    // 清空旧路由表
    run_command("ip route flush table " + table_name + " || true");
    run_command("ip -6 route flush table " + table_name + " || true");

    // 添加默认路由
    string via_ip4 = to_string(x) + "." + to_string(y) + ".0." + remote_unit.NAU_ID;
    string via_ip6 = to_string(x) + ":" + to_string(y) + "::" + remote_unit.NAU_ID;
    run_command("ip route add default via " + via_ip4 + " dev " + tunnel_name + " table " + table_name);
    run_command("ip -6 route add default via " + via_ip6 + " dev " + tunnel_name + " table " + table_name);

    // 添加下挂路由
    run_command("ip route add " + local_down_ip4 + " dev " + "enp3" + " table " +table_name);
    run_command("ip -6 route add " + local_down_ip6 + " dev " + "enp3" + " table " +table_name);

    //加了几条策略路由，限制只有下行流量进行协议转换，上行流量则不做处理
    //先添加下行流量处理程序
    run_command("iptables -t mangle -A PREROUTING -i " + tunnel_name + " -j JOOL_SIIT --instance " + "\"example\"");
    run_command("ip6tables -t mangle -A PREROUTING -i " + tunnel_name + " -j JOOL_SIIT --instance " + "\"example\"");
    //再添加上行流量放行程序
    run_command("iptables -t mangle -A PREROUTING -i enp0 -j ACCEPT");
    run_command("ip6tables -t mangle -A PREROUTING -i enp0 -j ACCEPT");
    // 完成
    cout << "成功创建路由表：" << table_name << "规则配置" << endl;
}

// 新增设备检测子函数：传入新设备列表、历史MAC映射，返回新增设备
void check_new_units(const vector<Unit>& units, const map<string, bool>& old_macs, vector<Unit>& new_units)  {
    for (const auto& u : units) {
        if (!old_macs.count(u.UIDMAC)) {
            new_units.push_back(u);
            cout << "新增设备 - IP: " << u.UIDIP << ", ID: " << u.NAU_ID << endl;
        }
    }
}

// 更新时间映射，程序里保存的应该是旧的映射，每次读取完结构体以后进行映射更新
void time_map_update(map<string, string> &unit_time_map, vector<Unit> &units) {
    for(const auto& unit : units) {
        unit_time_map[unit.UIDIP] = unit.UTIME;
    }
}

// 新旧更新时间对比
void compare_all_units(vector<Unit> &units, map<string, string> &unit_time_map, vector<Unit> &changed_units) {
    for (auto &pair : unit_time_map) {
        const string &uid = pair.first;          // map 中的 UIDIP
        string &saved_timestamp = pair.second;   // map 中的历史时间戳

        // 在 units 中查找对应的 Unit
        auto it_unit = find_if(units.begin(), units.end(),
                                    [&uid](const Unit &u) { return u.UIDIP == uid; });

        if (it_unit != units.end()) {
            // 找到对应的 Unit
            if (it_unit->UTIME == saved_timestamp) {
                cout << "No change for UIDIP: " << uid << endl;
            } else {
                Unit changed_unit = *it_unit;//找到变化的设备，保存其信息
                changed_units.push_back(changed_unit);
                cout << "Time changed for UIDIP: " << uid << endl;
                // 更新 map 中的时间戳
                saved_timestamp = it_unit->UTIME;
            }
        } else {
            cout << "UIDIP not found in units vector: " << uid << endl;
        }
    }
}

Unit find_subnet_fromLocalMap(vector<Unit> tms)
{
    for (const auto& tm : tms) {
    if (tm.UIDMAC.empty()) {
        return tm;
    }
}
    return Unit{};
}

vector<Unit> find_subnet_fromRemoteMap(vector<Unit> tms)
{
    vector<Unit> remote_units;
    for (const auto& tm : tms) {
    if (tm.UIDMAC.empty()) {
        remote_units.push_back(tm);
    }
}
     return remote_units;
}
/**
 * 主函数：周期检查配置，按需创建/更新隧道和路由
 */
int main() {
    char remote_file_path[256] = "/home/remote_test_gre.json";//对端表位置--一会儿记得改成新的
    //std::string rm_remote_file = "rm " + std::string(remote_file_path);
    char local_file_path[256] = "/home/local_test_gre.json";//本地表位置，这都是新改的
    //std::string rm_local_file = "rm " + std::string(local_file_path);
    vector<Unit> local_units;           // 存储解析后的本地Unit结构体
    vector<Unit> remote_units;          // 存储解析后的对端Unit结构体
    Unit local_unit;                    //代替原有的config存储本地的信息（主要是接入单元ID和下挂网段）
    vector<Unit> remote_unit;                   //存储对端的信息
    map<string, string> unit_time_map;//存储终端标识和时间戳的映射
    vector<Unit> changed_units;//存储发生了变化的unit信息，数据库的行
    vector<Unit> new_units;
    map<string, bool> old_macs;//上一次的所有对端
    int last_size = 0;

    //完成初始化，第一次读取数据 把本地数据保存起来 还得考虑里面找到那个MAC是空的
    //run_command(rm_local_file);
    //run_command(rm_remote_file);
    local_units = nau_json_tm_load_gre(local_file_path);
    local_unit = find_subnet_fromLocalMap(local_units);
    remote_units = nau_json_tm_load_gre(remote_file_path);
    remote_unit = find_subnet_fromRemoteMap(remote_units);

    time_map_update(unit_time_map, remote_unit);

    for (const auto& u : remote_unit) old_macs[u.UIDMAC] = true;//这应该是对端吧？这得问一下
    last_size = remote_units.size();
    cout << "初始设备数: " << last_size << endl;

    int last_proto = -1;               // 记录上一次的协议版本（初始值无效）
    string last_local_up_ip;     // 记录上一次的本地设备上行ip
    string last_local_down_ip;     // 记录上一次的本地设备下行ip
    int last_down_proto = -1;

    // 启动日志：显示配置文件路径和检查间隔
    cout << "GRE隧道管理器启动" << endl;
    cout << "本地表文件路径: " << local_file_path << endl;
    cout << "检查间隔: " << check_interval << "秒" << endl;

    // 无限循环：周期检查配置并更新隧道
    while (true) {
        try {
            // 1. 读取当前配置（协议版本和本地设备ID）
            local_units = nau_json_tm_load_gre(local_file_path);
            local_unit = find_subnet_fromLocalMap(local_units);

            // 1.5. 开启JOOL
            if (local_unit.UIDIP.find(':') != string::npos){
                if (last_down_proto == 4 || last_down_proto == -1){
                   run_command("/home/stop_NAT64.sh");
                   run_command("/home/6_start_up_NAT64.sh"); 
                   last_down_proto = 6;
                }
            }
            else{
                if (last_down_proto == 6 || last_down_proto == -1)
                {
                    run_command("/home/stop_NAT64.sh");
                    run_command("/home/4_start_up_NAT64.sh");
                    last_down_proto = 4;
                }
            }

            // 2. 从MongoDB获取对端设备列表
            remote_units.clear();//清空结构体数组，准备重新读取

            //一会儿记得改成新的
            remote_units = nau_json_tm_load_gre(remote_file_path);
            remote_unit = find_subnet_fromRemoteMap(remote_units);

            // 3. 若协议版本或本地设备IP变化，需要重建隧道//这里暂时用v4地址代替原来的了！！！！
            if (stoi(local_unit.sat_protocol) != last_proto || local_unit.IFUP4!= last_local_up_ip || local_unit.UIDIP != last_local_down_ip) {
                cout << "[INFO] 配置发生变化，开始重建隧道..." << endl;

                // 第一步：清理所有旧隧道
                clean_all_gre_tunnels();

                // 第二步：为每个对端设备创建隧道＋配置策略路由
                cout << "[INFO] 发现" << remote_unit.size() << "个对端设备，开始创建隧道..." << endl;
                for (const auto& unit : remote_unit) {
                    // 创建隧道
                    create_gre_tunnel(unit, local_unit);
                    // 建立策略路由表
                    create_gre_table(unit, local_unit);
                    // 配置规则
                    config_gre_table(unit, local_unit);
                }
                // 更新上次配置
                last_proto = stoi(local_unit.sat_protocol);
                last_local_up_ip = local_unit.IFUP4;//这里也是说，全用了v4地址，得再观察观察
                last_local_down_ip = local_unit.UIDIP;
                continue;
            } 
            
            // 4. 若新增某个对端，新建特定隧道
            if (remote_unit.size() > last_size) check_new_units(remote_unit, old_macs, new_units);  // 调用子函数
            for (auto &unit : new_units) {
                    // 创建隧道
                    create_gre_tunnel(unit, local_unit);
                    // 建立策略路由表
                    create_gre_table(unit, local_unit);
                    // 配置规则
                    config_gre_table(unit, local_unit);
            }
            // 更新历史数据
            old_macs.clear();
            new_units.clear();
            for (const auto& u : remote_unit) old_macs[u.UIDMAC] = true;
            cout << "上次设备数: " << last_size << "当前设备数: " <<  remote_unit.size() << endl;
            last_size = remote_unit.size();

            // 5. 若某个的更新时间变化，重建特定隧道
            compare_all_units(remote_unit, unit_time_map, changed_units);//读新的，和旧的比
            for (auto &unit : changed_units) {
                    // 创建隧道
                    create_gre_tunnel(unit, local_unit);
                    // 建立策略路由表
                    create_gre_table(unit, local_unit);
                    // 配置规则
                    config_gre_table(unit, local_unit);
            }
            unit_time_map.clear();
            changed_units.clear();
            time_map_update(unit_time_map, remote_unit);//更新旧的，等待下一次
            // 更新上次配置
            last_proto = stoi(local_unit.sat_protocol);
            last_local_up_ip = local_unit.IFUP4;//又一处，都用v4地址的，也是要观察效果
            last_local_down_ip = local_unit.UIDIP;

        } catch (const exception& e) {  // 捕获所有异常并打印错误信息
            cerr << "[ERROR] " << e.what() << endl;
        }

        // 等待下一个检查周期（暂停指定秒数）
        this_thread::sleep_for(chrono::seconds(check_interval));
    }

    return 0;  // 理论上不会执行到这里（无限循环）
}
