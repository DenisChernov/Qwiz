#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qdesktopwidget>
#include <qlistwidget.h>
#include <boost/filesystem.hpp>
#include <iterator>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <qdir.h>
#include <qdiriterator.h>
#include <qcolor.h>
#include <qlist.h>
#include <qlabel.h>
#include <QString>
#include <qtextstream.h>
#include <boost/random.hpp>
#include <boost/generator_iterator.hpp>
#include "firebirdEngine.h"
#include "sgfxi.h"

using namespace std;
using namespace boost::filesystem;

namespace Ui {
class MainWindow;
}

const size_t MAX_STAGE = 20;

struct stage1{
	string paintName;
	string paintPath;
	string fioAuthor_correct;
	vector<string> fioAuthor_invalid;
	QByteArray img;
};

enum stages {PAINTS, QUESTIONS};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
	firebirdEngine* fb;
	size_t currentStage = 0;
    ~MainWindow();

private:
    Ui::MainWindow *ui;

	string randomseed(size_t max, bool isBtns);
	void makeBackground();
	void connectToBD();
	void startModule_1();
	void startModule_2();
	void clearScreen();
	void prepareAnswersBtns();
	void makeStages_1();
	void makeStages_2();
	void prepareScreenForStage();
	void goresult();
	void setStage();
	void preparePaintInfoBlock(string title);

	QLabel* paintInfo = NULL;
	map<string, vector<string>> foundAutorPaintings();
	map<string, stage2> foundQuestions();
	vector<int> used;
	size_t desktopWidth = 0;
	size_t desktopHeight = 0;
	vector<stage1> module1_randomQuestions;
	vector<stage2> module2_randomQuestions;
	stages currentModule;
	int rightFrontierRandomize_module1 = 0;
	int rightFrontierRandomize_module2 = 0;

	int countAnswersRemain = 4;
	int countCorrectAnswers = 0;
	int countWrongAnswers = 0;

public slots:
	void onModule1Click();
	void onModule2Click();

	void onCorrectAnswerClick();
	void onWrongAnswerClick();
};



#endif // MAINWINDOW_H


