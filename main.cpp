#include <cassert>
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <span>
#include <stack>
#include <fstream>
#include <iterator>

#include "LazyRational.h"
#include "MultiInd.h"

enum class Operator {
    PLUS,
    MINUS,
    TIMES,
    DIVIDE
};

constexpr LazyRational apply_op(Operator op, LazyRational a, LazyRational b)
{
    switch (op)
    {
        case Operator::PLUS:
            return a + b;
        case Operator::MINUS:
            return a - b;
        case Operator::TIMES:
            return a * b;
        case Operator::DIVIDE:
            return a / b;
    }
}

// see https://scicomp.stackexchange.com/questions/40726/finding-all-valid-combinations-of-numeric-inputs-and-operators-in-a-reverse-poli
std::vector<std::string> tree_recurse(int N)
{
    if (N == 1)
        return {"l"};

    std::vector<std::string> res;

    for (int n = 1; n < N; ++n)
    {
        auto left_iter = tree_recurse(N - n);
        auto right_iter = tree_recurse(n);

        for (auto& left : left_iter)
            for (auto& right : right_iter)
                res.emplace_back(left + right + 'b');
    }

    return res;
}

template<size_t N>
LazyRational eval_tree(const std::string& tree, std::span<int, N> numbers, std::span<Operator, N-1> ops,
                       std::stack<LazyRational, std::vector<LazyRational>>& work)
{
    assert(tree.size() == 2*N - 1);

    int n_ind = 0;
    int o_ind = 0;

    for (auto c : tree) {
        if (c == 'l')
            work.push(numbers[n_ind++]);
        else {
            assert(work.size() >= 2);

            auto right = work.top();
            work.pop();
            auto left = work.top();
            work.pop();
            work.push(apply_op(ops[o_ind++], left, right));
        }
    }

    assert(n_ind == N && o_ind == N - 1);
    assert(work.size() == 1);
    auto ret = work.top();
    work.pop();
    return ret;
}

template<size_t N>
std::string eval_tree_string(const std::string& tree, std::span<int, N> numbers, std::span<Operator, N-1> ops)
{
    static constexpr auto k_op_str = "+-*/";

    assert(tree.size() == 2*N - 1);

    int n_ind = 0;
    int o_ind = 0;

    std::string ret;

    for (auto c : tree) {
        ret += ' ';
        if (c == 'l')
            ret += std::to_string(numbers[n_ind++]);
        else
            ret += k_op_str[static_cast<int>(ops[o_ind++])];
    }

    return ret;
}

int main(int argc, const char* argv[])
{
    auto args = std::span(argv, argc);
    bool write_output = args.size() > 1 && std::string(args[1]) == "yes";

    constexpr std::array<Operator, 4> k_ops = {Operator::PLUS, Operator::MINUS, Operator::TIMES, Operator::DIVIDE};
    constexpr int k_N = 25;
    constexpr int k_choose = 5;
    constexpr int k_max = 100;

    // get vector of strings
    auto trees = tree_recurse(k_choose);

    // loop over all permutations of numbers 1 - k_N
    std::vector<int> all_numbers(k_N);
    std::iota(all_numbers.begin(),all_numbers.end(),1);

    // evaluation stack
    std::stack<LazyRational, std::vector<LazyRational>> work;

    // to count the number of expressions with a given output that succeeded or failed
    std::array<size_t, k_max> succeeded = {};
    std::array<size_t, k_max> failed = {};

    std::array<std::string, k_max> eval_strings = {};

    // to write results
    std::ofstream succeeded_file;
    std::ofstream failed_file;
    if (write_output) {
        succeeded_file = std::ofstream("succeeded.txt", std::ios::out | std::ios::trunc);
        failed_file = std::ofstream("failed.txt", std::ios::out | std::ios::trunc);
    }

    size_t iter_count = 0;

    // combinations of numbers
    for (auto multi_num = MultiIndCombo<k_N, k_choose>(); multi_num.valid(); ++multi_num) {
        auto numbers = multi_num.extract<int, std::dynamic_extent>(all_numbers);
        std::array<bool, k_max> found = {}; // initialize all to false

        // permutations of numbers
        do {
            // permuatations for all operators with replacement
            for (auto multi_op = MultiInd<4, 4>(); multi_op.valid(); ++multi_op)
            {
                auto ops = multi_op.extract<const Operator, 4>(k_ops);

                for (const auto &tree: trees)
                {
                    auto res = eval_tree<k_choose>(tree, numbers, ops, work);
                    res.reduce();
                    if (res.denom == 1 && res.num >= 0 && res.num < k_max) {
                        found[res.num] = true;

                        if (write_output && eval_strings[res.num].empty())
                            eval_strings[res.num] = eval_tree_string<k_choose>(tree, numbers, ops);
                    }
                }
            }
        }
        while (std::next_permutation(numbers.begin(), numbers.end()));

        // count successes and failures and optionally write to output
        for (size_t i = 0; i < k_max; ++i) {
            if (found[i]) {
                ++succeeded[i];

                if (write_output) {
                    std::ranges::copy(numbers, std::ostream_iterator<int>(succeeded_file, " "));
                    succeeded_file << i;
                    succeeded_file << eval_strings[i] << std::endl;
                }
            }
            else {
                ++failed[i];

                if (write_output) {
                    std::ranges::copy(numbers, std::ostream_iterator<int>(failed_file, " "));
                    failed_file << i << std::endl;
                }
            }
        }

        // print progress
        if (++iter_count % 100 == 0)
            std::cout << iter_count << "\n";

        // reset eval_strings if we're writing output
        if (write_output)
            eval_strings = {};
    }

    // print results
    size_t tot_succeeded = 0;
    size_t tot_failed = 0;

    std::cout << "i succeeded failed fraction_success\n";

    for (size_t i = 0; i < k_max; ++i) {
        tot_succeeded += succeeded[i];
        tot_failed += failed[i];
        std::cout << i << " " << succeeded[i] << " " << failed[i] << " "
            << double(succeeded[i]) / double(succeeded[i] + failed[i]) << "\n";
    }

    std::cout << "\ntotal " << tot_succeeded << " " << tot_failed << " "
        << double(tot_succeeded) / double(tot_succeeded + tot_failed) << "\n";

    return 0;
}
