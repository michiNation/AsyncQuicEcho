cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(GFLAGS_INCLUDE_DIR /usr/lib/x86_64-linux-gnu)
set(sodium_DIR /usr/lib/x86_64-linux-gnu)
set(CMAKE_MODULE_PATH 
"/home/michael/quic/mvfst/cmake" 
"/home/michael/quic/mvfst/build_1" 
"/home/michael/quic/mvfst/_build/build"
"/home/michael/quic/mvfst/_build/deps/lib/cmake/fmt"
"/home/michael/quic/mvfst/_build/deps/lib/cmake/folly"
"/home/michael/quic/mvfst/_build/deps/lib/cmake/fizz"
)
project(AsyncQuic VERSION 1.0)

LINK_DIRECTORIES(
	/usr/lib/x86_64-linux_gnu/
	)

# Dependencies
find_package(Boost 1.62
  REQUIRED COMPONENTS
  iostreams
  system
  thread
  filesystem
  regex
  context
  date_time
  program_options
)

#finding packages
find_package(Threads)
find_package(OpenSSL REQUIRED)
find_package(fmt PATHS /home/michael/quic/mvfst/_build/deps/lib/cmake/fmt)
find_package(folly PATHS /home/michael/quic/mvfst/_build/deps/lib/cmake/folly)
find_package(sodium PATHS /home/michael/quic/mvfst/cmake)
find_package(Fizz PATHS /home/michael/quic/mvfst/_build/deps/lib/cmake/fizz)
find_package(gflags REQUIRED)
find_package(Glog REQUIRED)

add_executable(AsyncQuic src/main.cpp src/Client.cpp src/Server.cpp)
#add_executable(AsyncQuic src/main.cpp src/Client.cpp)
#add_executable(AsyncQuic src/main.cpp src/Server.cpp)
#add_executable(AsyncQuic src/main.cpp)



