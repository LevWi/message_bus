#include <iostream>
#include "message_bus.h"

using namespace std;
using namespace chrono_literals;

int main()
{
    MessageBus bus;

    {
        std::promise<int> p;

        {
            p.get_future();
            //p.get_future(); Error std::future_error: Future already retrieved
        }

        auto thrd = std::thread( [p = std::move(p)]() mutable {
            this_thread::sleep_for(100ms);
            p.set_value(10);
            //p.get_future(); Error std::future_error: Future already retrieved
            cout << "OK" << endl;
        } );

        thrd.join();
    }

    {
        std::future<int> f;
        {
            std::promise<int> p;
            f = p.get_future();
            cout << (f.valid() ? "future valid" : "future unvalid") << endl;
        }
        cout << (f.valid() ? "future valid" : "future unvalid") << endl;
        //f.get(); Error std::future_error: Broken promise
    }

    auto check_subscriber_simple = [&bus](auto value) {
        using type_t = typename std::remove_reference<decltype(value)>::type;
        auto fut = bus.add_order<type_t>();

        cout << "value = " << value  << "\n";

        cout << "subs count = " << bus.orders_count()  << "\n";

        auto thrd = std::thread( [&]{
            this_thread::sleep_for(100ms);
            //bus.send(value); //TODO only move works well
            bus.send(std::move(value));
        } );

        bool res = fut.wait_for(500ms);

        cout << "wait_for result =" << res << "\n";

        cout << "subs count = " << bus.orders_count()  << "\n";

        cout << "result =" << fut.get()->payload << "\n";

        thrd.join();

        cout << "==============================\n";
    };


    check_subscriber_simple(std::string("TEST1"));
    check_subscriber_simple(41);
    check_subscriber_simple(42l);
    check_subscriber_simple(43.0);


    cout << "==============================\n";
    cout << "==============================\n";
    cout << "==============================\n";

    auto check_subscriber_simple2 = [&bus](auto value) {
        using type_t = typename std::remove_reference<decltype(value)>::type;
        auto fut = bus.add_order<type_t>();

        cout << "value = " << value  << "\n";

        cout << "subs count = " << bus.orders_count()  << "\n";

        auto thrd = std::thread( [&]{
            this_thread::sleep_for(100ms);
            bus.send(std::move(value));
        } );

        type_t v {};
        bool res = fut.wait_for(500ms, v);

        cout << "wait_for result =" << res << "\n";

        cout << "subs count = " << bus.orders_count()  << "\n";

        cout << "result =" << v << "\n";

        thrd.join();

        cout << "==============================\n";
    };

    check_subscriber_simple2(std::string("TEST1"));
    check_subscriber_simple2(343434);
    check_subscriber_simple2(4444l);
    check_subscriber_simple2(5555.0);

    return 0;
}
