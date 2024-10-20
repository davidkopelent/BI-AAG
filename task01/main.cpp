#ifndef __PROGTEST__
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <functional>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <optional>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <vector>

using State = unsigned;
using Symbol = uint8_t;
using Word = std::vector<Symbol>;

struct MISNFA
{
    std::set<State> states;
    std::set<Symbol> alphabet;
    std::map<std::pair<State, Symbol>, std::set<State>> transitions;
    std::set<State> initialStates;
    std::set<State> finalStates;
};

struct DFA
{
    std::set<State> states;
    std::set<Symbol> alphabet;
    std::map<std::pair<State, Symbol>, State> transitions;
    State initialState;
    std::set<State> finalStates;

    bool operator==(const DFA &x) const = default;
};
#endif

class Determinize
{
public:
    Determinize(const MISNFA &NFA);
    DFA convert(void);
    DFA complement(void);

private:
    void removeUselessStates(void);
    void setFinalStates(void);
    void swapStates(void);

    MISNFA nfa;
    DFA dfa;
    State next_state;
    std::map<std::set<State>, State> references;
    std::queue<std::set<State>> to_discover;
    std::map<State, std::set<State>> predecessors;
    std::set<State> redundant;
};

Determinize::Determinize(const MISNFA &NFA) : nfa(NFA)
{
    next_state = 0;
    dfa.states.insert(next_state);
    dfa.initialState = next_state;
    dfa.alphabet = nfa.alphabet;
    references.insert({nfa.initialStates, next_state++});
    to_discover.push(nfa.initialStates);
}

void Determinize::removeUselessStates(void)
{
    std::set<State> reachable;
    std::queue<State> Q;
    Q.push(dfa.initialState);

    while (!Q.empty())
    {
        State current = Q.front();
        reachable.insert(current);
        Q.pop();

        for (const auto &alpha : dfa.alphabet)
        {
            auto it = dfa.transitions.find({current, alpha});
            if (it != dfa.transitions.end())
            {
                State nextState = it->second;
                if (reachable.find(nextState) == reachable.end())
                {
                    Q.push(nextState);
                }
            }
        }
    }

    std::set<State> canReachFinal;
    std::queue<State> reverseQueue;

    for (const auto &finalState : dfa.finalStates)
    {
        reverseQueue.push(finalState);
    }

    while (!reverseQueue.empty())
    {
        State current = reverseQueue.front();
        canReachFinal.insert(current);
        reverseQueue.pop();

        for (const auto &state : dfa.states)
        {
            for (const auto &alpha : dfa.alphabet)
            {
                auto it = dfa.transitions.find({state, alpha});
                if (it != dfa.transitions.end() && it->second == current)
                {
                    if (canReachFinal.find(state) == canReachFinal.end())
                    {
                        reverseQueue.push(state);
                    }
                }
            }
        }
    }

    for (const auto &state : dfa.states)
    {
        if (reachable.find(state) == reachable.end() || (canReachFinal.find(state) == canReachFinal.end() && dfa.finalStates.find(state) == dfa.finalStates.end()))
        {
            redundant.insert(state);
        }
    }

    for (const auto &state : redundant)
    {
        dfa.states.erase(state);
        dfa.finalStates.erase(state);
    }

    std::map<std::pair<State, Symbol>, State> non_redundant_transitions;
    for (const auto &t : dfa.transitions)
    {
        if ((redundant.find(t.first.first) == redundant.end()) && (redundant.find(t.second) == redundant.end()))
            non_redundant_transitions.insert({t.first, t.second});
    }
    
    if (dfa.states.empty())
        dfa.states.insert(dfa.initialState);
    dfa.transitions = non_redundant_transitions;
}

void Determinize::setFinalStates(void)
{
    for (const auto &a : references)
    {
        for (const auto &b : nfa.finalStates)
        {
            if (a.first.find(b) != a.first.end())
                dfa.finalStates.insert(a.second);
        }
    }
}

void Determinize::swapStates(void)
{
    State rejectState = next_state++;

    for (const auto &state : dfa.states)
    {
        for (const auto &alpha : dfa.alphabet)
        {
            if (dfa.transitions.find({state, alpha}) == dfa.transitions.end())
            {
                if (dfa.states.find(rejectState) == dfa.states.end())
                {
                    dfa.states.insert(rejectState);
                    for (const auto &alpha : dfa.alphabet)
                    {
                        dfa.transitions[{rejectState, alpha}] = rejectState;
                    }
                }
                dfa.transitions[{state, alpha}] = rejectState;
            }
        }
    }

    std::set<State> newFinalStates;

    for (const auto &state : dfa.states)
    {
        if (dfa.finalStates.find(state) == dfa.finalStates.end())
        {
            newFinalStates.insert(state);
        }
    }

    dfa.finalStates = newFinalStates;
}

