#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include<map>
#include<algorithm>
#include <iomanip>
using namespace std;
/**
*	@Author ��̩��  ��˧  ������  ������
*	@Time 2020/10/25
*   @FileCoding  ASNI
*
*/

const char Monocular_operator[15] = { '+','-','*','/','!','%','~','&','|','^','=','<','>',':','?' }; //��Ŀ�����
const string Binocular_operator[] = { "++","--","&&","||","<=","!=","==",">=","+=","-=","*=","/=" }; //˫Ŀ�����
const char Delimiter[8] = { ',','(',')','{','}',';','[',']' }; //���
const string Keyword[19] = { "break","case","continue","do","default","else",
				"for","if","include","main","return","switch","typedef","void","while","#","unsigned","true","false" };//���ƹؼ���
const string Variable_type[7] = { "int","byte","long","short","float","double","char" }; //��������
const string typeName[9] = { "��Ŀ�����","˫Ŀ�����","���","���ƹؼ���","��������","������","�ַ�","��ֵ","ע��" };
const string errTypeName[] = { "��ʶ����ʾ����","ȱ��������","��ֵ��ʾ����","�޷�ʶ��ı��" };

typedef struct LexItem {
	int rowNum;
	int typeId;           //1:��Ŀ�����  2:˫Ŀ�����  3:���    4�����ƹؼ���   5����������   6 :������  7 �ַ�  8 ��ֵ    9ע��
	string value;
	LexItem(int x, int y, string z) :rowNum(x), typeId(y), value(z) {}
};

/*�쳣������Ϣ��*/
typedef struct errItem {
	int rowNum;
	int typeId;     //1. ��ʶ����ʾ����   2.�ַ����ַ������� 3. �޷�ʶ����
	string  value;
	errItem(int x, int y, string z) :rowNum(x), typeId(y), value(z) {}
};

typedef pair<string, int>PAIR;

struct CmpByValue {
	bool operator()(const PAIR& lhs, const PAIR& rhs) {
		return lhs.second > rhs.second;
	}
};

/** ���շ������ */
vector<LexItem> analyResult;
/** �������ݻ��� */
vector<string> fileMessage;    //�ļ���Ϣ
/*�ļ���*/
string fileName;
/*������Ϣ����*/
vector<errItem>errMessage;


