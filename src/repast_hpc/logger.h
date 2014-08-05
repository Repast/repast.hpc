/*
 *   Repast for High Performance Computing (Repast HPC)
 *
 *   Copyright (c) 2010 Argonne National Laboratory
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with
 *   or without modification, are permitted provided that the following
 *   conditions are met:
 *
 *     Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *     Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *     Neither the name of the Argonne National Laboratory nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE TRUSTEES OR
 *   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *   EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 *  logger.h
 *
 *  Created on:
 *      Author: nick
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <string>
#include <vector>
#include <map>

#define MAX_CONFIG_FILE_SIZE 16384

typedef enum _LogLevel {DEBUG, INFO, WARN, ERROR, FATAL} LOG_LEVEL;

class Appender {

public:
	Appender(const std::string name);
	virtual void write(const std::string& line) = 0;
	virtual void close() {}

	const std::string& name() const {
		return _name;
	}

	virtual ~Appender() = 0;

protected:
	const std::string _name;
};

class Logger {

public:
	Logger(const std::string, LOG_LEVEL, int proc_id);

	void log(LOG_LEVEL, const std::string msg);
	void close();
	void add_appender(Appender *appender);

private:
	const std::string name;
	const LOG_LEVEL level;
	int proc_id;
	std::vector<Appender*> appenders;

	void format_msg(LOG_LEVEL level, const std::string& msg, std::string& to_format);
};

class AppenderBuilder {

public:
	AppenderBuilder(const std::string name);

	std::string name;
	std::string file_name;
	long max_size;
	int max_idx;

	Appender* build();
};

class Log4CL;

class Log4CLConfigurator {

private:
	void error_warn();

	std::string error;
	int line, proc_id;

	std::map<std::string, AppenderBuilder*> app_map;
	std::map<std::string, Logger*> logger_map;
	// key: logger name, value: vector of appenders names for
	// that logger
	std::map<std::string, std::vector<std::string>*> logger_app_map;

	int parse_level(const std::string& str) const;

	void create_root_logger(const std::string& value);
	void create_logger(const std::string& key, const std::string& value);
	void create_named_logger(const std::string& name, const std::string& value);

	void create_appender(const std::string& key, const std::string& value);
	void create_appender_file(const std::string& key, const std::string& value);
	void create_appender_size(const std::string& key, const std::string& value);
	void create_appender_bidx(const std::string& key, const std::string& value);

	Log4CL* create_log4cl();

	AppenderBuilder* get_appender_builder(const std::string& key);

public:
	Log4CLConfigurator();
	Log4CL* configure(const std::string& config_file, int proc_id, boost::mpi::communicator* comm = 0, int maxConfigFileSize = MAX_CONFIG_FILE_SIZE);
};

class Log4CL {
	friend class Log4CLConfigurator;

public:
	~Log4CL();
	static Log4CL* instance();
	static void configure(int, const std::string&, boost::mpi::communicator* comm = 0, int maxConfigFileSize = MAX_CONFIG_FILE_SIZE);
	static void configure(int);

	Logger& get_logger(std::string logger_name);
	void close();

protected:
	Log4CL();

private:
	static Log4CL *_instance;

	std::map<std::string, Logger*> logger_map;
	std::vector<Appender *> appenders;
};


#endif /* LOGGER_H_ */
