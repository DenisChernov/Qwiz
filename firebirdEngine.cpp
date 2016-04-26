#include "firebirdEngine.h"


#define EMBEDED 1

firebirdEngine::firebirdEngine() {

}


firebirdEngine::~firebirdEngine() {
}

bool firebirdEngine::connect() {
	try {
		QFile file("db.fdb");
		if (file.exists()) {
#ifdef EMBEDED
			db = IBPP::DatabaseFactory("", "db.fdb", "SYSDBA", "masterkey");
#else
			db = IBPP::DatabaseFactory("localhost", "db.fdb", "SYSDBA", "masterkey");
#endif
			db->Connect();
//	  	  db->Drop();
		}
		else {


#ifdef EMBEDED
			db = IBPP::DatabaseFactory("", "db.fdb", "SYSDBA", "masterkey", "", "", "PAGE_SIZE = 4096");
#else
			db = IBPP::DatabaseFactory("localhost", "db.fdb", "SYSDBA", "masterkey", "", "", "PAGE_SIZE = 4096");
#endif

			db->Create(3);
			db->Connect();
		}
		if (db->Connected()) {
			tr = IBPP::TransactionFactory(db);
			query = IBPP::StatementFactory(db, tr);
			return true;
		}
	}
	catch (IBPP::Exception &e) {
		cout << e.ErrorMessage();
	}
	return false;
}

void firebirdEngine::disconnect() {
	db->Disconnect();
}

void firebirdEngine::createTables() {
	tr->Start();

	query->Execute("CREATE TABLE paint (id smallint not null primary key, name varchar(120) not null, filepath varchar(1024), guessed smallint not null, notguessed smallint not null, filedata blob sub_type 0);");
	query->Execute("CREATE TABLE author (id smallint not null, fio varchar(30) not null primary key);");
	query->Execute("CREATE TABLE qwiz (id smallint not null primary key, autor smallint, paint smallint);");
	query->Execute("CREATE TABLE questions (id smallint not null primary key, question varchar (1024), correct varchar(256), wrong_1 varchar(256), wrong_2 varchar(256), wrong_3 varchar(256), guessed smallint not null, notguessed smallint not null);");
	
	tr->Commit();
}

void replaceAll(string& source, const string& from, const string& to)
{
	string newString;
	newString.reserve(source.length());  // avoids a few memory allocations

	string::size_type lastPos = 0;
	string::size_type findPos;

	while (string::npos != (findPos = source.find(from, lastPos)))
	{
		newString.append(source, lastPos, findPos - lastPos);
		newString += to;
		lastPos = findPos + from.length();
	}

	// Care for the rest after last occurrence
	newString += source.substr(lastPos);

	source.swap(newString);
}

void firebirdEngine::makeLists(map<string, vector<string>> autor_paints, map<string, stage2> questions) {
	tr->Start();
	int i_paint = 1;
	int i_autor = 1;
	map<string, vector<string>>::iterator it_autor = autor_paints.begin();
	while (it_autor != autor_paints.end()) {
		vector<string>::iterator it_paints = it_autor->second.begin();
		query->Execute(boost::str(boost::format("INSERT INTO AUTHOR (id, fio) VALUES (%d, '%s')") % i_autor % it_autor->first.data()));
		while (it_paints != it_autor->second.end()) {
			blob = IBPP::BlobFactory(db, tr);
			//QFile* img = new QFile("c:\\IMG_0084.jpg");
			string filepath = it_paints->data();
			replaceAll(filepath, "/", "\\\\");
			QFile* img = new QFile(QString::fromStdString(filepath));
			img->open(QIODevice::ReadOnly);
			QByteArray image = img->readAll();
			
			blob->Save(image.toStdString());

/*			query->Execute(boost::str(boost::format("INSERT INTO PAINT (id, name, filepath, guessed, notguessed) VALUES (%d, '%s', '%s', %d, %d)") 
										% i_paint 
										% QFileInfo(it_paints->data()).baseName().toStdString() 
										% it_paints->data() 
										% 0 
										% 0
										)); */
			query->Prepare("INSERT INTO PAINT (id, name, filepath, guessed, notguessed, filedata) VALUES (?, ?, ?, ?, ?, ?)");
			query->Set(1, i_paint);
			query->Set(2, QFileInfo(it_paints->data()).baseName().toStdString());
			query->Set(3, it_paints->data());
			query->Set(4, 0);
			query->Set(5, 0);
			query->Set(6, blob);
			query->Execute();

			query->Execute(boost::str(boost::format("INSERT INTO QWIZ (id, autor, paint) VALUES (%d, %d, %d)") 
										% i_paint 
										% i_autor
										% i_paint));
			++it_paints;
			++i_paint;
		}
		++it_autor;
		++i_autor;
	}
	map<string, stage2>::iterator it_question = questions.begin();
	int id = 1;
	while (it_question != questions.end()) {
		vector<string> wrongAnswers = it_question->second.wrongAnswers;
		query->Execute(boost::str(boost::format("INSERT INTO questions (id, question, correct, wrong_1, wrong_2, wrong_3, guessed, notguessed) VALUES (%d, '%s', '%s', '%s', '%s', '%s', %d, %d)") 
												% id
												% it_question->second.question 
												% it_question->second.correctAnswer
												% wrongAnswers.at(0)
												% wrongAnswers.at(1)
												% wrongAnswers.at(2)
												% 0
												% 0));
		++it_question;
		++id;
	}

	tr->Commit();
}