set(LIBS  
            #/home/michael/quic/mvfst/_build/deps/lib 
            /home/michael/quic/mvfst/_build/build/quic/common/test/libmvfst_test_utils.a 
            /home/michael/quic/mvfst/_build/deps/lib/libgmock_main.a 
            /home/michael/quic/mvfst/_build/deps/lib/libgmock.a 
            /home/michael/quic/mvfst/_build/deps/lib/libgtest.a 
            /home/michael/quic/mvfst/_build/build/quic/fizz/client/libmvfst_fizz_client.a 
            /home/michael/quic/mvfst/_build/build/quic/client/libmvfst_client.a 
            /home/michael/quic/mvfst/_build/build/quic/server/libmvfst_server.a 
            /home/michael/quic/mvfst/_build/build/quic/server/libmvfst_server_state.a 
            /home/michael/quic/mvfst/_build/build/quic/fizz/handshake/libmvfst_fizz_handshake.a 
            /home/michael/quic/mvfst/_build/build/quic/dsr/libmvfst_dsr_frontend.a 
            /home/michael/quic/mvfst/_build/build/quic/dsr/libmvfst_dsr_types.a 
            /home/michael/quic/mvfst/_build/build/quic/common/libmvfst_transport_knobs.a 
            /home/michael/quic/mvfst/_build/build/quic/api/libmvfst_transport.a 
            /home/michael/quic/mvfst/_build/build/quic/common/libmvfst_buf_accessor.a 
            /home/michael/quic/mvfst/_build/build/quic/state/libmvfst_state_ack_handler.a 
            /home/michael/quic/mvfst/_build/build/quic/state/libmvfst_state_datagram_handler.a 
            /home/michael/quic/mvfst/_build/build/quic/congestion_control/libmvfst_cc_algo.a 
            /home/michael/quic/mvfst/_build/build/quic/codec/libmvfst_codec_pktrebuilder.a 
            /home/michael/quic/mvfst/_build/build/quic/happyeyeballs/libmvfst_happyeyeballs.a 
            /home/michael/quic/mvfst/_build/build/quic/common/libmvfst_looper.a 
            /home/michael/quic/mvfst/_build/build/quic/loss/libmvfst_loss.a 
            /home/michael/quic/mvfst/_build/build/quic/d6d/libmvfst_d6d_state_functions.a 
            /home/michael/quic/mvfst/_build/build/quic/common/libmvfst_socketutil.a 
            /home/michael/quic/mvfst/_build/build/quic/state/libmvfst_state_pacing_functions.a 
            /home/michael/quic/mvfst/_build/build/quic/state/libmvfst_state_simple_frame_functions.a 
            /home/michael/quic/mvfst/_build/build/quic/state/libmvfst_state_functions.a 
            /home/michael/quic/mvfst/_build/build/quic/codec/libmvfst_codec_pktbuilder.a 
            /home/michael/quic/mvfst/_build/build/quic/state/libmvfst_state_stream.a 
            /home/michael/quic/mvfst/_build/build/quic/flowcontrol/libmvfst_flowcontrol.a 
            /home/michael/quic/mvfst/_build/build/quic/logging/libmvfst_qlogger.a 
            /home/michael/quic/mvfst/_build/build/quic/state/libmvfst_state_stream_functions.a 
            /home/michael/quic/mvfst/_build/build/quic/state/libmvfst_state_machine.a 
            /home/michael/quic/mvfst/_build/build/quic/d6d/libmvfst_d6d_types.a 
            /home/michael/quic/mvfst/_build/build/quic/codec/libmvfst_codec.a 
            /home/michael/quic/mvfst/_build/build/quic/handshake/libmvfst_handshake.a 
            /home/michael/quic/mvfst/_build/build/quic/codec/libmvfst_codec_packet_number_cipher.a 
            /home/michael/quic/mvfst/_build/build/quic/codec/libmvfst_codec_decode.a 
            /home/michael/quic/mvfst/_build/build/quic/codec/libmvfst_codec_types.a 
            /home/michael/quic/mvfst/_build/build/quic/common/libmvfst_bufutil.a 
            /home/michael/quic/mvfst/_build/build/quic/libmvfst_constants.a 
            /home/michael/quic/mvfst/_build/build/quic/libmvfst_exception.a 
            /home/michael/quic/mvfst/_build/deps/lib/libfizz.a 
            /home/michael/quic/mvfst/_build/deps/lib/libfolly.a 
            /usr/lib/x86_64-linux-gnu/libgflags.so.2.2.2 
            /usr/lib/x86_64-linux-gnu/libboost_context.so.1.71.0 
            /usr/lib/x86_64-linux-gnu/libboost_filesystem.so.1.71.0 
            /usr/lib/x86_64-linux-gnu/libboost_program_options.so.1.71.0 
            /usr/lib/x86_64-linux-gnu/libboost_regex.so.1.71.0 
            /usr/lib/x86_64-linux-gnu/libboost_system.so.1.71.0 
            /usr/lib/x86_64-linux-gnu/libboost_thread.so.1.71.0 
            /usr/lib/x86_64-linux-gnu/libboost_atomic.so.1.71.0 
            /home/michael/quic/mvfst/_build/deps/lib/libfmt.a 
            /usr/lib/x86_64-linux-gnu/libevent.so 
            /usr/lib/x86_64-linux-gnu/libz.so 
            /usr/lib/x86_64-linux-gnu/liblzma.so 
            /usr/lib/x86_64-linux-gnu/liblz4.so 
            /usr/lib/x86_64-linux-gnu/libsnappy.so 
            /usr/lib/x86_64-linux-gnu/libiberty.a 
            /usr/lib/x86_64-linux-gnu/libsodium.so 
            /usr/lib/x86_64-linux-gnu/libunwind.so.8 
            /usr/lib/x86_64-linux-gnu/libdouble-conversion.so 
            /usr/lib/x86_64-linux-gnu/libglog.so 
            /usr/lib/x86_64-linux-gnu/libssl.so 
            /usr/lib/x86_64-linux-gnu/libcrypto.so 
            /home/michael/quic/mvfst/_build/deps/lib/libzstd.so 
            /usr/lib/x86_64-linux-gnu/libsodium.so 
            /usr/lib/x86_64-linux-gnu/libz.so 
            /usr/lib/x86_64-linux-gnu/librt.so  
            Threads::Threads          
        )

#include directories HeaderFiles
target_include_directories(AsyncQuic PUBLIC 
                                    ${CMAKE_CURRENT_SOURCE_DIR}/inc/
                                    /home/michael/quic/mvfst/_build/deps/include/
                                    /home/michael/quic/mvfst/
                                    )
target_link_libraries(AsyncQuic PUBLIC ${LIBS} ${CMAKE_DL_LIBS})

