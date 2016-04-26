#ifndef SGFXI_H
#define SGFXI_H

#include <string>
#include <vector>

using namespace std;

struct stage2{
	string question;
	string correctAnswer;
	vector<string> wrongAnswers;
};

#endif