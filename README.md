Daemonize
================

地址
https://github.com/switch-st/Daemonize.git

说明
 * 
 * linux环境下设置后台进程库。提供设置后台服务，设置pid文件，注册信号处理函数等功能。
 * 依赖stl、boost
 * 
 * std::string& BaseDir(void) ： 获取/设置基准目录(不是运行目录)
 * std::string& PidFile(void) ： 获取/设置pid文件名(pid文件会在基准目录下创建)
 * int SetDefaultBaseDir(void) ： 设置默认基准目录(可执行程序所在目录)
 * int SetDefaultPidFile(void) ： 设置默认pid文件名
 * int CheckPidFile(void) ： 检查pid文件是否存在(0：不存在，非0：存在)
 * int WritePidFile(void) ： 写入pid文件
 * int SetDaemonize(void) ： 设置后台服务
 * int RegistSigHandler(const int sig, const SigHandler_t& hand) / int RegistSigHandler(const std::vector< int >& vSigs, const std::vector< SigHandler_t >& vHands) ： 注册信号处理函数
 * int SetSigHandler(void) ： 设置信号处理函数
 * 
 * class Daemonize ： 基础类
 * class DaemonizeSingleton ： 单例类
 * 
 * 欢迎补充！

调用
	调用方式详见测试文件test.cpp。

			by switch
			switch.st@gmail.com

