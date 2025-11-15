#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <stdexcept>

// Reactor 模式核心组件
class EventHandler {
public:
    virtual ~EventHandler() = default;
    virtual void handle_event(int fd, uint32_t events) = 0;
};

class EventDemultiplexer {
public:
    virtual ~EventDemultiplexer() = default;
    virtual int wait_for_events(std::vector<epoll_event>& events, int timeout) = 0;
    virtual void register_event(int fd, uint32_t events) = 0;
    virtual void modify_event(int fd, uint32_t events) = 0;
    virtual void remove_event(int fd) = 0;
};

class EpollDemultiplexer : public EventDemultiplexer {
public:
    EpollDemultiplexer();
    ~EpollDemultiplexer();
    
    int wait_for_events(std::vector<epoll_event>& events, int timeout) override;
    void register_event(int fd, uint32_t events) override;
    void modify_event(int fd, uint32_t events) override;
    void remove_event(int fd) override;
    
private:
    int epoll_fd;
};

// 线程池和工作队列
class ThreadPool {
public:
    ThreadPool(size_t num_threads = 8);
    ~ThreadPool();
    
    template<typename F>
    void enqueue(F&& task);
    
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

class Reactor {
public:
    Reactor();
    ~Reactor();
    
    void register_handler(int fd, EventHandler* handler, uint32_t events);
    void modify_handler(int fd, uint32_t events);
    void remove_handler(int fd);
    void run();
    void stop();
    
private:
    std::unique_ptr<EventDemultiplexer> demultiplexer;
    std::unordered_map<int, EventHandler*> handlers;
    std::atomic<bool> running;
    std::thread reactor_thread;
    ThreadPool threads_pool;
    void event_loop();
};


// 具体事件处理器
class Acceptor : public EventHandler {
public:
    Acceptor(Reactor& reactor, int port);
    ~Acceptor();
    
    void handle_event(int fd, uint32_t events) override;
    
private:
    Reactor& reactor;
    int listen_fd;
    
    void setup_listener(int port);
};

class Connection : public EventHandler {
public:
    Connection(Reactor& reactor, int fd);
    ~Connection();
    
    void handle_event(int fd, uint32_t events) override;
    void send_data(const std::string& data);
    
private:
    Reactor& reactor;
    int conn_fd;
    std::string buffer;
    std::mutex buffer_mutex;
    
    void handle_read();
    void handle_write();
};

// ===================== 实现部分 =====================

// EpollDemultiplexer 实现
EpollDemultiplexer::EpollDemultiplexer() {
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        throw std::runtime_error("Failed to create epoll instance");
    }
}

EpollDemultiplexer::~EpollDemultiplexer() {
    if (epoll_fd != -1) {
        close(epoll_fd);
    }
}

int EpollDemultiplexer::wait_for_events(std::vector<epoll_event>& events, int timeout) {
    return epoll_wait(epoll_fd, events.data(), events.size(), timeout);
}

void EpollDemultiplexer::register_event(int fd, uint32_t events) {
    epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        throw std::runtime_error("Failed to register event");
    }
}

void EpollDemultiplexer::modify_event(int fd, uint32_t events) {
    epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) == -1) {
        throw std::runtime_error("Failed to modify event");
    }
}

void EpollDemultiplexer::remove_event(int fd) {
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
        throw std::runtime_error("Failed to remove event");
    }
}

// Reactor 实现
Reactor::Reactor() : running(false) {
    demultiplexer = std::make_unique<EpollDemultiplexer>();
}

Reactor::~Reactor() {
    stop();
}

void Reactor::register_handler(int fd, EventHandler* handler, uint32_t events) {
    handlers[fd] = handler;
    demultiplexer->register_event(fd, events);
}

void Reactor::modify_handler(int fd, uint32_t events) {
    auto it = handlers.find(fd);
    if (it != handlers.end()) {
        demultiplexer->modify_event(fd, events);
    }
}

void Reactor::remove_handler(int fd) {
    demultiplexer->remove_event(fd);
    auto it = handlers.find(fd);
    if (it != handlers.end()) {
        handlers.erase(it);
    }
}

void Reactor::run() {
    if (running) return;
    
    running = true;
    reactor_thread = std::thread(&Reactor::event_loop, this);
}

void Reactor::stop() {
    if (!running) return;
    
    running = false;
    if (reactor_thread.joinable()) {
        reactor_thread.join();
    }
}

void Reactor::event_loop() {
    const int MAX_EVENTS = 64;
    std::vector<epoll_event> events(MAX_EVENTS);
    
    while (running) {
        int num_events = demultiplexer->wait_for_events(events, 100); // 100ms timeout
        
        for (int i = 0; i < num_events; ++i) {
            int fd = events[i].data.fd;
            uint32_t revents = events[i].events;
            
            auto it = handlers.find(fd);
            if (it != handlers.end()) {
                threads_pool.enqueue([fd, revents, it]() {
                    it->second->handle_event(fd, revents);
                });
                // it->second->handle_event(fd, revents); 
            }
        }
    }
}

