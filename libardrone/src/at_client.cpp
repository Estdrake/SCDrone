#include "at_client.h"
#include "config.h"

#include <chrono>
#include <regex>

#include "at_cmd.h"


	#define AT_CMD_INTERVAL 30

	ATClient::ATClient(ATQueue* queue,QObject* parent) : QObject(parent),ref_mode(DEFAULT_FLAG)
	{
		this->queue = queue;
		this->sender = new QHostAddress(WIFI_ARDRONE_IP);
		this->port = AT_PORT;
		this->sequence_nbr = 1;
		this->socket = new QUdpSocket(this);
		this->socket->bind(QHostAddress(WIFI_CLIENT_IP), port);
		connect(socket, &QUdpSocket::readyRead, this, &ATClient::on_read_ready);
	}
		
	ATClient::~ATClient()
	{
		if(this->socket != nullptr)
		{
			delete this->socket;
		}
		delete this->sender;
	};

	std::thread ATClient::start()
	{

		return std::thread([this] { this->run_service(); });
	}

void ATClient::set_ref(ref_flags f)
{
	this->ref_mode.store(f);
}

void ATClient::run_service()
	{
	int err_write = 0;
		std::vector<std::string> cmd;
		std::string c;
		std::regex r = std::regex("\\$seq");
		for(;;)
		{
			auto start = std::chrono::high_resolution_clock::now();
			bool has_data = false;
			c = queue->pop_wait(std::chrono::milliseconds(20),&has_data);
			if(!has_data)
			{
				c = at_format_pcmd(HOVERING, 0, 0, 0, 0);
			}
			if(c[0] == '@')
			{
				break;
			}
			cmd.push_back(c);
			cmd.push_back(at_format_ref(this->ref_mode.load()));
			c = "";
			for (auto i : cmd) {
				c += std::regex_replace(i, r, std::to_string(sequence_nbr));
				sequence_nbr++;
			}
			QByteArray d = c.c_str();
			int writeSize = socket->writeDatagram(d, d.size(), *sender, port);
			if(writeSize == -1)
			{
				err_write++;
				if(err_write == 3)
				{
					return;
				}
				
			}
			auto end = std::chrono::high_resolution_clock::now();

			long ellapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
			if(ellapsed < AT_CMD_INTERVAL)
			{
				// attend la prochaine execution
				std::this_thread::sleep_for(std::chrono::milliseconds(AT_CMD_INTERVAL - ellapsed));
			}
			cmd.clear();
		}

	}

	void ATClient::on_read_ready()
	{
		while(socket->hasPendingDatagrams())
		{
			QByteArray dg;
			dg.resize(socket->pendingDatagramSize());

			socket->readDatagram(dg.data(), dg.size(), sender, &port);
			qDebug() << "Read AT " << dg.data();
		}
	}
