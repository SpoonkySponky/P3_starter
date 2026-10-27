
#include "tools/calculator.h"
#include <stack>
#include <string>
#include <vector>

static int precedence(CalcToken::Kind k) {
    if (k == CalcToken::Kind::Times ||
        k == CalcToken::Kind::Divide) {
        return 2;
    }

    return 1;
}

static bool is_operator(CalcToken::Kind k) {
    return k == CalcToken::Kind::Plus ||
           k == CalcToken::Kind::Minus ||
           k == CalcToken::Kind::Times ||
           k == CalcToken::Kind::Divide;
}

static bool goes_first(CalcToken::Kind top,
                       CalcToken::Kind current) {
    return is_operator(top) &&
           precedence(top) >= precedence(current);
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

    double right = values.top();
    values.pop();

    double left = values.top();
    values.pop();

    if (op == CalcToken::Kind::Plus) {
        values.push(left + right);
    }
    else if (op == CalcToken::Kind::Minus) {
        values.push(left - right);
    }
    else if (op == CalcToken::Kind::Times) {
        values.push(left * right);
    }
    else if (op == CalcToken::Kind::Divide) {
        if (right == 0.0) {
            *error = "division by zero";
            return false;
        }

        values.push(left / right);
    }
    else {
        *error = "cannot solve expression";
        return false;
    }

    return true;
}

bool calc_evaluate(const std::vector<CalcToken>& tokens,
                   double* out,
                   std::string* error) {
    if (out == nullptr || error == nullptr) {
        return false;
    }

    std::stack<double> values;
    std::stack<CalcToken::Kind> ops;

    for (const CalcToken& t : tokens) {
        if (t.kind == CalcToken::Kind::Number) {
            values.push(t.value);
        }
        else if (t.kind == CalcToken::Kind::LParen) {
            ops.push(t.kind);
        }
        else if (t.kind == CalcToken::Kind::RParen) {
            while (!ops.empty() &&
                   ops.top() != CalcToken::Kind::LParen) {
                if (!apply_top(values, ops, error)) {
                    return false;
                }
            }

            if (ops.empty()) {
                *error = "cannot solve expression";
                return false;
            }

            ops.pop();
        }
        else if (is_operator(t.kind)) {
            while (!ops.empty() &&
                   goes_first(ops.top(), t.kind)) {
                if (!apply_top(values, ops, error)) {
                    return false;
                }
            }

            ops.push(t.kind);
        }
        else {
            *error = "cannot solve expression";
            return false;
        }
    }

    while (!ops.empty()) {
        if (ops.top() == CalcToken::Kind::LParen) {
            *error = "cannot solve expression";
            return false;
        }

        if (!apply_top(values, ops, error)) {
            return false;
        }
    }

    if (values.size() != 1) {
        *error = "cannot solve expression";
        return false;
    }

    *out = values.top();
    error->clear();

    return true;
}
