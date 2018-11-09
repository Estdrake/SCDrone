#ifndef WIFI_TEST_H
#define WIFI_TEST_H

#include <QNetworkConfigurationManager>
#include <QNetworkConfiguration>
#include <QNetworkSession>


inline void test_find_wifi() {
	QNetworkConfiguration net_cfg;
	QStringList wifi_list;

	QNetworkConfigurationManager ncm;

	QList<QNetworkConfiguration> net_cfg_list = ncm.allConfigurations();
	wifi_list.clear();
	for(auto& x : net_cfg_list)
	{
		if(x.bearerType() == QNetworkConfiguration::BearerWLAN)
		{
			if(x.name() == "")
			{
				qDebug() << "Unknow(Other Network)";
			} else
			{
				qDebug() << x.name() << " " << x.type();
			}
			
		}
	}
}

inline void execute_wifi_test()
{
	test_find_wifi();
}


#endif
