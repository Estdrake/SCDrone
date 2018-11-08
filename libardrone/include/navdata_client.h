#ifndef _NAVDATA_CLIENT_H_
#define _NAVDATA_CLIENT_H_

#include "navdata_client.h"
#include "config.h"
#include "navdata_common.h"
#include "thread.h"

#include <iostream>

class NavDataClient{

    public:
        NavDataClient();
        ~NavDataClient();

        void run_service();

    private:

};

#endif // _NAVDATA_CLIENT_H_
