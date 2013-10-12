#ifndef SOCKET_MANAGER_H
#define SOCKET_MANAGER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

class sock_info_t
{
	private:
		int		    m_listen_fd;
        int         m_cli_fd;
		socklen_t   m_len;
		struct sockaddr_in m_serv_addr;
		struct sockaddr_in m_cli_addr;
	public:
		sock_info_t()
		{
			m_listen_fd = 0;
			m_len = 0;
		};

		inline int get_serv_sock()
		{
			return m_listen_fd;
		}

		inline sockaddr_in get_cli()
		{
			return m_cli_addr;
		}

        inline int get_cli_sock()
        {
            return m_cli_fd;
        }

		virtual void init();   
        virtual bool http_init(char* ip, int port);
};

#endif
