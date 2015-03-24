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
 *  logger.cpp
 *
 *  Created on: Dec 16, 2008
 *      Author: nick
 */

#include <mpi.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <algorithm>
#include <iomanip>
#include <map>
#include <ctime>

#include <boost/filesystem.hpp>
#include <boost/mpi/collectives.hpp>

#include "logger.h"
#include "io.h"
#include "RepastErrors.h"


namespace fs = boost::filesystem;


using namespace std;

typedef enum _TOKEN {
	END = 0, ROOT, LOGGER, APPENDER, APPENDER_FILE, APPENDER_SIZE, APPENDER_BIDX, ERRORT
} TOKEN;

const string ROOT_LOGGER_TAG = "logger.root";
const string LOGGER_TAG = "logger.";
const string APPENDER_TAG = "appender.";
const string FILE_TAG = ".File";
const string SIZE_TAG = ".MaxFileSize";
const string BACK_IDX_TAG = ".MaxBackupIndex";

// DEBUG, INFO, WARN, ERROR, FATAL
const int LEVEL_COUNT = 5;
const char *LEVELS[5] = { "DEBUG", "INFO", "WARN", "ERROR", "FATAL" };

int count_char(const string& str, const char& to_count) {
	int count = 0;
	for (int i = 0, n = str.length(); i < n; i++) {
		if (str[i] == to_count)
			count++;
	}
	return count;
}

bool ends_with(const string& str, const string& ends) {
	size_t pos = str.rfind(ends, str.length());
	if (pos == string::npos)
		return false;

	return str.length() - pos == ends.length();
}

class ConfigLexer {

private:
	int _line;
//	ifstream* in;
	istringstream* in;

	string _value, _key, _error;

	void format_error(const char* msg);
	bool is_root();
	bool is_logger();
	bool is_appender();
	bool is_appender_file();
	bool is_appender_size();
	bool is_appender_bidx();

public:
	ConfigLexer(const string& file_name, boost::mpi::communicator* comm = 0, int maxConfigFileSize = MAX_CONFIG_FILE_SIZE);
	~ConfigLexer();

	TOKEN next_token();
	string key();
	string value();
	string error();
	int line();
	void reset();
};

ConfigLexer::ConfigLexer(const string& file_name, boost::mpi::communicator* comm, int maxConfigFileSize) :
	_line(0), _value(""), _key(""), _error("") {

  char* CONFIGFILEBUFFER = new char[maxConfigFileSize];                            // All procs allocate memory for the properties file

  if(comm == 0 || comm->rank() == 0){                                          // If no communicator is passed, all ranks read props file
    ifstream fileInStream(file_name.c_str());
    if (fileInStream.is_open()){
      fileInStream.read(CONFIGFILEBUFFER, maxConfigFileSize);
      // Check if fail:
      if(fileInStream.gcount() >= (maxConfigFileSize - 1)){
        throw repast::Repast_Error_41(maxConfigFileSize, fileInStream.gcount(), file_name); // Config file exceeds maximum allowed size
      }
      CONFIGFILEBUFFER[fileInStream.gcount()] = '\0'; // Add a null terminator
      fileInStream.close();
    } else {
      throw repast::Repast_Error_42(file_name); // Config file not found
    }
  }
  if(comm != 0){                                                               // If a communicator was passed, proc 0 broadcasts to all other procs
    MPI_Bcast(CONFIGFILEBUFFER, maxConfigFileSize, MPI_CHAR, 0, *comm);
  }

  std::string P(CONFIGFILEBUFFER);
  delete CONFIGFILEBUFFER;

  in = new istringstream(P, ios_base::in);

	if (in == NULL || in->fail()) {
		string err = "Error opening config file '" + file_name + "'";
		if (in != NULL)
			delete in;
		throw repast::Repast_Error_43(file_name); // Unknown error
	}
}

ConfigLexer::~ConfigLexer() {
//	if (in != NULL)
//		in->close();
	delete in;
}

bool ConfigLexer::is_root() {
	return _key == ROOT_LOGGER_TAG;
}

bool ConfigLexer::is_logger() {
	return _key.find(LOGGER_TAG) == 0;
}

// its an appender if it starts with appender
// and there is a single dot
bool ConfigLexer::is_appender() {
	return _key.find(APPENDER_TAG) == 0 && count_char(_key, '.') == 1;
}

