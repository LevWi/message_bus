#ifndef MESSAGE_BUS_H
#define MESSAGE_BUS_H

#include <memory>
#include <unordered_map>
#include <future>

//todo Delete it
#ifndef COMMONAPI_INTERNAL_COMPILATION
#define COMMONAPI_INTERNAL_COMPILATION
#endif

#include "CommonAPI/Variant.hpp"

#undef COMMONAPI_INTERNAL_COMPILATION

using mes_id_t = std::size_t;

template <class T>
constexpr mes_id_t gen_mes_id(mes_id_t id = 0)
{
    //static_assert ( typeid(T).hash_code() != 0 , "");
    return id == 0 ? typeid(T).hash_code() : id;
}

struct None { };

template<class ...Types>
class MessageBus {
public:
    using Box_t = CommonAPI::Variant<None, Types...>;
    using Promise_t = std::promise<Box_t>;
    using Future_t = std::future<Box_t>;

    template<class T>
    struct Order {
        mes_id_t id;
        Future_t fut;
        MessageBus& bus;

        Order(mes_id_t id, Future_t&& f, MessageBus& b) : id(id),
            fut(std::forward<Future_t>(f)), bus(b) { }
        Order(Order&& v) = default;
        Order (const Order &) = delete;
        Order& operator=(const Order &) = delete;

        template<class Duration = std::chrono::milliseconds>
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

        template<class Duration = std::chrono::milliseconds>
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

        Box_t get_box() {
            return fut.get();
        }

        ~Order() {
            if (fut.valid() && id != 0 &&
                    fut.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
                bus.delete_order(id);
            }
        }
    };

    template<class T>
    std::pair<Order<T>, bool> add_order(mes_id_t id = 0) {
        Promise_t prom;
        auto mid = gen_mes_id<T>(id);
        auto ord = Order<T>(mid,  prom.get_future(), *this );

        std::lock_guard<std::mutex> lock(mtx);
        auto result = orders.emplace( std::make_pair( mid, std::move(prom) ) );
        return std::make_pair(std::move(ord), result.second);
    }

    template<class Duration = std::chrono::milliseconds>
    Box_t wait_for(mes_id_t id, Duration period) {
        Promise_t prom;
        auto fut = prom.get_future();
        {
            std::lock_guard<std::mutex> lock(mtx);
            auto result = orders.emplace( std::make_pair( id, std::move(prom) ) );
        }
        Box_t box(None{});
        if (fut.wait_for(period) == std::future_status::ready) {
            box = fut.get();
        } else {
            std::lock_guard<std::mutex> lock(mtx);
            orders.erase(id);
        }
        return box;
    }

    template<class T>
    bool send(T&& value, mes_id_t id = 0) {
        std::lock_guard<std::mutex> lock(mtx);
        bool result = false;
        auto mid = gen_mes_id<T>(id);
        auto it = orders.find(mid);
        if (it != orders.end()) {
            result = true;
            it->second->set_value( Box_t(std::forward<T>(value)) );
            orders.erase(it);
        }
        return result;
    }

    bool delete_order(mes_id_t id) {
        std::lock_guard<std::mutex> lock(mtx);
        return orders.erase(id) != 0;
    }

    std::size_t orders_count() {
        std::lock_guard<std::mutex> lock(mtx);
        return orders.size();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mtx);
        orders.clear();
    }
private:
    std::mutex mtx;
    std::unordered_map<mes_id_t, Promise_t> orders;
};

#endif // MESSAGE_BUS_H
