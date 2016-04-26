#include "mainwindow.h"
#include "ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	makeBackground();
	connect(ui->pbExit, SIGNAL(clicked()), QApplication::instance(), SLOT(quit()));
	connectToBD();
}

MainWindow::~MainWindow()
{
	fb->disconnect();
	delete ui;
}

void MainWindow::makeBackground() {

	desktopHeight = QApplication::desktop()->screenGeometry().height();
	size_t desktopHeight_half = desktopHeight / 2;
	desktopWidth = QApplication::desktop()->screenGeometry().width();
	size_t desktopWidth_half = desktopWidth / 2;

	//QPixmap bkg("pictures/background.jpg");
/*	QPixmap bkg(":/images/pictures/background.jpg");
	if (QApplication::desktop()->screenGeometry().width() >= QApplication::desktop()->screenGeometry().height()) {
		bkg = bkg.scaledToWidth(QApplication::desktop()->screenGeometry().width());
	}
	else {
		bkg = bkg.scaledToHeight(QApplication::desktop()->screenGeometry().height());
	}

	QPalette palette;
	palette.setBrush(QPalette::Background, bkg);

	this->setPalette(palette);*/

	QPixmap bkg(":/images/pictures/background.jpg");
	if (bkg.size().height() <= bkg.size().width()) {
		bkg = bkg.scaledToWidth(desktopWidth);
	}
	else {
		bkg = bkg.scaledToHeight(desktopHeight);
	}
	bkg = bkg.scaled(QSize(desktopWidth, desktopHeight), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	//bkg = bkg.scaled(this->size(), Qt::KeepAspectRatio);
	QPalette palette;
	palette.setBrush(QPalette::Background, bkg);

//	this->setPalette(palette);
	ui->lblImage->setGeometry(desktopWidth / 2 - bkg.width() / 2, desktopHeight / 2 - bkg.height() / 2, bkg.width(), bkg.height());
	ui->lblImage->setPixmap(bkg);
	ui->lblImage->show();




	ui->lblLogo->move(desktopWidth - ui->lblLogo->width(), 40);
	ui->lblLogo->show();

	ui->pbModule_1->move(desktopWidth_half + ui->pbModule_1->width() / 1.5, desktopHeight_half - ui->pbModule_1->height() - 40);
	ui->pbModule_2->move(desktopWidth_half + ui->pbModule_2->width() / 1.5, desktopHeight_half + 40);
	connect(ui->pbModule_1, SIGNAL(clicked()), this, SLOT(onModule1Click()));
	connect(ui->pbModule_2, SIGNAL(clicked()), this, SLOT(onModule2Click()));
	ui->pbModule_1->show();
	ui->pbModule_2->show();

	QString currentStage;
	ui->lblCurrentStage->setText("1 / " + QString::fromStdString(boost::lexical_cast<string>(MAX_STAGE)));
	ui->lblCurrentStage->move(desktopWidth - ui->lblCurrentStage->width() - 30, 40);
	ui->lblCurrentStage->hide();

	ui->lblQuestion->resize(desktopWidth - 300, desktopHeight - 600);
	ui->lblQuestion->move(desktopWidth_half - ui->lblQuestion->width() / 2, 200);
	ui->lblQuestion->hide();

	ui->gbResult->hide();

	ui->pbExit->move(desktopWidth - ui->pbExit->width() - 40, desktopHeight - ui->pbExit->height() - 40);
	ui->pbExit->show();
}

void MainWindow::connectToBD() {
	fb = new firebirdEngine();
	if (fb->connect()) {
//		fb->createTables();
//		fb->makeLists(foundAutorPaintings(), foundQuestions());
		rightFrontierRandomize_module1 = boost::lexical_cast<int>(fb->getCount_Paint());
		rightFrontierRandomize_module2 = boost::lexical_cast<int>(fb->getCount_Question());
	}
}


map<string, vector<string>> MainWindow::foundAutorPaintings() {
	map<string, vector<string>> autor_paint;
	vector<string> paints;
	string autor;
	QDir dir = QDir::current();
	dir.cd("pictures");
	for (const QString& d : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
		paints.clear();
		autor = d.toStdString();
		QDir autor_dir = dir.path() + "/" + d;
		for (const QString& f : autor_dir.entryList(QDir::Files)) {
			paints.push_back(QString(autor_dir.path() + "/" + f).toStdString());
		}
		autor_paint[autor] = paints;
	}
	return autor_paint;
}

map<string, stage2> MainWindow::foundQuestions() {
	map<string, stage2> questions;
	stage2 stage;
	string fName;
	QDir dir = QDir::current();
	QFile fls;
	dir.cd("questions");
		for (const QString& f : dir.entryList(QDir::Files)) {
			stage.wrongAnswers.clear();
			fName = f.toStdString();
			fls.setFileName("questions/" + f);
			fls.open(QIODevice::ReadOnly | QIODevice::Text);
			QTextStream in(&fls);
			QString line = in.readLine();
			while (!line.isNull()) {
				string tmp = line.toStdString();
				if (tmp[0] == '+') {
					stage.correctAnswer = tmp.substr(1, tmp.length() - 1);
				}
				else if (tmp[0] == '-'){
					stage.wrongAnswers.push_back(tmp.substr(1, tmp.length() - 1));
				}
				else {
					stage.question = tmp;
				}
				line = in.readLine();
			}
			questions[fName] = stage;
		}
	return questions;
}


void MainWindow::onModule1Click() {
	currentModule = PAINTS;
	clearScreen();
	makeStages_1();
	startModule_1();
}

void MainWindow::onModule2Click() {
	currentModule = QUESTIONS;
	clearScreen();
	makeStages_2();
	startModule_2();
}

void MainWindow::clearScreen() {
	ui->lblLogo->hide();
	ui->pbExit->hide();
	switch (currentModule) {
		case PAINTS: {
			QPalette p;
			p.setColor(QPalette::Background, QColor("black"));
			ui->centralWidget->setStyleSheet("");
			this->setPalette(p);
			ui->lblImage->hide();
			if (paintInfo != NULL) {
				paintInfo->hide();
			}
			break;
		} 
	}

	QList<QPushButton *> list = this->findChildren<QPushButton *>();
	for each (QPushButton* btn in list)	{
		btn->hide();
	}
}

void MainWindow::prepareAnswersBtns() {
	QList<QPushButton *> list = this->findChildren<QPushButton *>();
	for each (QPushButton* btnAnswers in list)	{
		string btnName = QString(btnAnswers->objectName()).toStdString();
		if (btnName.substr(0, 9) == "btnAnswer") {
			btnAnswers->setObjectName("todelete");
			btnAnswers->hide();
			btnAnswers->deleteLater();
		}
	}
		size_t number = 1;
		
		for (int i = 0; i < 4; ++i) {
			try {
					QPushButton* btn = new QPushButton(this);
/*					QLabel* lbl = new QLabel(this);
					lbl->resize(220, 120);
					lbl->move((number % 2 == 0 ? desktopWidth / 2 - btn->width() - 40 : desktopWidth / 2 + 40), (number < 3 ? desktopHeight - 40 - btn->height() * 2 : desktopHeight - 40 - btn->height()));
					lbl->show();
					lbl->setObjectName(QString::fromStdString("lblAnswer_" + boost::lexical_cast<string>(number)));
					lbl->setStyleSheet("QLabel {						background - color: rgba(44, 127, 255, 0.4);					border - style: outset;					border - width: 2px;					border - radius: 10px;					border - color: beige;				font: bold 14px;					min - width: 10em;				padding: 6px;			}");
					lbl->setText(lbl->objectName());*/
					btn->resize(220, 120);
					switch (i) {
						case 0: {
							//btn->move((number % 2 == 0 ? desktopWidth / 2 - btn->width() - 40 : desktopWidth / 2 + 40), (number < 3 ? desktopHeight - 40 - btn->height() * 2 : desktopHeight - 40 - btn->height()));

							btn->move(desktopWidth / 2 - (btn->width() * 2) - 20, desktopHeight - btn->height() - 20);
							break;
						}
						case 1: {
							//btn->move((number % 2 == 0 ? desktopWidth / 2 - btn->width() - 40 : desktopWidth / 2 + 40), (number < 3 ? desktopHeight - 40 - btn->height() * 2 : desktopHeight - 40 - btn->height()));
							btn->move(desktopWidth / 2 - (btn->width()), desktopHeight - btn->height() - 20);
							break;
						}
						case 2: {
							//btn->move((number % 2 == 0 ? desktopWidth / 2 - btn->width() - 40 : desktopWidth / 2 + 40), (number < 3 ? desktopHeight - 40 - btn->height() * 2 : desktopHeight - 40 - btn->height()));
							btn->move(desktopWidth / 2 + 20, desktopHeight - btn->height() - 20);
							break;
						}
						case 3: {
							//btn->move((number % 2 == 0 ? desktopWidth / 2 - btn->width() - 40 : desktopWidth / 2 + 40), (number < 3 ? desktopHeight - 40 - btn->height() * 2 : desktopHeight - 40 - btn->height()));
							btn->move(desktopWidth / 2 + (btn->width()) + 40, desktopHeight - btn->height() - 20);
							break;
						}
					}
						btn->show();
						btn->setStyleSheet("QPushButton {\n   color: white;\n  	background-color: rgba(44, 127,255, 0.3);\n     border-style: outset;\n     border-width: 2px;\n     border-radius: 10px;\n     border-color: beige;\n     font: bold 14px;\n     min-width: 10em;\n     padding: 6px;\n}\n\nQPushButton:hover {\n     background-color: rgba(44, 127,255, 0.6);\n}");
						btn->setObjectName(QString::fromStdString("btnAnswer_" + boost::lexical_cast<string>(number)));
					++number;
			}
			catch (...) {

			}
		}
	countAnswersRemain = 4;
	ui->lblCurrentStage->show();
}

void MainWindow::makeStages_1() {
	module1_randomQuestions.clear();
	used.clear();
	ui->gbResult->hide();
	for (int i = 1; i <= MAX_STAGE; ++i) {
		stage1 paint;

		int id = boost::lexical_cast<int>(randomseed(rightFrontierRandomize_module1, false));
		pair<string, string> paintData = fb->getPaint_byID(id);
		//string author_correct = fb->getAuthor_byPaint(paintData.first);
		string author_correct = fb->getAuthor_byPaint(id);
		vector<string> authorInvalid = fb->getThreeRandomAuthor(author_correct);
		paint.paintName = paintData.first;
		paint.paintPath = paintData.second;
		paint.fioAuthor_correct = author_correct;
		paint.img = fb->getPaintImg_byID(id);
		
		vector<string>::iterator it = authorInvalid.begin();
		while (it != authorInvalid.end()) {
			paint.fioAuthor_invalid.push_back(*it);
			++it;
		}

		module1_randomQuestions.push_back(paint);
	}

}

void MainWindow::makeStages_2() {
	module2_randomQuestions.clear();
	used.clear();
	ui->gbResult->hide();

	for (int i = 1; i <= MAX_STAGE; ++i) {
		stage2 question;
		int id = boost::lexical_cast<int>(randomseed(rightFrontierRandomize_module2, false));
		vector<string> q = fb->getQuestion_byID(id);
		question.question = q.at(0);
		question.correctAnswer = q.at(1);
		question.wrongAnswers.push_back(q.at(2));
		question.wrongAnswers.push_back(q.at(3));
		question.wrongAnswers.push_back(q.at(4));
		module2_randomQuestions.push_back(question);
	}
}


void MainWindow::startModule_1() {
	currentStage = 0;
	setStage();
	countCorrectAnswers = 0;
	prepareScreenForStage();
	ui->lblImage->show();
}

void MainWindow::startModule_2() {
	currentStage = 0;
	setStage();
	countCorrectAnswers = 0;
	ui->lblQuestion->show();
	prepareScreenForStage();
}

string MainWindow::randomseed(size_t max, bool isBtns) {
	typedef boost::mt19937 RNGType;
	RNGType rng(time(0));
	boost::uniform_int<> many(1, max);
	boost::variate_generator< RNGType, boost::uniform_int<> >
		dice(rng, many);
	
	if (!isBtns) {
		bool found = true;
		int id = 0;
		while (found) {
			id = dice();
			found = false;
			vector<int>::iterator it = used.begin();
			
			while (it != used.end()) {
				if (id == *it) {
					found = true;
					break;
				}
				++it;
			}

			if (!found) {
				used.push_back(id);
			}
		}
		return boost::lexical_cast<string>(id);
	}
	else {
		return boost::lexical_cast<string>(dice());
	}
}

void MainWindow::onCorrectAnswerClick() {
	if (countWrongAnswers == 0) {
		++countCorrectAnswers;
		switch (currentModule) {
			case PAINTS: {
				fb->setPaintGuessed(module1_randomQuestions.at(currentStage).paintName);
				break;
			}
			case QUESTIONS: {
				fb->setQuestionGuessed(module2_randomQuestions.at(currentStage).question);
				break;
			}
		}
		
	}
	++currentStage;
	setStage();
	if (currentStage < MAX_STAGE) {
		if (countWrongAnswers > 0) {
			switch (currentModule) {
				case PAINTS: {
					fb->setPaintUnguessed(module1_randomQuestions.at(currentStage - 1).paintName);
					break;
				}
				case QUESTIONS: {
					fb->setQuestionUnguessed(module2_randomQuestions.at(currentStage - 1).question);
					break;
				}
			}
		}
		prepareScreenForStage();
		countWrongAnswers = 0;
	}
	else {
		goresult();
	}
}

void MainWindow::onWrongAnswerClick() {
	QPushButton* btn = (QPushButton*)sender();
	btn->deleteLater();

	--countAnswersRemain;
	++countWrongAnswers;
	if (countAnswersRemain >= 1) {

	}
	else {
		++currentStage;
		prepareScreenForStage();
	}
}

void MainWindow::preparePaintInfoBlock(string title) {
	if (paintInfo != NULL) {
		delete paintInfo;
	}
	QFont font;
	font.setPixelSize(19);
	font.setBold(true);
	//btn->move(desktopWidth / 2 - (btn->width() * 2) - 20, desktopHeight - btn->height() - 20);

	paintInfo = new QLabel(this);
	paintInfo->setText(QString::fromStdString(title));
	paintInfo->setScaledContents(true);
	paintInfo->setWordWrap(true);
	paintInfo->setStyleSheet("QLabel {\n     background-color: rgba(167, 167, 167, 0.7);\n     border-color: beige;\n	border-style: outset;\n	border-width: 2px;\n     border-radius: 10px;\n     font: bold 25px;\n     min-width: 10em;\n	color:  #21557c;\n}");
	paintInfo->setFont(font);
	paintInfo->setGeometry(desktopWidth / 4, 40, desktopWidth / 2, 60);
	paintInfo->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	paintInfo->show();
}

void MainWindow::prepareScreenForStage() {
	prepareAnswersBtns();
	
	vector<stage1>::iterator it_1 = module1_randomQuestions.begin();
	vector<stage2>::iterator it_2 = module2_randomQuestions.begin();
	size_t correctBtn;
	it_1 += currentStage;
	it_2 += currentStage;
	vector<string>::iterator wrong;
	string correct;

	switch (currentModule) {
		case PAINTS: {
			//QPixmap bkg(QString::fromStdString(it_1->paintPath));
			QPixmap bkg;
			bkg.loadFromData(it_1->img);
			if (bkg.size().height() <= bkg.size().width()) {
				bkg = bkg.scaledToWidth(desktopWidth);
			}
			else {
				bkg = bkg.scaledToHeight(desktopHeight);
			}
			bkg = bkg.scaled(QSize(desktopWidth, desktopHeight), Qt::KeepAspectRatio, Qt::SmoothTransformation);
			//bkg = bkg.scaled(this->size(), Qt::KeepAspectRatio);
			
			ui->lblImage->setGeometry(desktopWidth / 2 - bkg.width() / 2, desktopHeight / 2 - bkg.height() / 2, bkg.width(), bkg.height());
			ui->lblImage->setPixmap(bkg);
//			QPalette p;
//			p.setBrush(this->backgroundRole(), bkg);
			//this->setPalette(p);

			//ui->centralWidget->setStyleSheet("background-image: url(\"" + QString::fromStdString(it_1->paintPath) + "\"); background-position: center; background-repeat: no-repeat; position: fixed; background-size: auto auto;");
//			ui->centralWidget->setStyleSheet("background: white url(:/images/pictures/background.jpg)");
			ui->lblCurrentStage->setStyleSheet("color: orange; background-color: transparent;");
			correctBtn = boost::lexical_cast<size_t>(randomseed(4, true));
			wrong = it_1->fioAuthor_invalid.begin();
			correct = it_1->fioAuthor_correct;

			preparePaintInfoBlock(it_1->paintName);
			break;
		}
		case QUESTIONS: {
			ui->lblQuestion->setText(QString::fromStdString(it_2->question));
			correctBtn = boost::lexical_cast<size_t>(randomseed(4, true));
			wrong = it_2->wrongAnswers.begin();
			correct = it_2->correctAnswer;
			break;
		}
	}

	size_t i = 1;
	QList<QPushButton *> list = this->findChildren<QPushButton *>();
	for each (QPushButton* btn in list)	{
		string btnName = QString(btn->objectName()).toStdString();
		if (btnName.substr(0, 9) == "btnAnswer") {
			//btn->setText(btn->objectName());
			if (i == correctBtn) {

				btn->setText(QString::fromStdString(correct));
				connect(btn, SIGNAL(clicked()), this, SLOT(onCorrectAnswerClick()));
				//btn->setStyleSheet("QPushButton {background-color: green;}");
			}
			else {
				btn->setText(QString::fromStdString(*wrong++));
				connect(btn, SIGNAL(clicked()), this, SLOT(onWrongAnswerClick()));
				//btn->setStyleSheet("QPushButton {background-color: red;}");
			}
			++i;
		}
	}
	i = 1;
/*	QList<QLabel *> listLbl = this->findChildren<QLabel *>();
	for each (QLabel* lbl in listLbl)	{
		string lblName = QString(lbl->objectName()).toStdString();
		if (lblName.substr(0, 9) == "lblAnswer") {
			//lbl->setText(lbl->objectName());
			if (i == correctBtn) {

				lbl->setText(QString::fromStdString(correct));
				//connect(btn, SIGNAL(clicked()), this, SLOT(onCorrectAnswerClick()));
				//btn->setStyleSheet("QPushButton {background-color: green;}");
			}
			else {
				lbl->setText(QString::fromStdString(*wrong++));
				//connect(btn, SIGNAL(clicked()), this, SLOT(onWrongAnswerClick()));
				//btn->setStyleSheet("QPushButton {background-color: red;}");
			}
			++i;
		}
	}*/
}

void MainWindow::goresult() {
	clearScreen();
	makeBackground();
	ui->lclCorrect->display(countCorrectAnswers);
	ui->gbResult->show();
}

void MainWindow::setStage() {
	ui->lblCurrentStage->setText(QString::fromStdString(boost::lexical_cast<string>(currentStage+1)) + " / " + QString::fromStdString(boost::lexical_cast<string>(MAX_STAGE)));
}

