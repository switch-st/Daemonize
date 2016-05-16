#ifndef SWITCH_DAEMONIZE_H
#define SWITCH_DAEMONIZE_H


/**
 *  v0.1
 * 欢迎补充！
 **/

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string>
#include <vector>
#include <sstream>
#ifndef DAEMONIZE_NO_BOOST
#include <boost/serialization/singleton.hpp>
#include <boost/noncopyable.hpp>
#endif // DAEMONIZE_NO_BOOST


#ifndef NAMESPACE_SWITCH_TOOL
    #define NAMESPACE_SWITCH_TOOL
    #define OPEN_NAMESPACE_SWITCHTOOL       namespace Switch { \
												namespace Tool {
    #define CLOSE_NAMESPACE_SWITCHTOOL      	}; \
											};
    #define USING_NAMESPACE_SWITCHTOOL      using namespace Switch::Tool;
#endif


OPEN_NAMESPACE_SWITCHTOOL

static std::string strPidFileName = "";
static void DeletePidFile(void)
{
	unlink(strPidFileName.c_str());
}

#ifndef DAEMONIZE_NO_BOOST
class Daemonize : public boost::noncopyable
#else
class Daemonize
#endif // DAEMONIZE_NO_BOOST
{
public:
	typedef void (*SigHandler_t)(int);
	Daemonize(const std::string& sBaseDir = "", const std::string& sPidFile = "")
	{
		m_sBaseDir = sBaseDir;
		m_sPidFile = sPidFile;
		m_nLockFd = -1;
		m_bDaemoned = false;
	}

	std::string& BaseDir(void) { return m_sBaseDir;}
	std::string& PidFile(void) { return m_sPidFile;}

	int SetDefaultBaseDir(void)
	{
		std::stringstream linkPath;
		char exePath[256];
		char* ch;

		memset(exePath, 0, 256);
		linkPath << "/proc/" << getpid() << "/exe";
		if (readlink(linkPath.str().c_str(), exePath, 256) == -1)
		{
			return -1;
		}
		exePath[255] = '\0';
        ch = strrchr(exePath, '/');
        if (ch != NULL)
        {
            *(ch + 1) = '\0';
        }
        else
		{
            return -1;
        }
		m_sBaseDir = exePath;

		return 0;
	}

	int SetDefaultPidFile(void)
	{
        m_sPidFile = "pid.pid";
        return 0;
	}

	/// 0 -> not exist
	/// -1 -> exist
	int CheckPidFile(void)
	{
		pid_t pid;
		if (GetPidFromFile(pid) == -1)
		{
			return 0;
		}
		if (kill(pid, 0) == 0)
		{
			return -1;
		}
		if (errno == EPERM)
		{
			return -1;
		}

		return 0;
	}

	int WritePidFile(void)
	{
		int fd, ret;
		std::string sFileName = m_sBaseDir + "/" + m_sPidFile;

		if (CreateDir(sFileName.c_str()) == -1)
		{
            return -1;
		}
		fd = open(sFileName.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0600);
		if (fd == -1)
		{
			return -1;
		}
		std::stringstream sPid;
		sPid << getpid();
		ret = write(fd, sPid.str().c_str(), strlen(sPid.str().c_str()));
		close(fd);
		Switch::Tool::strPidFileName = sFileName;
		atexit(&DeletePidFile);

		return ret >= 0 ? 0 : -1;
	}

	int SetDaemonize(void)
	{
        pid_t pid;
        int fd;

		if (m_bDaemoned)
		{
			return -1;
		}
        if ((pid = fork()) < 0)
        {
            return -1;
        }
        else if (pid != 0)
        {
            exit(0);
        }
        setsid();
        int ret = chdir("/");
        umask(0);
        fd = open("/dev/null", O_RDWR);
        dup2(fd, 0);
        dup2(fd, 1);
        dup2(fd, 2);
        close(fd);
        m_bDaemoned = true;

        return ret;
	}

	int RegistSigHandler(const int sig, const SigHandler_t& hand)
	{
		m_vSigHandlers.push_back(std::make_pair(sig, hand));
		return 0;
	}

	int RegistSigHandler(const std::vector< int >& vSigs, const std::vector< SigHandler_t >& vHands)
	{
		if (vSigs.size() != vHands.size())
		{
			return -1;
		}
		for (size_t i = 0; i < vHands.size(); ++i)
		{
			m_vSigHandlers.push_back(std::make_pair(vSigs[i], vHands[i]));
		}
		return 0;
	}

	int SetSigHandler(void)
    {
        struct sigaction sa;

        for (size_t i = 0; i < m_vSigHandlers.size(); ++i)
		{
			sa.sa_handler = m_vSigHandlers[i].second;
			sigemptyset(&sa.sa_mask);
			sa.sa_flags = 0;
            sigaction(m_vSigHandlers[i].first, &sa, NULL);
		}

		return 0;
	}

private:
	int GetPidFromFile(pid_t& pid)
	{
		int fd;
		char buf[8];
		std::string sFileName = m_sBaseDir + "/" + m_sPidFile;

		fd = open(sFileName.c_str(), O_RDONLY);
		if (fd == -1)
		{
			return -1;
		}
		memset(buf, 0, 8);
		if (read(fd, buf, 8) == -1)
		{
			close(fd);
            return -1;
		}
		close(fd);
		buf[7] = '\0';
		std::stringstream convert;
		convert << buf;
		convert >> pid;
		if (pid <= 0)
		{
			return -1;
		}

		return 0;
	}

    int CreateDir(const char *path)
    {
        int i, len;

        len = strlen(path);
        char dir_path[len+1];
        dir_path[len] = '\0';

        strncpy(dir_path, path, len);

        for (i=0; i<len; i++)
        {
            if (dir_path[i] == '/' && i > 0)
            {
                dir_path[i]='\0';
                if (access(dir_path, F_OK) < 0)
                {
                    if (mkdir(dir_path, 0755) < 0)
                    {
                        return -1;
                    }
                }
                dir_path[i]='/';
            }
        }

        return 0;
    }

private:
	std::string						m_sBaseDir;
	std::string						m_sPidFile;
	int								m_nLockFd;
	bool							m_bDaemoned;
	std::vector< std::pair< int, SigHandler_t > >			m_vSigHandlers;
};

#ifndef DAEMONIZE_NO_BOOST
class DaemonizeSingleton : public Daemonize, public boost::serialization::singleton< DaemonizeSingleton >
{
protected:
	DaemonizeSingleton(void) {}
	~DaemonizeSingleton(void) {}
};
#endif // DAEMONIZE_NO_BOOST


CLOSE_NAMESPACE_SWITCHTOOL


#endif // SWITCH_DAEMONIZE_H
