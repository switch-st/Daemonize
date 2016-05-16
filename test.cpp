#include <iostream>
#include "Daemonize.hpp"

using namespace std;
USING_NAMESPACE_SWITCHTOOL

void hand1(int)
{
    cout << "==INT" << endl;
}
void hand2(int)
{
    cout << "==TERM" << endl;
}
void hand3(int)
{
    cout << "==QUIT" << endl;
}

int main(void)
{
#ifndef DAEMONIZE_NO_BOOST
	DaemonizeSingleton& daemon = DaemonizeSingleton::get_mutable_instance();
#else
	Daemonize daemon;
#endif // DAEMONIZE_NO_BOOST

	daemon.BaseDir() = "/tmp/aa/bb/cc";
	daemon.PidFile() = "test.pid";

	//daemon.SetDefaultBaseDir();
	//daemon.SetDefaultPidFile();

	cout << "SetDaemonize = " << daemon.SetDaemonize() << endl;

	cout << "CheckPidFile = " << daemon.CheckPidFile() << endl;
	cout << "WritePidFile = " << daemon.WritePidFile() << endl;

	daemon.RegistSigHandler(SIGINT, hand1);
	daemon.RegistSigHandler(SIGTERM, hand2);
	daemon.RegistSigHandler(SIGQUIT, hand3);
	daemon.SetSigHandler();

	while (1)
	{
		sleep(60);
		break;
	}

}

