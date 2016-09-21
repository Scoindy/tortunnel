/*-
 * Copyright (c) 2009, Moxie Marlinspike
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of this program nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "Network.h"

using namespace boost::asio;

void Network::socketReadComplete(boost::shared_ptr<ip::tcp::socket> socket,
				 boost::shared_ptr<char> buf,
				 std::string *result,
				 SocketSuckHandler handler,
				 const boost::system::error_code &err,
				 size_t transferred)
{
  if (err) {
    handler(boost::system::error_code());
    return;
  }

  result->append(buf.get(), transferred);
  socketReadResponse(socket, buf, result, handler);
}

void Network::socketReadResponse(boost::shared_ptr<ip::tcp::socket> socket,
				 boost::shared_ptr<char> buf,
				 std::string *result,
				 SocketSuckHandler handler)
{
  socket->async_read_some(boost::asio::buffer(buf.get(), 1024),
			  boost::bind(&Network::socketReadComplete,
				      socket, buf, result, handler, 
				      _1,
				      placeholders::bytes_transferred));
}

void Network::requestSent(boost::shared_ptr<ip::tcp::socket> socket,
			  boost::shared_ptr<std::string> request,
			  std::string *result,
			  SocketSuckHandler handler,
			  const boost::system::error_code &err)
{
  if (err) {
    handler(err);
    return;
  }

  boost::shared_ptr<char> buf(new char[1024]);
  socketReadResponse(socket, buf, result, handler);
}


void Network::sockConnected(boost::shared_ptr<ip::tcp::socket> socket,
			    boost::shared_ptr<std::string> request,
			    std::string *result,
			    SocketSuckHandler handler, 
			    const boost::system::error_code &err)
{
  if (err) {
    handler(err);
    return;
  }

  async_write(*socket, boost::asio::buffer(*request), 
	      boost::bind(&Network::requestSent, socket, request, 
			  result, handler, _1));
}


void Network::suckUrlToString(boost::asio::io_service &io_service,
			      std::string &ip,
			      int port,
			      boost::shared_ptr<std::string> request, 
			      std::string *result, 
			      SocketSuckHandler handler)
{
  boost::shared_ptr<ip::tcp::socket> socket(new ip::tcp::socket(io_service));
  ip::tcp::endpoint server(ip::address_v4::from_string(ip), port);
  socket->async_connect(server, boost::bind(&Network::sockConnected, socket, request, result, 
					    handler, _1));
}
