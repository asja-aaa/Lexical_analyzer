#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include<map>
#include<algorithm>
#include <iomanip>
using namespace std;
/**
*	@Author 张泰岩  马帅  蒋黎明  黄青松
*	@Time 2020/10/25
*   @FileCoding  ASNI
*
*/

const char Monocular_operator[15] = { '+','-','*','/','!','%','~','&','|','^','=','<','>',':','?' }; //单目运算符
const string Binocular_operator[] = { "++","--","&&","||","<=","!=","==",">=","+=","-=","*=","/=" }; //双目运算符
const char Delimiter[8] = { ',','(',')','{','}',';','[',']' }; //界符
const string Keyword[19] = { "break","case","continue","do","default","else",
				"for","if","include","main","return","switch","typedef","void","while","#","unsigned","true","false" };//控制关键字
const string Variable_type[7] = { "int","byte","long","short","float","double","char" }; //基本类型
const string typeName[9] = { "单目运算符","双目运算符","界符","控制关键字","基本类型","变量名","字符","数值","注释" };
const string errTypeName[] = { "标识符表示错误","缺少右引号","数值表示错误","无法识别的标记" };

typedef struct LexItem {
	int rowNum;
	int typeId;           //1:单目运算符  2:双目运算符  3:界符    4：控制关键字   5：基本类型   6 :变量名  7 字符  8 数值    9注释
	string value;
	LexItem(int x, int y, string z) :rowNum(x), typeId(y), value(z) {}
};

/*异常错误信息项*/
typedef struct errItem {
	int rowNum;
	int typeId;     //1. 标识符表示错误   2.字符或字符串出错 3. 无法识别标记
	string  value;
	errItem(int x, int y, string z) :rowNum(x), typeId(y), value(z) {}
};

typedef pair<string, int>PAIR;

struct CmpByValue {
	bool operator()(const PAIR& lhs, const PAIR& rhs) {
		return lhs.second > rhs.second;
	}
};

/** 最终分析结果 */
vector<LexItem> analyResult;
/** 分析数据缓存 */
vector<string> fileMessage;    //文件信息
/*文件名*/
string fileName;
/*错误信息汇总*/
vector<errItem>errMessage;


/*读取txt分析文件 将其存入 @fileMeessage*/
void readSourceFile();
/*词法分析入口*/
void LexAnalysis();
/*寻找界符下表*/
int  findDelimiter(char c);   //寻找界符下表
/*寻找单目运算符下表*/
int findMonOperator(char c);
/*寻找string 的下表*/
int  findStrIndex(string str, const string  strarray[], int length);
/*结果展示*/
void showResult();
/*写入文件*/
void writeLexFile();
/*@strarray里是否包含@str*/
int findHasStrIndex(string str, const string strarray[], int lenth);
/*是否到达结束点*/
bool isComplete(char c);
/*找到多行注释结尾*/
bool findMulNoteLocation(int line, int i, int res[]);
/*找到多行注释的值*/
string findmulNoteVal(int startline, int starpoint, int loc[]);
/*通过类型获取不同类型的符号表*/
vector<LexItem> getLexitemByType(int typeId);
/*找到结束点*/
int findCompleteIndex(string str);
/*判断变量名是否合法*/
bool judgeIdisLegal(string str);
/*获取字符 ’  “的结尾*/
int findstrCompleteIndex(string str);
/*获取数字的结尾*/
bool judgeNumlegal(string str);
/*打印各个Lex出现的次数*/
void printValueCounts();
/*通过str查找类型*/
int getTypeByValue(string str);
/*主分析函数*/
void  LexAnalysis();

int  main() {
	readSourceFile();
	LexAnalysis();
	showResult();
	writeLexFile();

}

