#include <iostream>
#include "message_bus.h"

using namespace std;

int main()
{
    MessageBus bus;

    auto fut = bus.add_order<int>();
    bus.add_order<float>();

    auto fut3 = bus.add_order<std::string>();

    cout << "size = " << sizeof (bus) << "\n";

    auto t2 = std::thread( [&]{ bus.send(float(10));

        cout << "t2 OK \n";

    } );

    auto t3 = std::thread( [&]{ bus.send(std::string("OKKK!!!!"));

        cout << "t2 OK \n";

    } );

    auto t = std::thread( [&]{ bus.send(int(10)); } );

    auto res = fut.get();
    t.join();
    t2.join();
    t3.join();
    cout << "res = " << ((Message<int>*)res.get())->payload << "\n";
    cout << "res = " << ((Message<std::string>*)fut3.get().get())->payload << "\n";

    return 0;
}