string firebirdEngine::getCount_Author() {
	int count;
	if (db->Connected()) {
		try {
			tr->Start();
			query->Execute("SELECT COUNT(*) FROM AUTHOR");
			query->Fetch();
			query->Get(1, count);
		}
		catch (IBPP::Exception& e){
			return boost::lexical_cast<string>(e.ErrorMessage());
		}
	}
	return boost::lexical_cast<string>(count);
}

string firebirdEngine::getCount_Paint() {
	int count;
	if (db->Connected()) {
		try {
			tr->Start();
			query->Execute("SELECT COUNT(*) FROM PAINT");
			query->Fetch();
			query->Get(1, count);
		}
		catch (IBPP::Exception& e){
			return boost::lexical_cast<string>(e.ErrorMessage());
		}
	}
	return boost::lexical_cast<string>(count);
}

string firebirdEngine::getCount_Question() {
	int count;
	if (db->Connected()) {
		try {
			tr->Start();
			query->Execute("SELECT COUNT(*) FROM QUESTIONS");
			query->Fetch();
			query->Get(1, count);
		}
		catch (IBPP::Exception& e){
			return boost::lexical_cast<string>(e.ErrorMessage());
		}
	}
	return boost::lexical_cast<string>(count);
}

string firebirdEngine::getCount_Qwiz() {
	int count;
	if (db->Connected()) {
		try {
			tr->Start();
			query->Execute("SELECT COUNT(*) FROM QWIZ");
			query->Fetch();
			query->Get(1, count);
		}
		catch (IBPP::Exception& e){
			return boost::lexical_cast<string>(e.ErrorMessage());
		}
	}
	return boost::lexical_cast<string>(count);
}

pair<string, string> firebirdEngine::getPaint_byID(int id) {
	string name;
	string filepath;

	if (db->Connected()) {
		try {
			tr->Start();
			query->Prepare("SELECT NAME, FILEPATH FROM PAINT WHERE id = ?");
			query->Set(1, id);
			query->Execute();
			query->Fetch();
			query->Get(1, name);
			query->Get(2, filepath);
			return make_pair(name, filepath);
		}
		catch (IBPP::Exception& e) {
			return make_pair("error", e.ErrorMessage());
		}
	}
	return make_pair("", "");
}

QByteArray firebirdEngine::getPaintImg_byID(int id) {
	QByteArray img;
	IBPP::Blob blob;
	char* buf;
	int size;
	int largest;
	int segments;

	if (db->Connected()) {
		try {
			tr->Start();
			query->Prepare("SELECT filedata FROM PAINT WHERE id = ?");
			query->Set(1, id);
			query->Execute();
			query->Fetch();
			blob = IBPP::BlobFactory(db, tr);
			query->Get(1, blob);
			blob->Open();
			
			blob->Info(&size, &largest, &segments);
			buf = new char[size];
			int offset = 0;
			for (int s = 1; s <= segments; s++) {
				int r;
				r = blob->Read(&buf[offset], largest);
				offset += r;
			}
			blob->Close();
			img = QByteArray::fromRawData(buf, size);
			tr->Commit();
		}
		catch (IBPP::Exception& e) {

		}
	}
	return img;
}