DFA Determinize::convert(void)
{
    while (!to_discover.empty())
    {
        for (const auto &alpha : nfa.alphabet)
        {
            std::set<State> merged_states;

            for (const auto &state : to_discover.front())
            {
                auto it = nfa.transitions.find({state, alpha});

                if (it != nfa.transitions.end())
                {
                    for (const auto &transition : it->second)
                        merged_states.insert(transition);
                }
            }

            if (!merged_states.empty())
            {
                if (references.find(merged_states) == references.end())
                {
                    dfa.states.insert(next_state);
                    references.insert({merged_states, next_state++});
                    to_discover.push(merged_states);
                }

                dfa.transitions.insert({{references[to_discover.front()], alpha}, references[merged_states]});
                predecessors[references[merged_states]].insert(references[to_discover.front()]);
            }
        }

        to_discover.pop();
    }

    setFinalStates();
    swapStates();
    removeUselessStates();
    return dfa;
}

DFA Determinize::complement(void)
{
    return convert();
}

DFA complement(const MISNFA &nfa)
{
    Determinize det(nfa);
    return det.complement();
}

bool run(const DFA &dfa, const Word &word)
{
    State currentState = dfa.initialState;

    for (Symbol sym : word)
    {
        auto it = dfa.transitions.find({currentState, sym});
        if (it == dfa.transitions.end())
        {
            return false;
        }
        currentState = it->second;
    }

    return dfa.finalStates.count(currentState) > 0;
}

