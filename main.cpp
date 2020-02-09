#include <iostream>
#include "message_bus.h"

using namespace std;
using namespace chrono_literals;

template<class ...Args>
struct TEST {
    std::tuple< std::vector<Args> ...> values;
    std::tuple< std::vector<typename MessageBus<Args...>:: template Order<Args>> ...> orders;
};

int main()
{
    MessageBus<int, float, std::string> bus;

    cout << "bus size = " << sizeof(bus)  << "\n";

    cout << "subs count = " << bus.orders_count()  << "\n";
    {
        TEST<int, float, std::string> test_env;

        auto add_value = [&test_env](auto value) {
            using type_t = typename std::remove_reference<decltype(value)>::type;

            std::get<std::vector<type_t>>(test_env.values).push_back(value);
        };

        auto add_order = [&](int id, auto value) {
            using type_t = typename std::remove_reference<decltype(value)>::type;
            using bus_type_t = typename std::remove_reference<decltype(bus)>::type;

            auto order = bus.add_order<type_t>(id);

            std::get< std::vector< typename bus_type_t:: template Order<type_t>>>(test_env.orders).push_back(std::move(order));
        };

        for(int i = 1 ; i <= 1000 ; i++) {
            int x = i;
            float y = i * 2;
            std::string z = "Test#";
            z.append(std::to_string(i));

            add_value(x);
            add_value(y);
            add_value(z);

            add_order(i, x);
            add_order(i, y);
            add_order(i, z);
        }

        cout << "subs count = " << bus.orders_count()  << endl;
        cout << "==============================\n";
    }
    cout << "subs count = " << bus.orders_count()  << "\n";

    /*
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

    auto check_subscriber_simple = [&bus](auto&& value) {
        using type_t = typename std::remove_reference<decltype(value)>::type;
        auto fut = bus.add_order<type_t>();

        cout << "order size = " << sizeof(fut)  << "\n";

        cout << "value = " << value  << "\n";

        cout << "subs count = " << bus.orders_count()  << "\n";

        auto thrd = std::thread( [&]{
            this_thread::sleep_for(100ms);
            //bus.send(value); //works too
            bus.send(std::move(value));
        } );

        bool res = fut.wait_for(500ms);

        cout << "wait_for result =" << res << "\n";

        cout << "subs count = " << bus.orders_count()  << "\n";

        auto v = fut.get();
        cout << "result =" << v << "\n";

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

    auto check_subscriber_simple2 = [&bus](auto&& value) {
        using type_t = typename std::remove_reference<decltype(value)>::type;
        auto fut = bus.add_order<type_t>();

        cout << "value = " << value  << "\n";

        cout << "subs count = " << bus.orders_count()  << "\n";

        auto thrd = std::thread( [&]{
            this_thread::sleep_for(100ms);
            bus.send(value);
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

    cout << "==============================\n";
    cout << "==============================\n";
    cout << "==============================\n";

    auto check_subscriber_simple3 = [&bus](auto&& value) {
        using type_t = typename std::remove_reference<decltype(value)>::type;
        auto fut = bus.add_order<type_t>();

        cout << "value = " << value  << "\n";

        cout << "subs count = " << bus.orders_count()  << "\n";

        auto thrd = std::thread( [&]{
            this_thread::sleep_for(100ms);
            bus.send(value);
        } );

        type_t v {};

        auto res = fut.get(v);

        cout << "fut.get(v) result =" << res << "\n";

        cout << "subs count = " << bus.orders_count()  << "\n";

        cout << "result =" << v << "\n";

        thrd.join();

        cout << "==============================\n";
    };

    check_subscriber_simple3(std::string("TEST1"));
    check_subscriber_simple3(343434);
    check_subscriber_simple3(4444l);
    check_subscriber_simple3(5555.0);

    cout << "subs count = " << bus.orders_count()  << "\n";
    {
        auto fut = bus.add_order<double>();
        cout << "subs count = " << bus.orders_count()  << "\n";
    }
    cout << "subs count = " << bus.orders_count()  << "\n";

    */
    return 0;
}
