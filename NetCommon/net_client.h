#pragma once
#include "net_common.h"
#include "net_message.h"
#include "net_tsqueue.h"
#include "net_connection.h"

namespace olc
{
	namespace net
	{
		// setting up asio and setting up the connection
		template <typename T>
		class client_interface
		{

			client_interface() : m_socket(m_context)
			{
				// initialise the socket with the io context, so it can do stuff
			}

			virtual ~client_interface()
			{
				Disconnect();
			}

		public:

			// connect to server with hostname/ip-address and port
			bool Connect(const std::string& host, const uint16_t port)
			{
				try
				{
					// create connection
					m_connection = std::make_unique<connection<T>>(); // todo

					// resolve hostname/ip-address into tangiable physical address
					asio::ip::tcp::resolver resolver(m_context);
					m_endpoints = resolver.resolve(host, std::to_string(port));

					// tell the connection object to connect to server
					m_connection->ConnectToServer(m_endpoints);

					// start context thread
					thrContext = std::thread([this]() {m_context.run(); })
				}
				catch (std::exception& e)
				{
					std::cerr << "Client Exception: " << e.what() << "\n";
					return false;
				}

				return true;
			}

			// disconnect from server
			void Disconnect()
			{
				if (IsConnected())
				{
					m_connection->Disconnect();
				}

				m_context.stop();
				if (thrContext.joinable())
					thrContext.join();

				// destroy the connection object
				m_connection.release();
			}

			bool IsConnected()
			{
				if (m_connection)
					return m_connection->IsConnected();
				else
					return false;
			}

			tsqueue<owned_message<T>>& Incoming()
			{
				return m_qMessagesIn;
			}

		protected:
			// asio context handles the data transfer....
			asio::io_context m_context;

			// needs a thread of its own to execute its work commands
			std::thread thrContext;

			// this is the hardware socket that is conneced to the server
			asio::ip::tcp::socket m_socket;

			//the client has a single instance of a "connection" object, which handles data transfer
			std::unique_ptr<connection<T>> m_connection;

		private:
			// this is the thread safe queue of incoming messages from server
			tsqueue<owned_message<T>> m_qMessagesIn;
			
		};
	}
}