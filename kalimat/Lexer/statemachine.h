/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef STATEMACHINE_H
#define STATEMACHINE_H


// Unlike traditionl lexical analyzers, accepting a token is a characteristic  of the transition,
// not the state.
class TransitionTestPair
{
public:
        Predicate *c;
        State s;
        Action *action;
        bool accepting;
        TokenType acceptedType;
public:
        TransitionTestPair();
        TransitionTestPair(Predicate *c1, State s1);
        TransitionTestPair(Predicate *c1, State s1, TokenType _acceptedType);
        TransitionTestPair(Predicate *c1, State s1, Action *_action);
        TransitionTestPair(Predicate *c1, State s1, TokenType _acceptedType, Action *_action);
};


typedef QVector<TransitionTestPair> TransitionTestMap;
typedef QMap<State, TransitionTestMap> TransitionTable;

struct StateMachine
{
    TransitionTable _transitions; // a transition table is a QMap<State, QVector<ConditionAndAction> >
                                 // i.e given a State it gives us a list of "condition/action" pairs that we can use to specify
                                // state transitions.

public:
    // given a current state and input, gives us the next state
    bool Transition(State s, Buffer &_buffer, State & nextState,bool &accept, TokenType & acceptedType, Action*& action);

    // Add: takes a current state 's', a predicate 'c', a next state 's2', flags if the state needs them, and a tokenType for
    // the accepted token if the state is accepting.
    void add         (State s, Predicate *c, State s2);
    void add         (State s, Predicate *c, State s2, Action *action);

    void addAccepting(State s, Predicate *c, State s2, TokenType accepted);
    void addAccepting(State s, Predicate *c, State s2, TokenType accepted, Action *action);
    // another version of add with simplified default parameters

    QVector<Predicate *> GetPossibleTransitions(State s);

    // are there any transitions from this state?
    bool StateHasTests(State s);

    // checks if the state has the 'retract' flag
    bool  IsStateGoBack(State s);

    bool StateHasFlag(State s,int flag);
};
#endif // STATEMACHINE_H
