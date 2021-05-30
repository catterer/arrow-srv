#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <string>

#include <nghttp2/asio_http2_server.h>
#include <boost/asio.hpp>

namespace h2 = nghttp2::asio_http2;

int main(int argc, char *argv[]) try {
    // Check command line arguments.
    if (argc < 5) {
        std::cerr << "Usage: asio-sv2 <address> <port> <threads> <doc-root> "
            << "[<private-key-file> <cert-file>]\n";
        return 1;
    }

    boost::system::error_code ec;

    std::string addr = argv[1];
    std::string port = argv[2];
    std::size_t num_threads = std::stoi(argv[3]);
    std::string docroot = argv[4];

    h2::server::http2 server;

    server.num_threads(num_threads);

    server.handle("/", [&docroot](const h2::server::request &req, const h2::server::response &res) {
            std::cout << "request\n";
            res.write_head(200, h2::header_map{});
            auto count = std::make_shared<int>(10);
            res.end([count, &res] (uint8_t* buf, std::size_t len, uint32_t* flags) mutable -> ssize_t {
                        std::cout << "end_cb " << *count << "\n";
                        if (!*count) {
                            *flags |= NGHTTP2_DATA_FLAG_EOF;
                            std::cout << "done\n";
                            return 0L;
                        }
                        --(*count);
                        if (*count % 2) {
                            std::cout << "defer\n";
                            auto timer = std::make_shared<boost::asio::steady_timer>(res.io_service(), boost::asio::chrono::milliseconds(300));
                            timer->async_wait([&res, timer] (auto ec) {
                                std::cout << "timer cb " << ec.message() << "\n";
                                res.resume();
                            });
                            return NGHTTP2_ERR_DEFERRED;
                        }

                        std::cout << "write\n";
                        auto data = std::string("chunk ") + std::to_string(*count) + "\n";
                        data.resize(std::min(len, data.size()));
                        std::copy(data.begin(), data.end(), buf);
                        return data.size();
                    });
    });

    if (server.listen_and_serve(ec, addr, port))
        std::cerr << "error: " << ec.message() << std::endl;

    return 0;
} catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << "\n";
    return 1;
}

