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
		// ----------------------------------
		//   OUTPUT CONSOLE (UNSORTED SINK)
		// ---------------------------------
		// Create an output to stdout
		fstlog::output out_console = fstlog::output_console();
		// Create a sink with unsorted behavior
		// sink_unsort logs messages without sorting.
		// This is suitable for single threaded simple use cases.
		fstlog::sink sink_console = fstlog::sink_unsort(
			fstlog::formatter_txt("sink_unsort + output_console: {time} {level} {logger} {message}"),
			out_console);
		
		// --------------------------------
		//    OUTPUT STDERR (SORTED SINK)
		// --------------------------------
		// Create an output to stderr
		fstlog::output out_error = fstlog::output_stderr();
		// Create a sink with sorted behavior
		fstlog::sink sink_error = fstlog::sink_sort(
			fstlog::formatter_txt("sink_sort + output_stderr: {time} {level} {logger} {message}"),
			out_error);
		// Because the system operates asynchronously, some messages may appear out of order. 
		// While reading log buffers, new messages can still be added to these buffers.
		// If a message is added to a buffer that has already been partially read, 
		// it might be displayed later in the sequence than another message 
		// that was added to a different buffer that hasn’t been read yet.

		// ------------------------------
		//   OUTPUT FILE (UNSORTED SINK)
		// ------------------------------
		// Create an output to a file (with truncation)
		fstlog::output out_file = fstlog::output_file("file.log", true);
		// Create a sink
		fstlog::sink sink_file = fstlog::sink_unsort(
			fstlog::formatter_txt("sink_unsort + output_file: {time} {level} {logger} {message}"),
			out_file);
		// The truncation flag is set to false by default (append to existing file).


		// -------------------------------
		//  OUTPUT STREAM (UNSORTED SINK)
		// -------------------------------
		// Create a shared stringstream
		auto stream_1 = std::make_shared<std::stringstream>();
		// Create an output from the stream
		fstlog::output out_stream = fstlog::output_stream(stream_1);
		// Create a sink
		fstlog::sink sink_stream = fstlog::sink_unsort(
			fstlog::formatter_txt("sink_unsort + output_stream: {time} {level} {logger} {message}"),
			out_stream);
		// Important: The stream must not be accessed while 
		// logging is in progress to avoid race conditions.

		// ----------------------------------
		//  OUTPUT STREAM MT (UNSORTED SINK)
		// ----------------------------------
		// Create a shared stringstream and mutex
		auto stream_2 = std::make_shared<std::stringstream>();
		auto stream_mutex = std::make_shared<std::mutex>();
		// Create an output with thread safety
		fstlog::output out_stream_mt = fstlog::output_stream_mt(stream_2, stream_mutex);
		// create a sink
		fstlog::sink sink_stream_mt = fstlog::sink_unsort(
			fstlog::formatter_txt("sink_unsort + output_stream_mt: {time} {level} {logger} {message}"),
			out_stream_mt);
		// output_stream_mt allows concurrent access via a mutex.
		// Always lock the mutex when accessing the stream externally.

		// ---------------------------------
		//  OUTPUT C STREAM (UNSORTED SINK)
		// ---------------------------------
		// output_cstream (output to a C stream)
		// the stream can not be read/written while it is used for logging
		// The FILE* must remain valid for the lifetime of the fstlog::output object
		// and its managed by the user
		
		// Open a C file stream
		FILE* c_stream{ nullptr };
#ifdef _WIN32
		fopen_s(&c_stream, "c_file.log", "wb");
#else
		c_stream = fopen("c_file.log", "wb");
#endif
		if (!c_stream) throw std::runtime_error("Opening file: c_file.log failed!");
		
		// Create an output from the C stream
		fstlog::output out_cstream = fstlog::output_cstream(c_stream);
		// create a sink
		fstlog::sink sink_cstream = fstlog::sink_unsort(
			fstlog::formatter_txt("sink_unsort + output_cstream: {time} {level} {logger} {message}"),
			out_cstream);
		// Important: The FILE* pointer must remain valid for the lifetime of the output_cstream object.
		// Do not close it before destroying all copies of the out_cstream object.
		// Lifetime of c_stream must be managed by the user.


		// Create a core and logger
		fstlog::core my_core("my_core");
		std::cout << "fstlog version: " << my_core.version() << "\n\n";
		fstlog::logger my_logger(my_core, "my_logger");
		// add sinks to core
		my_core.add_sink(sink_console);
		my_core.add_sink(sink_error);
		my_core.add_sink(sink_file);
		my_core.add_sink(sink_stream);
		my_core.add_sink(sink_stream_mt);
		my_core.add_sink(sink_cstream);
		
		// log with logger
		LOG_INFO(my_logger, "Hello {}!", "World");
		
		// call flush() to force processing of all logs
		// flush() is called automatically when a core is stopped or destroyed
		my_core.flush();
		// Access stream_2 (must lock the mutex)
		{
			std::lock_guard<std::mutex> grd(*stream_mutex);
			std::cout << stream_2->str();
		}
		// Release the sink to safely access stream_1
		my_core.release_sink(sink_stream);
		std::cout << stream_1->str();
		
		// -------------------------
		//  OUTPUT C STREAM CLEANUP
		// -------------------------
		// Flush logs
		my_core.flush();
		// Release sink
		my_core.release_sink(sink_cstream);
		// Destroy all copies of sink_cstream
		sink_cstream = fstlog::sink{};
		// Destroy all copies of out_cstream
		out_cstream = fstlog::output{};
		// Close the C stream manually
		fclose(c_stream);
	}
	catch (const std::exception& ex) {
		std::cout << ex.what();
	}
}