// its an appender.File if starts with
// appender., has two dots, and ends with File.
bool ConfigLexer::is_appender_file() {
	return _key.find(APPENDER_TAG) == 0 && count_char(_key, '.') == 2 && ends_with(_key, FILE_TAG);
}

// starts with appender, has two dots
// and ends with SIZE tag.
bool ConfigLexer::is_appender_size() {
	return _key.find(APPENDER_TAG) == 0 && count_char(_key, '.') == 2 && ends_with(_key, SIZE_TAG);
}

// starts with appender, has two dots
// and ends with BAC_IDX tag
bool ConfigLexer::is_appender_bidx() {
	return _key.find(APPENDER_TAG) == 0 && count_char(_key, '.') == 2 && ends_with(_key, BACK_IDX_TAG);
}

void ConfigLexer::format_error(const char* msg) {
	stringstream str;
	str << "Error in line " << _line << ": " << msg << endl;
	_error = str.str();
}

string ConfigLexer::key() {
	return _key;
}

string ConfigLexer::value() {
	return _value;
}

string ConfigLexer::error() {
	return _error;
}

int ConfigLexer::line() {
	return _line;
}

TOKEN ConfigLexer::next_token() {
	string str;
	while (getline(*in, str)) {
		_line++;
		repast::str_trim(str);
		if (str.length() > 0 && str[0] != '#') {
			size_t pos = str.find_first_of("=");
			if (pos == string::npos) {
				format_error("'=' is missing.");
				return ERRORT;
			}

			_key = str.substr(0, pos);
			repast::str_trim(_key);
			if (_key.length() == 0) {
				format_error("key is missing.");
				return ERRORT;
			}

			_value = "";
			if (str.length() > pos) {
				_value = str.substr(pos + 1, str.length());
			}
			repast::str_trim(_value);
			if (_value.length() == 0) {
				format_error("value is missing.");
				return ERRORT;
			}

			if (is_root())
				return ROOT;
			if (is_logger())
				return LOGGER;
			if (is_appender())
				return APPENDER;
			if (is_appender_file())
				return APPENDER_FILE;

			if (is_appender_size())
				return APPENDER_SIZE;
			if (is_appender_bidx())
				return APPENDER_BIDX;

			format_error("unexpected token");
			return ERRORT;
		}
	}
	return END;
}

void ConfigLexer::reset() {
	in->seekg(0, ios_base::beg);
}

Appender::Appender(const string name) :
	_name(name) {
}
Appender::~Appender() {
}

class CoutAppender: public Appender {

public:
	CoutAppender();
	~CoutAppender() {
	}
	;
	void write(const string& line);
};

class CerrAppender: public Appender {

public:
	CerrAppender();
	~CerrAppender() {
	}

	void write(const string& line);
};

CerrAppender::CerrAppender() :
	Appender("stderr") {
}

void CerrAppender::write(const string& line) {
	cout << line;
}

CoutAppender::CoutAppender() :
	Appender("stdout") {
}

void CoutAppender::write(const string& line) {
	cout << line;
}

class RollingFileAppender: public Appender {

public:
	RollingFileAppender(const string name, const string file_name, int max_backup, int max_size);
	~RollingFileAppender();

	virtual void write(const string& log_line);
	virtual void close();

private:
	MPI_File out;
	string file_name;
	int max_backup;
	long max_size, cur_size;
	bool isOpen;

	void resize_check();
	void init_cur_size();

};

RollingFileAppender::RollingFileAppender(const string name, const string file_name, int max_backup, int max_size) :
	Appender(name), file_name(file_name), max_backup(max_backup), max_size(max_size), cur_size(-1), isOpen(false) {

}

RollingFileAppender::~RollingFileAppender() {
	if (isOpen) {
		MPI_File_sync(out);
		MPI_File_close(&out);
	}
}

void RollingFileAppender::close() {
	if (isOpen) {
		MPI_File_sync(out);
		MPI_File_close(&out);
		isOpen = false;
	}
}

void RollingFileAppender::init_cur_size() {

	// check to see if the file exists
	ifstream fin(file_name.c_str(), ifstream::in);
	if (fin.fail()) {
		// file does not yet exist
		cur_size = 0;
	} else {
		fin.seekg(0, ios_base::end);
		// size in bytes
		cur_size = fin.tellg();
		fin.close();
	}

}

