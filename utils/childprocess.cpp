/*
 *  childprocess.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 9/10/08.
 *  Copyright 2008 Reykjavik University. All rights reserved.
 *
 */

#include "childprocess.h"
#include <iostream>
#include <sstream>
#include <errno.h>

using namespace cadiaplayer::utils;
using namespace std;

bool ChildProcess::start(boost::asio::io_service& service, boost::asio::ip::tcp::socket& socket)
{
	cerr << "Child Process starting..." << endl;
	int pfd1[2];
	int pfd2[2];
	
	if(pipe(pfd2) == -1 || pipe(pfd1) == -1)
  	{
  		std::cerr << "Pipes not created" << std::endl;
		std::cerr << get_errno_string() <<  std::endl;
		
		return false;
  	}
	if((m_pid = fork()) < 0)
	{
		std::cerr << "Unable to fork" << std::endl;
		return false;
	}
	m_running = true;
	if(!m_pid)
	{
		cerr << "...new process forked..." << endl;
		socket.close();
		service.stop();
		
		close(pfd1[0]);
		close(pfd2[1]);
		
		// Connect the stdin and stdout to my own pipe fd.
		if(pfd1[0] != STDIN_FILENO)
		{
			close(STDIN_FILENO);
			if(dup2(pfd2[0], STDIN_FILENO) != STDIN_FILENO)
			{
				std::cerr << "StdIn failed to connect." << std::endl;
				exit(-1);
			}
		}
		if(pfd2[1] != STDOUT_FILENO)
		{
			close(STDOUT_FILENO);
			if(dup2(pfd1[1], STDOUT_FILENO) != STDOUT_FILENO)
			{
				std::cerr << "StdOut failed to connect." << std::endl;
				exit(-1);
			}
		}
		// Execute the child process program
		if(m_arguments.size())
		{
			std::stringstream args;
			for(std::size_t n = 0 ; n < m_arguments.size() ; n++)
			{
				args << " " << m_arguments[n];
			}
			std::cerr << "...executing : " << m_program << args.str() << std::endl;
			execlp(m_program.c_str(), m_program.c_str(), args.str().c_str(), (char *)0);
		}
		else
		{	
			std::cerr << "...executing : " << m_program << " [no arguments]" << std::endl;
			execlp(m_program.c_str(), m_program.c_str(), (char *)0);
		}
		// Exit the process when the program is done executing.
		std::cerr << "Could not exec the child process image" << std::endl;
		exit(-1);
	}
	
	std::cerr << "Connecting to the pipes of the new child process" << std::endl;
	close(pfd1[1]);
	close(pfd2[0]);
	m_inpipe = pfd1[0];
	m_outpipe = pfd2[1];
	// Connect the stdin and stdout to my own pipe fd.
	m_infd = fdopen(pfd1[0], "r");
	if(!m_infd)
	{
		m_running = false;
		std::cerr << get_errno_string() <<  std::endl;
	}
	m_outfd = fdopen(pfd2[1], "w");
	if(!m_outfd)
	{
		m_running = false;
		std::cerr << get_errno_string() <<  std::endl;
	}
	//usleep(200);
	return m_running;
}

