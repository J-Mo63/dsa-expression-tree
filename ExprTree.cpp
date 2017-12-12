/*
 * Assignment 1
 * Jonathan Moallem
 * 12582589
 */

#include "ExprTree.h"
#include <sstream>
#include <iostream>

/*
 * Forward declarations
 */
int findSize(TreeNode* node);
void log(string s);
string trim(const string& str);
string operatorToString(Operator op);

vector<string> buildShunt(vector<string> tokens);
bool hasHigherOrder(string current, string last);

int performOperation(int a, int b, Operator op);

string postfix(TreeNode* node);
string prefix(TreeNode* node);
string infix(TreeNode* node);






/*
 * Helper functions that tests whether a string is a non-negative integer.
 */
bool isdigit(const char & c){

    switch (c) {
    case '0' :
    case '1' :
    case '2' :
    case '3' :
    case '4' :
    case '5' :
    case '6' :
    case '7' :
    case '8' :
    case '9' : return true;
    }

    return false;
   
}
bool is_number(const std::string & s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

/*
 * Helper function that converts a string to an int.
 */
int to_number(const std::string & s){
    return atoi(s.c_str());
}

/*
 * Helper function that converts a number to a string.
 */
string to_string(const int& n){
    std::stringstream stream;
    stream << n;
    return stream.str();
}

/*
 * Helper function that creates a TreeNode with the appropriate operator
 * when given a string that's "+", "-", "*" or "/". If the string is wrong
 * it gives a NoOp value.
 */
TreeNode* createOperatorNode(const string& op){

    if (op == "+") return new TreeNode(Plus);
    if (op == "-") return new TreeNode(Minus);
    if (op == "*") return new TreeNode(Times);
    if (op == "/") return new TreeNode(Divide);
    return new TreeNode(NoOp);

}








/*
 * Basic constructor that sets up an empty Expr Tree.
 */
ExprTree::ExprTree(){
    root = 0;
    _size = 0;
}

/*
 * Constructor that takes a TreeNode and sets up an ExprTree with that node at the root.
 */
ExprTree::ExprTree(TreeNode* r){
    root = r;
    _size = findSize(root);
}

/*
 * Destructor to clean up the tree.
 */
ExprTree::~ExprTree(){
    // nothing to do here...
}

/*
 * This is a helper function to find the size of a tree from a given node.
 */
int findSize(TreeNode* node) {
    if (node == 0) {
        return 0;
    }
    return 1 + findSize(node->getLeftChild()) + findSize(node->getRightChild());
}











/*
 * This function takes a string representing an arithmetic expression and breaks
 * it up into components (number, operators, parentheses).
 * It returns the broken up expression as a vector of strings.
 */
vector<string> ExprTree::tokenise(string expression){
    vector<string> output;
    string item;
    std::deque<string> numberBuff;

    // Loop through the expression...
    for (int i = 0; i < expression.length(); i++) {
        std::stringstream ss;
        ss << expression[i];
        ss >> item;

        // If current item is a space then dump and purge cache to output
        if (expression[i] == ' ') {
            string collection = "";
            while (!numberBuff.empty()) {
                collection += numberBuff.back();
                numberBuff.pop_back();
            }
            if (collection != "") {
                output.push_back(collection);
            }
            numberBuff.clear();
        }
        else {
            // If it's not a number then dump and purge cache to output
            // Then append the operator
            if (!is_number(item)) {
                string collection = "";
                while (!numberBuff.empty()) {
                    collection += numberBuff.back();
                    numberBuff.pop_back();
                }
                if (collection != "") {
                    output.push_back(collection);
                }
                numberBuff.clear();

                output.push_back(item);
            }
            // If it's a number then put it into the buffer in case it's more than one digit
            else {
                numberBuff.push_front(item);
            }
        }
    }
    // Dump and purge any remaing values in the cache
    string collection = "";
    while (!numberBuff.empty()) {
        collection += numberBuff.back();
        numberBuff.pop_back();
    }
    if (collection != "") {
        output.push_back(collection);
    }
    numberBuff.clear();

    return output;
}











/*
 * This function takes a vector of strings representing an expression (as produced
 * by tokenise(string), and builds an ExprTree representing the same expression.
 */
ExprTree ExprTree::buildTree(vector<string> tokens){
    // Convert tokens to a postfix vector that has had a Shunting-Yard algorithm run on it
    vector<string> shunt = buildShunt(tokens);

    std::stack<TreeNode*> stack;

    // Loop through the values of the shunted vector...
    for (int i = 0; i < shunt.size(); i++) {
        // If value is a number, add to stack
        if (is_number(shunt[i])) {
            stack.push(new TreeNode(to_number(shunt[i])));
        }
        // Otherwise create a node from the value and make the top
        // two values on the stack its children
        else {
            TreeNode* node = createOperatorNode(shunt[i]);
            node->setRightChild(stack.top());
            stack.pop();
            node->setLeftChild(stack.top());
            stack.pop();

            stack.push(node);
        }
    }

    // Create a new expression tree with the remaining node as its root
    ExprTree e = ExprTree(stack.top());

    return e;
}

/*
 * This is a helper function to return a postfix vector that has 
 * had a Shunting-Yard algorithm run on it. It takes a single infix
 * vector of tokenised values.
 */
vector<string> buildShunt(vector<string> tokens) {
    vector<string> output;
    std::stack<string> stack;

    // Loop through the tokens...
    for (int i = 0; i < tokens.size(); i++) {
        // if the token is a number add it to output
        if (is_number(tokens[i])) {
            output.push_back(tokens[i]);
        }
        else {
            // If it is a left bracket or the stack is empty, push it to the stack
            if (stack.empty() || tokens[i] == "(") {
                stack.push(tokens[i]);
            }
            // If it is a right bracket then dump values from stack to output
            // until the matching bracket is found - then discard them
            else if (tokens[i] == ")") {
                while (!stack.empty() && stack.top() != "(") {
                    output.push_back(stack.top());
                    stack.pop();
                }
                stack.pop();
            }
            // If it is an operator with a higher order, push it to stack
            else if (hasHigherOrder(tokens[i], stack.top())) {
                stack.push(tokens[i]);
            }
            // Otherwise dump from stack to output until something with a higher order is found
            else {
                while (!stack.empty() && !hasHigherOrder(tokens[i], stack.top())) {
                    output.push_back(stack.top());
                    stack.pop();
                }
                stack.push(tokens[i]);
            }
        }
    }
    // Dump any remaining values in the stack to output
    while (!stack.empty()) {
        output.push_back(stack.top());
        stack.pop();
    }

    return output;
}

/*
 * This is a helper function to return the precedence value of an operator
 * that has been converted to a string.
 */
int getPrecedence(string op) {
    if (op == "+" || op == "-") {
        return 2;
    }
    else if (op == "*" || op == "/") {
        return 3;
    }
    else {
        return 0;
    }
}

/*
 * This is a simple helper function to return whether one operator has a higher operation
 * order than another value.
 */
bool hasHigherOrder(string current, string last) {
    if (getPrecedence(current) > getPrecedence(last)) {
        return true;
    }
    return false;
}











/*
 * This function takes a TreeNode and does the maths to calculate
 * the value of the expression it represents.
 */
int ExprTree::evaluate(TreeNode* node){
    // If it is a value node then return its value
    if (node->isValue()) {
        return node->getValue();
    }
    int left = evaluate(node->getLeftChild());
    int right = evaluate(node->getRightChild());
    // Otherwise return the result of this operation on each of its childern
    return performOperation(left, right, node->getOperator());
}

/*
 * This function is a helper method that returns the result of an operation on two values.
 */
int performOperation(int a, int b, Operator op) {
    switch (op) {
        case Plus : return a + b;
        case Minus : return a - b;
        case Times : return a * b;
        case Divide : return a / b;
        default: return 0;
    }
}

/*
 * When called on an ExprTree, this function calculates the value of the
 * expression represented by the whole tree.
 */
int ExprTree::evaluateWholeTree(){
    return evaluate(root);
}
















/*
 * Given an ExprTree t, this function returns a string
 * that represents that same expression as the tree in
 * prefix notation.
 */
string ExprTree::prefixOrder(const ExprTree& t){
    return trim(prefix(t.root));
}

/*
 * I didn't like definition of the prefixOrder() function so I decided to make it call my
 * own version of it so I could use recursion.
 */
string prefix(TreeNode* node){
    string output = "";

    // Get node value
    if (node->getOperator() == 0) {
        output += to_string(node->getValue()) + " ";
    }
    else {
        output += operatorToString(node->getOperator()) + " ";
    }

    // Recurse left
    if (node->getLeftChild() != 0) {
        output += prefix(node->getLeftChild());
    }

    // Recurse right
    if (node->getRightChild() != 0) {
        output += prefix(node->getRightChild());
    }

    return output;
}

/*
 * Given an ExprTree t, this function returns a string
 * that represents that same expression as the tree in
 * infix notation.
 */
string ExprTree::infixOrder(const ExprTree& t){
    return trim(infix(t.root));
}

/*
 * I didn't like definition of the infixOrder() function so I decided to make it call my
 * own version of it so I could use recursion.
 */
string infix(TreeNode* node){
    string output = "";

    // Recurse left
    if (node->getLeftChild() != 0) {
        output += infix(node->getLeftChild());
    }

    // Get node value
    if (node->getOperator() == 0) {
        output += to_string(node->getValue()) + " ";
    }
    else {
        output += operatorToString(node->getOperator()) + " ";
    }

    // Recurse right
    if (node->getRightChild() != 0) {
        output += infix(node->getRightChild());
    }

    return output;
}

/*
 * Given an ExprTree t, this function returns a string
 * that represents that same expression as the tree in
 * postfix notation.
 */
string ExprTree::postfixOrder(const ExprTree& t){
    return trim(postfix(t.root));
}

/*
 * I didn't like definition of the postfixOrder() function so I decided to make it call my
 * own version of it so I could use recursion.
 */
string postfix(TreeNode* node){
    string output = "";

    // Recurse left
    if (node->getLeftChild() != 0) {
        output += postfix(node->getLeftChild()) + " ";
    }

    // Recurse right
    if (node->getRightChild() != 0) {
        output += postfix(node->getRightChild()) + " ";
    }

    // Get node value
    if (node->getOperator() == 0) {
        output += to_string(node->getValue());
    }
    else {
        output += operatorToString(node->getOperator());
    }

    return output;
}

/*
 * This is a helper function to convert operators to strings representing those operations.
 */
string operatorToString(Operator op) {
    switch (op) {
        case 1 : return "+";
        case 2 : return "-";
        case 3 : return "*";
        case 4 : return "/";
        default: return 0;
    }
}

/*
 * This is a helper function to trim the trailing space from the output of my recursive
 * traversal algorithms so that the CxxTest file doesn't chuck and tantrum about comparing the
 * two strings.
 */
string trim(const string& str) {
    size_t first = str.find_first_not_of(' ');
    if (string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}












/*
 * Returns the size of the tree. (i.e. the number of nodes in it)
 */
int ExprTree::size(){ return _size; }

/*
 * Returns true if the tree contains no nodes. False otherwise.
 */
bool ExprTree::isEmpty(){ return _size == 0; }

TreeNode * ExprTree::getRoot(){ return root; }












/*
 * This is a helper function I used in order to easily get debugging output in my console.
 */
void log(string s) {
    std::cout << std::endl << "<=== BEGIN LOG ===>" << std::endl;
    std::cout << s;
    std::cout << std::endl << "<===  END LOG  ===>" << std::endl << std::endl;
}