void RollingFileAppender::resize_check() {

	if (cur_size == -1)
		init_cur_size();

	if (cur_size > max_size) {
		if (isOpen) {
			// close the file
			MPI_File_sync(out);
			MPI_File_close(&out);
			isOpen = false;
		}

		for (int i = max_backup - 1; i >= 0; i--) {
			ostringstream from;
			if (i == 0) {
				from << file_name;
			} else {
				from << file_name << i;
			}

			ifstream fin(from.str().c_str(), ifstream::in);

			if (!fin.fail()) {
				fin.close();
				ostringstream to;
				to << file_name << (i + 1);
				rename(from.str().c_str(), to.str().c_str());
			}
		}

		cur_size = 0;
	}

	if (!isOpen) {

		// reopen the mpi out
		int mode = MPI_MODE_CREATE | MPI_MODE_WRONLY | MPI_MODE_APPEND;
		fs::path filepath(file_name);
		if (!fs::exists(filepath.parent_path())) {
			fs::create_directories(filepath.parent_path());
		}
		//out = MPI::File::Open(MPI::COMM_SELF, file_name.c_str(), mode, MPI::INFO_NULL);
		MPI_File_open(MPI_COMM_SELF, (char*)file_name.c_str(), mode, MPI_INFO_NULL, &out);
		isOpen = true;
	}
}

void RollingFileAppender::write(const string& log_line) {
	resize_check();
	int count = log_line.length();
	MPI_Status status;
	MPI_File_write(out, (void*)log_line.c_str(), count, MPI_CHAR, &status);
	cur_size += count;
}

Logger::Logger(const string name, LOG_LEVEL level, int proc_id) :
	name(name), level(level), proc_id(proc_id) {
}

void Logger::format_msg(LOG_LEVEL level, const string& msg, string& to_format) {
	string ts;
	repast::timestamp(ts);
	ostringstream os;
	os << ts << " [" << proc_id << "] " << LEVELS[level] << " " << name << " " << msg << endl;
	to_format = os.str();
}

void Logger::log(LOG_LEVEL level, const std::string msg) {
	string formatted_msg;
	if (level >= this->level) {
		formatted_msg = "";
		format_msg(level, msg, formatted_msg);
		for (vector<Appender*>::iterator iter = appenders.begin(); iter != appenders.end(); ++iter) {
			Appender* app = *iter;
			app->write(formatted_msg);
		}
	}
}

void Logger::close() {
	for (vector<Appender*>::iterator iter = appenders.begin(); iter != appenders.end(); ++iter) {
		Appender* app = *iter;
		app->close();
	}
}

void Logger::add_appender(Appender *appender) {
	appenders.push_back(appender);
}

AppenderBuilder::AppenderBuilder(const string name) :
	name(name) {
}

Appender* AppenderBuilder::build() {
	if (name == "stdout") {
		return new CoutAppender();
	} else if (name == "stderr") {
		return new CerrAppender();
	} else {
		return new RollingFileAppender(name, file_name, max_idx, max_size);
	}
}

Log4CLConfigurator::Log4CLConfigurator() :
	line(0) {
	app_map["stdout"] = new AppenderBuilder("stdout");
	app_map["stderr"] = new AppenderBuilder("stderr");
}

void Log4CLConfigurator::error_warn() {
	cerr << "WARN: " << error << endl;
}

int Log4CLConfigurator::parse_level(const string& str) const {
	for (int i = 0; i < LEVEL_COUNT; i++) {
		if (str == LEVELS[i])
			return i;
	}
	return -1;
}

void Log4CLConfigurator::create_root_logger(const string& value) {
	create_named_logger("root", value);
}

void Log4CLConfigurator::create_logger(const string& key, const string& value) {
	string name = key.substr(LOGGER_TAG.length());
	if (name.length() == 0) {
		ostringstream os;
		os << "Error in line " << line << ", logger is missing a name";
		error = os.str();
		error_warn();
		return;
	}

	create_named_logger(name, value);
}

void Log4CLConfigurator::create_appender(const string& key, const string& value) {
	string name = key.substr(APPENDER_TAG.length());
	if (name.length() == 0) {
		ostringstream os;
		os << "Error in line " << line << ", appender is missing a name";
		error = os.str();
		error_warn();
		return;
	}

	if (app_map.find(name) == app_map.end()) {
		// add an appender builder for this appender name
		AppenderBuilder* builder = new AppenderBuilder(name);
		app_map[name] = builder;
		builder->max_idx = 1;
		// megabyte
		builder->max_size = 1024 * 1024 * 10;
	}
}

