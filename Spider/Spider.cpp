#include <iostream>
#include <fstream>
#include "Base.h"
#include "Index.h"
#include "MovePage.h"
#include "Search.h"
#include <vector>


struct Connect
{
public:
	void set_connect(std::string host_in, std::string port_in, std::string dbname_in, std::string user_in, std::string password_in) {
		 host= host_in;
		 port= port_in;
		 dbname= dbname_in;
		 user= user_in;
		 password= password_in;
	}
	std::string get_connect() {
		std::string connect = "host=" + host + " port=" + port + " dbname=" + dbname + " user=" + user + " password=" + password + " ";
		return connect;
	}
protected:
	std::string host;
	std::string port;
	std::string dbname;
	std::string user;
	std::string password;
};


int main()
{
	Connect cn;
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	std::vector<std::string> bufer;
	std::ifstream fin("conf.ini");
	if (fin.is_open()) {
		for(int i=0;i<8;++i) {
			std::string str;
			std::getline(fin, bufer[i]);
			// Обработка строки str
		}
	}
	else 
		std::cout << "Файл не найден" << std::endl;
	cn.set_connect(bufer[0], bufer[1], bufer[2], bufer[3], bufer[4]);
	try {
		pqxx::connection c(cn.get_connect());
		pqxx::work w{ c };

		std::shared_ptr<Base> base = std::make_shared<Base>(w);
		base->create_table();
		w.commit();

	}
	catch (pqxx::sql_error e)
	{
		std::cout << e.what() << std::endl;

	}
	catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	}
	return 0;