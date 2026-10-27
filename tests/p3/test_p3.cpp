// tests/p3/test_p3.cpp — YOUR P3 TEST SUITE.
//
// The five tests below are worked examples. Keep them, and add your own
// until you have AT LEAST 15 (spec §6 lists what must be covered).
// Name every test <area>_<what>: reg_ for the registry, ws_ for the
// workspace, calc_ for the calculator.
//
// Build & run:   cmake --build build && ./build/test_p3
// One area:      ./build/test_p3 reg_        (names starting with "reg_")
// One test:      ./build/test_p3 =reg_add_and_find

#include "../mini_test.h"
#include "../test_helpers.h"     // TempDir, write_text, args1, ...

#include "tools/calculator.h"
#include "tools/tool_registry.h"
#include "tools/workspace.h"

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

// A tool whose name you choose — use it to fill the registry in tests.
class NamedTool : public Tool {
public:
    explicit NamedTool(std::string n) : n_(std::move(n)), schema_(JsonValue::object()) {}
    std::string name() const override { return n_; }
    std::string description() const override { return "test tool"; }
    const JsonValue& schema() const override { return schema_; }
    ToolResult invoke(const JsonValue&, ToolContext&) override { return ToolResult::success(n_); }
private:
    std::string n_;
    JsonValue   schema_;   // an empty JSON object: {}
};

// Tokenize with the provided tokenizer, then run YOUR calc_evaluate.
bool eval(const std::string& expr, double* value, std::string* error) {
    std::vector<CalcToken> tokens;
    if (!calc_tokenize(expr, &tokens, error)) return false;
    return calc_evaluate(tokens, value, error);
}

}  // namespace

// ---------------------------------------------------------------- examples

TEST(reg_add_and_find) {
    ToolRegistry r;
    auto t = std::make_unique<NamedTool>("alpha");
    Tool* raw = t.get();                        // remember the address before moving
    REQUIRE(r.add(std::move(t)));
    CHECK(r.size() == 1);
    CHECK(r.find("alpha") == raw);              // same object, not a copy
    CHECK(r.find("beta") == nullptr);
}

TEST(reg_duplicate_keeps_original) {
    ToolRegistry r;
    auto first = std::make_unique<NamedTool>("alpha");
    Tool* original = first.get();
    REQUIRE(r.add(std::move(first)));
    CHECK(!r.add(std::make_unique<NamedTool>("alpha")));
    CHECK(r.find("alpha") == original);
    CHECK(r.size() == 1);
}

TEST(ws_rejects_parent_escape) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("notes/todo.txt", &out, &reason));   // sanity: accepts *something*
    CHECK(out == ws.root() / "notes" / "todo.txt");
    CHECK(!ws.resolve("../etc/passwd", &out, &reason));
    CHECK(!reason.empty());
}

TEST(calc_precedence) {
    double v = 0;
    std::string err;
    REQUIRE(eval("2 + 3 * 4", &v, &err));
    CHECK(v == 14);
}

TEST(calc_division_by_zero) {
    double v = 0;
    std::string err;
    CHECK(!eval("1 / 0", &v, &err));
    CHECK(err == "division by zero");
}

// ------------------------------------------------------------- your tests
//
// Still to write (spec §6 has the full list). Suggested names:
//   reg_null_rejected, reg_for_each_visits_each_once
//   reg_10000_tools               REQUIRED: print bucket_count / load_factor /
//                                 max_probe_length, check the bound
//   reg_alternative_hash          REQUIRED: longest chain of a weak hash
//   ws_<case>                     REQUIRED: every row of the table in spec §4.4
//   calc_<case>                   left-to-right, parentheses, negatives

TEST(reg_null_rejected) {
    ToolRegistry r;

    CHECK(!r.add(nullptr));
    CHECK(r.size() == 0);
}

TEST(reg_for_each_visits_each_once) {
    ToolRegistry r;
    REQUIRE(r.add(std::make_unique<NamedTool>("alpha")));
    REQUIRE(r.add(std::make_unique<NamedTool>("beta")));
    REQUIRE(r.add(std::make_unique<NamedTool>("gamma")));

    int count = 0;
    r.for_each([&count](const Tool&) {
        count++;
    });

    CHECK(count == 3);
}

TEST(reg_10000_tools) {
    ToolRegistry r;
    Tool* first = nullptr;

    for (int i = 0; i < 10000; i++) {
        std::string name = "tool_" + std::to_string(i);
        auto tool = std::make_unique<NamedTool>(name);

        if (i == 0) {
            first = tool.get();
        }

        REQUIRE(r.add(std::move(tool)));
        CHECK(r.load_factor() <= 1.0);
    }

    CHECK(r.size() == 10000);
    CHECK(r.find("tool_0") == first);
    CHECK(r.max_probe_length() <= 64);

    for (int i = 0; i < 10000; i++) {
        std::string name = "tool_" + std::to_string(i);
        CHECK(r.find(name) != nullptr);
    }

    std::printf("buckets: %zu\n", r.bucket_count());
    std::printf("load factor: %f\n", r.load_factor());
    std::printf("max probe length: %zu\n", r.max_probe_length());

    std::vector<int> counts(r.bucket_count(), 0);

    for (int i = 0; i < 10000; i++) {
        std::string name = "tool_" + std::to_string(i);
        std::size_t hash = name[0] + name.size();
        counts[hash % r.bucket_count()]++;
    }

    int longest = 0;
    for (int count : counts) {
        if (count > longest) {
            longest = count;
        }
    }

    std::printf("weak hash longest chain: %d\n", longest);
}