void  LexAnalysis() {
	int offset = 0;
	for (int index = 0; index < fileMessage.size(); index++) {
		string line = fileMessage[index];
		if (offset >= line.length()) { offset = 0; continue; }
		for (int i = 0 + offset; i < line.length(); i++) {
			if (offset > 0) offset = 0;
			//注释单独判断
			if (line[i] == '/') {
				if (i < line.size() - 1) {
					if (line[i + 1] == '/') { //单行注释
						string val = "";
						if (i != line.size() - 2) { val = line.substr(i + 2); } //非单行末尾
						LexItem  SNote(index, 9, val);
						analyResult.push_back(SNote);
						break;
					}
					else if (line[i + 1] == '*') { //多行注释
						int loc[] = { 0,0 };
						if (findMulNoteLocation(index, i + 2, loc) == false) {  //多行注释出错  程序直接终止
							showResult();
							cout << "第 " << index << " 行 ，注释出错，未找到注解结尾，程序终止！" << endl;
							exit(-1);
						}
						string mval = findmulNoteVal(index, i + 2, loc);
						LexItem MNote(index, 9, mval);
						analyResult.push_back(MNote);
						index = loc[0] - 1;
						offset = loc[1] + 3;
						if (offset == 3) {
							offset = 2;
						}
						break;
					}
				}
			}

			if (line[i] == ' ' || line[i] == '\t') { continue; } //过滤空格 和制表符
			else if (line[0] == '#') { break; } //过滤头文件
			else if (findDelimiter(line[i]) != -1) {   //界符
				string temp = " "; temp[0] = line[i];
				LexItem delimiter(index, 3, temp);
				analyResult.push_back(delimiter);
			}
			else if (findMonOperator(line[i]) != -1) {     //运算符   
				if (i != line.length() - 1) {   //是否为为本行最后一个字符
					string str = line.substr(i, 2);
					if (findStrIndex(str, Binocular_operator, 12) != -1) {  //双目运算符
						LexItem Bopt(index, 2, Binocular_operator[findStrIndex(str, Binocular_operator, 12)]);
						analyResult.push_back(Bopt); i++;
					}
					else {                      //单目运算符
						string temp = " "; temp[0] = line[i];
						LexItem opt(index, 1, temp);
						analyResult.push_back(opt);

						int s = 10;
					}
				}
				else {  //单目运算符
					string temp = " "; temp[0] = line[i];
					LexItem opt1(index, 1, temp);
					analyResult.push_back(opt1);
				}
			}
			else if (((char)line[i] >= 'a' && (char)line[i] <= 'z') || ((char)line[i] >= 'A' && (char)line[i] <= 'Z') || (char)line[i] == '_') { //控制关键字   基本类型   变量名
				int K = findHasStrIndex(line.substr(i), Keyword, 19);
				int V = findHasStrIndex(line.substr(i), Variable_type, 7);
				if (K != -1) {   //可能为控制关键字
					if (i + K - 1 == line.size() || isComplete(line[i + K])) {
						LexItem kw(index, 4, line.substr(i, K));
						analyResult.push_back(kw);
						i += K - 1;
					}
				}
				else if (V != -1) { //可能为基本类型
					if (i + V - 1 == line.size() || isComplete(line[i + V])) {
						LexItem variable(index, 5, line.substr(i, V));
						analyResult.push_back(variable);
						i += V - 1;
					}
				}
				else {//变量名  ID
					int IDI = findCompleteIndex(line.substr(i));
					string IDval = line.substr(i, IDI);
					if (judgeIdisLegal(IDval)) {    //合法标识符
						LexItem id(index, 6, IDval);
						analyResult.push_back(id);
					}
					else {
						errItem iderr(index, 1, IDval);  //非法标识符
						errMessage.push_back(iderr);
					}
					i += IDI - 1;
				}
			}
			else if (line[i] >= 48 && line[i] <= 57) { //数值        
				int NID = findCompleteIndex(line.substr(i));
				string numVal = line.substr(i, NID);
				if (judgeNumlegal(numVal)) {
					LexItem num(index, 8, numVal);
					analyResult.push_back(num);
				}
				else {
					errItem numerr(index, 3, numVal);
					errMessage.push_back(numerr);
				}
				i += NID - 1;
			}
			else if (line[i] == '\'' || line[i] == '"') {  //字符    C语言字符串只能放在一行  不能拆开  即 “ 或 ‘  不能出现在不同的行数
				int backindex = findstrCompleteIndex(line.substr(i));
				if (backindex == -1) {   //字符或字符串出错  缺少右引号
					errItem Serr(index, 2, line.substr(i));
					errMessage.push_back(Serr);
					break;
				}
				else {
					LexItem str(index, 7, line.substr(i, backindex));
					analyResult.push_back(str);
					i += backindex - 1;
				}
			}
			else {   //无法识别标记
				int c = findCompleteIndex(line.substr(i));
				errItem value(index, 4, line.substr(i,c));
				errMessage.push_back(value);
				i += c - 1;
			}
		}

	}
}

void readSourceFile() {
	std::cout << "请输入要分析的源代码文件  :  ";
	cin >> fileName;
	ifstream file(fileName);

	if (!file.is_open()) {
		cout << "文件打开错误！请检查文件路径：" << fileName << endl;
		exit(1);
	}
	char buffer[255];
	while (!file.eof()) {
		file.getline(buffer, 255);
		fileMessage.push_back(buffer);
	}
	file.close();
}

void writeLexFile() {
	string outfile = fileName.substr(0, fileName.size() - 4) + "Out.txt";
	ofstream file(outfile);
	for (int i = 0; i < analyResult.size(); i++) {
		file << "< " << analyResult[i].rowNum << " , " << analyResult[i].value << " , " << typeName[analyResult[i].typeId - 1] << " >" << endl;
	}
	file.close();
}

int  findDelimiter(char c) {
	for (int i = 0; i < strlen(Delimiter); i++) {
		if (Delimiter[i] == c)return i;
	}
	return -1;
}

int findMonOperator(char c) {
	for (int i = 0; i < strlen(Monocular_operator); i++) {
		if (Monocular_operator[i] == c)return i;
	}
	return -1;
}

int  findStrIndex(string str, const string  strarray[], int length) {
	for (int i = 0; i < length; i++) {
		if (strarray[i] == str)return i;
	}
	return -1;
}

