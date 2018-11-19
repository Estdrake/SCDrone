
#include "navdata_client.h"
#include "config.h"
#include "navdata_common.h"
#include <thread>
#include "at_cmd.h"

#define NAVDATA_HEADER 0x55667788
#define NAVDATA_HEADER_SIZE 16

void print_navdata_dump(navdata_t* nd)
{
	printf("ARDrone state : %d\n", nd->ardrone_state);
	printf("Sequence : %d\n", nd->sequence);
	printf("Vision defined : %d\n", nd->vision_defined);
}

void print_navdata_demo_dump(navdata_demo_t* demo)
{
	printf("Demo size : %d\n", demo->size);
	printf("Control state %d\n", demo->ctrl_state);
	printf("Battery voltage (mV) %d\n", demo->vbat_flying_percentage);
	printf("Theta %f Phi %f Psi %f\n", demo->theta,demo->phi, demo->psi);
	printf("Altitude %d\n", demo->altitude);
	printf("Velocity X : %f Y : %f Z %f\n", demo->vx, demo->vy, demo->vz);
}

bool start_by_navdata_header(const uint8_t* buffer)
{
	return (buffer[0] == 0x88 && buffer[1] == 0x77 && buffer[2] == 0x66 && buffer[3] == 0x55);
}

NavDataClient::NavDataClient(NAVQueue* queue) : local_endpoint(udp::v4(),NAVDATA_PORT),socket(ios,local_endpoint) {
	this->queue = queue;
	this->remote_endpoint = udp::endpoint(ip::address::from_string(WIFI_ARDRONE_IP), NAVDATA_PORT);
	this->recv_buffer = new uint8_t[542];
}

NavDataClient::~NavDataClient()
{
	delete recv_buffer;
}


void NavDataClient::init_communication()
{
	uint8_t flag = 1, len = sizeof(flag);
	socket.send_to(buffer(&flag, 1), remote_endpoint);
}


void NavDataClient::run_service() {

	init_communication();
	int  rcv;
	navdata_t navdata;
	navdata_demo_t nd_demo;
	while (!stopRequested())
	{

		try {
			rcv = socket.receive(buffer(recv_buffer, 4086));
		} catch(asio::system_error& ex)
		{
			continue;
		}
		if (rcv > sizeof(navdata_t))
		{
			memcpy(&navdata, recv_buffer, sizeof(navdata_t));
			if(navdata.header == NAVDATA_HEADER)
			{
				//print_navdata_dump(&navdata);
				for(int i = NAVDATA_HEADER_SIZE; i < rcv-4;i+=4) // saute les deux shorts du header d'une trame
				{
					navdata_tag_t tag = (navdata_tag_t)recv_buffer[i];
					unsigned short size = (unsigned short)recv_buffer[i + 2];
					if(tag == NAVDATA_DEMO && size > 0)
					{
						memcpy(&nd_demo, recv_buffer+i, sizeof(navdata_demo_t));
						//print_navdata_demo_dump(&nd_demo);
						queue->push(nd_demo);
						break;
					}
					
				}
				
			}


		}



	}
}