#ifndef __PROGTEST__
int main()
{
    MISNFA in0 = {
        {0, 1, 2},
        {'e', 'l'},
        {
            {{0, 'e'}, {1}},
            {{0, 'l'}, {1}},
            {{1, 'e'}, {2}},
            {{2, 'e'}, {0}},
            {{2, 'l'}, {2}},
        },
        {0},
        {1, 2},
    };
    auto out0 = complement(in0);
    assert(run(out0, {}) == true);
    assert(run(out0, {'e', 'l'}) == true);
    assert(run(out0, {'l'}) == false);
    assert(run(out0, {'l', 'e', 'l', 'e'}) == true);
    MISNFA in1 = {
        {0, 1, 2, 3},
        {'g', 'l'},
        {
            {{0, 'g'}, {1}},
            {{0, 'l'}, {2}},
            {{1, 'g'}, {3}},
            {{1, 'l'}, {3}},
            {{2, 'g'}, {1}},
            {{2, 'l'}, {0}},
            {{3, 'l'}, {1}},
        },
        {0},
        {0, 2, 3},
    };
    auto out1 = complement(in1);
    assert(run(out1, {}) == false);
    assert(run(out1, {'g'}) == true);
    assert(run(out1, {'g', 'l'}) == false);
    assert(run(out1, {'g', 'l', 'l'}) == true);
    MISNFA in2 = {
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
        {'a', 'b'},
        {
            {{0, 'a'}, {1}},
            {{0, 'b'}, {2}},
            {{1, 'a'}, {3}},
            {{1, 'b'}, {4}},
            {{2, 'a'}, {5}},
            {{2, 'b'}, {6}},
            {{3, 'a'}, {7}},
            {{3, 'b'}, {8}},
            {{4, 'a'}, {9}},
            {{5, 'a'}, {5}},
            {{5, 'b'}, {10}},
            {{6, 'a'}, {8}},
            {{6, 'b'}, {8}},
            {{7, 'a'}, {11}},
            {{8, 'a'}, {3}},
            {{8, 'b'}, {9}},
            {{9, 'a'}, {5}},
            {{9, 'b'}, {5}},
            {{10, 'a'}, {1}},
            {{10, 'b'}, {2}},
            {{11, 'a'}, {5}},
            {{11, 'b'}, {5}},
        },
        {0},
        {5, 6},
    };
    auto out2 = complement(in2);
    assert(run(out2, {}) == true);
    assert(run(out2, {'a'}) == true);
    assert(run(out2, {'a', 'a', 'a', 'a', 'a', 'b'}) == true);
    assert(run(out2, {'a', 'a', 'a', 'c', 'a', 'a'}) == false);
    MISNFA in3 = {
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
        {'e', 'j', 'k'},
        {
            {{0, 'e'}, {1}},
            {{0, 'j'}, {2}},
            {{0, 'k'}, {3}},
            {{1, 'e'}, {2}},
            {{1, 'j'}, {4}},
            {{1, 'k'}, {5}},
            {{2, 'e'}, {6}},
            {{2, 'j'}, {0}},
            {{2, 'k'}, {4}},
            {{3, 'e'}, {7}},
            {{3, 'j'}, {2}},
            {{3, 'k'}, {1}},
            {{4, 'e'}, {4}},
            {{4, 'j'}, {8}},
            {{4, 'k'}, {7}},
            {{5, 'e'}, {4}},
            {{5, 'j'}, {0}},
            {{5, 'k'}, {4}},
            {{6, 'e'}, {7}},
            {{6, 'j'}, {8}},
            {{6, 'k'}, {4}},
            {{7, 'e'}, {3}},
            {{7, 'j'}, {1}},
            {{7, 'k'}, {8}},
            {{8, 'e'}, {2}},
            {{8, 'j'}, {4}},
            {{8, 'k'}, {9}},
            {{9, 'e'}, {4}},
            {{9, 'j'}, {0}},
            {{9, 'k'}, {4}},
        },
        {0},
        {1, 6, 8},
    };
    auto out3 = complement(in3);
    assert(run(out3, {}) == true);
    assert(run(out3, {'b', 'e', 'e'}) == false);
    assert(run(out3, {'e', 'e', 'e'}) == false);
    assert(run(out3, {'e', 'j'}) == true);
    assert(run(out3, {'e', 'k', 'j', 'e', 'j', 'j', 'j', 'e', 'k'}) == true);
    MISNFA in4 = {
        {0, 1, 2, 3, 4, 5},
        {'e', 'n', 'r'},
        {
            {{0, 'e'}, {1}},
            {{0, 'n'}, {1}},
            {{0, 'r'}, {2}},
            {{1, 'e'}, {2}},
            {{1, 'n'}, {3}},
            {{1, 'r'}, {3}},
            {{2, 'e'}, {3}},
            {{2, 'n'}, {3}},
            {{2, 'r'}, {1}},
            {{3, 'e'}, {1}},
            {{3, 'n'}, {1}},
            {{3, 'r'}, {2}},
            {{4, 'r'}, {5}},
        },
        {0},
        {4, 5},
    };
    auto out4 = complement(in4);
    assert(run(out4, {}) == true);
    assert(run(out4, {'e', 'n', 'r', 'e', 'n', 'r', 'e', 'n', 'r', 'e', 'n', 'r'}) == true);
    assert(run(out4, {'n', 'e', 'r', 'n', 'r', 'r', 'r', 'n', 'e', 'n', 'n', 'm', '\x0c', '\x20'}) == false);
    assert(run(out4, {'r', 'r', 'r', 'e', 'n'}) == true);
    MISNFA in5 = {
        {0, 1, 2, 3, 4, 5, 6},
        {'l', 'q', 't'},
        {
            {{0, 'l'}, {2, 4, 5}},
            {{0, 'q'}, {2}},
            {{0, 't'}, {1}},
            {{1, 'l'}, {0, 2}},
            {{1, 'q'}, {1, 4}},
            {{1, 't'}, {0, 2}},
            {{2, 'l'}, {5}},
            {{2, 'q'}, {0, 4}},
            {{2, 't'}, {0}},
            {{3, 'l'}, {3, 4}},
            {{3, 'q'}, {0}},
            {{3, 't'}, {0, 2}},
            {{4, 't'}, {4}},
            {{5, 'l'}, {0, 2}},
            {{5, 'q'}, {4, 5}},
            {{5, 't'}, {0, 2}},
            {{6, 'l'}, {4, 6}},
            {{6, 'q'}, {0}},
            {{6, 't'}, {0, 2}},
        },
        {2, 4},
        {0, 1, 3, 5, 6},
    };
    auto out5 = complement(in5);
    assert(run(out5, {}) == true);
    assert(run(out5, {'q', 'q'}) == true);
    assert(run(out5, {'q', 'q', 'l'}) == false);
    assert(run(out5, {'q', 'q', 'q'}) == false);

    MISNFA in6 = {
        {0},
        {'\x00'},
        {
            {{0, '\x00'}, {0}},
        },
        {0},
        {0},
    };
    auto out6 = complement(in6);
    assert(run(out6, {'q', 'q', 'l'}) == false);
}
#endif

