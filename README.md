# fstlog
Fast, asynchronous, low footprint, C++ logging library.

## Features
- C++ 20/17
- Low latency of log call in application code.
- Asynchronous high throughput log formatting in background thread.
- Custom allocator support (std::pmr::polymorphic_allocator used by default).
- Multiple logging policies: guaranteed / non guaranteed / low latency.
- Compilation without exceptions is supported, (with suitable allocator).
- Custom formatting, std::format syntax is used for log calls and formatter setup.
- Multiple formatter options, with varying capabilities and performance.
- Simple log message filtering (by log channel and log level).
- Compile time log level filtering.
- No 3rd party dependencies.

## Benchmarks
* https://denesderhan.github.io/logbench_results/

### Alpha release 
The library is in alpha release.
Testing and feedback is greatly appreciated!

## Build, install

### Windows (Visual Studio)
- Download and extract the project.
- Open a Visual Studio x64 Native Tools Command Prompt

```cmd
**********************************************************************
** Visual Studio 2022 Developer Command Prompt v17.13.7
** Copyright (c) 2022 Microsoft Corporation
**********************************************************************
[vcvarsall.bat] Environment initialized for: 'x64'

cd your_path_to/fstlog
mkdir build
cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja install
```
fstlog.lib will be in build\bin\Static\fstlog\lib
includes in build\bin\Static\fstlog\include
example binaries in build\bin\Static

### Linux
```shell
sudo apt install build-essential git cmake ninja-build pkgconf
git clone http://github.com/denesderhan/fstlog
cd fstlog && mkdir build && cd build
cmake .. -G Ninja
sudo ninja install
```
To check successfull installation:
```shell
cmake --find-package -DNAME=fstlog -DCOMPILER_ID=GNU -DLANGUAGE=C -DMODE=EXIST
pkg-config --modversion libfstlog
```
Example binaries will be in build/bin/Static

### Hello World
```c++
#include <iostream>

#include <fstlog/core.hpp>
#include <fstlog/logger/logger.hpp>
#include <fstlog/logger/log_macro.hpp>
#include <fstlog/sink/sink_sort.hpp>
#include <fstlog/formatter/formatter_txt.hpp>
#include <fstlog/output/output_console.hpp>

int main()
{
	try {
		//create core
		fstlog::core my_core("my_core");
		std::cout << "fstlog version: " << my_core.version() << "\n\n";
		//create sink
		fstlog::sink my_sink = fstlog::sink_sort(
			fstlog::formatter_txt(),
			fstlog::output_console());
		//assign sink to core
		my_core.add_sink(my_sink);

		//create logger
		fstlog::logger my_logger(my_core);
		
		//log with logger
		LOG_INFO(my_logger, "Hello {}!", "World");
	}
	catch (const std::exception& ex) {
		std::cout << ex.what() << '\n';
	}
}

```

## System design, components
The central part of the system is the fstlog::core object. 
Loggers are assigned to cores at construction, they send the log messages to the core 
through a buffer. The core's background thread processes the messages using sinks.
A sink is constructed with a formatter, an output and a filter, then assigned to the core.
The core is not a singleton, there can be multiple core instances at the same time.

## Loggable types
Primitive types, string types, loggable container types of loggable types 
(nested containers, custom containers).
See example: log_calls.cpp

## Formatting syntax
Log messages use the std::format syntax, with the following exceptions:
- positional arguments are not supported, {arg-id:} arg_id is ignored.
- placeholders in format specification {:{1}.{2}} are not supported.
- chrono formatting not supported.
- depending on formatter type, additional options can be ignored.

See examples: log_calls.cpp; formatter_config.cpp

## Object lifetime and resource management
The core, sink, formatter, output instances are reference counting smart pointer like objects.
The constructor/factory function allocates and constructs the underlying implementation object. Lifetime and resource management is taken care of by the library. 

## fstlog::core
```c++
#include <fstlog/core.hpp>
```
The fstlog::core class is the central part of the system, it collects, formats 
and transmits the log messages to the destinations.
There can be multiple cores in a program but they are separated from each other.
Every core starts a background thread at construction, this thread asynchronously 
processes the log messages. The processing is done by the use of sinks, 
a core can have multiple sinks, but a sink can be assigned only to one core at a time.

## Loggers
The logger's function is to send messages to the core. Every logger has a link to an internal buffer, 
for transmitting log messages asynchronously to the core.
The logger serializes the log messages into this buffer, doing as much work as possible 
during compile time. Depending on the log policy, it will notify/wake up the core's background thread 
if there is work to do.