/*@strarray里是否包含@str*/
int findHasStrIndex(string str, const string strarray[], int lenth) {
	int maxlength = str.size();
	for (int i = 0; i < lenth; i++) {
		if (strarray[i].size() <= maxlength) {
			if (strarray[i] == str.substr(0, strarray[i].size()))return strarray[i].size();
		}
	}
	return -1;
}

/*是否到达结束点*/
bool isComplete(char c) {
	if (c == ' ')return true;
	for (int i = 0; i < strlen(Delimiter); i++) {
		if (Delimiter[i] == c)return true;
	}
	for (int i = 0; i < strlen(Monocular_operator); i++) {
		if (Monocular_operator[i] == c)return true;
	}
	return false;
}

/*找到结束点*/
int findCompleteIndex(string str) {
	for (int i = 0; i < str.length(); i++) {
		if (isComplete(str[i]))return i;
	}
	return str.length();
}

int findstrCompleteIndex(string str) {
	for (int i = 1; i < str.length(); i++) {
		if (str[i] == '\'' || str[i] == '"')return i+1;
	}
	return -1;
}
/*找到多行注释结尾*/
bool findMulNoteLocation(int line, int i, int res[]) {

	for (int first = i; first < fileMessage[line].size(); first++) {//在同一行
		if (first < fileMessage[line].size()) {
			if (fileMessage[line][first] == '*' && fileMessage[line][first + 1] == '/') {
				res[0] = line;
				res[1] = first - 1;
				return true;
			}
		}
	}
	for (int index = line + 1; index < fileMessage.size(); index++) {
		for (int j = 0; j < fileMessage[index].size(); j++) {
			if (index < fileMessage.size()) {
				if (fileMessage[index][j] == '*' && fileMessage[index][j + 1] == '/') {
					res[0] = index;
					res[1] = j - 1; if (res[1] < 0)res[1] = 0;
					return true;
				}
			}
		}
	}

	return false;
}

/*找到多行注释的值*/
string findmulNoteVal(int startline, int starpoint, int loc[]) {
	string mulvalue = "";
	if (starpoint <= loc[1]) mulvalue += fileMessage[startline].substr(starpoint, loc[1] - starpoint + 1);

	for (int index = startline + 1; index <= loc[0]; index++) {
		if (index == loc[0]) {
			mulvalue += fileMessage[index].substr(0, loc[1]);
		}
		else {
			mulvalue += fileMessage[index];
		}
	}

	return mulvalue;

}

void showResult() {
	for (int i = 0; i < analyResult.size(); i++) {
		cout << "< " << analyResult[i].rowNum << " , " << analyResult[i].value << " , " << typeName[analyResult[i].typeId - 1] << " >" << endl;
	}
	if (errMessage.size() != 0) { //错误信息
		cout << "--------------------------------------------------" << endl;
		cout << "编译过程出现的错误:" << endl;
		for (int i = 0; i < errMessage.size(); i++) {
			cout << "< " << errMessage[i].rowNum << " , " << errMessage[i].value << " , " << errTypeName[errMessage[i].typeId - 1] << " >" << endl;
		}
	}
	printValueCounts();
	cout << "--------------------------------------------------" << endl;
	cout << "代码总行数:" << fileMessage.size() << endl;
	for (int i = 0; i < 9; i++) {
		vector<LexItem> item = getLexitemByType(i + 1);
		cout << typeName[i] << " : " << item.size()<<endl;
	}


}

/*通过类型获取不同类型的符号表*/
vector<LexItem> getLexitemByType(int typeId) {
	vector<LexItem> ans;
	for (auto item : analyResult) {
		if (item.typeId == typeId)ans.push_back(item);
	}
	return ans;

}
void printValueCounts() {
	map<string, int> typeCount;
	for (auto item : analyResult) {
		if (typeCount.count(item.value) <= 0) {
			typeCount.insert(pair<string,int>(item.value, 1));
		}else {
			typeCount[item.value]++;
		}
	}

	//对map中元素按value排序
	vector<PAIR>forSort(typeCount.begin(), typeCount.end());
	sort(forSort.begin(),forSort.end(),CmpByValue());
	cout << "--------------------------------------------------" << endl;
	for (auto item : forSort) {
		cout << setiosflags(ios::left) << setw(20);
		cout << typeName[getTypeByValue(item.first)-1];
		cout << setiosflags(ios::left) << setw(20);
		cout << item.first << item.second << endl;
	}


}
int getTypeByValue(string str) {
	for (auto item : analyResult) {
		if (item.value == str)return item.typeId;
	}
	return -1;
}
bool judgeIdisLegal(string str) {
	for (auto c : str) {
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || (c >= 48 && c <= 57))continue;
		return false;
	}
	return true;
}

bool judgeNumlegal(string str) {
	for (int i = 0; i < str.size(); i++) {
		if (str[i] >= 48 && str[i] <= 57) { continue; }
		else if (str[i] == '.' && i != str.size()) { continue; }
		else { return false; }
	}
	return true;
}
