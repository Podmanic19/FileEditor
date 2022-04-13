#include <iostream>
#include <string>
#include <fstream>
#include <unordered_map>
#include <vector> 
#include <optional>
#include <algorithm>
#include <sstream>

const short LINE_OFFSET = 1;
const std::string WHITESPACE = " \n\r\t\f\v";

enum class ErrorType {
	none, noInput, unsupportedCommand, invalidRange
};


enum class RangeType {
	valid, invalid, none
};

struct Text {
	bool saved = true;
	std::vector <std::string> text;
	std::string fileName;

};

Text fileLoading(const std::string& path) {
	std::ifstream input;
	std::string line;
	std::vector<std::string> textVector;
	input.open(path);

	if (!input.is_open()) {
		std::ofstream outfile("path");				// if file doesnt exist create it
		Text text;
		text.text = textVector;
		text.fileName = path;
		return text;
	}

	while (std::getline(input, line)) {
		if (input.good()) {
			line += "\n";
		}
		textVector.push_back(line);
	}

	if (input.fail() && !input.eof()) {
		throw "Failed to open specified file.";
	}

	if (!input.eof()) {
		throw "Error while reading file.";
	}

	Text text;
	text.text = textVector;
	text.fileName = path;

	return text;
}

void appendText(Text& text, std::pair<size_t, size_t> range, std::string toAppend) {
	std::string inputLine;
	std::vector<std::string> lines;

	range.second++;

	if (!text.text.empty()) {
		if (text.text[text.text.size() - 1].back() != '\n' && range.second == text.text.size())
			text.text[text.text.size() - 1] += '\n';
	}

	while (range.second > text.text.size()) {
		text.text.push_back("\n");
	}

	if (!toAppend.empty()) {
		text.saved = false;
		text.text.insert(text.text.begin() + range.second, toAppend + "\n");
		return;
	}

	while (true) {
		getline(std::cin, inputLine);
		if (inputLine != ".") lines.push_back(inputLine + '\n');
		else break;
	}

	if (!lines.empty()) {
		text.saved = false;
		text.text.insert(text.text.begin() + range.second, lines.begin(), lines.end());
	}
}

void printText(const Text& text, std::pair<size_t, size_t> range) {
	if (text.text.size() == 0) return;
	for (size_t i = range.first; i <= range.second; i++) {
		std::cout << text.text[i];
	}
}

void deleteText(Text& text, std::pair<size_t, size_t> range) {

	if (range.first > text.text.size() - LINE_OFFSET || text.text.size() == 0) return;					// trying to delete nonexistent line

	text.text.erase(text.text.begin() + range.first, text.text.begin() + range.second + LINE_OFFSET);

	text.saved = false;
}

void replaceText(Text& text, std::pair<size_t, size_t> range, std::string toAppend) {
	deleteText(text, range);
	range.second = range.first - LINE_OFFSET;
	appendText(text, range, toAppend);
}

void commandQuit(const Text& text, bool force) {
	if (!force) {
		if (text.saved) exit(0);
		else std::cout << "You have unsaved changes" << std::endl;
	}
	else {
		exit(0);
	}
}

void commandWrite(Text& text) {
	std::ofstream ofs;

	ofs.open(text.fileName);
	if (ofs.fail()) throw "Failed to open specified file.";

	if (text.saved == true) return;

	for (auto textLine : text.text) {
		ofs << textLine;
	}
	text.saved = true;
}

std::string ltrim(const std::string& s) {
	size_t start = s.find_first_not_of(WHITESPACE);
	return (start == std::string::npos) ? "" : s.substr(start);
}

std::vector<std::string> separateArgs(std::string input) {
	input = ltrim(input);
	std::vector<std::string> args;
	args.resize(3);
	std::string buff = input;
	size_t i = 0;

	for (i = 0; i < buff.size(); i++) {									// load command
		if (input[i] == ' ') break;
		args[0] += input[i];
	}

	if (i == buff.size()) return args;

	buff = ltrim(buff.substr(++i));

	for (i = 0; i < buff.size(); i++) {									// load command
		if (buff[i] == ' ') break;
		args[1] += buff[i];
	}
	if (i == buff.size()) return args;
	args[2] = buff.substr(i + 1);										// optional parameters for append oneliner
	return args;
}