AppenderBuilder* Log4CLConfigurator::get_appender_builder(const string& key) {
	size_t start_pos = APPENDER_TAG.length();
	size_t end_pos = key.rfind(".", key.length());
	if (start_pos == end_pos) {
		ostringstream os;
		os << "Error in line " << line << ", appender is missing a name";
		error = os.str();
		error_warn();
		return NULL;
	}

	ostringstream stream;
	for (size_t i = start_pos; i < end_pos; i++) {
		stream << key[i];
	}
	string name = stream.str();

	AppenderBuilder *builder;
	map<string, AppenderBuilder*>::const_iterator item = app_map.find(name);
	if (item == app_map.end()) {
		builder = new AppenderBuilder(name);
		app_map[name] = builder;
		builder->max_idx = 1;
		// megabyte
		builder->max_size = 1024 * 1024 * 10;
	} else {
		builder = item->second;
	}

	return builder;
}

void Log4CLConfigurator::create_appender_file(const string& key, const string& value) {
	AppenderBuilder *builder = get_appender_builder(key);
	if (builder != NULL) {
		// split file_name on last "." and append a _proc_id to it
		// if no "." then just append "_"
		string file_name = value;
		string parent_path;
		size_t pos = value.find_last_of('/');
		if (pos != string::npos) {
			file_name = value.substr(pos + 1);
			parent_path = value.substr(0, pos + 1);
		}

		pos = file_name.find_last_of('.');
		ostringstream os;
		if (pos != string::npos) {
			os << file_name.substr(0, pos) << "_" << proc_id << file_name.substr(pos, file_name.length());
		} else {
			os << file_name << "_" << proc_id;
		}
		file_name = os.str();

		builder->file_name = parent_path + file_name;
	}
}

void Log4CLConfigurator::create_appender_size(const string& key, const string& value) {
	AppenderBuilder* builder = get_appender_builder(key);
	if (builder != NULL) {
		// given in K but we store in bytes
		builder->max_size = atol(value.c_str()) * 1024;
	}

}

void Log4CLConfigurator::create_appender_bidx(const string& key, const string& value) {
	AppenderBuilder* builder = get_appender_builder(key);
	if (builder != NULL) {
		builder->max_idx = atoi(value.c_str());
	}
}

void Log4CLConfigurator::create_named_logger(const string& name, const string& value) {

	string token;
	istringstream stream(value);
	int level = -1;
	bool do_level = true;
	while (getline(stream, token, ',')) {
		repast::str_trim(token);
		if (do_level) {
			transform(token.begin(), token.end(), token.begin(), ::toupper);
			level = parse_level(token);
			do_level = false;
		} else {
			// its an appender name
			if (app_map.find(token) == app_map.end()) {
				// add an appender builder for this appender name
				AppenderBuilder* builder = new AppenderBuilder(token);
				app_map[token] = builder;
			}

			vector<string> *v;
			map<string, vector<string>*>::const_iterator item = logger_app_map.find(name);
			if (item == logger_app_map.end()) {
				v = new vector<string> ();
				logger_app_map[name] = v;
			} else {
				v = item->second;
			}
			v->push_back(token);

		}
	}

	if (level == -1) {
		error = "Invalid log level for " + name + " logger";
		error_warn();
		return;

	}

	Logger* logger = new Logger(name, (LOG_LEVEL) level, proc_id);
	logger_map[name] = logger;
}

Log4CL* Log4CLConfigurator::configure(const string& config_file, int proc_id, boost::mpi::communicator* comm, int maxConfigFileSize) {
	ConfigLexer lexer(config_file, comm, maxConfigFileSize);
	this->proc_id = proc_id;

	TOKEN tok;

	while (((tok = lexer.next_token()) != END)) {
		line = lexer.line();
		switch (tok) {
		case ROOT:
			create_root_logger(lexer.value());
			break;
		case LOGGER:
			create_logger(lexer.key(), lexer.value());
			break;
		case APPENDER:
			create_appender(lexer.key(), lexer.value());
			break;
		case APPENDER_FILE:
			create_appender_file(lexer.key(), lexer.value());
			break;
		case APPENDER_SIZE:
			create_appender_size(lexer.key(), lexer.value());
			break;
		case APPENDER_BIDX:
			create_appender_bidx(lexer.key(), lexer.value());
			break;
		case ERRORT:
			error = lexer.error();
			error_warn();
			break;
		default:
			error_warn();
		}
	}

	return create_log4cl();
}

