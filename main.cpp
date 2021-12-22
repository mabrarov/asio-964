#if defined(WIN32)
#include <tchar.h>
#endif

#include <cstdlib>
#include <iostream>
#include <chrono>
#include <thread>
#include <boost/asio.hpp>

namespace {

const boost::asio::ip::port_type port = 2000;

void server()
{
  boost::asio::io_service io_service;
  boost::system::error_code ec;
  boost::asio::ip::tcp::acceptor server(io_service,
      boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::loopback(), port));
  boost::asio::ip::tcp::socket s(io_service);

  server.accept(s, ec);
  if (ec)
  {
    std::cerr << "accept() failed: " << ec << '\n';
    return;
  }
  boost::asio::write(s, boost::asio::buffer("start\n", 6), ec);
  if (ec)
  {
    std::cerr << "write() failed: " << ec << '\n';
    return;
  }
  std::this_thread::sleep_for(std::chrono::seconds(130));
  boost::asio::write(s, boost::asio::buffer("end\n", 4), ec);
  if (ec)
  {
    std::cerr << "write() failed: " << ec << '\n';
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

  s.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::loopback(), port), ec);
  if (ec)
  {
    std::cerr << "connect(loopback, " <<  port << ") failed: " << ec << '\n';
    return 1;
  }

  s.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
  if (ec)
  {
    std::cerr << "shutdown(shutdown_send) failed: " << ec << '\n';
    return 2;
  }

  for (;;)
  {
    char buffer[512];
    size_t n = s.read_some(boost::asio::buffer(buffer, sizeof(buffer)), ec);
    if (ec)
    {
      std::cerr << "read_some() failed: " << ec << '\n';
      return 3;
    }

    std::cout.write(buffer, n);
    std::cout.flush();
  }

#else

  boost::system::error_code ec;
  boost::asio::ip::tcp::iostream s;

  s.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::loopback(), port));
  if (!s)
  {
    std::cerr << "connect(loopback, " <<  port << ") failed: " << ec << '\n';
    return 1;
  }

  s.rdbuf()->shutdown(boost::asio::ip::tcp::socket::shutdown_send);
  std::string line;
  while (std::getline(s, line))
  {
    std::cout << line << std::endl;
  }
  ec = s.error();
  if (ec)
  {
    std::cerr << "getline() failed: " << ec << '\n';
    return 2;
  }

#endif

  return EXIT_SUCCESS;
}