- Every logger has a log level to filter messages early, the log level can be set 
at runtime or compile time depending on the logger's type.
- Every logger has a channel, there are 256 channels, the default is '1' 
the '0' channel is used by fstlog for self logging.
- The logger's name and the thread's name can be set to provide information in the logs,
these names are not used for any other purpose and are not unique.

fstlog has multiple logger types:
### fstlog::logger
```c++
#include <fstlog/logger/logger.hpp>
```
This logger uses thread_local storage to provide thread safe logging.
All instances of the class use the same thread_local buffer, if they are 
logging in the same thread and are bound to the same core.
(One common buffer per thread per core for all fstlog::logger instances.)
With the limitation that only the first created 32 cores can be used.

### fstlog::logger_mt
```c++
#include <fstlog/logger/logger_mt.hpp>
```
This is a classic thread safe logger that uses a mutex. 
For the use case, when allocating a buffer for all threads is not desired 
or thread_local storage can not be used. It locks a mutex when accessing the internal buffer, 
this can cause lock contention and worse performance if the logging frequency is high.
Every instance has its own buffer.

### fstlog::logger_st
```c++
#include <fstlog/logger/logger_st.hpp>
```
This logger is a non thread safe logger, for single threaded logging,
intended for best performance. Every instance has its own buffer.

### fstlog::logger_st_fix
```c++
#include <fstlog/logger/logger_st_fix.hpp>
```
This logger is a variant of fstlog::logger_st, with compile time values 
for the level, channel, logger and thread names.


## Formatters
The formatters convert the messages to the desired format.
Formatters are initialized with a format pattern, using the std::format syntax
with the following differences:
- Argument id {arg_id:} is mandatory, only the following id-s can be used:
	message, level/severity, time/timestamp, logger, thread, policy, channel, file, line, function
- The precision in the replacement field for the timestamp, is used for seconds precision.
	The time is formatted in the local zone by default, if the first character of the strftime
	string is 'U' or 'L' it is used for determining UTC / local time zone formatting.

See example: formatter_config.cpp

### Custom formatting
- the default formatting pattern is:

```c++
std::string_view default_format_string{ 
	"{timestamp:.6%Y-%m-%d %H:%M:%S %z} {severity} {file}:{line} {message}" }
```
- Example of initializing a formatter with a pattern using all fields:

```c++
auto my_formatter = fstlog::formatter_txt("{timestamp:.3U%Y-%m-%d %H:%M:%S +0000} {severity} {policy} {channel} {thread} {logger} {file}:{line} {function} {message}");
```

Formatters are created with factory functions, which return type erased fstlog::formatter instances.

### formatter_txt
```c++
#include <fstlog/formatter/formatter_txt.hpp>
```
This formatter implements most of the formatting features of the std::format specification.
It uses an internal formatting implementation and does not depend on std::format.

### formatter_txt_fast
```c++
#include <fstlog/formatter/formatter_txt_fast.hpp>
```
This formatter implements minimal formatting features of the std::format specification and
also uses the internal implementation.

### formatter_stdformat
```c++
#include <fstlog/formatter/formatter_stdformat.hpp>
```
This formatter implements most of the formatting features of the std::format specification
and uses C++20 std::format internally.

### formatter_null
```c++
#include <fstlog/formatter/formatter_null.hpp>
```
Null formatter for testing/benchmarking.

## Outputs
The outputs are the destinations of the formatted log messages.
Outputs are created with factory functions, which return type erased fstlog::output instances.

### output_console
```c++
#include <fstlog/output/output_console.hpp>
```

### output_file
```c++
#include <fstlog/output/output_file.hpp>
```

### output_stream
```c++
#include <fstlog/output/output_stream.hpp>
```
The stream can not be used while a core uses it.

### output_stream_mt
```c++
#include <fstlog/output/output_stream_mt.hpp>
```
The stream can only be used after locking the mutex if a core uses it.

### output_cstream
```c++
#include <fstlog/output/output_cstream.hpp>
```
The stream can not be used while a core uses it, stream lifetime is not managed.

### output_null
```c++
#include <fstlog/output/output_null.hpp>
```
See example: sink_output.cpp

## Filter
```c++
#include <fstlog/filter/filter.hpp>
```
Filters are used by the sinks to filter log messages, filtering is done by
log level and log channel.
See example: filter.cpp

## Sinks
Sinks are created with factory functions, which return type erased fstlog::sink instances.

The sink's flush interval and in case of a sorted_sink the buffer size, can be set at construction time.
Sinks are flushed at the set intervals or when the buffers are full.

## Sorted sink
This sink has an internal buffer to sort messages when multiple 
loggers and/or threads are logging simultaneously
```c++
#include <fstlog/sink/sink_sort_.hpp>
```

