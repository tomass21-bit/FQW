#pragma once
#include <pqxx/pqxx>
#include <iostream>
#include <windows.h>

class Base {
public:
	
	Base(pqxx::work& w_in);
	void create_table();
	

protected:
	pqxx::work& w;
	
};