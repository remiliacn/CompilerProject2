/*
 * Copyright (C) Mohsen Zohrevandi, 2017
 *               Rida Bazzi 2019
 * Do not share this file with anyone
 */
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <map>
#include "string"
#include "lexer.h"

using namespace std;
vector<pair<string, vector<string>>> ruleList;
vector<string> addOrder;
vector<string> terminalOrder;
map<string, vector<string>> firstSet;
map<string, vector<string>> followSet;
bool hasUseless = true;
vector<string> rhs;
vector<string> lhs;
vector<string> terminals;
vector<string> nonTerminals;
map <string, int> symbols;
bool generateSymbols[100];
int symbolSize = 0;
LexicalAnalyzer lexer;
Token t;

//Parsing
void parse_rule_list();
void parse_rule();
void parse_RHS();
void parse_id_list();

//Utility
Token peek();
bool ifNotFind(vector<string> vec, const string& value);
void syntax_error();
bool isNonterminal(string var);
bool isTerminal(string var);

bool isNonterminal(string var){
    return !ifNotFind(nonTerminals, var);
}

bool isTerminal(string var){
    return !ifNotFind(terminals, var);
}

Token peek(){
    t = lexer.GetToken();
    lexer.UngetToken(t);
    return t;
}

bool ifNotFind(vector<string> vec, const string& value){
    return (find(vec.begin(), vec.end(), value) == vec.end());
}

void syntax_error(){
    cout << "SYNTAX ERROR !!!" << endl;
    exit(1);
}

void parse_rule_list(){
    t = lexer.GetToken();

    if(t.token_type == ID){
        string ruleName = t.lexeme;
        lexer.UngetToken(t);
        parse_rule();

        ruleList.emplace_back(ruleName, rhs);
        rhs.clear();

        t = peek();
        if(t.token_type == ID){
            parse_rule_list();

        }else if(t.token_type == DOUBLEHASH) {
            return;

        } else{
            syntax_error();
        }

    } else{
        syntax_error();
    }
}

void parse_rule(){
    t = lexer.GetToken();
    if(t.token_type == ID) {
        lhs.push_back(t.lexeme);
        t = lexer.GetToken();

        if (t.token_type == ARROW) {
            parse_RHS();
            t = lexer.GetToken();
            if (t.token_type != HASH) {
                syntax_error();
            }

        } else{
            syntax_error();
        }

    } else{
        syntax_error();
    }
}

void parse_RHS() {
    t = lexer.GetToken();
    if(t.token_type == ID) {
        rhs.push_back(t.lexeme);

        t = peek();
        if (t.token_type == ID) {
            parse_id_list();
        }

    }else if(t.token_type == HASH){
        lexer.UngetToken(t);

    }else{
        syntax_error();
    }
}

void parse_id_list() {
    t = lexer.GetToken();
    if(t.token_type == ID) {
        rhs.push_back(t.lexeme);

        t = peek();
        if (t.token_type == ID) {
            parse_id_list();
        }
    } else{
        syntax_error();
    }
}


void ReadGrammar()
{
    parse_rule_list();
    terminalOrder.emplace_back("#");
    terminalOrder.emplace_back("$");

    for(auto &i : ruleList){
        if(ifNotFind(nonTerminals, i.first)){
            nonTerminals.push_back(i.first);
            addOrder.push_back(i.first);
        }

        for(auto &item : i.second){
            if(ifNotFind(lhs, item)){
                if(ifNotFind(terminals, item)){
                    terminals.push_back(item);
                    terminalOrder.push_back(item);
                }

            }else{
                if(ifNotFind(nonTerminals, item)){
                    nonTerminals.push_back(item);
                    addOrder.push_back(item);
                }
            }
        }
    }

    symbols["#"] = 0;
    generateSymbols[0] = true;
    symbols["$"] = 1;
    symbolSize += 2;

    for(const auto & terminal : terminals){
        symbols[terminal] = symbolSize;
        generateSymbols[symbolSize++] = true;
    }

    for(auto &i : nonTerminals) {
        symbols[i] = symbolSize;
        generateSymbols[symbolSize++] = false;
    }
}

// Task 1
void printForTask1()
{
    string output;
    for(auto &i : terminals){
        output += i + " ";
    }
    output += " ";

    for(auto &i : nonTerminals) {
        output += i + " ";
    }

    cout << output << endl;
}


bool isGenerating = false;

void isGenerate(bool *useless){
    bool isChanged;
    do {
        isChanged = false;
        for (auto &item : ruleList) {
            string left = item.first;
            vector<string> rightRules = item.second;
            for (auto &rightRule : rightRules) {
                int index = symbols[rightRule];
                isGenerating = useless[index];
                if (!isGenerating) {
                    break;
                }
            }

            if (item.second.empty() || isGenerating) {
                int index = symbols[left];
                if(!useless[index]){
                    useless[index] = true;
                    isChanged = true;
                }
            }
        }

    }while(isChanged);
}