/*��ȡtxt�����ļ� ������� @fileMeessage*/
void readSourceFile();
/*�ʷ��������*/
void LexAnalysis();
/*Ѱ�ҽ���±�*/
int  findDelimiter(char c);   //Ѱ�ҽ���±�
/*Ѱ�ҵ�Ŀ������±�*/
int findMonOperator(char c);
/*Ѱ��string ���±�*/
int  findStrIndex(string str, const string  strarray[], int length);
/*���չʾ*/
void showResult();
/*д���ļ�*/
void writeLexFile();
/*@strarray���Ƿ����@str*/
int findHasStrIndex(string str, const string strarray[], int lenth);
/*�Ƿ񵽴������*/
bool isComplete(char c);
/*�ҵ�����ע�ͽ�β*/
bool findMulNoteLocation(int line, int i, int res[]);
/*�ҵ�����ע�͵�ֵ*/
string findmulNoteVal(int startline, int starpoint, int loc[]);
/*ͨ�����ͻ�ȡ��ͬ���͵ķ��ű�*/
vector<LexItem> getLexitemByType(int typeId);
/*�ҵ�������*/
int findCompleteIndex(string str);
/*�жϱ������Ƿ�Ϸ�*/
bool judgeIdisLegal(string str);
/*��ȡ�ַ� ��  ���Ľ�β*/
int findstrCompleteIndex(string str);
/*��ȡ���ֵĽ�β*/
bool judgeNumlegal(string str);
/*��ӡ����Lex���ֵĴ���*/
void printValueCounts();
/*ͨ��str��������*/
int getTypeByValue(string str);
/*����������*/
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
			//ע�͵����ж�
			if (line[i] == '/') {
				if (i < line.size() - 1) {
					if (line[i + 1] == '/') { //����ע��
						string val = "";
						if (i != line.size() - 2) { val = line.substr(i + 2); } //�ǵ���ĩβ
						LexItem  SNote(index, 9, val);
						analyResult.push_back(SNote);
						break;
					}
					else if (line[i + 1] == '*') { //����ע��
						int loc[] = { 0,0 };
						if (findMulNoteLocation(index, i + 2, loc) == false) {  //����ע�ͳ���  ����ֱ����ֹ
							showResult();
							cout << "�� " << index << " �� ��ע�ͳ���δ�ҵ�ע���β��������ֹ��" << endl;
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

			if (line[i] == ' ' || line[i] == '\t') { continue; } //���˿ո� ���Ʊ��
			else if (line[0] == '#') { break; } //����ͷ�ļ�
			else if (findDelimiter(line[i]) != -1) {   //���
				string temp = " "; temp[0] = line[i];
				LexItem delimiter(index, 3, temp);
				analyResult.push_back(delimiter);
			}
			else if (findMonOperator(line[i]) != -1) {     //�����   
				if (i != line.length() - 1) {   //�Ƿ�ΪΪ�������һ���ַ�
					string str = line.substr(i, 2);
					if (findStrIndex(str, Binocular_operator, 12) != -1) {  //˫Ŀ�����
						LexItem Bopt(index, 2, Binocular_operator[findStrIndex(str, Binocular_operator, 12)]);
						analyResult.push_back(Bopt); i++;
					}
					else {                      //��Ŀ�����
						string temp = " "; temp[0] = line[i];
						LexItem opt(index, 1, temp);
						analyResult.push_back(opt);

						int s = 10;
					}
				}
				else {  //��Ŀ�����
					string temp = " "; temp[0] = line[i];
					LexItem opt1(index, 1, temp);
					analyResult.push_back(opt1);
				}
			}
			else if (((char)line[i] >= 'a' && (char)line[i] <= 'z') || ((char)line[i] >= 'A' && (char)line[i] <= 'Z') || (char)line[i] == '_') { //���ƹؼ���   ��������   ������
				int K = findHasStrIndex(line.substr(i), Keyword, 19);
				int V = findHasStrIndex(line.substr(i), Variable_type, 7);
				if (K != -1) {   //����Ϊ���ƹؼ���
					if (i + K - 1 == line.size() || isComplete(line[i + K])) {
						LexItem kw(index, 4, line.substr(i, K));
						analyResult.push_back(kw);
						i += K - 1;
					}
				}
				else if (V != -1) { //����Ϊ��������
					if (i + V - 1 == line.size() || isComplete(line[i + V])) {
						LexItem variable(index, 5, line.substr(i, V));
						analyResult.push_back(variable);
						i += V - 1;
					}
				}
				else {//������  ID
					int IDI = findCompleteIndex(line.substr(i));
					string IDval = line.substr(i, IDI);
					if (judgeIdisLegal(IDval)) {    //�Ϸ���ʶ��
						LexItem id(index, 6, IDval);
						analyResult.push_back(id);
					}
					else {
						errItem iderr(index, 1, IDval);  //�Ƿ���ʶ��
						errMessage.push_back(iderr);
					}
					i += IDI - 1;
				}
			}
			else if (line[i] >= 48 && line[i] <= 57) { //��ֵ        
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
			else if (line[i] == '\'' || line[i] == '"') {  //�ַ�    C�����ַ���ֻ�ܷ���һ��  ���ܲ�  �� �� �� ��  ���ܳ����ڲ�ͬ������
				int backindex = findstrCompleteIndex(line.substr(i));
				if (backindex == -1) {   //�ַ����ַ�������  ȱ��������
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
			else {   //�޷�ʶ����
				int c = findCompleteIndex(line.substr(i));
				errItem value(index, 4, line.substr(i,c));
				errMessage.push_back(value);
				i += c - 1;
			}
		}

	}
}

void readSourceFile() {
	std::cout << "������Ҫ������Դ�����ļ�  :  ";
	cin >> fileName;
	ifstream file(fileName);

	if (!file.is_open()) {
		cout << "�ļ��򿪴��������ļ�·����" << fileName << endl;
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

/*@strarray���Ƿ����@str*/
int findHasStrIndex(string str, const string strarray[], int lenth) {
	int maxlength = str.size();
	for (int i = 0; i < lenth; i++) {
		if (strarray[i].size() <= maxlength) {
			if (strarray[i] == str.substr(0, strarray[i].size()))return strarray[i].size();
		}
	}
	return -1;
}

/*�Ƿ񵽴������*/
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

/*�ҵ�������*/
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
/*�ҵ�����ע�ͽ�β*/
bool findMulNoteLocation(int line, int i, int res[]) {

	for (int first = i; first < fileMessage[line].size(); first++) {//��ͬһ��
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

/*�ҵ�����ע�͵�ֵ*/
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
	if (errMessage.size() != 0) { //������Ϣ
		cout << "--------------------------------------------------" << endl;
		cout << "������̳��ֵĴ���:" << endl;
		for (int i = 0; i < errMessage.size(); i++) {
			cout << "< " << errMessage[i].rowNum << " , " << errMessage[i].value << " , " << errTypeName[errMessage[i].typeId - 1] << " >" << endl;
		}
	}
	printValueCounts();
	cout << "--------------------------------------------------" << endl;
	cout << "����������:" << fileMessage.size() << endl;
	for (int i = 0; i < 9; i++) {
		vector<LexItem> item = getLexitemByType(i + 1);
		cout << typeName[i] << " : " << item.size()<<endl;
	}


}

/*ͨ�����ͻ�ȡ��ͬ���͵ķ��ű�*/
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

	//��map��Ԫ�ذ�value����
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
