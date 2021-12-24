#if defined(WIN32)
#include <tchar.h>
#endif

#include <cstdlib>
#include <iostream>
#include <chrono>
#include <thread>
#include <boost/numeric/conversion/cast.hpp>
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
    std::cerr << "server: accept() failed: " << ec << '\n';
    return;
  }
  boost::asio::write(s, boost::asio::buffer("start\n", 6), ec);
  if (ec)
  {
    std::cerr << "server: write() failed: " << ec << '\n';
    return;
  }
  std::this_thread::sleep_for(std::chrono::seconds(130));
  boost::asio::write(s, boost::asio::buffer("end\n", 4), ec);
  if (ec)
  {
    std::cerr << "server: write() failed: " << ec << '\n';
    return;
  }

  s.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
  if (ec)
  {
    std::cerr << "server: shutdown(shutdown_send) failed: " << ec << '\n';
  }

  // Wait for EOF before closing connection
  do
  {
    char buffer[512];
    s.read_some(boost::asio::buffer(buffer, sizeof(buffer)), ec);
  } while (!ec);

  if (boost::asio::error::eof != ec)
  {
    std::cerr << "server: read_some() failed: " << ec << '\n';
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
    std::cerr << "client: connect(loopback, " <<  port << ") failed: " << ec << '\n';
    return 1;
  }

  for (;;)
  {
    char buffer[512];
    size_t n = s.read_some(boost::asio::buffer(buffer, sizeof(buffer)), ec);
    if (ec && boost::asio::error::eof != ec)
    {
      std::cerr << "client: read_some() failed: " << ec << '\n';
      return 3;
    }

    std::cout.write(buffer, boost::numeric_cast<std::streamsize>(n));
    std::cout.flush();

    if (boost::asio::error::eof == ec)
    {
      break;
    }
  }

  s.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
  if (ec)
  {
    std::cerr << "client: shutdown(shutdown_send) failed: " << ec << '\n';
    return 2;
  }

#else

  boost::system::error_code ec;
  boost::asio::ip::tcp::iostream s;

  s.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::loopback(), port));
  if (!s)
  {
    std::cerr << "client: connect(loopback, " <<  port << ") failed: " << ec << '\n';
    return 1;
  }

  std::string line;
  while (std::getline(s, line))
  {
    std::cout << line << std::endl;
  }
  ec = s.error();
  if (ec && boost::asio::error::eof != ec)
  {
    std::cerr << "client: getline() failed: " << ec << '\n';
    return 2;
  }

  s.rdbuf()->shutdown(boost::asio::ip::tcp::socket::shutdown_send);

#endif

  return EXIT_SUCCESS;
}