bool ChildProcess::start()
{
	cerr << "Child Process starting..." << endl;
	int pfd1[2];
	int pfd2[2];
	
	if(pipe(pfd2) == -1 || pipe(pfd1) == -1)
  	{
  		std::cerr << "Pipes not created" << std::endl;
		std::cerr << get_errno_string() <<  std::endl;
		
		return false;
  	}
	if((m_pid = fork()) < 0)
	{
		std::cerr << "Unable to fork" << std::endl;
		return false;
	}
	m_running = true;
	if(!m_pid)
	{	
		close(pfd1[0]);
		close(pfd2[1]);
		
		// Connect the stdin and stdout to my own pipe fd.
		if(pfd1[0] != STDIN_FILENO)
		{
			close(STDIN_FILENO);
			if(dup2(pfd2[0], STDIN_FILENO) != STDIN_FILENO)
			{
				std::cerr << "StdIn failed to connect." << std::endl;
				exit(-1);
			}
		}
		if(pfd2[1] != STDOUT_FILENO)
		{
			close(STDOUT_FILENO);
			if(dup2(pfd1[1], STDOUT_FILENO) != STDOUT_FILENO)
			{
				std::cerr << "StdOut failed to connect." << std::endl;
				exit(-1);
			}
		}
		// Execute the child process program
		if(m_arguments.size())
		{
			std::stringstream args;
			for(std::size_t n = 0 ; n < m_arguments.size() ; n++)
			{
				args << " " << m_arguments[n];
			}
			execlp(m_program.c_str(), m_program.c_str(), args.str().c_str(), (char *)0);
		}
		else
			execlp(m_program.c_str(), m_program.c_str(), (char *)0);
		// Exit the process when the program is done executing.
		std::cerr << "Could not exec the child process image" << std::endl;
		exit(-1);
	}
	
	close(pfd1[1]);
	close(pfd2[0]);
	m_inpipe = pfd1[0];
	m_outpipe = pfd2[1];
	// Connect the stdin and stdout to my own pipe fd.
	m_infd = fdopen(pfd1[0], "r");
	m_outfd = fdopen(pfd2[1], "w");
	//usleep(200);
	return m_running;
}
void ChildProcess::stop()
{
	try
	{
		if(m_running)
		{
			std::cerr << "Calling wait...";
			int status;
			wait(&status);
			std::cerr << "wait done." << std::endl;
			
			//close(m_inpipe);
			//close(m_outpipe);
			if(fclose(m_infd))
				std::cerr << "Error detected when closing input stream of childprocess." << std::endl;
			if(fclose(m_outfd))
				std::cerr << "Error detected when closing input stream of childprocess." << std::endl;
		}
		else
			std::cerr << "Stop called when not running process" << std::endl;
	}
	catch(...)
	{
		std::cerr << "Stop call on process threw an error" << std::endl;
	}
	m_running = false;
}
bool ChildProcess::writeToStdIn(std::string str, bool debug)
{
	if(debug)
	{
		if(str.empty())
			std::cerr << "[empty string]" << std::endl;
		else if(str[str.size()-1] == '\n')
		{	
			std::string tmp = str;			
			tmp[tmp.size()-1] = '\0';
			std::cerr << "writeToStdIn called with:'" << tmp << "[\\n]'" << std::endl;
		}
		else
			std::cerr << "writeToStdIn called with:'" << str << "'" << std::endl;
	}
	try
	{
		if(!m_running)
		{
			std::cerr << "Child process not running." << std::endl;
			return false;
		}
		if(debug)
			std::cerr << "Retrieving filedescriptor of pipe to stdout of process..." << std::endl;
		int fno = fileno(m_outfd);
		if(debug)
			std::cerr << "...filedescriptor of pipe is " << fno << std::endl;
		if(!is_valid_fd(fno))
		{
			std::cerr << "File descriptor to child process not valid." << std::endl;
			return false;
		}
		int written = write(fno, str.c_str(), str.size());
		if(debug)
			std::cerr << written << " characters written to child process" << std::endl;
		bool ok = (written  == (int)str.size());
		return ok;
	}
	catch(...)
	{
		std::cerr << "Write call on process threw an error" << std::endl;
		return false;
	}
}
std::string ChildProcess::readFromStdOut()
{
	try
	{
		if(!m_running)
			return "";
		int size;
		if((size=read(fileno(m_infd), m_buffer, STD_BUFFER_SIZE)) < 0)
			return "";
		if(size >= STD_BUFFER_SIZE)
			m_buffer[STD_BUFFER_SIZE-1] = '\0';
		else
			m_buffer[size] = '\0';
		std::string str(m_buffer);
		return str;
	}
	catch(...)
	{
		return "";
	}
}
