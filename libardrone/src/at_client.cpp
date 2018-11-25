#include "at_client.h"
#include "config.h"

#include <chrono>
#include <regex>

#include "at_cmd.h"


#define AT_CMD_INTERVAL 30

ATClient::ATClient(ATQueue* queue, QObject* parent) : QObject(parent), ref_mode(DEFAULT_FLAG)
{
	this->queue = queue;
	this->sender = new QHostAddress(WIFI_ARDRONE_IP);
	this->port = AT_PORT;
	this->sequence_nbr = 1;
	this->speed_drone = { 0,0,0,0 };
	this->prog_flag = HOVERING;
	this->socket = new QUdpSocket(this);
	this->socket->bind(QHostAddress(WIFI_CLIENT_IP), port);
	connect(socket, &QUdpSocket::readyRead, this, &ATClient::on_read_ready);
}

ATClient::~ATClient()
{
	delete this->socket;
	delete this->sender;
}

void ATClient::setSpeedX(x_direction d, float x)
{
	if (d == LEFT)
		x *= -1.0f;
	speed s = speed_drone;
	s.x = x;
	speed_drone = s;
}

void ATClient::setSpeedY(y_direction d, float y)
{
	if (d == LOWER)
		y *= -1.0f;
	speed s = speed_drone;
	s.y = y;
	speed_drone = s;
}

void ATClient::setSpeedZ(z_direction d, float z)
{
	if (d == FORWARD)
		z *= -1.0f;
	speed s = speed_drone;
	s.z = z;
	speed_drone = s;
}

void ATClient::setSpeedR(x_direction d, float r)
{
	if (d == LEFT)
		r *= -1.0f;
	speed s = speed_drone;
	s.r = r;
	speed_drone = s;
}

void ATClient::hover()
{
	speed_drone = { 0,0,0,0 };
	prog_flag = HOVERING;
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
	while (!stopRequested())
	{
		auto start = std::chrono::high_resolution_clock::now();
		bool has_data = false;
		c = queue->pop_wait(std::chrono::milliseconds(20), &has_data);
		if (!has_data)
		{
			speed s = speed_drone;
			c = at_format_pcmd(prog_flag, s.x, s.z, s.y, s.r);
		}
		cmd.push_back(c);
		cmd.push_back(at_format_ref(this->ref_mode.load()));
		c = "";
		for (auto i : cmd)
		{
			c += std::regex_replace(i, r, std::to_string(sequence_nbr));
			sequence_nbr++;
		}
		QByteArray d = c.c_str();
		int writeSize = socket->writeDatagram(d, d.size(), *sender, port);
		if (writeSize == -1)
		{
			err_write++;
			if (err_write == 3)
			{
				return;
			}
		}
		auto end = std::chrono::high_resolution_clock::now();

		long ellapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		if (ellapsed < AT_CMD_INTERVAL)
		{
			// attend la prochaine execution
			std::this_thread::sleep_for(std::chrono::milliseconds(AT_CMD_INTERVAL - ellapsed));
		}
		cmd.clear();
	}
	qDebug() << "ATClient exiting";
}

void ATClient::on_read_ready()
{
	while (socket->hasPendingDatagrams())
	{
		QByteArray dg;
		dg.resize(socket->pendingDatagramSize());

		socket->readDatagram(dg.data(), dg.size(), sender, &port);
		qDebug() << "Read AT " << dg.data();
	}
}

DroneControl::DroneControl(ATQueue* queue)
{
	this->started.store(false);
	this->queue = queue;
	this->interval_msg = 60ms;
}

void DroneControl::rotate(x_direction x, float speed)
{
	stop();
	started = true;
	this->futureObj = this->exitSignal.get_future();
	std::cout << "Start rotation " << speed << std::endl;
	this->futureCurrent = std::async(std::launch::async, [&, x, speed]()
	{
		float s = speed;
		if (x == LEFT)
			s *= -1.0f;
		while (this->futureObj.wait_for(30ms) == std::future_status::timeout) {
			this->queue->push(at_format_pcmd(HOVERING, 0, 0, 0, s));
		}
		std::cout << "End rotation" << std::endl;
	});;
}

void DroneControl::move_x(x_direction x, float speed)
{
	stop();
	started = true;
	this->futureObj = this->exitSignal.get_future();
	std::cout << "Start moving x " << speed << std::endl;
	this->futureCurrent = std::async(std::launch::async, [&, x, speed]()
	{
		float s = speed;
		if (x == LEFT)
			s *= -1.0f;
		while (this->futureObj.wait_for(30ms) == std::future_status::timeout) {
			this->queue->push(at_format_pcmd(COMBINED_YAW, s, 0, 0, 0));
		}
		std::cout << "End moving x" << std::endl;
	});
}

void DroneControl::move_y(y_direction y, float speed)
{
	stop();
	started = true;
	this->futureObj = this->exitSignal.get_future();
	std::cout << "Start moving y " << speed << std::endl;
	this->futureCurrent = std::async(std::launch::async, [&, y, speed]()
	{
		float s = speed;
		if (y == LOWER)
			s *= -1.0f;
		while (this->futureObj.wait_for(30ms) == std::future_status::timeout) {
			this->queue->push(at_format_pcmd(HOVERING, 0, 0, s, 0));
		}
		std::cout << "End moving y" << std::endl;
	});;
}

void DroneControl::move_z(z_direction z, float speed)
{
	stop();
	started = true;
	this->futureObj = this->exitSignal.get_future();
	std::cout << "Start moving z " << speed << std::endl;
	this->futureCurrent = std::async(std::launch::async, [&,z,speed]()
	{
		float s = speed;
		if (z == FORWARD)
			s *= -1.0f;
		while (this->futureObj.wait_for(30ms) == std::future_status::timeout) {
			this->queue->push(at_format_pcmd(COMBINED_YAW, 0,s, 0, 0));
		}
		std::cout << "End moving z" << std::endl;
	});
}

void DroneControl::move_xy(x_direction x, y_direction y, float speedX, float speedY)
{
	stop();
	started = true;
	this->futureObj = this->exitSignal.get_future();
	std::cout << "Start moving x " << speedX << " y " <<  speedY << std::endl;
	this->futureCurrent = std::async(std::launch::async, [&,x,y,speedX,speedY]()
	{
		float sX = speedX;
		float sY = speedY;
		if (x == LEFT)
			sX *= -1.0f;
		if (y == LOWER)
			sY *= -1.0f;
		while (this->futureObj.wait_for(30ms) == std::future_status::timeout) {
			this->queue->push(at_format_pcmd(COMBINED_YAW, sX, 0, sY, 0));
		}
		std::cout << "End moving xy" << std::endl;
	});
}

void DroneControl::set_interval(milliseconds ms) const
{
	this->set_interval(ms);
}

bool DroneControl::wait_for(milliseconds ms) const
{
	return this->wait_for(ms);
}

void DroneControl::stop()
{
	if (started) {
		std::cout << "Stopping current movement" << std::endl;
		this->exitSignal.set_value();
		this->futureCurrent.wait();
		started = false;
		queue->empty();
		exitSignal = std::promise<void>();
	}
	started = false;
}
