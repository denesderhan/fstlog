//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).

#include <iostream>
#include <sstream>

#include <fstlog/core.hpp>
#include <fstlog/formatter/formatter_txt.hpp>
#include <fstlog/logger/log_macro.hpp>
#include <fstlog/logger/logger.hpp>
#include <fstlog/output/output_console.hpp>
#include <fstlog/output/output_cstream.hpp>
#include <fstlog/output/output_file.hpp>
#include <fstlog/output/output_stream.hpp>
#include <fstlog/output/output_stream_mt.hpp>
#include <fstlog/sink/sink_sort.hpp>
#include <fstlog/sink/sink_unsort.hpp>

int main()
{
	try {
		//create core
		fstlog::core my_core("my_core");
		std::cout << "fstlog version: " << my_core.version() << "\n\n";
		//create logger
		fstlog::logger my_logger(my_core, "my_logger");

		//-------------------
		// OUTPUT CONSOLE
		//-------------------
		// output_console (output to stdout)
		fstlog::output out_con = fstlog::output_console();
		
		//-------------------
		// UNSORTED SINK
		//-------------------
		// sink_unsort, messages will not be sorted by the timestamp
		// (the logs will be sorted naturally, if logging happens only 
		// in a single thread and by one and only one logger)
		fstlog::sink sink_unsort = fstlog::sink_unsort(
			fstlog::formatter_txt("from sink_unsort: {time} {level} {logger} {message}"),
			out_con);

		//-------------------
		// OUTPUT STDERR
		//-------------------
		// output_stderr (output to stderr)
		fstlog::output out_err = fstlog::output_stderr();
		
		//-------------------
		// SORTED SINK
		//-------------------
		// sink_sort, log messages will be sorted,
		// some messages can be out of order due to the asynchronous nature of the system.
		// In the time frame of reading the log buffers, messages can be added to these buffers.
		// If one is added to a buffer that was already read, it can be ordered after the other, 
		// that is added to an other buffer that was not yet read, regardless of the correct ordering. 
		fstlog::sink sink_sort = fstlog::sink_sort(
			fstlog::formatter_txt("from sink_sort: {time} {level} {logger} {message}"),
			out_err);
		
		//----------------
		// OUTPUT FILE
		//----------------
		// output_file (output to a file)
		// truncation can be specified by a bool value (default is false)
		fstlog::output out_file = fstlog::output_file("file.log", true);
		// create sink
		fstlog::sink sink_file = fstlog::sink_unsort(
			fstlog::formatter_txt("from sink_file: {time} {level} {logger} {message}"),
			out_file);
		
		//----------------
		// OUTPUT STREAM
		//----------------
		// output_stream (output to C++ stream)
		// the stream can not be read/written as long it is used for logging
		auto stream_1 = std::make_shared<std::stringstream>();
		fstlog::output out_stream = fstlog::output_stream(stream_1);
		// create sink
		fstlog::sink sink_stream = fstlog::sink_unsort(
			fstlog::formatter_txt("from sink_stream: {time} {level} {logger} {message}"),
			out_stream);
		
		//------------------
		// OUTPUT STREAM MT
		//------------------
		// output_stream_mt (output to C++ stream)
		// the stream can be read/written concurrently if the mutex is locked
		auto stream_2 = std::make_shared<std::stringstream>();
		auto stream_mutex = std::make_shared<std::mutex>();
		fstlog::output out_stream_mt = fstlog::output_stream_mt(stream_2, stream_mutex);
		// create sink
		fstlog::sink sink_stream_mt = fstlog::sink_unsort(
			fstlog::formatter_txt("from sink_stream_mt: {time} {level} {logger} {message}"),
			out_stream_mt);
		
		//---------------------
		// OUTPUT C STREAM
		//---------------------
		// output_cstream (output to a C stream)
		// the stream can not be read/written while it is used for logging
		// The FILE* must remain valid for the lifetime of the fstlog::output object,
		// and its managed by the user
		FILE* c_strm{ nullptr };
#ifdef _WIN32
		fopen_s(&c_strm, "c_file.log", "wb");
#else
		c_strm = fopen("c_file.log", "wb");
#endif
		if (!c_strm) throw std::runtime_error("Opening file: c_file.log failed!");

		fstlog::output out_cstrm = fstlog::output_cstream(c_strm);
		// create sink
		fstlog::sink sink_cstrm = fstlog::sink_unsort(
			fstlog::formatter_txt("from sink_cstrm: {time} {level} {logger} {message}"),
			out_cstrm);
		// add sink to core
		my_core.add_sink(sink_cstrm);
		// log
		LOG_INFO(my_logger, "Hello {}!", "c_stream");
		//flushing sink to file
		my_core.flush();
		// fstlog::output is a reference counted smart pointer like object
		// to close the underlying output all references must be destroyed.
		// the sink holds an fstlog::output object and the core holds a
		// an fstlog::sink object
		my_core.release_sink(sink_cstrm);
		sink_cstrm = fstlog::sink{};
		out_cstrm = fstlog::output{};
		// closing stream, no one can use it any longer
		fclose(c_strm);

		// add sinks to core
		my_core.add_sink(sink_unsort);
		my_core.add_sink(sink_sort);
		my_core.add_sink(sink_file);
		my_core.add_sink(sink_stream);
		my_core.add_sink(sink_stream_mt);
		//log with logger
		LOG_INFO(my_logger, "Hello {}!", "World");
		// stop() flushes sinks, stops background thread
		my_core.stop();

		// in general stream_1 can not be read/written while it is used for logging (concurrency)
		// in this case it would be safe, the core's background thread has exited (stop())
		my_core.release_sink(sink_stream);
		sink_stream = fstlog::sink{};
		out_stream = fstlog::output{};
		std::cout << "\ncontents of stream_1:\n" << stream_1->str() << "\n";

		{
			std::lock_guard<std::mutex> grd(*stream_mutex);
			std::cout << "contents of stream_2:\n" << stream_2->str() << "\n";
		}
	}
	catch (const std::exception& ex) {
		std::cout << ex.what() << '\n';
	}
}