// ThreadPool 实现
ThreadPool::ThreadPool(size_t num_threads) : stop(false) {
    for (size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    condition.wait(lock, [this] {
                        return stop || !tasks.empty();
                    });
                    
                    if (stop && tasks.empty()) {
                        return;
                    }
                    
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    
    condition.notify_all();
    for (std::thread& worker : workers) {
        worker.join();
    }
}

template<typename F>
void ThreadPool::enqueue(F&& task) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.emplace(std::forward<F>(task));
    }
    condition.notify_one();
}

// Acceptor 实现
Acceptor::Acceptor(Reactor& reactor, int port) : reactor(reactor) {
    setup_listener(port);
    reactor.register_handler(listen_fd, this, EPOLLIN | EPOLLET);
}

Acceptor::~Acceptor() {
    if (listen_fd != -1) {
        close(listen_fd);
    }
}

void Acceptor::setup_listener(int port) {
    listen_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (listen_fd == -1) {
        throw std::runtime_error("Failed to create socket");
    }
    
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(listen_fd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        close(listen_fd);
        throw std::runtime_error("Failed to bind socket");
    }
    
    if (listen(listen_fd, SOMAXCONN) == -1) {
        close(listen_fd);
        throw std::runtime_error("Failed to listen on socket");
    }
}

void Acceptor::handle_event(int fd, uint32_t events) {
    if (events & (EPOLLERR | EPOLLHUP)) {
        // 处理错误
        reactor.remove_handler(fd);
        return;
    }
    
    // 接受所有挂起的连接
    while (true) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int conn_fd = accept4(listen_fd, (sockaddr*)&client_addr, &client_len, SOCK_NONBLOCK);
        
        if (conn_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 所有连接已处理
                break;
            } else {
                // 处理错误
                perror("accept");
                break;
            }
        }
        
        // 创建新的连接处理器
        auto conn_handler = new Connection(reactor, conn_fd);
        reactor.register_handler(conn_fd, conn_handler, EPOLLIN | EPOLLET | EPOLLRDHUP);
    }
}

// Connection 实现
Connection::Connection(Reactor& reactor, int fd) 
    : reactor(reactor), conn_fd(fd) {}

Connection::~Connection() {
    if (conn_fd != -1) {
        close(conn_fd);
    }
}

void Connection::handle_event(int fd, uint32_t events) {
    if (events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
        // 连接关闭或出错
        reactor.remove_handler(fd);
        delete this;
        return;
    }
    
    if (events & EPOLLIN) {
        handle_read();
    }
    
    if (events & EPOLLOUT) {
        handle_write();
    }
}

void Connection::handle_read() {
    char buf[4096];
    ssize_t bytes_read;
    
    while (true) {
        bytes_read = read(conn_fd, buf, sizeof(buf));
        if (bytes_read == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 所有数据已读取
                break;
            } else {
                // 读取错误
                reactor.remove_handler(conn_fd);
                delete this;
                return;
            }
        } else if (bytes_read == 0) {
            // 客户端关闭连接
            reactor.remove_handler(conn_fd);
            delete this;
            return;
        }
        
        // 处理数据（这里简单回显）
        // {
        //     std::lock_guard<std::mutex> lock(buffer_mutex);
        //     buffer.append(buf, bytes_read);
        // }
        std::cout << buffer.size() << std::endl;
        send_data(buffer); // 回显数据
    }
}

void Connection::handle_write() {
    std::string data_to_send;
    {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        if (buffer.empty()) {
            // 没有数据需要发送，取消监听写事件
            reactor.modify_handler(conn_fd, EPOLLIN | EPOLLET | EPOLLRDHUP);
            return;
        }
        data_to_send = buffer;
    }
    
    ssize_t bytes_sent = write(conn_fd, data_to_send.data(), data_to_send.size());
    if (bytes_sent == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // 稍后重试
            return;
        } else {
            // 写入错误
            reactor.remove_handler(conn_fd);
            delete this;
            return;
        }
    }
    
    // 移除已发送的数据
    {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        if (bytes_sent > 0) {
            buffer.erase(0, bytes_sent);
        }
    }
    
    if (buffer.empty()) {
        // 所有数据已发送，取消监听写事件
        reactor.modify_handler(conn_fd, EPOLLIN | EPOLLET | EPOLLRDHUP);
    }
}

void Connection::send_data(const std::string& data) {
    {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        buffer.append(data);
    }
    
    // 注册写事件
    reactor.modify_handler(conn_fd, EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP);
}

// 主函数
int main() {
    try {
        Reactor reactor;
        Acceptor acceptor(reactor, 8080);
        
        // 启动Reactor事件循环
        reactor.run();
        
        std::cout << "Reactor server running on port 8080. Press Enter to exit..." << std::endl;
        std::cin.get();
        
        reactor.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}