Log4CL* Log4CLConfigurator::create_log4cl() {
	Log4CL* log4CL = new Log4CL();
	// create the appenders
	map<string, Appender*> amap;

	for (map<string, AppenderBuilder*>::const_iterator iter = app_map.begin(); iter != app_map.end(); ++iter) {
		AppenderBuilder *builder = iter->second;
		Appender *appender = builder->build();
		amap[iter->first] = appender;
		log4CL->appenders.push_back(appender);
	}

	for (map<string, Logger*>::const_iterator iter = logger_map.begin(); iter != logger_map.end(); ++iter) {

		Logger* logger = iter->second;
		log4CL->logger_map[iter->first] = logger;
		vector<string>* app_list = logger_app_map[iter->first];

		for (vector<string>::const_iterator siter = app_list->begin(); siter != app_list->end(); ++siter) {
			string app_name = *siter;
			logger->add_appender(amap[app_name]);
		}
	}

	// make sure there is a root logger and if not make one.
	if (logger_map.find("root") == logger_map.end()) {
		Logger *root = new Logger("root", WARN, proc_id);
		log4CL->logger_map["root"] = root;

		if (amap.find("stdout") == amap.end()) {
			Appender* out = new CoutAppender();
			root->add_appender(out);
			log4CL->appenders.push_back(out);
		} else {
			root->add_appender(amap["stdout"]);
		}
	}

	for (map<string, AppenderBuilder*>::const_iterator iter = app_map.begin(); iter != app_map.end(); ++iter) {
		AppenderBuilder *builder = iter->second;
		delete builder;
	}
	app_map.clear();

	for (map<string, Logger*>::const_iterator iter = logger_map.begin(); iter != logger_map.end(); ++iter) {

		vector<string>* app_list = logger_app_map[iter->first];
		delete app_list;
	}
	logger_map.clear();

	return log4CL;
}

Logger& Log4CL::get_logger(std::string logger_name) {
	map<string, Logger*>::const_iterator item = logger_map.find(logger_name);
	if (item == logger_map.end()) {
		cerr << "Unable to find logger '" << logger_name << "'. Returning root logger." << endl;
		return *logger_map["root"];
	}

	return *(item->second);
}

Log4CL::Log4CL() {
}

Log4CL::~Log4CL() {
	for (map<string, Logger*>::iterator iter = logger_map.begin(); iter != logger_map.end(); ++iter) {
		Logger *logger = iter->second;
		logger->close();
		delete logger;
	}

	for (vector<Appender *>::iterator iter = appenders.begin(); iter != appenders.end(); ++iter) {
		Appender *appender = *iter;
		delete appender;
	}

	delete _instance;
}

Log4CL* Log4CL::_instance = 0;

Log4CL* Log4CL::instance() {
	return _instance;
}

void Log4CL::configure(int proc_id, const std::string& config_file, boost::mpi::communicator* comm, int maxConfigFileSize) {

	try {
		Log4CLConfigurator configurator;
		_instance = configurator.configure(config_file, proc_id, comm, maxConfigFileSize);

	} catch (invalid_argument& ex) {
		cerr << "ERROR opening logging config file " << ex.what() << endl;
		delete _instance;
		_instance = NULL;
		return;
	}
}

void Log4CL::configure(int proc_id) {
	_instance = new Log4CL();
	Logger *root = new Logger("root", WARN, proc_id);
	_instance->logger_map["root"] = root;

	CoutAppender *out = new CoutAppender();
	root->add_appender(out);
	_instance->appenders.push_back(out);

}

void Log4CL::close() {
	for (map<string, Logger*>::iterator iter = logger_map.begin(); iter != logger_map.end(); ++iter) {
		Logger *logger = iter->second;
		logger->close();
	}
}

/*
 int main(int argc, char **argv) {

 MPI::Init(argc, argv);
 int proc_id = MPI::COMM_WORLD.Get_rank();

 Log4CL::configure(proc_id, "../config.props");
 Logger& logger = Log4CL::instance()->get_logger("root");
 logger.log(ERROR, "root msg\n");

 Logger& logger2 = Log4CL::instance()->get_logger("debug.log");
 logger2.log(ERROR, "logger msg\n");

 Log4CL::instance()->close();

 MPI::Finalize();
 }
 */
