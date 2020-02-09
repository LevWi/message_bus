#ifndef MESSAGE_BUS_H
#define MESSAGE_BUS_H

#include <memory>
#include <vector>
#include <future>

//todo Delete it
#ifndef COMMONAPI_INTERNAL_COMPILATION
#define COMMONAPI_INTERNAL_COMPILATION
#endif

//#include "CommonAPI/CommonAPI.hpp"
#include "CommonAPI/Variant.hpp"

#undef COMMONAPI_INTERNAL_COMPILATION

using mes_id_t = std::size_t;

template <class T>
constexpr mes_id_t gen_mes_id(mes_id_t id = 0)
{
    //TODO static_assert ( typeid(T).hash_code() != 0 , "");
    if (id == 0) {
        return typeid(T).hash_code();
    }
    return id;
}


template<class ...Types>
class MessageBus {
public:
    using Box_t = CommonAPI::Variant<Types...>;
    using Promise_t = std::promise<Box_t>;
    using Future_t = std::future<Box_t>;

    template<class T>
    struct Order {
        mes_id_t id;
        Future_t fut;
        MessageBus& bus;

        Order(mes_id_t id, Future_t&& f, MessageBus& b) : id(id), fut(std::move(f)), bus(b) { }
        Order(Order&& v) = default;
        Order (const Order &) = delete;
        Order& operator=(const Order &) = delete;

        template<class Duration = std::chrono::microseconds>
        bool wait_for(Duration period, T& value) {
            if (fut.wait_for(period) == std::future_status::ready) {
                id = 0;
                Box_t v = fut.get();
                if ( v.template isType<T>()) {
                    value = std::move(v.template get<T>());
                    return true;
                }
            }
            return false;
        }

        template<class Duration = std::chrono::microseconds>
        bool wait_for(Duration period) {
            if (fut.wait_for(period) == std::future_status::ready) {
                id = 0;
                return true;
            }
            return false;
        }

        T get() {
            return fut.get().template get<T>();
        }

        bool get(T& v) {
            Box_t c = fut.get();
            if ( c.template isType<T>()) {
                v = std::move(c.template get<T>());
                return true;
            }
            return false;
        }

        Box_t get_container() {
            return fut.get();
        }

        ~Order() {
            if (fut.valid() && id != 0 &&
                    fut.wait_for(std::chrono::microseconds(0)) != std::future_status::ready) {
                bus.delete_order(id);
            }
        }
    };


    template<class T>
    Order<T> add_order(mes_id_t id = 0) {
        Promise_t prom;
        auto mid = gen_mes_id<T>(id);
        auto ord = Order<T>(mid,  prom.get_future(), *this );

        std::lock_guard<std::mutex> lock(mtx);
        consumers.push_back( std::make_pair( mid, std::move(prom) ) );
        return ord;
    }

    template<class T>
    bool send(T&& value, mes_id_t id = 0) {
        std::lock_guard<std::mutex> lock(mtx);
        bool result = false;
        const auto expected_id = gen_mes_id<T>(id);
        for(auto el = consumers.begin() ; el != consumers.end() ; el++ ) {
            if (el->first == expected_id) {
                el->second.set_value( Box_t(std::forward<T>(value)) );
                result = true;
                consumers.erase(el);
                break;
            }
        }
        return result;
    }

    bool delete_order(mes_id_t id) {
        std::lock_guard<std::mutex> lock(mtx);
        bool result = false;
        for(auto el = consumers.begin() ; el != consumers.end() ; el++ ) {
            if (el->first == id) {
                consumers.erase(el);
                result = true;
                break;
            }
        }
        return result;
    }

    std::size_t orders_count() {
        std::lock_guard<std::mutex> lock(mtx);
        return consumers.size();
    }
private:
    std::mutex mtx;
    std::vector<std::pair<mes_id_t, Promise_t>> consumers;
};

#endif // MESSAGE_BUS_H