bool rea = true;
void isReachable(bool *reachable, vector<pair<string, vector<string>>> ruleGenRules) {
    bool isChanged;
    do{
        isChanged = false;
        for (auto &item : ruleGenRules) {
            string left = item.first;
            vector<string> rightRules = item.second;

            int index = symbols[left];
            if (reachable[index]) {
                for (auto &rightRule : rightRules) {
                    int tempIdx = symbols[rightRule];
                    if(!reachable[tempIdx]){
                        reachable[tempIdx] = true;
                        isChanged = true;
                    }
                }
            }
        }

    }while(isChanged);
}

vector<pair<string, vector<string>>> ruleGen;
vector<pair<string, vector<string>>> useful;
void getUseless(){

    bool reachableSymbols[symbolSize];
    isGenerate(generateSymbols);
    for(auto &item : ruleList){
        string left = item.first;
        vector<string> rightRules = item.second;
        if (!rightRules.empty()){
            for(auto &rightRule : rightRules){
                int idx = symbols[rightRule];
                isGenerating = generateSymbols[idx];
                if (!isGenerating)
                    break;
            }

        } else{
            isGenerating = true;
        }

        if(isGenerating){
            ruleGen.emplace_back(item.first, item.second);
        } else{
            hasUseless = false;
        }
    }

    if(!ruleGen.empty()){
        int index = symbols[ruleList[0].first];
        for(int i = 0; i < symbolSize; i++){
            bool reachable = (i == index);
            reachableSymbols[i] = reachable;
        }

        //get reachable array
        isReachable(reachableSymbols, ruleGen);
        for(auto &i : ruleGen){
            for(auto &j : i.second){
                int idx = symbols[i.first];
                rea = reachableSymbols[idx];
                if(!rea){
                    break;
                }
            }

            if(rea){
                useful.emplace_back(i.first, i.second);
            } else{
                hasUseless = false;
            }
        }
    }
}

// Task 2
void RemoveUselessSymbols() {
    //update useful
    getUseless();
    if(!useful.empty()){
        string output;
        for(auto &item : useful){
            output += item.first + " -> ";
            if(!item.second.empty()){
                for(auto &rightRule : item.second){
                    output += rightRule + " ";
                }
            }else{
                output += "#";
            }
            output += "\n";
        }
        cout << output;

    }else{
        cout << "";
    }
}

void getFirst(){
    bool isChanged;
    do{
        isChanged = false;
        //bool isEpsilon = true;

        for (auto &item : ruleList){
            string left = item.first;
            vector<string> rightStatement = item.second;
            if (firstSet.count(left) == 0){
                firstSet[left] = vector<string>();
            }

            if (rightStatement.empty()){
                if (ifNotFind(firstSet[left], "#")){
                    firstSet[left].push_back("#");
                    isChanged = true;
                }

            } else{
                bool hasEpsilon;
                for (size_t i = 0; i < rightStatement.size(); i++){

                    if (isTerminal(rightStatement[i])){
                        if (ifNotFind(firstSet[left], rightStatement[i])){
                            firstSet[left].push_back(rightStatement[i]);
                            isChanged = true;
                        }
                        break;

                    } else if (isNonterminal(rightStatement[i])){
                        hasEpsilon = false;
                        if (!firstSet[rightStatement[i]].empty()) {
                            vector<string> set = firstSet[rightStatement[i]];
                            for (auto &setItem : set) {
                                if (ifNotFind(firstSet[left], setItem) && setItem != "#") {
                                    firstSet[left].push_back(setItem);
                                    isChanged = true;
                                }
                                if (setItem == "#") {
                                    hasEpsilon = true;
                                }
                            }

                            if (!hasEpsilon) {
                                break;

                            } else if (i + 1 == rightStatement.size()){
                                if (ifNotFind(firstSet[left], "#")){
                                    firstSet[left].push_back("#");
                                    isChanged = true;
                                }
                            }

                        } else{
                            break;
                        }
                    }
                }
            }

        }

    } while(isChanged);

}

void printFirst(){
    for (auto &item : addOrder){
        cout << "FIRST(" << item << ") = { ";
        vector<string> set = firstSet[item];
        size_t i = 0;
        for (const auto & j : terminalOrder){
            if (!ifNotFind(set, j)){
                cout << j;
                if (i + 1 != set.size()){
                    cout << ", ";
                    i++;
                } else{
                    break;
                }
            }
        }

        cout << " }" << endl;
    }
}