## Unsorted sink
This sink outputs messages without the overhead of sorting.
```c++
#include <fstlog/sink/sink_unsort_.hpp>
```

## Null sink
Drops all messages.
```c++
#include <fstlog/sink/sink_null.hpp>
```

See example: sink_output.cpp

### Logging policies
fstlog can use different logging policies for every log call. 
The policy is a class template passed as a template parameter to the log<>() call.
The policies are:

#### fstlog::log_policy_guaranteed
```c++
#include <fstlog/logger/log_policy_guaranteed.hpp>
```
Processing of the message is guaranteed.
The client code will block until there is space in the buffer to serialize the message.
This policy is used by the LOG(...), LOG_INFO(...) ... macros.

#### fstlog::log_policy_nonguaranteed
```c++
#include <fstlog/logger/log_policy_nonguaranteed.hpp>
```
Processing of the message is not guaranteed, the client code will continue 
without blocking if the buffer is full, but will wake up the core and/or request a flush if needed.
This policy is used by the LOG_NG(...), LOG_NG_INFO(...) ... macros.

#### fstlog::log_policy_lowlatency
```c++
#include <fstlog/logger/log_policy_lowlatency.hpp>
```
Processing of the message is not guaranteed, the client code will continue 
without blocking if the buffer is full and will not wake up or notify the core.
The core has to be set to poll the buffer frequently.
The core's polling interval can be set at any time by calling 
core::poll_interval(std::chrono::milliseconds interval).
This policy is used by the LOG_LL(...), LOG_LL_INFO(...) ... macros.

See example: multi_threaded.cpp

### Configuration Macros
Macros usable in client code, (define before including headers).

```c++
#define FSTLOG_COMPILETIME_LOGLEVEL Info
```
- Setting this macro will remove the lower level log calls (Trace, Debug in the example)
in compile time. The values for the macro can be: All, Trace, Debug, Info, Warn, Error, Fatal, None

```c++
#define FSTLOG_FLUSH_EVERY
```
- Defining this macro forces the client code to call the core's flush() method 
immediately after each log() call, to behave like a synchronous logger.
The continuous buffer flushing will have a performance impact!
This blocking flush can also be made, calling the core::flush() method.

```c++
#define FSTLOG_NO_NESTING
```
- Defining this macro disables logging of nested containers.


### CMake options
```
-DFSTLOG_EXAMPLES=ON
```
- Build example binaries.

```
-DBUILD_SHARED_LIBS=ON
```
- Build fstlog as a shared library.

```
-DFSTLOG_TESTS=ON
```
- Generate tests, (Catch2 v3 is required), works only with a static build ( -DBUILD_SHARED_LIBS=OFF ).

```
-DFSTLOG_DEBUG=ON
```
- Used only on windows and affects only Debug builds. In windows release and debug binaries are incompatible. To provide better performance in debug builds, fstlog's Debug build on windows is an optimized build by default, This option switches to the unoptimized build to debug fstlog itself.

```
-DFSTLOG_DEFAULT_BUFFERSIZE=16*1024
```
- Sets the default size for the log buffers (in bytes).

```
-DFSTLOG_MAX_BUFFERSIZE=128*1024
```
- Sets the maximum allowed size for the log buffers (in bytes).

```
-DFSTLOG_MIN_BUFFERSIZE=1024
```
- Sets the minimum allowed size for the log buffers (in bytes).

```
-DFSTLOG_MAX_MESSAGESIZE=32*1024
```
- Sets the maximum allowed log message size in the buffer (in bytes).

```
-DFSTLOG_NO_NESTING=ON
```
- Disable logging of nested containers in client code. 
This macro is passed to the compiler of the client code by CMake.

```
-DFSTLOG_CONTAINER_NESTING_DEPTH=10
```
- Limits the processing depth of nested containers in the background thread, 
(recursive function call safeguard).

```
-DFSTLOG_POLLINTERVAL=0
```
- Sets the default buffer polling interval for the background thread (in milliseconds), 
default zero means no polling. 

```
-DFSTLOG_SINK_FLUSHINTERVAL=0
```
- Sets the default flush interval for the sinks (in milliseconds),
default zero means no periodic flushing.

```
-DFSTLOG_ALLOCATOR=../fstlog/include/fstlog/detail/pmr_allocator.hpp
```
- Copies the file into the source tree and uses it for the allocator.
Default is the pmr allocator, the malloc_allocator can be used 
if compiling with disabled exceptions.
```
-DFSTLOG_ALLOCATOR=../fstlog/include/fstlog/detail/malloc_allocator.hpp
```
