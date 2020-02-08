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
    virtual ~MessageI() {};
};


template<class T>
struct Message : MessageI {
    using payload_type = T;

    Message(T&& v) : payload(std::forward<T>(v)) {

    }

    T payload;

    ~Message() {}
};


class MessageBus {
    using Promise_t = std::promise<std::unique_ptr<MessageI>>;

    std::mutex mtx;

    std::vector<std::pair<mes_id_t, Promise_t>> consumers;


public:
    template<class T, mes_id_t id = 0>
    std::future<std::unique_ptr<MessageI>> add_order() {
        Promise_t prom;
        auto fut = prom.get_future();
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
                el->second.set_value( std::unique_ptr<MessageI>(
                                          static_cast<MessageI*>( new Message<T>( std::forward<T>(value) ) ) ) );
                result = true;
                consumers.erase(el);
                break;
            }
        }
        return result;
    }
};

#endif // MESSAGE_BUS_H