void getFollow() {
    bool isChanged, hasEpsilon;
    string firstSymbol = nonTerminals[0];
    followSet[firstSymbol].push_back("$");

    for (auto &item : ruleList){
        string left = item.first;
        vector<string> rights = item.second;
        for (size_t idx = 0; idx < rights.size(); idx++){
            if (isNonterminal(rights[idx])){
                for (size_t idx2 = idx + 1; idx2 < rights.size(); idx2++){
                    hasEpsilon = false;
                    if (!ifNotFind(firstSet[rights[idx2]], "#")){
                        hasEpsilon = true;
                    }

                    for (auto & element : firstSet[rights[idx2]]){
                        if(ifNotFind(followSet[rights[idx]], element) && element != "#"){
                            followSet[rights[idx]].push_back(element);
                        }
                    }

                    if (!hasEpsilon){
                        break;
                    }
                }
            }
        }
    }

    do {
        isChanged = false;
        for (auto &item : ruleList){
            string left = item.first;
            vector<string> rights = item.second;

            for (size_t idx = 0; idx < rights.size(); idx++){
                hasEpsilon = true;
                if (isNonterminal(rights[idx])){
                    if(idx != rights.size() - 1){
                        for (size_t idx2 = idx + 1; idx2 < rights.size(); idx2++){
                            if (ifNotFind(firstSet[rights[idx2]], "#")){
                                hasEpsilon = false;
                                break;
                            }
                        }

                        if (hasEpsilon){
                            for (auto &element : followSet[left]){
                                if (ifNotFind(followSet[rights[idx]], element)){
                                    followSet[rights[idx]].push_back(element);
                                    isChanged = true;
                                }
                            }
                        }
                    } else{
                        for (auto &element : followSet[left]){
                            if (ifNotFind(followSet[rights[idx]], element)){
                                followSet[rights[idx]].push_back(element);
                                isChanged = true;
                            }
                        }
                    }
                }
            }
        }

    } while (isChanged);
}

void printFollow(){
    getFirst();
    for (auto &item : terminals){
        firstSet[item].push_back(item);
    }
    firstSet["#"].push_back("#");

    getFollow();
    for (auto &item : addOrder){
        cout << "FOLLOW(" << item << ") = { ";
        vector<string> set = followSet[item];
        size_t i = 0;

        for (const auto & j : terminalOrder){
            if (!ifNotFind(set, j)){
                cout << j;
                if (i + 1 != set.size()){
                    cout << ", ";
                    i++;
                } else{
                    break;
                }
            }
        }

        cout << " }" << endl;
    }
}

vector<string> rhsFirst(const vector<string>& ruleBody) {
    vector<string> result;
    bool hasEpsilon = true;
    if (ruleBody.empty()) {
        result.emplace_back("#");
    }else{
        for(auto &item : ruleBody){
            hasEpsilon = !ifNotFind(firstSet[item], "#");

            for(auto &firstSetItem : firstSet[item]){
                if(ifNotFind(result, firstSetItem) && firstSetItem != "#"){
                    result.push_back(firstSetItem);
                }
            }

            if(!hasEpsilon){
                break;
            }
        }

        if(hasEpsilon){
            result.emplace_back("#");
        }
    }
    return result;
}

bool checkIfIntersec(const vector<string>& vec1, const vector<string>& vec2){
    for(auto &item : vec1){
        if(!ifNotFind(vec2, item)){
            return true;
        }
    }
    return false;
}
// Task 5
void checkPP()
{
    string output;
    getUseless();
    firstSet["#"].push_back("#");
    if(!hasUseless){
        output = "NO";

    }else{
        getFirst();
        bool ifIntersec = true;

        for(auto &item : terminals){
            firstSet[item].push_back(item);
        }
        firstSet["#"].push_back("#");

        for(int idx = 0; idx < ruleList.size(); idx++){
            for(int idx2 = idx + 1; idx2 < ruleList.size(); idx2++){
                if(ruleList[idx].first == ruleList[idx2].first){
                    vector<string> first1 = rhsFirst(ruleList[idx].second);
                    vector<string> first2 = rhsFirst(ruleList[idx2].second);

                    ifIntersec = checkIfIntersec(first1, first2);
                    if(ifIntersec){
                        goto outLoop;
                    }
                }
            }
        }

        outLoop:

        bool ifIntersec2 = false;
        if(ifIntersec){
            output = "NO";
        }else{
            /*getFirst();
            for (auto &item : terminals){
                firstSet[item].push_back(item);
            }
            firstSet["#"].push_back("#");

            */
            getFollow();
            for(auto &item : nonTerminals){
                if(!(ifNotFind(firstSet[item], "#"))){
                    ifIntersec2 = checkIfIntersec(firstSet[item], followSet[item]);
                }
                if (ifIntersec2){
                    break;
                }
            }

            output = ifIntersec2 ? "NO" : "YES";
        }
    }
    cout << output;
}


int main (int argc, char* argv[])
{
    int task;
    if (argc < 2)
    {
        cout << "Error: missing argument\n";
        //return 1;
    }

    /*
       Note that by convention argv[0] is the name of your executable,
       and the firstSet argument to your program is stored in argv[1]
     */

    task = atoi(argv[1]);


    ReadGrammar();  // Reads the input grammar from standard input
    // and represent it internally in data structures
    // ad described in project 2 presentation file

    switch (task) {
        case 1: printForTask1();
            break;

        case 2:
            RemoveUselessSymbols();
            break;

        case 3:
            getFirst();
            printFirst();
            break;

        case 4:
            printFollow();
            break;

        case 5:
            checkPP();
            break;

        default:
            cout << "Error: unrecognized task number " << task << "\n";
            break;
    }

    return 0;
}