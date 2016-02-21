/*
 *  childprocess.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 9/10/08.
 *  Copyright 2008 Reykjavik University. All rights reserved.
 *
 */

#ifndef CHILDPROCESS_H
#define CHILDPROCESS_H

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <sys/wait.h>

namespace cadiaplayer{
	namespace utils{
		typedef std::vector<std::string> Arguments;
		
		const std::size_t STD_BUFFER_SIZE = 16384;
		
		class ChildProcess : public std::streambuf
			{
			private:
				std::string		m_program;
				Arguments		m_arguments;
				bool			m_running;
				int				m_pid;
				FILE*			m_infd;
				FILE*			m_outfd;
				int				m_inpipe;
				int				m_outpipe;
				char			m_buffer[STD_BUFFER_SIZE];
			public:
				ChildProcess(std::string program) : m_program(program), m_running(false), m_infd(0), m_outfd(0){};
				void addArgument(std::string arg) {m_arguments.push_back(arg);};
				bool start();
				bool start(boost::asio::io_service& service, boost::asio::ip::tcp::socket& socket);
				void stop();
				bool isRunning(){return m_running;};
				
				bool writeToStdIn(std::string str, bool debug=false);
				std::string readFromStdOut();
				
				static bool is_valid_fd(int fd)
				{
					return fcntl(fd, F_GETFL) != -1 || errno != EBADF;
				};
				static std::string get_errno_string()
				{
					std::string str;
					switch (errno) 
					{
						case EACCES:
							str = "EACCES - Operation is prohibited by locks held by other processes. Or, operation is prohibited because the file has been memory-mapped by another process.";
							break;
						case EAGAIN:
							str = "EAGAIN - Operation is prohibited by locks held by other processes. Or, operation is prohibited because the file has been memory-mapped by another process.";
							break;
						case EBADF:
							str = "EBADF - fd is not an open file descriptor, or the command was F_SETLK or F_SETLKW and the file descriptor open mode doesn't match with the type of lock requested.";
							break;
						case EDEADLK:
							str = "EDEADLK - It was detected that the specified F_SETLKW command would cause a deadlock.";
							break;
						case EFAULT:
							str = "EFAULT - lock is outside your accessible address space.";
							break;
						case EINTR:
							str = "EINTR - For F_SETLKW, the command was interrupted by a signal. For F_GETLK and F_SETLK, the command was interrupted by a signal before the lock was checked or acquired. Most likely when locking a remote file (e.g. locking over NFS), but can sometimes happen locally.";
							break;
						case EINVAL:
							str = "EINVAL - For F_DUPFD, arg is negative or is greater than the maximum allowable value. For F_SETSIG, arg is not an allowable signal number.";
							break;
						case EMFILE:
							str = "EMFILE - For F_DUPFD, the process already has the maximum number of file descriptors open.";
							break;
						case ENOLCK:
							str = "ENOLCK - Too many segment locks open, lock table is full, or a remote locking protocol failed (e.g. locking over NFS).";
							break;
						case EPERM:
							str = "EPERM - Attempted to clear the O_APPEND flag on a file that has the append-only attribute set.";
							break;
							
						default:
							str = "Unknown error or no error (";
							str += errno;
							str += ").";
							break;
					}
					return str;
				};
			};
	}
}
#endif