/*string firebirdEngine::getAuthor_byPaint(string paint) {
	string fio;
	if (db->Connected()) {
		try {
			tr->Start();
			query->Prepare("SELECT AUTHOR.FIO FROM PAINT "
							"LEFT JOIN QWIZ ON QWIZ.PAINT = PAINT.ID "
							"LEFT JOIN AUTHOR ON AUTHOR.ID = QWIZ.AUTOR "
							"WHERE PAINT.NAME = ?;");
			query->Set(1, paint);
			query->Execute();
			query->Fetch();
			query->Get(1, fio);
			return fio;
		}
		catch (IBPP::Exception& e) {
			return e.ErrorMessage();
		}
	}
	return "";
}*/

string firebirdEngine::getAuthor_byPaint(int id) {
	string fio;
	if (db->Connected()) {
		try {
			tr->Start();
			query->Prepare("SELECT AUTHOR.FIO FROM PAINT "
				"LEFT JOIN QWIZ ON QWIZ.PAINT = PAINT.ID "
				"LEFT JOIN AUTHOR ON AUTHOR.ID = QWIZ.AUTOR "
				"WHERE PAINT.ID = ?;");
			query->Set(1, id);
			query->Execute();
			query->Fetch();
			query->Get(1, fio);
			return fio;
		}
		catch (IBPP::Exception& e) {
			return e.ErrorMessage();
		}
	}
	return "";
}

vector<string> firebirdEngine::getThreeRandomAuthor(string notthisAuthor) {
	vector<string> invalidAuthors;
	string fio;
	if (db->Connected()) {
		try {
			tr->Start();
			query->Prepare("SELECT FIO FROM AUTHOR WHERE FIO != ? ORDER BY RAND() ROWS 3");
			query->Set(1, notthisAuthor);
			query->Execute();
			while (query->Fetch()) {
				query->Get(1, fio);
				invalidAuthors.push_back(fio);
			}
			
		}
		catch (IBPP::Exception& e) {
			vector<string> error;
			error.push_back(e.ErrorMessage());
		}
	}
	return invalidAuthors;
}

void firebirdEngine::setPaintGuessed(string paintName) {
	if (db->Connected()) {
		try {
			tr->Start();
			string q = boost::str(boost::format("UPDATE PAINT SET guessed = guessed + 1 WHERE NAME = '%s'") % paintName);
			query->Execute(q);
			tr->Commit();
		}
		catch (IBPP::Exception& e) {

		}
	}
}

void firebirdEngine::setPaintUnguessed(string paintName) {
	if (db->Connected()) {
		try {
			tr->Start();
			string q = boost::str(boost::format("UPDATE PAINT SET notguessed = notguessed + 1 WHERE NAME = '%s'") % paintName);
			query->Execute(q);
			tr->Commit();
		}
		catch (IBPP::Exception& e) {

		}
	}
}

void firebirdEngine::setQuestionGuessed(string question) {
	if (db->Connected()) {
		try {
			tr->Start();
			string q = boost::str(boost::format("UPDATE QUESTIONS SET guessed = guessed + 1 WHERE QUESTION = '%s'") % question);
			query->Execute(q);
			tr->Commit();
		}
		catch (IBPP::Exception& e) {

		}
	}
}

void firebirdEngine::setQuestionUnguessed(string question) {
	if (db->Connected()) {
		try {
			tr->Start();
			string q = boost::str(boost::format("UPDATE QUESTIONS SET notguessed = notguessed + 1 WHERE QUESTION = '%s'") % question);
			query->Execute(q);
			tr->Commit();
		}
		catch (IBPP::Exception& e) {

		}
	}
}

vector<string> firebirdEngine::getQuestion_byID(int id) {
	vector<string> question;
	if (db->Connected()) {
		try {
			tr->Start();
			query->Prepare("SELECT question, correct, wrong_1, wrong_2, wrong_3 FROM QUESTIONS WHERE id = ?");
			query->Set(1, id);
			query->Execute();
			string q;
			string c;
			string w1;
			string w2;
			string w3;
			while (query->Fetch()) {
				query->Get(1, q);
				query->Get(2, c);
				query->Get(3, w1);
				query->Get(4, w2);
				query->Get(5, w3);
			}
			question.push_back(q);
			question.push_back(c);
			question.push_back(w1);
			question.push_back(w2);
			question.push_back(w3);
		}
		catch (IBPP::Exception& e) {
			question.push_back(e.ErrorMessage());
		}
	}

	return question;
}