std::pair<size_t, size_t> getRange(std::string rangeStr, size_t textSize) {
	std::vector<std::string> positions;
	positions.resize(2);
	std::pair<size_t, size_t> range;
	int vectorIndex = 0;

	if (rangeStr.empty()) return std::make_pair(0, textSize - LINE_OFFSET);

	for (size_t i = 0; i < rangeStr.size(); i++) {

		if (vectorIndex == 0 && rangeStr[i] == ',') vectorIndex = 1;
		else positions[vectorIndex] += rangeStr[i];

	}

	if (vectorIndex == 1) {
		size_t start = !positions[0].empty() ? std::stoi(positions[0]) - LINE_OFFSET : 0;
		size_t end = !positions[1].empty() ? std::stoi(positions[1]) - LINE_OFFSET : textSize - LINE_OFFSET;
		range.first = start;
		range.second = std::min(end, textSize - LINE_OFFSET);
	}
	else {
		size_t start = std::stoi(positions[0]) - LINE_OFFSET;
		range.first = start;
		range.second = range.first;
	}
	return range;
}

ErrorType validateRange(std::string rangeStr, size_t textLen) {
	std::string range = rangeStr;
	std::vector<std::string> positions(2);
	int vectorIndex = 0;

	for (size_t i = 0; i < range.size(); i++) {														// read characters from input to convert them
		if (isdigit(range[i])) positions[vectorIndex] += range[i];
		else if (vectorIndex == 0 && range[i] == ',') vectorIndex = 1;								// switch positions after ,
		else return ErrorType::invalidRange;														// non ',' character in range
	}

	if (vectorIndex == 1) {																			// two numbers 
		size_t start = positions[0].empty() ? 1 : std::stoi(positions[0]);
		size_t end = positions[1].empty() ? textLen : std::stoi(positions[1]);
		return end >= start && start > 0 ? ErrorType::none : ErrorType::invalidRange;
	}																								// if only ',' was detected, it's correct
	return ErrorType::none;
}

ErrorType validateOneliners(std::vector<std::string>& args, size_t textLen) {
	try {
		if (args[1].empty() || !std::all_of(args[1].begin(), args[1].end(), ::isdigit)) throw std::invalid_argument("Invalid arg.");
		std::stoi(args[1]);																// check if arg[1] is convertable to line
		args[2] = ltrim(args[2]);
		return ErrorType::none;
	}
	catch (const std::invalid_argument&) {												// if not

		if (args[0] == "c" && validateRange(args[1], textLen) == ErrorType::none) {
			args[2] = ltrim(args[2]);
			return ErrorType::none;
		}

		if (args[2].empty()) {
			args[2] = args[1];
			args[1] = "";
		}
		else {
			args[1] += " " + args[2];
			args[2] = args[1];
			args[1] = "";
		}

	}
	return ErrorType::none;
}

ErrorType validateArgs(Text text, std::vector<std::string>& args) {
	if (args[0] == "q" || args[0] == "q!" || args[0] == "w")
		return (!args[1].empty() || !args[2].empty()) ? ErrorType::unsupportedCommand : ErrorType::none;
	else if (args[0] == "a" || args[0] == "c") return validateOneliners(args, text.text.size());
	else if (args[0] == "p" || args[0] == "d") return validateRange(args[1], text.text.size());
	else if (args[0].empty()) return ErrorType::noInput;
	else return ErrorType::unsupportedCommand;
}

std::vector<std::string> awaitCommand(Text text) {
	while (true) {
		std::string input;
		std::string command;
		std::vector<std::string> args;

		std::cout << "* ";
		getline(std::cin, input);

		args = separateArgs(input);
		auto validity = validateArgs(text, args);

		if (validity == ErrorType::none) return args;
		else if (validity == ErrorType::unsupportedCommand) printf("Unsupported command\n");
		else if (validity == ErrorType::invalidRange) printf("Invalid range\n");
	}
}

void execute(Text& text, std::vector<std::string> args) {
	std::pair<size_t, size_t> range = getRange(args[1], text.text.size());

	if (args[0] == "q") commandQuit(text, false);
	if (args[0] == "q!") commandQuit(text, true);
	if (args[0] == "w") commandWrite(text);
	if (args[0] == "p") printText(text, range);
	if (args[0] == "d") deleteText(text, range);
	if (args[0] == "a") appendText(text, range, args[2]);
	if (args[0] == "c") replaceText(text, range, args[2]);
}

int main(int argc, char* argv[]) {
	try {
		if (argc != 2) throw "Invalid number of parameters.";
		Text text = fileLoading(argv[1]);
		std::vector<std::string> args;
		while (true) {
			args = awaitCommand(text);
			execute(text, args);
		}
	}
	catch (const char* e) {
		printf("%s", e);
	}
	return 0;
}