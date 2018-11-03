
#include "navdata_client.h"
#include "config.h"
#include "navdata_common.h"

using boost::asio::ip::udp;
NavDataClient::NavDataClient(boost::asio::io_service& ios) : socket(ios,udp::endpoint(udp::v4(),NAVDATA_PORT)) {
    local = udp::endpoint(boost::asio::ip::address::from_string(WIFI_CLIENT_IP),NAVDATA_PORT);
    remote = udp::endpoint(boost::asio::ip::address::from_string(WIFI_ARDRONE_IP),NAVDATA_PORT);

}

NavDataClient::~NavDataClient() {

}


void NavDataClient::run_service() {
    std::cout << "Démarrage navdata sur " << local << std::endl;
    int nbrRead;
    boost::array<char,1> send_buf = { 0 };
    try {
        socket.connect(remote);
        socket.send(boost::asio::buffer(send_buf));
        nbrRead = socket.receive(boost::asio::buffer(recv_buf));
    } catch( const boost::system::system_error& ex) {
        std::cerr << "NAVDATA impossible de ce connecter a " << local << std::endl;
        return;
    }
    //BOOST_LOG_TRIVIAL(info) << "Démarrage navdata sur " << local;
    socket.close();
}