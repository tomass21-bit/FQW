#include "Base.h"



Base::Base(pqxx::work& w_in): w(w_in)
{}


void Base::create_table() {
	try {
		pqxx::result r1 = w.exec("CREATE TABLE IF NOT EXISTS Words (id SERIAL PRIMARY KEY, word VARCHAR(32)  NOT NULL)");
		pqxx::result r2 = w.exec("CREATE TABLE IF NOT EXISTS Documents (id SERIAL PRIMARY KEY, number VARCHAR(13), client_id integer NOT NULL REFERENCES Client (id) ON DELETE CASCADE )");
		pqxx::result r3 = w.exec("CREATE TABLE IF NOT EXISTS Word_Document (id SERIAL PRIMARY KEY, world_id integer NOT NULL REFERENCES Worlds (id) ON DELETE CASCADE, documents_id integer NOT NULL REFERENCES Documents (id) ON DELETE CASCADE, rate integer  )");
	}

	catch (pqxx::sql_error e)
	{
		std::cout << e.what() << std::endl;

	}
	catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	}

}
