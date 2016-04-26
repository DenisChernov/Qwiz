#pragma once

#define IBPP_WINDOWS
#define MSVC

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <QFileInfo>
#include <QTextStream>
#include <algorithm>
#include <QByteArray>
#include "core\ibpp.h"
#include "sgfxi.h"


using namespace std;

class firebirdEngine
{
private:
	IBPP::Database db;
	IBPP::Transaction tr;
	IBPP::Statement query;
	IBPP::Blob blob;

public:
	firebirdEngine();
	~firebirdEngine();

	/*
		Создает соединение и инициализирует курсоры.
		@return true/false в зависимости от попытки подключиться
	*/
	bool connect();
	void disconnect();

	void createTables();
	void makeLists(map<string, vector<string>>, map<string, stage2>);
	void setPaintGuessed(string paintName);
	void setPaintUnguessed(string paintName);
	void setQuestionGuessed(string question);
	void setQuestionUnguessed(string question);

	string getCount_Author();
	string getCount_Paint();
	string getCount_Question();
	string getCount_Qwiz();
	vector<string> getQuestion_byID(int id);
	pair<string, string> getPaint_byID(int id);
	QByteArray getPaintImg_byID(int id);
	//string getAuthor_byPaint(string paintName);
	string getAuthor_byPaint(int id);
	vector<string> getThreeRandomAuthor(string notthisAuthor);
	
};

