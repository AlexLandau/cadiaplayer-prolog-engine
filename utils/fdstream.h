/*
 *  fdstream.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 9/11/08.
 *  Copyright 2008 Reykjavik University. All rights reserved.
 *
 *  Modified version of code by Nicolai M. Josuttis
 *
 * Disclaimer from Nicolai M. Josuttis:
 *
 * The following code example is taken from the book
 * "The C++ Standard Library - A Tutorial and Reference"
 * by Nicolai M. Josuttis, Addison-Wesley, 1999
 *
 * (C) Copyright Nicolai M. Josuttis 1999.
 * Permission to copy, use, modify, sell and distribute this software
 * is granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied
 * warranty, and with no claim as to its suitability for any purpose.
 */

#ifndef FDSTREAM_H
#define FDSTREAM_H

#include <iostream>
#include <streambuf>
#include <cstdio>

// for write():
#ifdef _MSC_VER
# include <io.h>
#else
# include <unistd.h>
#endif

namespace cadiaplayer {
	namespace utils {
		
		class fdstreambuf : public std::streambuf 
		{
		protected:
			int fd;    // file descriptor
		public:
			// constructor
			fdstreambuf (int _fd) : fd(_fd) {}
		protected:
			// write one character
			virtual int_type overflow (int_type c) 
			{
				if (c != EOF) 
				{
					char z = c;
					if (write (fd, &z, 1) != 1) 
					{
						return EOF;
					}
				}
				return c;
			}
			// write multiple characters
			virtual
			std::streamsize xsputn (const char* s,
									std::streamsize num) 
			{
				return write(fd,s,num);
			}
			// read one character
			virtual int_type underflow (int_type c) 
			{
				if (c != EOF) 
				{
					char z = c;
					if (read (fd, &z, 1) != 1) 
					{
						return EOF;
					}
				}
				return c;
			}
			// read multiple characters
			virtual
			std::streamsize xsgetn (char* s,
									std::streamsize num) 
			{
				return read(fd,s,num);
			}
		};
		
		class fdostream : public std::ostream 
		{
		protected:
			fdstreambuf buf;
		public:
			fdostream (int fd) : std::ostream(0), buf(fd) 
			{
				rdbuf(&buf);
			}
		};
		
		class fdistream : public std::istream 
		{
		protected:
			fdstreambuf buf;
		public:
			fdistream (int fd) : std::istream(0), buf(fd) 
			{
				rdbuf(&buf);
			}
		};
	}
}
#endif // FDSTREAM_H
