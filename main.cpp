#if defined(WIN32)
#include <tchar.h>
#endif

#include <cstdio>
#include <chrono>
#include <thread>
#include <boost/asio.hpp>

namespace {

using namespace boost::asio::ip;

void server()
{
  boost::asio::io_service io_service;
  boost::system::error_code ec;
  tcp::acceptor server(io_service, tcp::endpoint(boost::asio::ip::address_v4::loopback(), 2000));
  tcp::socket s(io_service);

  server.accept(s, ec);
  if (ec)
  {
    fprintf(stderr, "accept() failed: %s\n", ec.message().c_str());
    return;
  }
  boost::asio::write(s, boost::asio::buffer("start\n", 6), ec);
  if (ec)
  {
    fprintf(stderr, "write() failed: %s\n", ec.message().c_str());
    return;
  }
  std::this_thread::sleep_for(std::chrono::seconds(130));
  boost::asio::write(s, boost::asio::buffer("end\n", 4), ec);
  if (ec)
  {
    fprintf(stderr, "write() failed: %s\n", ec.message().c_str());
    return;
  }
}

}

#if defined(WIN32) && !defined(__MINGW32__)
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char* argv[])
#endif
{
  std::thread(server).detach();

#ifdef RAW
  boost::asio::io_service io_service;
  boost::system::error_code ec;
  boost::asio::ip::tcp::socket s(io_service);

  s.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::loopback(), 2000), ec);
  if (ec)
  {
    fprintf(stderr, "connect(loopback, 2000) failed: %s\n", ec.message().c_str());
    return 1;
  }

  s.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
  if (ec)
  {
    fprintf(stderr, "shutdown(shutdown_send) failed: %s\n", ec.message().c_str());
    return 2;
  }

  for (;;)
  {
    char buffer[512];
    size_t n = s.read_some(boost::asio::buffer(buffer, sizeof(buffer)), ec);
    if (ec)
    {
      fprintf(stderr, "read_some() failed: %s\n", ec.message().c_str());
      return 3;
    }
    fwrite(buffer, 1, n, stdout);
    fflush(stdout);
  }

  return 0;
#else

  boost::system::error_code ec;
  boost::asio::ip::tcp::iostream s;

  s.connect(tcp::endpoint(boost::asio::ip::address_v4::loopback(), 2000));
  if (!s)
  {
    fprintf(stderr, "connect(loopback, 2000) failed: %s\n", s.error().message().c_str());
    return 1;
  }

  s.rdbuf()->shutdown(boost::asio::ip::tcp::socket::shutdown_send);
  std::string line;
  while (std::getline(s, line))
  {
    printf("%s\n", line.c_str());
    fflush(stdout);
  }
  ec = s.error();
  if (ec)
  {
    fprintf(stderr, "getline() failed: %s\n", ec.message().c_str());
    return 2;
  }

  return 0;
#endif
}
