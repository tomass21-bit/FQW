#include "D:/GIT/FQW/boost_1_82_0/libs/beast/example/common/root_certificates.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/thread_pool.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <iomanip>


using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
namespace http = boost::beast::http;

class loader
{
public:
    loader(boost::asio::io_context& ioc_) : ioc(ioc_) {}

    std::vector<std::string> get_href(std::string& page) {
        const std::sregex_token_iterator End;
        std::vector<std::string> vLinks;
        const std::regex r("<a href=\"(.*?)\"");
        std::smatch m;
        size_t end = page.find("</html>");
        
            if (regex_search(page, m, r)&&end!= std::string::npos) {
                for (std::sregex_token_iterator i(page.begin(), page.end(), r, 1); i != End; ++i)
                {

                    //vLinks.push_back(*i);
                    std::cout << *i << std::endl; // *i only yields the captured part
                }

                       
            }
       return vLinks;


    }

    void load(std::string const& sUri, std::vector<std::string>& vres)
    {
        std::smatch mr;
        if (std::regex_match(sUri, mr, rUri))
        {
            for (auto const& sm : mr)
            {
                {
                    //std::lock_guard<std::mutex> lg{ mtx };
                    //std::cout << sm << std::endl;
                }
            }

            if (mr[1].str() == "http")
            {
                vres = loadHttp(mr);
            }
            else
            {
                vres = loadHttps(mr);
            }
        }
        else
        {
            {
                std::lock_guard<std::mutex> lg{ mtx };
                std::cerr << "load() std::regex_match() failed: " + sUri << "\n\n";
            }
        }
    }

    std::vector<std::string> loadHttp(std::smatch const& mr)
    {
        std::vector<std::string> vLinks;
        try {
            std::string const target = (mr[3].length() == 0 ? "/" : mr[3].str());
            int version = 11; // или 10 для http 1.0

            tcp::socket socket{ ioc };
            tcp::resolver resolver{ ioc };
            std::string const host = mr[2];
            auto const results = resolver.resolve(host, "80");
            
            boost::asio::connect(socket, results.begin(), results.end());
            http::request<http::string_body> req{ http::verb::get, target, version };
            req.set(http::field::host, host);
            req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            http::write(socket, req);
            boost::beast::flat_buffer buffer;
            http::response<http::dynamic_body> res;
            http::read(socket, buffer, res);

            std::string sBody = boost::beast::buffers_to_string(res.body().data());
            //std::ofstream ofs{ "out.txt" }; // запись html-страницы в файл
            //ofs << sBody;
            //ofs.close();

            vLinks = get_href(sBody);

            boost::system::error_code ec;
            socket.shutdown(tcp::socket::shutdown_both, ec);
            if (ec && ec != boost::system::errc::not_connected)
                throw boost::system::system_error{ ec };

        }
        catch (std::exception const& e)
        {
            {
                std::lock_guard<std::mutex> lg{ mtx };
                std::cerr << "loadHttp(): " << e.what() << std::endl;
            }
        }
        return vLinks;
    }

    std::vector<std::string> loadHttps(std::smatch const& mr)
    {
        std::vector<std::string> vLinks;
        try {

            std::string const host = mr[2];
            std::string const target = (mr[3].length() == 0 ? "/" : mr[3].str());
            int version = 11; // или 10 для http 1.0

            ssl::context ctx{ ssl::context::sslv23_client };
            load_root_certificates(ctx);
            ssl::stream<tcp::socket> stream{ ioc, ctx };
            if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str()))
            {
                boost::system::error_code ec{ static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category() };
                throw boost::system::system_error{ ec };
            }
            tcp::resolver resolver{ ioc };
            auto const results = resolver.resolve(host, "443");
            boost::asio::connect(stream.next_layer(), results.begin(), results.end());
            stream.handshake(ssl::stream_base::client);
            http::request<http::string_body> req{ http::verb::get, target, version };
            req.set(http::field::host, host);
            req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            http::write(stream, req);
            boost::beast::flat_buffer buffer;
            http::response<http::dynamic_body> res;
            http::read(stream, buffer, res);

            std::string sBody = boost::beast::buffers_to_string(res.body().data());
            std::ofstream ofs{ "out.txt" }; // запись html-страницы в файл
            ofs << sBody;
            ofs.close();

            vLinks = get_href(sBody);

            boost::system::error_code ec;
            stream.shutdown(ec);
            if (ec == boost::asio::error::eof)
            {
                // Rationale:
                // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
                ec.assign(0, ec.category());
            }
            if (ec)
                throw boost::system::system_error{ ec };

        }
        catch (std::exception const& e)
        {
            {
                std::lock_guard<std::mutex> lg{ mtx };
                std::cerr << "loadHttps(): " << e.what() << std::endl;
            }
        }
        return vLinks;
    }

    loader(loader const&) = delete;
    loader& operator=(loader const&) = delete;

private:
    std::regex rUri{ "^(?:(https?)://)([^/]+)(/.*)?" };

    std::mutex mtx;
    boost::asio::io_context& ioc;
    
};

// протестировано в MSVS2017
int main()
{
    system("chcp 1251");

    boost::asio::io_context ioc;
    loader ldr{ ioc };

    std::vector<std::string> vUri
    {
        "https://www.boost.org/users/history/version_1_84_0.html" // начальная ссылка
    };

    std::vector<std::vector<std::string>> vres;

    int dpth = 2; // глубина обхода
    while (dpth--)
    {
        vres.clear();
        vres.reserve(vUri.size()); // чтобы не переаллоцировался

        // вектор vUri обрабатывается пулом из 8 потоков
        boost::asio::thread_pool tpool{ 8 };
        for (auto const& sUri : vUri)
        {
            vres.emplace_back();
            boost::asio::post(tpool, std::bind(&loader::load, &ldr, std::cref(sUri), std::ref(vres.back())));
        }
        tpool.join();

        // вывод результата
        std::cout << "\nСтраниц: " << vUri.size() << " **************************************************************************************************\n\n";
        for (int i = 0; i < vres.size(); ++i)
        {
            std::cout << "Ссылок: " << std::setw(6) << std::left << vres.at(i).size() << vUri.at(i) << "\n\n";
            for (auto const& str : vres.at(i))
            {
                //std::cout << str << std::endl; // вывод найденных ссылок
            }
        }

        // перед заходом на следующую итерацию перекидываем все найденные ссылки из vres в vUri 
        vUri.clear();
        for (auto const& vct : vres)
        {
            vUri.insert(vUri.end(), vct.begin(), vct.end());
        }
    }

}






//void delete_html(std::string& page) {
//
//
//    size_t start = page.find("<");            // находим позицию подстроки
//    size_t end = page.find(">");
//    while (end != std::string::npos) {
//        page.erase(start, end + 1 - start);
//        start = page.find("<");
//        end = page.find(">");
//    }
//}

