#ifndef MESSAGE_BUS_H
#define MESSAGE_BUS_H

#include <memory>
#include <vector>
#include <future>

using mes_id_t = std::size_t;

template <class T>
constexpr mes_id_t gen_mes_id(mes_id_t id = 0)
{
    if (id == 0) {
        return typeid(T).hash_code();
    }
    return id;
}


struct MessageI {
    virtual ~MessageI() = default;
};


template<class T>
struct Message : MessageI {
    using payload_type = T;

    //Message(T&& v) : payload(std::forward<T>(v)) { }      TODO not worked well

    Message(T&& v) {
        payload = std::forward<T>(v);
    }

    T payload;
};



class MessageBus {
    using Promise_t = std::promise<std::unique_ptr<MessageI>>;
    using Future_t = std::future<std::unique_ptr<MessageI>>;

    std::mutex mtx;

    std::vector<std::pair<mes_id_t, Promise_t>> consumers;

public:
    template<class T>
    struct FutureWrapper {
        Future_t fut;

        FutureWrapper(Future_t&& f) : fut(std::forward<Future_t>(f)) {}

        template<class Duration = std::chrono::microseconds>
        bool wait_for(Duration period, T& value) {
            if (fut.wait_for(period) == std::future_status::ready) {
                if (auto ptr = fut.get()) {
                    value = std::move(static_cast<Message<T>*>(ptr.get())->payload);
                    return true;
                }
            }
            return false;
        }

        template<class Duration = std::chrono::microseconds>
        bool wait_for(Duration period) {
            if (fut.wait_for(period) == std::future_status::ready) {
                return true;
            }
            return false;
        }

        std::unique_ptr<Message<T>> get() {
            auto ptr = fut.get();
            return std::unique_ptr<Message<T>>( static_cast<Message<T>*>(ptr.release()) );
        }
    };


    template<class T, mes_id_t id = 0>
    FutureWrapper<T> add_order() {
        Promise_t prom;
        auto fut = FutureWrapper<T>( prom.get_future() );
        std::lock_guard<std::mutex> lock(mtx);
        consumers.push_back( std::make_pair( gen_mes_id<T>(id), std::move(prom) ) );
        return fut;
    }

    template<class T>
    bool send(T&& value, mes_id_t id = 0) {
        std::lock_guard<std::mutex> lock(mtx);
        bool result = false;
        const auto expected_id = gen_mes_id<T>(id);
        for(auto el = consumers.begin() ; el != consumers.end() ; el++ ) {
            if (el->first == expected_id) {
                auto ptr = new Message<T>(std::forward<T>(value));
                el->second.set_value( std::unique_ptr<MessageI>(
                                          static_cast<MessageI*>( ptr ) ));
                result = true;
                consumers.erase(el);
                break;
            }
        }
        return result;
    }

    auto orders_count() {
        std::lock_guard<std::mutex> lock(mtx);
        return consumers.size();
    }
};

#endif // MESSAGE_BUS_H
