// src/tools/calculator_eval.cpp — YOU IMPLEMENT (spec §4.6).
//
// calc_evaluate: evaluate a well-formed token list with two stacks
// (one for numbers, one for operators and '('). See spec §4.6 and the
// worked trace in Appendix C. std::stack or std::vector are both fine.

#include "tools/calculator.h"
#include <stack>
#include <string>
#include <vector>

static int precedence(CalcToken::Kind k) { //determines operation precedence
    if (k == CalcToken::Kind::Times || k == CalcToken::Kind::Divide){
        return 2; //Returns 2 if the operation is multiply or divide
    }
    return 1; //returns 1 for addition and subtraction
}
static bool is_operator(CalcToken::Kind k) {
    return k == CalcToken::Kind::Plus ||
           k == CalcToken::Kind::Minus ||
           k == CalcToken::Kind::Times ||
           k == CalcToken::Kind::Divide;
}

static bool goes_first(CalcToken::Kind top, CalcToken::Kind current) {
    return is_operator(top) && precedence(top) >= precedence(current);
}

static bool apply_top(std::stack<double>& values,
                      std::stack<CalcToken::Kind>& ops,
                      std::string* error) {
    if (ops.empty() || values.size() < 2) {
        *error = "cannot solve expression";
        return false;
    }
    CalcToken::Kind op = ops.top();
    ops.pop(); 
    double right = values.top();  //First pop is for the right number 
    values.pop();
    double left = values.top(); //Second pop is for the left number
    values.pop();

    if (op == CalcToken::Kind::Plus){
        values.push(left + right);
    }else if (op == CalcToken::Kind::Minus){
        values.push(left - right);
    }else if (op == CalcToken::Kind::Times){
        values.push(left * right);
    }else{  
        if (right == 0.0) {
            *error = "division by zero";
            return false;
        }
        values.push(left / right);
    }
    return true;
}


bool calc_evaluate(const std::vector<CalcToken>& tokens, double* out,
                   std::string* error) {
    std::stack<double>          values;
    std::stack<CalcToken::Kind> ops;

    for (const CalcToken* t : tokens){
        if(t.kind == CalcToken::Kind::Number){
            values.push(t.value);
        }else if(t.kind == CalcToken::Kind::LParen){
            ops.push(t.kind);
        }else if(t.kind == CalcToken::Kind::RParen){
            while(!ops.empty() && is_operator(ops.top())){
                bool success = apply_top(values, ops, error);
                if(success == false){
                    return false;
                }
            }
            if(ops.empty()){
                *error = "cannot solve expression";
                return false;
            }
            ops.pop();
        }else if(is_operator(t.kind)){
            while(!ops.empty() && goes_first(ops.top(), t.kind)){
                bool success = apply_top(values, ops, error);
                if(success == false){
                    return false;
                }
            }
            ops.push(t.kind);
        }else{
            if(error != nullptr){
                *error = "cannot solve expression";
            }
            return false;
        }
        
        while(!ops.empty() && is_operator(ops.top())){
            bool success = apply_top(values, ops, error);
            if(success == false){
                return false;
            }
        }
            if(!opts.empty() || values.size() != 1){
                *error = "cannot solve expression";
                return false;
            }
        *out = values.top();
        return true;
}