TEST(reg_alternative_hash) {
    ToolRegistry r;

    for (int i = 0; i < 10000; i++) {
        std::string name = "tool_" + std::to_string(i);
        REQUIRE(r.add(std::make_unique<NamedTool>(name)));
    }

    std::vector<int> counts(r.bucket_count(), 0);

    for (int i = 0; i < 10000; i++) {
        std::string name = "tool_" + std::to_string(i);

        // A simple alternative hash
        std::size_t hash = name[0] + name.size();
        counts[hash % r.bucket_count()]++;
    }

    int longest = 0;
    for (int count : counts) {
        if (count > longest) {
            longest = count;
        }
    }

    std::printf("alternative hash longest chain: %d\n", longest);
    CHECK(longest > 0);
}
//WS Tests
TEST(ws_accepts_normal_path) {
    Workspace ws("test_workspace");
    fs::path out;
    std::string reason;

    CHECK(ws.resolve("notes/todo.txt", &out, &reason));
    CHECK(out == ws.root() / "notes/todo.txt");
}

TEST(ws_normalizes_path) {
    Workspace ws("test_workspace");
    fs::path out;
    std::string reason;

    CHECK(ws.resolve("./notes//todo.txt", &out, &reason));
    CHECK(out == ws.root() / "notes/todo.txt");
}

TEST(ws_accepts_current_directory) {
    Workspace ws("test_workspace");
    fs::path out;
    std::string reason;

    CHECK(ws.resolve(".", &out, &reason));
    CHECK(out == ws.root());
}

TEST(ws_normalizes_parent_inside) {
    Workspace ws("test_workspace");
    fs::path out;
    std::string reason;

    CHECK(ws.resolve("a/../b", &out, &reason));
    CHECK(out == ws.root() / "b");
}

TEST(ws_normalizes_parent_to_root) {
    Workspace ws("test_workspace");
    fs::path out;
    std::string reason;

    CHECK(ws.resolve("a/..", &out, &reason));
    CHECK(out == ws.root());
}

TEST(ws_accepts_new_path) {
    Workspace ws("test_workspace");
    fs::path out;
    std::string reason;

    CHECK(ws.resolve("new/dir/file.txt", &out, &reason));
    CHECK(out == ws.root() / "new/dir/file.txt");
}

TEST(ws_accepts_dots_in_filename) {
    Workspace ws("test_workspace");
    fs::path out;
    std::string reason;

    CHECK(ws.resolve("my..notes.txt", &out, &reason));
    CHECK(out == ws.root() / "my..notes.txt");
}

TEST(ws_rejects_empty_path) {
    Workspace ws("test_workspace");
    fs::path out;
    std::string reason;

    CHECK(!ws.resolve("", &out, &reason));
    CHECK(reason == "empty path");
}

TEST(ws_rejects_nul) {
    Workspace ws("test_workspace");
    fs::path out;
    std::string reason;
    std::string path = std::string("notes") + '\0' + "/x";

    CHECK(!ws.resolve(path, &out, &reason));
}

TEST(ws_rejects_absolute_path) {
    Workspace ws("test_workspace");
    fs::path out;
    std::string reason;

    CHECK(!ws.resolve("/etc/passwd", &out, &reason));
}

TEST(ws_rejects_dotdot) {
    Workspace ws("test_workspace");
    fs::path out;
    std::string reason;

    CHECK(!ws.resolve("..", &out, &reason));
}

TEST(ws_rejects_parent_path) {
    Workspace ws("test_workspace");
    fs::path out;
    std::string reason;

    CHECK(!ws.resolve("../etc/passwd", &out, &reason));
}

TEST(ws_rejects_multiple_parent_path) {
    Workspace ws("test_workspace");
    fs::path out;
    std::string reason;

    CHECK(!ws.resolve("a/../../b", &out, &reason));
}

TEST(ws_rejects_path_that_returns_inside) {
    Workspace ws("test_workspace");
    fs::path out;
    std::string reason;

    CHECK(!ws.resolve("notes/../../ws/notes/todo.txt", &out, &reason));
}

//Calculator Tests
TEST(calc_left_to_right) {
    double value = 0;
    std::string error;

    REQUIRE(eval("10 - 4 - 3", &value, &error));
    CHECK(value == 3);
}

TEST(calc_division_left_to_right) {
    double value = 0;
    std::string error;

    REQUIRE(eval("100 / 10 / 5", &value, &error));
    CHECK(value == 2);
}

TEST(calc_parentheses) {
    double value = 0;
    std::string error;

    REQUIRE(eval("(2 + 3) * 4", &value, &error));
    CHECK(value == 20);
}

TEST(calc_nested_precedence) {
    double value = 0;
    std::string error;

    REQUIRE(eval("2 * (3 + 4) - 5", &value, &error));
    CHECK(value == 9);
}

TEST(calc_negative_numbers) {
    double value = 0;
    std::string error;

    REQUIRE(eval("-3 * -3", &value, &error));
    CHECK(value == 9);
}

TEST(calc_decimal_result) {
    double value = 0;
    std::string error;

    REQUIRE(eval("7 / 2", &value, &error));
    CHECK(value == 3.5);
}

TEST(calc_extra_expression_one) {
    double value = 0;
    std::string error;

    REQUIRE(eval("(8 - 2) / 3", &value, &error));
    CHECK(value == 2);
}

TEST(calc_extra_expression_two) {
    double value = 0;
    std::string error;

    REQUIRE(eval("2 + 3 * (4 - 1)", &value, &error));
    CHECK(value == 11);
}

int main(int argc, char** argv) { return mt::run_all(argc, argv); }
